#include "esp_hidh.h"
#include "esp_log.h"

static const char *TAG = "esp_hidh";
static void (*s_callback)(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param) = NULL;

esp_err_t esp_hidh_register_callback(void (*callback)(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param))
{
    s_callback = callback;
    ESP_LOGI(TAG, "HID Host callback registered");
    return ESP_OK;
}

esp_err_t esp_hidh_init(const esp_hidh_config_t *config)
{
    if (config && config->callback) {
        s_callback = config->callback;
    }
    ESP_LOGI(TAG, "HID Host initialized (legacy stub)");
    return ESP_OK;
}
