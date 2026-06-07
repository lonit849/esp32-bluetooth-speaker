# ESP32 Bluetooth Speaker with PAM8403 Amplifier and 16x2 LCD Display

A complete, well-organized C implementation of a Bluetooth audio speaker using ESP32 Dev Kit V1, PAM8403 amplifier, and a 16x2 LCD display with I2C interface.

## Features

✅ **Bluetooth A2DP Sink** - Receive audio from any Bluetooth device  
✅ **I2S Audio Interface** - Digital audio output via I2S protocol  
✅ **PAM8403 Support** - Class-D amplifier (3W @ 4Ω)  
✅ **16x2 LCD Display** - Real-time status display via I2C  
✅ **I2C Device Scanner** - Auto-detect LCD address during initialization  
✅ **Status LED** - Visual feedback for system status  
✅ **Error Handling** - Comprehensive error checking and logging  
✅ **Modular Design** - Clean, organized code structure  
✅ **FreeRTOS** - Efficient task-based architecture  

## Hardware Requirements

### Components
- **ESP32 Dev Kit V1** - Main microcontroller
- **PAM8403 Amplifier Module** - Audio amplifier (~3W)
- **16x2 LCD Display** - With PCF8574 I2C backpack
- **Speaker(s)** - 4-8Ω, 3W
- **Power Supply** - 5V/2A
- **Status LED** - Optional (with 220Ω resistor)

### Pin Configuration

**I2C (LCD)**:
- GPIO 21 (SDA)
- GPIO 22 (SCL)
- Default LCD address: 0x27 (configurable in config.h)

**I2S (Audio)**:
- GPIO 25 (DOUT - Data to PAM8403)
- GPIO 26 (BCLK)
- GPIO 27 (LRCK)

**Other**:
- GPIO 2 (Status LED)

## Quick Start

### Prerequisites
- ESP-IDF v5.0 or later
- ESP32 Dev Kit V1
- All hardware components connected

### Build & Flash

```bash
# Clone repository
git clone https://github.com/lonit849/esp32-bluetooth-speaker.git
cd esp32-bluetooth-speaker

# Build
idf.py build

# Flash
idf.py -p /dev/ttyUSB0 flash monitor
```

### Initial Setup

1. Power on the ESP32
2. LED blinks twice (startup indication)
3. LCD displays "ESP32 Speaker / Initializing..."
4. Once ready: "Ready / Waiting..."
5. Open Bluetooth settings and connect to "ESP32-Speaker"
6. Play audio - LCD updates in real-time

## I2C Address Scanner

The system automatically scans the I2C bus and finds connected devices. Output in serial monitor:

```
I (123) LCD: Scanning I2C bus (0-127)...
I (456) LCD: Found I2C device at address: 0x27
I (789) LCD: I2C scan complete. Found 1 devices
```

If your LCD has a different address, update `config.h`:
```c
#define LCD_ADDR 0x27  // Change to your address (0x20-0x27)
```

## Hardware Wiring

### I2C LCD Connection
```
ESP32          LCD (PCF8574)
GPIO21 (SDA) → SDA
GPIO22 (SCL) → SCL
5V           → VCC
GND          → GND
```

### I2S Audio (PAM8403)
```
ESP32          PAM8403
GPIO25 (DOUT) → IN-
5V            → VCC
GND           → GND

PAM8403        Speaker
OUT+          → Speaker +
OUT-          → Speaker -
```

### Status LED (Optional)
```
ESP32          LED
GPIO2 ------[220Ω]------→ LED+ (long leg)
GND                 ----→ LED- (short leg)
```

## Display States

### During Initialization
```
Line 1: ESP32 Speaker
Line 2: Initializing...
```

### Ready (Waiting for Connection)
```
Line 1: Ready
Line 2: Waiting...
```

### Connected & Playing
```
Line 1: BT Connected
Line 2: Playing
```

### Disconnected
```
Line 1: BT Disconnected
Line 2: (empty)
```

## Code Organization

```
main/
├── main.c              # Application entry point
├── bluetooth.c         # Bluetooth A2DP implementation
├── audio.c             # I2S audio driver
├── led.c               # LED control
├── lcd.c               # LCD I2C driver with scanner
└── include/
    ├── config.h        # Centralized configuration
    ├── bluetooth.h     # Bluetooth API
    ├── audio.h         # Audio API
    ├── led.h           # LED API
    └── lcd.h           # LCD API
```

## API Overview

### LCD Functions
```c
esp_err_t lcd_init(const lcd_config_t *config);
esp_err_t lcd_clear(void);
esp_err_t lcd_set_cursor(uint8_t col, uint8_t row);
esp_err_t lcd_write_string(const char *str);
esp_err_t lcd_printf(uint8_t col, uint8_t row, const char *format, ...);
uint8_t lcd_i2c_scan(uint8_t *address_list, uint8_t max_count);
```

### Bluetooth Functions
```c
esp_err_t bt_init(void);
bool bt_is_connected(void);
const char* bt_get_remote_device_name(void);
```

### Audio Functions
```c
esp_err_t audio_init(const i2s_config_t *config);
esp_err_t audio_write(const uint8_t *data, size_t len, uint32_t timeout_ms);
esp_err_t audio_set_sample_rate(uint32_t sample_rate);
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| LCD not displaying | Check I2C connections (GPIO21/22), verify power supply |
| LCD shows garbage | Wrong I2C address - check serial output for scanned addresses |
| No Bluetooth | Verify antenna connection, check serial logs for errors |
| No audio output | Verify PAM8403 power, check GPIO25 connection |
| I2C errors | Use shielded cables, keep lines short (<30cm) |

## Performance

- **Bluetooth**: v4.2 (BR/EDR)
- **Audio**: SBC codec, 44.1kHz/48kHz, 16-bit PCM, Stereo
- **Latency**: ~100-200ms
- **I2C Speed**: 100kHz
- **Power**: ~80mA (Bluetooth) + ~50-500mA (PAM8403)

## Documentation

See **WIRING.md** for detailed hardware setup and troubleshooting.

## Future Enhancements

- AVRCP support (play/pause/next)
- Volume control
- Equalizer
- OTA firmware updates
- Web configuration interface

## License

Provided for educational and hobby purposes.

## Support

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32 Reference](https://www.espressif.com/)
- [PCF8574 I2C Expander](https://www.nxp.com/docs/en/data-sheet/pcf8574.pdf)

---

**Version**: 1.1  
**Last Updated**: 2024  
**Target**: ESP32 Dev Kit V1 with ESP-IDF v5.0+
