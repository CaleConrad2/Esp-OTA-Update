
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs_flash.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "protocol_examples_common.h"

static const char *TAG = "blinky_OTA";
#define LED GPIO_NUM_2
#define UDP_PORT 3333 // same as controller

/* ----- tiny OTA agent: waits for UDP “O” or “T” byte ----- */
static void ota_task(void *arg) {
  /* 1. open UDP socket */
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  struct sockaddr_in bind_addr = {.sin_family = AF_INET,
                                  .sin_port = htons(UDP_PORT),
                                  .sin_addr.s_addr = htonl(INADDR_ANY)};
  bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr));

  for (;;) {
    uint8_t cmd;
    struct sockaddr_in from;
    socklen_t flen = sizeof(from);
    int n = recvfrom(sock, &cmd, 1, 0, (struct sockaddr *)&from, &flen);
    if (n != 1)
      continue; // ignore noise

    const char *file = (cmd == '1')   ? "/ota1.bin"
                       : (cmd == '2') ? "/ota2.bin"
                                      : NULL;
    if (!file)
      continue;

    /* 2. build URL from sender IP + chosen file */
    char url[64];
    sprintf(url, "http://%s%s", inet_ntoa(from.sin_addr), file);
    ESP_LOGI(TAG, "OTA → %s", url);

    esp_http_client_config_t http = {.url = url,
                                     .transport_type = HTTP_TRANSPORT_OVER_TCP,
                                     .timeout_ms = 15000};
    esp_https_ota_config_t ota_cfg = {.http_config = &http};

    if (esp_https_ota(&ota_cfg) == ESP_OK)
      esp_restart();
    ESP_LOGE(TAG, "update failed");
  }
}

// blink payload
void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(example_connect());

  xTaskCreate(ota_task, "ota", 8 * 1024, NULL, 5, NULL);

  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  ESP_LOGI(TAG, "Blink period = %d ms", 3000);
  while (true) {
    gpio_set_level(LED, 1);
    vTaskDelay(pdMS_TO_TICKS(3000 / 5));
    gpio_set_level(LED, 0);
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}
