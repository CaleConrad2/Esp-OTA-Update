/*  OTA Controller – hosts exactly one file: /ota.bin  */
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char *TAG = "controller";

/* ───── SPIFFS with ota.bin ─────────────────────────────────────── */
static void mount_spiffs(void) {
  esp_vfs_spiffs_conf_t cfg = {
      .base_path = "/spiffs",
      .partition_label = "spiffs",
      .max_files = 2,
      .format_if_mount_failed = false // we already pre-flashed the image
  };
  ESP_ERROR_CHECK(esp_vfs_spiffs_register(&cfg));
  ESP_LOGI(TAG, "SPIFFS mounted");
}

/* ───── /ota.bin handler ────────────────────────────────────────── */
static esp_err_t ota_handler(httpd_req_t *req) {
  FILE *f = fopen("/spiffs/ota1.bin", "rb");
  if (!f) {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "ota1.bin missing");
    return ESP_OK;
  }
  httpd_resp_set_type(req, "application/octet-stream");

  char buf[1024];
  size_t n;
  long size = 0;
  while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
    if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
      fclose(f);
      return ESP_FAIL;
    }
    size += n;
  }
  fclose(f);
  httpd_resp_send_chunk(req, NULL, 0); // end of body
  ESP_LOGI(TAG, "Served ota1.bin (%ld B)", size);
  return ESP_OK;
}

static void start_http_server(void) {
  httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t h = NULL;
  ESP_ERROR_CHECK(httpd_start(&h, &cfg));

  httpd_uri_t uri = {
      .uri = "/ota1.bin", .method = HTTP_GET, .handler = ota_handler};
  ESP_ERROR_CHECK(httpd_register_uri_handler(h, &uri));
  ESP_LOGI(TAG, "HTTP server ready – http://<IP>/ota1.bin");
}

/* ───── app_main ───────────────────────────────────────────────── */
void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect()); // fill Wi-Fi SSID/PW once
  mount_spiffs();
  start_http_server();
}
