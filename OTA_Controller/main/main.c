#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char *TAG = "controller";

/* ---------- SPIFFS ---------- */
static void mount_spiffs(void) {
  esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = "spiffs",
      .max_files = 5,
      .format_if_mount_failed = true,
  };
  ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
  ESP_LOGI(TAG, "SPIFFS mounted");
}

/* ---------- URI handler ---------- */
static esp_err_t ota_get_handler(httpd_req_t *req) {
  FILE *f = fopen("/spiffs/ota.bin", "rb");
  if (!f) {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "OTA file missing");
    return ESP_OK;
  }

  httpd_resp_set_type(req, "application/octet-stream");

  char buf[1024];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
    if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
      fclose(f);
      return ESP_FAIL;
    }
  }
  fclose(f);
  httpd_resp_send_chunk(req, NULL, 0); // end marker
  ESP_LOGI(TAG, "Served /ota.bin");
  return ESP_OK;
}

/* ---------- HTTP server ---------- */
static void start_http_server(void) {
  httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;
  ESP_ERROR_CHECK(httpd_start(&server, &cfg));

  httpd_uri_t ota_uri = {
      .uri = "/ota.bin",
      .method = HTTP_GET,
      .handler = ota_get_handler,
  };
  httpd_register_uri_handler(server, &ota_uri);

  ESP_LOGI(TAG, "HTTP server ready – http://<board-IP>/ota.bin");
}

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(example_connect()); /* put SSID/PASS in menuconfig */

  mount_spiffs();
  start_http_server();
}
