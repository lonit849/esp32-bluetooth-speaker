#include "lcd.h"
#include "config.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static const char *TAG = "LCD";

// PCF8574 control register bits
#define PCF8574_RS (1 << 0)    // Register Select
#define PCF8574_RW (1 << 1)    // Read/Write
#define PCF8574_EN (1 << 2)    // Enable
#define PCF8574_BL (1 << 3)    // Backlight

static i2c_master_bus_handle_t bus_handle = NULL;
static i2c_master_dev_handle_t dev_handle = NULL;
static uint8_t lcd_address = 0x27;
static uint8_t lcd_cols = 16;
static uint8_t lcd_rows = 2;
static bool lcd_initialized = false;
static bool backlight_on = true;

/**
 * @brief Write 4-bit data to LCD via PCF8574
 */
static esp_err_t lcd_write_4bits(uint8_t data)
{
    esp_err_t ret;
    
    // Set enable high
    uint8_t byte = data | PCF8574_EN | (backlight_on ? PCF8574_BL : 0);
    ret = i2c_master_transmit(dev_handle, &byte, 1, I2C_TIMEOUT_MS);
    if (ret != ESP_OK) return ret;
    
    // Small delay for data setup
    vTaskDelay(pdMS_TO_TICKS(1));
    
    // Set enable low
    byte = data & ~PCF8574_EN | (backlight_on ? PCF8574_BL : 0);
    ret = i2c_master_transmit(dev_handle, &byte, 1, I2C_TIMEOUT_MS);
    if (ret != ESP_OK) return ret;
    
    // Delay for data hold
    vTaskDelay(pdMS_TO_TICKS(1));
    
    return ESP_OK;
}

/**
 * @brief Send command to LCD (8-bit mode via two 4-bit writes)
 */
static esp_err_t lcd_send_command(uint8_t cmd)
{
    esp_err_t ret;
    
    // High nibble (RS=0 for command)
    uint8_t byte = (cmd & 0xF0) | (backlight_on ? PCF8574_BL : 0);
    ret = lcd_write_4bits(byte);
    if (ret != ESP_OK) return ret;
    
    // Low nibble (RS=0 for command)
    byte = ((cmd & 0x0F) << 4) | (backlight_on ? PCF8574_BL : 0);
    ret = lcd_write_4bits(byte);
    if (ret != ESP_OK) return ret;
    
    vTaskDelay(pdMS_TO_TICKS(2));
    return ESP_OK;
}

/**
 * @brief Send data to LCD (8-bit mode via two 4-bit writes)
 */
static esp_err_t lcd_send_data(uint8_t data)
{
    esp_err_t ret;
    
    // High nibble (RS=1 for data)
    uint8_t byte = (data & 0xF0) | PCF8574_RS | (backlight_on ? PCF8574_BL : 0);
    ret = lcd_write_4bits(byte);
    if (ret != ESP_OK) return ret;
    
    // Low nibble (RS=1 for data)
    byte = ((data & 0x0F) << 4) | PCF8574_RS | (backlight_on ? PCF8574_BL : 0);
    ret = lcd_write_4bits(byte);
    if (ret != ESP_OK) return ret;
    
    vTaskDelay(pdMS_TO_TICKS(1));
    return ESP_OK;
}

/**
 * @brief Scan I2C bus for connected devices
 */
uint8_t lcd_i2c_scan(uint8_t *address_list, uint8_t max_count)
{
    uint8_t device_count = 0;
    ESP_LOGI(TAG, "Scanning I2C bus (0-127)...");
    
    if (address_list == NULL || dev_handle == NULL) {
        ESP_LOGE(TAG, "I2C not initialized or invalid address list");
        return 0;
    }
    
    for (int i = 0; i < 128; i++) {
        i2c_master_dev_handle_t temp_dev = NULL;
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = i,
            .scl_speed_hz = I2C_FREQ_HZ,
        };
        
        esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &temp_dev);
        if (ret == ESP_OK) {
            uint8_t dummy = 0;
            ret = i2c_master_transmit(temp_dev, &dummy, 0, 100);
            
            if (ret == ESP_OK && device_count < max_count) {
                address_list[device_count] = i;
                ESP_LOGI(TAG, "Found I2C device at address: 0x%02X", i);
                device_count++;
            }
            
            i2c_master_bus_rm_device(temp_dev);
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete. Found %d devices", device_count);
    return device_count;
}

/**
 * @brief Initialize I2C bus and LCD
 */
esp_err_t lcd_init(const lcd_config_t *config)
{
    esp_err_t ret;
    
    if (config == NULL) {
        ESP_LOGE(TAG, "Config pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    lcd_address = config->addr;
    lcd_cols = config->cols;
    lcd_rows = config->rows;
    
    ESP_LOGI(TAG, "Initializing I2C bus...");
    ESP_LOGI(TAG, "  SDA: GPIO%u, SCL: GPIO%u", config->sda_pin, config->scl_pin);
    ESP_LOGI(TAG, "  Frequency: %u Hz", config->freq_hz);
    
    // Initialize I2C master bus
    i2c_master_bus_config_t i2c_bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = config->scl_pin,
        .sda_io_num = config->sda_pin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    ret = i2c_new_master_bus(&i2c_bus_cfg, &bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C master bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "I2C bus created successfully");
    
    // Scan I2C bus
    uint8_t addresses[128];
    uint8_t device_count = 0;
    
    for (int i = 0; i < 128; i++) {
        i2c_master_dev_handle_t temp_dev = NULL;
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = i,
            .scl_speed_hz = config->freq_hz,
        };
        
        ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &temp_dev);
        if (ret == ESP_OK) {
            uint8_t dummy = 0;
            ret = i2c_master_transmit(temp_dev, &dummy, 0, 100);
            
            if (ret == ESP_OK && device_count < 128) {
                addresses[device_count] = i;
                ESP_LOGI(TAG, "Found I2C device at address: 0x%02X", i);
                device_count++;
            }
            
            i2c_master_bus_rm_device(temp_dev);
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete. Found %d devices", device_count);
    
    // Add LCD device
    i2c_device_config_t lcd_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = config->addr,
        .scl_speed_hz = config->freq_hz,
    };
    
    ret = i2c_master_bus_add_device(bus_handle, &lcd_dev_cfg, &dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add LCD device at 0x%02X: %s", config->addr, esp_err_to_name(ret));
        i2c_master_bus_del(bus_handle);
        bus_handle = NULL;
        return ret;
    }
    
    ESP_LOGI(TAG, "LCD device added at address 0x%02X", config->addr);
    
    // Initialize LCD in 4-bit mode
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Function set: 4-bit mode, 2 lines, 5x8 font
    ret = lcd_send_command(0x28);  // 0010 1000
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set LCD function");
        return ret;
    }
    
    // Display control: display on, cursor off, no blink
    ret = lcd_send_command(0x0C);  // 0000 1100
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set display control");
        return ret;
    }
    
    // Clear display
    ret = lcd_send_command(0x01);  // 0000 0001
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear display");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(2));
    
    // Entry mode set: increment cursor, no display shift
    ret = lcd_send_command(0x06);  // 0000 0110
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set entry mode");
        return ret;
    }
    
    lcd_initialized = true;
    ESP_LOGI(TAG, "LCD initialized successfully (16x2 at 0x%02X)", config->addr);
    
    return ESP_OK;
}

/**
 * @brief Deinitialize LCD and I2C
 */
esp_err_t lcd_deinit(void)
{
    esp_err_t ret = ESP_OK;
    
    if (dev_handle != NULL) {
        ret = i2c_master_bus_rm_device(dev_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to remove LCD device: %s", esp_err_to_name(ret));
        }
        dev_handle = NULL;
    }
    
    if (bus_handle != NULL) {
        ret = i2c_master_bus_del(bus_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete I2C bus: %s", esp_err_to_name(ret));
        }
        bus_handle = NULL;
    }
    
    lcd_initialized = false;
    ESP_LOGI(TAG, "LCD deinitialized");
    return ret;
}

/**
 * @brief Clear LCD display
 */
esp_err_t lcd_clear(void)
{
    if (!lcd_initialized) {
        ESP_LOGE(TAG, "LCD not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    return lcd_send_command(0x01);
}

/**
 * @brief Set cursor position
 */
esp_err_t lcd_set_cursor(uint8_t col, uint8_t row)
{
    if (!lcd_initialized) {
        ESP_LOGE(TAG, "LCD not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (col >= lcd_cols || row >= lcd_rows) {
        ESP_LOGE(TAG, "Cursor position out of range");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t row_offsets[] = {0x00, 0x40};
    uint8_t address = col + row_offsets[row];
    
    return lcd_send_command(0x80 | address);
}

/**
 * @brief Write string to LCD
 */
esp_err_t lcd_write_string(const char *str)
{
    if (!lcd_initialized) {
        ESP_LOGE(TAG, "LCD not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (str == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ESP_OK;
    for (int i = 0; str[i] != '\0' && i < lcd_cols; i++) {
        ret = lcd_send_data((uint8_t)str[i]);
        if (ret != ESP_OK) {
            break;
        }
    }
    
    return ret;
}

/**
 * @brief Write single character to LCD
 */
esp_err_t lcd_write_char(uint8_t ch)
{
    if (!lcd_initialized) {
        ESP_LOGE(TAG, "LCD not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    return lcd_send_data(ch);
}

/**
 * @brief Print formatted string to LCD
 */
esp_err_t lcd_printf(uint8_t col, uint8_t row, const char *format, ...)
{
    if (!lcd_initialized) {
        ESP_LOGE(TAG, "LCD not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = lcd_set_cursor(col, row);
    if (ret != ESP_OK) {
        return ret;
    }
    
    char buffer[17];  // Max 16 chars for 16-column display
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    return lcd_write_string(buffer);
}

/**
 * @brief Turn backlight on/off
 */
esp_err_t lcd_backlight(bool on)
{
    if (!lcd_initialized) {
        ESP_LOGE(TAG, "LCD not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    backlight_on = on;
    
    // Send a dummy byte to update backlight state
    uint8_t byte = backlight_on ? PCF8574_BL : 0;
    esp_err_t ret = i2c_master_transmit(dev_handle, &byte, 1, I2C_TIMEOUT_MS);
    
    return ret;
}
