#include "config.h"
#include "bluetooth.h"
#include "audio.h"
#include "led.h"
#include "lcd.h"

#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "MAIN";

static void system_init_task(void *arg)
{
    esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");

    // Initialize LED
    ret = led_init(LED_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LED init failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
        return;
    }
    led_blink(2, 100);

    // Initialize LCD
    lcd_config_t lcd_cfg = {
        .sda_pin = I2C_SDA_PIN,
        .scl_pin = I2C_SCL_PIN,
        .freq_hz = I2C_FREQ_HZ,
        .addr = LCD_ADDR,
        .cols = LCD_COLS,
        .rows = LCD_ROWS,
    };

    ret = lcd_init(&lcd_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD init failed: %s", esp_err_to_name(ret));
        led_blink(5, 100);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "LCD initialized");

    // Display startup message
    lcd_set_cursor(0, 0);
    lcd_write_string("ESP32 Speaker");
    lcd_set_cursor(0, 1);
    lcd_write_string("Initializing...");

    // Initialize audio (I2S)
    i2s_config_t audio_cfg = {
        .dout_pin = I2S_DOUT_PIN,
        .bclk_pin = I2S_BCLK_PIN,
        .lrck_pin = I2S_LRCK_PIN,
        .sample_rate = I2S_SAMPLE_RATE,
        .bit_width = 16,
    };

    ret = audio_init(&audio_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Audio init failed: %s", esp_err_to_name(ret));
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_write_string("Audio Error");
        led_blink(5, 100);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Audio initialized");

    // Initialize Bluetooth
    ret = bt_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth init failed: %s", esp_err_to_name(ret));
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_write_string("BT Error");
        led_blink(5, 100);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Bluetooth initialized");

    // Display ready message
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_write_string("Ready");
    lcd_set_cursor(0, 1);
    lcd_write_string("Waiting...");

    ESP_LOGI(TAG, "System ready. Connect via Bluetooth to 'ESP32-Speaker'");

    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "  ESP32 Bluetooth Speaker");
    ESP_LOGI(TAG, "  PAM8403 Amplifier + 16x2 LCD");
    ESP_LOGI(TAG, "=====================================");

    xTaskCreate(
        system_init_task,
        "SYS_INIT",
        4096,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL
    );

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
