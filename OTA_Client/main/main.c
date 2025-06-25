#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char *TAG = "OTA_client";

#define OTA_URL                                                                \
  "http://192.168.1.74/ota.bin" /* <-- change to controller IP                 \
                                 */

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  return ESP_OK; /* we don’t need any special events */
}

static void ota_task(void *arg) {
  /* --- 1. configure plain-HTTP client --- */
  esp_http_client_config_t http_cfg = {
      .url = OTA_URL,
      .event_handler = _http_event_handler,
      .timeout_ms = 15000,
  };

  esp_http_client_handle_t client = esp_http_client_init(&http_cfg);
  if (client == NULL) {
    ESP_LOGE(TAG, "HTTP client init failed");
    vTaskDelete(NULL);
  }

  ESP_LOGI(TAG, "Downloading %s", OTA_URL);
  if (esp_http_client_open(client, 0) != ESP_OK) {
    ESP_LOGE(TAG, "Unable to open HTTP connection");
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
  }

  int content_length = esp_http_client_fetch_headers(client);
  if (content_length < 0) {
    ESP_LOGE(TAG, "Invalid content length (%d)", content_length);
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
  }

  /* --- 2. prepare OTA partition --- */
  esp_ota_handle_t ota_handle = 0;
  const esp_partition_t *ota_up = esp_ota_get_next_update_partition(NULL);
  ESP_ERROR_CHECK(esp_ota_begin(ota_up, content_length, &ota_handle));

  /* --- 3. stream the image --- */
  char buf[1024];
  int read_len;
  int written = 0;
  while ((read_len = esp_http_client_read(client, buf, sizeof(buf))) > 0) {
    ESP_ERROR_CHECK(esp_ota_write(ota_handle, buf, read_len));
    written += read_len;
  }
  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  if (written != content_length) {
    ESP_LOGE(TAG, "Only %d / %d bytes written", written, content_length);
    esp_ota_abort(ota_handle);
    vTaskDelete(NULL);
  }

  /* --- 4. finalise and reboot --- */
  ESP_ERROR_CHECK(esp_ota_end(ota_handle));
  ESP_ERROR_CHECK(esp_ota_set_boot_partition(ota_up));

  ESP_LOGI(TAG, "OTA done → rebooting into new image");
  esp_restart();
}

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(example_connect()); /* connect to the same AP */

  xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
}
