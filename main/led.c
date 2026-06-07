#include "led.h"
#include "config.h"

#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "LED";

static uint8_t led_gpio_num = 0;
static bool led_initialized = false;

esp_err_t led_init(uint8_t gpio_num)
{
    esp_err_t ret;

    led_gpio_num = gpio_num;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", gpio_num, esp_err_to_name(ret));
        return ret;
    }

    ret = gpio_set_level(gpio_num, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO %d level: %s", gpio_num, esp_err_to_name(ret));
        return ret;
    }

    led_initialized = true;
    ESP_LOGI(TAG, "LED initialized on GPIO %d", gpio_num);

    return ESP_OK;
}

esp_err_t led_on(void)
{
    if (!led_initialized) {
        ESP_LOGE(TAG, "LED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return gpio_set_level(led_gpio_num, 1);
}

esp_err_t led_off(void)
{
    if (!led_initialized) {
        ESP_LOGE(TAG, "LED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return gpio_set_level(led_gpio_num, 0);
}

esp_err_t led_toggle(void)
{
    if (!led_initialized) {
        ESP_LOGE(TAG, "LED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    uint32_t level = gpio_get_level(led_gpio_num);
    return gpio_set_level(led_gpio_num, !level);
}

static void led_blink_task(void *arg)
{
    typedef struct {
        uint8_t count;
        uint32_t interval;
    } blink_params_t;

    blink_params_t *params = (blink_params_t *)arg;
    uint8_t count = params->count;
    uint32_t interval = params->interval;

    free(params);

    for (uint8_t i = 0; i < count; i++) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(interval));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(interval));
    }

    vTaskDelete(NULL);
}

esp_err_t led_blink(uint8_t count, uint32_t interval_ms)
{
    if (!led_initialized) {
        ESP_LOGE(TAG, "LED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    typedef struct {
        uint8_t count;
        uint32_t interval;
    } blink_params_t;

    blink_params_t *params = (blink_params_t *)malloc(sizeof(blink_params_t));
    if (params == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        return ESP_ERR_NO_MEM;
    }

    params->count = count;
    params->interval = interval_ms;

    BaseType_t xReturned = xTaskCreate(
        led_blink_task,
        "LED_BLINK",
        2048,
        (void *)params,
        configMAX_PRIORITIES - 1,
        NULL
    );

    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LED blink task");
        free(params);
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
