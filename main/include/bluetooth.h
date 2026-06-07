#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize Bluetooth A2DP Sink
 * @return ESP_OK on success
 */
esp_err_t bt_init(void);

/**
 * @brief Deinitialize Bluetooth
 * @return ESP_OK on success
 */
esp_err_t bt_deinit(void);

/**
 * @brief Get Bluetooth connection status
 * @return true if connected, false otherwise
 */
bool bt_is_connected(void);

/**
 * @brief Get remote device name
 * @return pointer to device name string
 */
const char* bt_get_remote_device_name(void);

#endif // BLUETOOTH_H
