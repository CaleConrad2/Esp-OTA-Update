idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_https_ota esp_http_client
                                 esp_wifi esp_netif nvs_flash)
