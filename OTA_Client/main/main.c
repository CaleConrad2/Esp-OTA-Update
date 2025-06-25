/*  OTA Client – pulls http://<controller-ip>/ota.bin and reboots  */
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char *TAG = "OTA_client";

/* Single blocking OTA; could be triggered by GPIO if desired         */
static void ota_task(void *arg) {
  esp_http_client_config_t http_cfg = {
      .url = "http://192.168.1.74/ota1.bin", // controller IP
      .timeout_ms = 15000,
      .transport_type = HTTP_TRANSPORT_OVER_TCP,
      .cert_pem = NULL, // No cert needed for HTTP
  };
  esp_https_ota_config_t ota_cfg = {.http_config = &http_cfg};

  ESP_LOGI(TAG, "Downloading %s", http_cfg.url);
  esp_err_t ret = esp_https_ota(&ota_cfg);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "OTA successful, rebooting");
    esp_restart();
  } else {
    ESP_LOGE(TAG, "OTA failed (%s)", esp_err_to_name(ret));
  }
  vTaskDelete(NULL);
}

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect()); // SSID/PW stored in NVS
  xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
}
