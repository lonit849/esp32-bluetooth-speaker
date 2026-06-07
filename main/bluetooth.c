#include "bluetooth.h"
#include "config.h"
#include "led.h"
#include "lcd.h"

#include <string.h>
#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_a2dp_api.h>
#include <esp_avrc_api.h>

static const char *TAG = "BT";

static bool bt_connected = false;
static char remote_device_name[32] = {0};

static void bt_a2dp_config_cb(void *arg, uint8_t *value, uint8_t len)
{
    ESP_LOGI(TAG, "A2DP Audio configuration: len=%u", len);
}

static void bt_a2dp_data_cb(const uint8_t *buf, uint32_t len)
{
    // Audio data callback
}

static void bt_a2dp_connection_state_cb(esp_a2d_conn_state_t state, esp_bd_addr_t remote_bda)
{
    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        bt_connected = true;
        ESP_LOGI(TAG, "A2DP Sink connected");
        led_blink(3, 100);
        lcd_set_cursor(0, 0);
        lcd_write_string("BT Connected");
    } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        bt_connected = false;
        ESP_LOGI(TAG, "A2DP Sink disconnected");
        led_off();
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_write_string("BT Disconnected");
    }
}

static void bt_a2dp_audio_state_cb(esp_a2d_audio_state_t state, esp_bd_addr_t remote_bda)
{
    if (state == ESP_A2D_AUDIO_STATE_STARTED) {
        ESP_LOGI(TAG, "A2DP audio stream started");
        led_on();
        lcd_set_cursor(0, 1);
        lcd_write_string("Playing");
    } else if (state == ESP_A2D_AUDIO_STATE_STOPPED) {
        ESP_LOGI(TAG, "A2DP audio stream stopped");
        led_off();
        lcd_set_cursor(0, 1);
        lcd_write_string("Stopped");
    }
}

static void bt_gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Authentication successful");
                strncpy(remote_device_name, (char *)param->auth_cmpl.device_name, 
                        sizeof(remote_device_name) - 1);
            }
            break;
        case ESP_BT_GAP_PIN_REQ_EVT:
            ESP_LOGI(TAG, "PIN code required");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '0';
            pin_code[1] = '0';
            pin_code[2] = '0';
            pin_code[3] = '0';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            break;
        default:
            break;
    }
}

esp_err_t bt_init(void)
{
    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BT controller: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable BT controller: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluedroid: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_gap_register_callback(bt_gap_callback);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GAP callback: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_a2d_register_callback(&bt_a2dp_connection_state_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register A2DP callback: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_a2d_register_sink_media_ctrl_stat_callback(&bt_a2dp_audio_state_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register A2DP audio state callback: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_a2d_sink_register_data_callback(bt_a2dp_data_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register A2DP data callback: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_a2d_sink_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize A2DP Sink: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_bt_dev_set_device_name(BT_DEVICE_NAME);
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    ESP_LOGI(TAG, "Bluetooth initialized as '%s'", BT_DEVICE_NAME);
    return ESP_OK;
}

esp_err_t bt_deinit(void)
{
    esp_a2d_sink_deinit();
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    ESP_LOGI(TAG, "Bluetooth deinitialized");
    return ESP_OK;
}

bool bt_is_connected(void)
{
    return bt_connected;
}

const char* bt_get_remote_device_name(void)
{
    return remote_device_name;
}
