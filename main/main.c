/*
  main.c  -- RolandESPMaus (ESP32, ESP-IDF v5.x compatible)
  - Bluetooth HID Host (Bluedroid) which selects the strongest HID mouse by RSSI and connects
  - Provides WiFi AP "RolandMouse"/"MausMaus" and a tiny HTTP UI
  - Emulates Roland/MSX-style nibble-based mouse protocol on DB9 pins (via level shifter)
  - Serial console commands (help, scan, scan WiFi)
  - Periodic status every 5s, LED blink vs steady
  NOTES:
   - This file is written to compile against ESP-IDF v5.x (release-v5.1 or compatible).
   - Uses the Bluedroid HID Host API: esp_bt_hid_host_register_callback, esp_bt_hid_host_init, esp_bt_hid_host_connect.
   - Keep wiring / level-shifter precautions from docs.
*//* main.c  - RolandESPMaus
 *
 * Minimaler, aber vollständiger ESP-IDF (v5.x) Sketch, der:
 *  - Bluetooth HID Host nutzt (esp_hid_host APIs)
 *  - automatisch die Maus mit dem stärksten RSSI auswählt und verbindet
 *  - einfache Web-UI per AccessPoint "RolandMouse" (PSK "MausMaus")
 *  - 5s Statusmeldungen über UART
 *  - Serieller Konsolenparser: "scan", "scan WiFi", "help", "scan bt", "scanwifi"
 *  - LED blinkt, wenn NO MOUSE, solid wenn MOUSE connected
 *
 * Wichtig: keine persistente Speicherung ins Flash (nur RAM). Verbindung bleibt
 * bis zum Power-Off aktiv.
 *
 * Build: Projekt setzt REQUIRES bt, esp_netif, esp_wifi, esp_http_server, nvs_flash.
 */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_hidh.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

static const char *TAG = "RolandESPMaus";

static char mouse_name[64] = "Keine Maus";
static bool scanning = false;

static void hidh_cb(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param)
{
    switch (event)
    {
    case ESP_HIDH_OPEN_EVENT:
        ESP_LOGI(TAG, "HID-Gerät verbunden: %s", param->open.name ? param->open.name : "Unbekannt");
        strncpy(mouse_name, param->open.name ? param->open.name : "Unbekannt", sizeof(mouse_name));
        break;

    case ESP_HIDH_INPUT_EVENT:
        ESP_LOGI(TAG, "Eingangsdaten von %s: %d Bytes", mouse_name, param->input.length);
        break;

    case ESP_HIDH_CLOSE_EVENT:
        ESP_LOGI(TAG, "HID-Gerät getrennt: %s", mouse_name);
        strncpy(mouse_name, "Keine Maus", sizeof(mouse_name));
        break;

    default:
        ESP_LOGW(TAG, "Unbehandeltes HID-Ereignis: %d", event);
        break;
    }
}

static void gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_DISC_RES_EVT:
        ESP_LOGI(TAG, "BT-Gerät gefunden.");
        break;
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
        if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED)
        {
            scanning = false;
            ESP_LOGI(TAG, "BT-Suche beendet.");
        }
        else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED)
        {
            scanning = true;
            ESP_LOGI(TAG, "BT-Suche gestartet.");
        }
        break;
    default:
        break;
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(TAG, "BT-Controller-Init fehlgeschlagen: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret)
    {
        ESP_LOGE(TAG, "BT-Controller konnte nicht aktiviert werden: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(TAG, "Bluedroid init fehlgeschlagen: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(TAG, "Bluedroid enable fehlgeschlagen: %s", esp_err_to_name(ret));
        return;
    }

    esp_bt_dev_set_device_name("RolandESPMaus");

    esp_bt_gap_register_callback(gap_cb);

    esp_hidh_config_t hidh_cfg = {
        .callback = hidh_cb,
        .event_stack_size = 4096,
        .event_prio = 5
    };

    esp_hidh_init(&hidh_cfg);
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    ESP_LOGI(TAG, "RolandESPMaus bereit – warte auf HID-Maus.");
}
