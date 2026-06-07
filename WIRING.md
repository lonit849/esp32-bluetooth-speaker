# Hardware Wiring Guide - ESP32 Bluetooth Speaker

## Quick Reference

### ESP32 Pinout

```
ESP32 Dev Kit V1 (Top View)
┌─────────────────────────────────┐
│ EN   GND  D35  D34  D33  D32    │
│ D36  D4   D2   D15  D13  D12    │ I2S: GPIO25, GPIO26, GPIO27
│ D39  D18  D19  D21  D22  RX0    │ I2C: GPIO21, GPIO22
│ D34  D5   D17  D16  D4   D0     │ LED: GPIO2
│ GND  D35  D34  D33  D32  5V     │
└─────────────────────────────────┘
```

## Complete Wiring Diagram

### 1. Power Distribution

```
┌─────────────┐
│  5V PSU     │
│  /USB       │ 2A minimum
└──────┬──────┘
       │
       ├─→ [100µF] → GND  (capacitor for stability)
       │
       ├─→ ESP32 5V
       ├─→ LCD VCC
       └─→ PAM8403 VCC

All GND connections together
```

### 2. ESP32 to LCD (I2C)

```
ESP32                LCD with PCF8574
┌─────────────┐     ┌──────────────┐
│ GPIO21 ─────┼─────┼─ SDA         │
│ GPIO22 ─────┼─────┼─ SCL         │
│ 5V ─────────┼─────┼─ VCC         │
│ GND ────────┼─────┼─ GND         │
└─────────────┘     └──────────────┘

Note: Pull-up resistors usually on LCD backpack
```

### 3. ESP32 to PAM8403 (I2S)

```
ESP32                PAM8403
┌─────────────┐     ┌──────────────┐
│ GPIO25 ─────┼─────┼─ IN-         │
│ GPIO26 ─────┼─────┼─ BCLK (opt)  │
│ GPIO27 ─────┼─────┼─ LRCK (opt)  │
│ 5V ─────────┼─────┼─ VCC         │
│ GND ────────┼─────┼─ GND         │
└─────────────┘     └──────────────┘
```

### 4. PAM8403 to Speaker

```
PAM8403          Speaker
┌──────────┐     ┌──────────┐
│ OUT+ ────┼─────┼─ + (Red) │
│ OUT- ────┼─────┼─ - (Blk) │
└──────────┘     └──────────┘

4-8Ω impedance, 3W max
Use 18-22 AWG wire
```

### 5. Status LED (Optional)

```
ESP32         Resistor    LED
GPIO2 ────[220Ω]───┬──→ + (long leg)
                   │
                  GND ──→ - (short leg)

Resistor limits current to ~11mA
```

## Breadboard Layout

```
┌─────────────────────────────────────────────────────────┐
│ 5V Rail                      GND Rail                   │
│  │                            │                         │
│ [100µF]                                                 │
│  │                            │                         │
│  └─ ESP32(5V)   LCD(VCC)   PAM(VCC)                    │
│  └─ PAM(GND)    LCD(GND)   ESP32(GND)                  │
│                                                         │
│ ESP32          LCD            PAM8403     Speaker      │
│ ┌─────────┐    ┌────────┐     ┌─────┐    ┌────┐       │
│ │GPIO21───┼────┼─SDA    │     │     │    │    │       │
│ │GPIO22───┼────┼─SCL    │     │     │    │    │       │
│ │GPIO25───┼─────────────┼─────┼─IN- │    │    │       │
│ │GPIO2────┼──[220R]─────┼─┬───┴─────┘    │    │       │
│ │         │         ┌───┘ │OUT+ ────────┼─+ │       │
│ │         │        GND    │OUT- ────────┼─- │       │
│ │         │               │             └────┘       │
│ └─────────┘               └─────┘                    │
└─────────────────────────────────────────────────────────┘
```

## PCF8574 I2C Address Selection

The LCD backpack address depends on A0-A2 pin settings:

```
A2  A1  A0  │  Address
─────────────┼──────────
0   0   0   │  0x20
0   0   1   │  0x21
0   1   0   │  0x22
0   1   1   │  0x23
1   0   0   │  0x24
1   0   1   │  0x25
1   1   0   │  0x26
1   1   1   │  0x27 (Most Common)
```

**Check your LCD backpack for solder bridges to determine address**

The auto-scanner will find your LCD during initialization and log the address.

## Connection Checklist

### Pre-Power Checks
- [ ] All GND connections are solid
- [ ] 5V supply properly connected
- [ ] No short circuits between 5V and GND
- [ ] All components seated properly
- [ ] I2C pull-ups present (usually on LCD backpack)

### Post-Flash Checks
Watch serial monitor for:
```
I (123) LCD: Scanning I2C bus (0-127)...
I (456) LCD: Found I2C device at address: 0x27
I (789) LCD: LCD initialized successfully
```

## I2C LCD Backpack (PCF8574) Pin Mapping

```
PCF8574 (I2C I/O Expander)

P0 → LCD RS  (Register Select)
P1 → LCD RW  (Read/Write)
P2 → LCD E   (Enable)
P3 → LCD BL  (Backlight)
P4 → LCD D4  (Data bit 4)
P5 → LCD D5  (Data bit 5)
P6 → LCD D6  (Data bit 6)
P7 → LCD D7  (Data bit 7)
```

## Common Issues & Solutions

### LCD Not Found
1. **Check I2C connections**: GPIO21 & GPIO22 must be connected
2. **Verify power**: LCD needs 5V supply
3. **Try different addresses**: Edit config.h and rebuild
4. **Check pull-ups**: I2C lines should have 4.7kΩ pull-ups (usually on backpack)

### LCD Displays Garbage
1. **Wrong I2C address**: Check serial output for correct address
2. **Loose connections**: Resolder I2C lines
3. **EMI/noise**: Use shielded cables, keep I2C lines < 30cm

### No Audio Output
1. **GPIO25 not connected**: Check PAM8403 IN- connection
2. **No PAM8403 power**: Verify 5V supply to VCC
3. **Speaker reversed**: Check polarity (+ and -)
4. **Loose connections**: Resolder audio lines

### I2C Communication Errors
1. **Loose connections**: Check all I2C wires are seated
2. **Missing pull-ups**: Add 4.7kΩ resistors if not on backpack
3. **Bus contention**: Remove power, check for shorts
4. **Capacitive coupling**: Use shorter cables

## Optional Improvements

### Audio Quality Enhancement
1. **Ferrite beads** on power lines near PAM8403
2. **Shielded twisted pair** for I2S data line
3. **Ground plane** if using custom PCB
4. **Star grounding**: Connect all GND to single point

### I2C Reliability
1. **Pull-up resistors**: 4.7kΩ (if not on backpack)
2. **Series resistors**: 100Ω on SDA/SCL for protection
3. **Decoupling capacitors**: 100µF + 10µF near ICs
4. **EMI shielding**: Route away from high-frequency signals

## Testing Procedure

### 1. Visual Inspection
- Check all solder joints
- Verify no wire shorts
- Confirm all connections are tight

### 2. Power Supply Test
```
With multimeter:
5V rail → GND: Should read 5.0V ± 0.2V
ESP32(5V) → GND: Should read 5.0V
LCD(VCC) → GND: Should read 5.0V
PAM(VCC) → GND: Should read 5.0V
```

### 3. I2C Test
- Boot ESP32
- Check serial for "I2C scan complete. Found X devices"
- Should find LCD at 0x27 (or your configured address)

### 4. Audio Test
- Power on system
- Connect via Bluetooth
- Play test tone
- Check speaker output

### 5. LCD Test
- Watch LCD during startup
- Should show "ESP32 Speaker" then "Ready"
- Verify text updates when connecting/playing

## Component Specifications

### Recommended Components
- **ESP32 Dev Kit V1**: 3.3V/5V dual supply
- **PAM8403**: Class-D amplifier, 3W@4Ω
- **16x2 LCD with PCF8574**: 5V operation
- **Power Supply**: 5V/2A minimum
- **Speaker**: 4-8Ω impedance
- **Resistors**: 220Ω (LED), 4.7kΩ (I2C pull-ups)
- **Capacitors**: 100µF, 10µF (power supply)

## Safety Guidelines

1. **Always disconnect power before wiring changes**
2. **Double-check connections before powering on**
3. **Use proper gauge wire** for power connections
4. **Don't exceed component ratings**
5. **Keep PAM8403 ventilated** during extended use
6. **Use fused power supply** for safety

## Helpful Resources

- [ESP32 Pinout](https://lastminuteengineers.com/esp32-pinout-reference/)
- [PCF8574 I2C I/O Expander](https://www.nxp.com/docs/en/data-sheet/pcf8574.pdf)
- [PAM8403 Datasheet](https://en.nssmc.com/product/detail/PAM8403.html)
- [LCD 16x2 Interface Guide](https://en.wikipedia.org/wiki/Hitachi_HD44780_LCD_controller)
