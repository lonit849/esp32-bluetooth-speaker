#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief I2S Configuration structure
 */
typedef struct {
    uint8_t dout_pin;   // Data output pin (to PAM8403 data input)
    uint8_t bclk_pin;   // Bit clock pin
    uint8_t lrck_pin;   // Left-right clock pin
    uint32_t sample_rate; // Sample rate in Hz
    uint32_t bit_width;   // Bit width (16, 24, 32)
} i2s_config_t;

/**
 * @brief Initialize I2S interface for audio output
 * @param config Pointer to I2S configuration
 * @return ESP_OK on success
 */
esp_err_t audio_init(const i2s_config_t *config);

/**
 * @brief Deinitialize I2S interface
 * @return ESP_OK on success
 */
esp_err_t audio_deinit(void);

/**
 * @brief Write audio data to I2S
 * @param data Pointer to audio data
 * @param len Length of data in bytes
 * @param timeout_ms Write timeout in milliseconds
 * @return ESP_OK on success
 */
esp_err_t audio_write(const uint8_t *data, size_t len, uint32_t timeout_ms);

/**
 * @brief Set I2S sample rate
 * @param sample_rate Sample rate in Hz
 * @return ESP_OK on success
 */
esp_err_t audio_set_sample_rate(uint32_t sample_rate);

#endif // AUDIO_H
