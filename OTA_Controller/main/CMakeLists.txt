idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_server spiffs
                                 esp_wifi esp_netif nvs_flash)
