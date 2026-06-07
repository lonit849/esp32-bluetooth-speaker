idf_component_register(SRCS "main.c" "bluetooth.c" "audio.c" "led.c" "lcd.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_bt esp_bt_device nvs_flash driver)
