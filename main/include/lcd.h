#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief I2C LCD Configuration structure
 */
typedef struct {
    uint8_t sda_pin;      // I2C SDA pin
    uint8_t scl_pin;      // I2C SCL pin
    uint32_t freq_hz;     // I2C frequency
    uint8_t addr;         // LCD I2C address
    uint8_t cols;         // LCD columns
    uint8_t rows;         // LCD rows
} lcd_config_t;

/**
 * @brief Initialize I2C bus and scan for devices
 * @param config Pointer to LCD configuration
 * @return ESP_OK on success
 */
esp_err_t lcd_init(const lcd_config_t *config);

/**
 * @brief Deinitialize LCD and I2C
 * @return ESP_OK on success
 */
esp_err_t lcd_deinit(void);

/**
 * @brief Scan I2C bus for connected devices
 * @param address_list Pointer to array to store found addresses
 * @param max_count Maximum number of devices to detect
 * @return Number of devices found
 */
uint8_t lcd_i2c_scan(uint8_t *address_list, uint8_t max_count);

/**
 * @brief Clear LCD display
 * @return ESP_OK on success
 */
esp_err_t lcd_clear(void);

/**
 * @brief Set cursor position
 * @param col Column (0 to LCD_COLS-1)
 * @param row Row (0 to LCD_ROWS-1)
 * @return ESP_OK on success
 */
esp_err_t lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * @brief Write string to LCD
 * @param str String to write
 * @return ESP_OK on success
 */
esp_err_t lcd_write_string(const char *str);

/**
 * @brief Write single character to LCD
 * @param ch Character to write
 * @return ESP_OK on success
 */
esp_err_t lcd_write_char(uint8_t ch);

/**
 * @brief Print formatted string to LCD (like printf)
 * @param col Column position
 * @param row Row position
 * @param format Format string
 * @return ESP_OK on success
 */
esp_err_t lcd_printf(uint8_t col, uint8_t row, const char *format, ...);

/**
 * @brief Turn backlight on/off
 * @param on true to turn on, false to turn off
 * @return ESP_OK on success
 */
esp_err_t lcd_backlight(bool on);

#endif // LCD_H
