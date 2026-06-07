#ifndef LED_H
#define LED_H

#include <stdint.h>

/**
 * @brief Initialize LED GPIO
 * @param gpio_num GPIO pin number
 * @return ESP_OK on success
 */
esp_err_t led_init(uint8_t gpio_num);

/**
 * @brief Turn LED on
 * @return ESP_OK on success
 */
esp_err_t led_on(void);

/**
 * @brief Turn LED off
 * @return ESP_OK on success
 */
esp_err_t led_off(void);

/**
 * @brief Toggle LED
 * @return ESP_OK on success
 */
esp_err_t led_toggle(void);

/**
 * @brief Blink LED (non-blocking)
 * @param count Number of blinks
 * @param interval_ms Interval between blinks in milliseconds
 * @return ESP_OK on success
 */
esp_err_t led_blink(uint8_t count, uint32_t interval_ms);

#endif // LED_H
