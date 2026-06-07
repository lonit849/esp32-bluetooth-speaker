#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Bluetooth Configuration
// ============================================
#define BT_DEVICE_NAME "ESP32-Speaker"

// ============================================
// I2S Configuration (for PAM8403 amplifier)
// ============================================
#define I2S_NUM I2S_NUM_0

// I2S GPIO Pins
#define I2S_DOUT_PIN 25  // GPIO 25 - Data output (to PAM8403)
#define I2S_BCLK_PIN 26  // GPIO 26 - Bit clock
#define I2S_LRCK_PIN 27  // GPIO 27 - Left/Right clock

// I2S Audio settings
#define I2S_SAMPLE_RATE 44100  // 44.1 kHz
#define I2S_BIT_WIDTH I2S_BITS_PER_SAMPLE_16BIT
#define I2S_CHANNEL_MODE I2S_CHANNEL_STEREO

// ============================================
// LED Configuration
// ============================================
#define LED_PIN 2  // GPIO 2 - Status LED
#define LED_BLINK_INTERVAL_MS 500

// ============================================
// I2C Configuration (for 16x2 LCD)
// ============================================
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_SDA_PIN 21        // GPIO 21 - I2C Data
#define I2C_SCL_PIN 22        // GPIO 22 - I2C Clock
#define I2C_FREQ_HZ 100000    // 100 kHz
#define I2C_TIMEOUT_MS 1000

// LCD Configuration
#define LCD_ADDR 0x27         // Default PCF8574 address (can be 0x20-0x27)
#define LCD_COLS 16           // 16 columns
#define LCD_ROWS 2            // 2 rows

// ============================================
// Logging Configuration
// ============================================
#define LOG_LEVEL ESP_LOG_INFO

#endif // CONFIG_H
