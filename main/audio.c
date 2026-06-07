#include "audio.h"
#include "config.h"

#include <esp_log.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>

static const char *TAG = "AUDIO";

static i2s_chan_handle_t tx_handle = NULL;
static uint32_t current_sample_rate = I2S_SAMPLE_RATE;

esp_err_t audio_init(const i2s_config_t *config)
{
    esp_err_t ret;

    if (config == NULL) {
        ESP_LOGE(TAG, "Config pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initializing I2S with sample rate: %u Hz", config->sample_rate);

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 6;
    chan_cfg.dma_frame_num = 240;
    chan_cfg.auto_clear = true;

    ret = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S channel: %s", esp_err_to_name(ret));
        return ret;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config->sample_rate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(config->bit_width, I2S_CHANNEL_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = config->bclk_pin,
            .ws = config->lrck_pin,
            .dout = config->dout_pin,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    ret = i2s_channel_init_std_mode(tx_handle, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2S standard mode: %s", esp_err_to_name(ret));
        i2s_del_channel(tx_handle);
        tx_handle = NULL;
        return ret;
    }

    ret = i2s_channel_enable(tx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S channel: %s", esp_err_to_name(ret));
        i2s_channel_disable(tx_handle);
        i2s_del_channel(tx_handle);
        tx_handle = NULL;
        return ret;
    }

    current_sample_rate = config->sample_rate;
    ESP_LOGI(TAG, "I2S initialized successfully");
    ESP_LOGI(TAG, "  DOUT: GPIO%u", config->dout_pin);
    ESP_LOGI(TAG, "  BCLK: GPIO%u", config->bclk_pin);
    ESP_LOGI(TAG, "  LRCK: GPIO%u", config->lrck_pin);

    return ESP_OK;
}

esp_err_t audio_deinit(void)
{
    esp_err_t ret = ESP_OK;

    if (tx_handle != NULL) {
        ret = i2s_channel_disable(tx_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to disable I2S channel: %s", esp_err_to_name(ret));
        }

        ret = i2s_del_channel(tx_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete I2S channel: %s", esp_err_to_name(ret));
        }

        tx_handle = NULL;
    }

    ESP_LOGI(TAG, "I2S deinitialized");
    return ret;
}

esp_err_t audio_write(const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (tx_handle == NULL) {
        ESP_LOGE(TAG, "I2S not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t bytes_written = 0;
    esp_err_t ret = i2s_channel_write(tx_handle, data, len, &bytes_written, timeout_ms);

    if (ret != ESP_OK && ret != ESP_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "I2S write error: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t audio_set_sample_rate(uint32_t sample_rate)
{
    esp_err_t ret;

    if (tx_handle == NULL) {
        ESP_LOGE(TAG, "I2S not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (sample_rate == current_sample_rate) {
        return ESP_OK;
    }

    ret = i2s_channel_reconfig_clk_div(tx_handle, sample_rate);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reconfigure I2S clock: %s", esp_err_to_name(ret));
        return ret;
    }

    current_sample_rate = sample_rate;
    ESP_LOGI(TAG, "I2S sample rate changed to %u Hz", sample_rate);

    return ESP_OK;
}
