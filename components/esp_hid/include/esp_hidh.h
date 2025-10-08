#pragma once
#include "esp_err.h"
#include "esp_bt_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ESP_HIDH_OPEN_EVENT = 0,
    ESP_HIDH_CLOSE_EVENT,
    ESP_HIDH_INPUT_EVENT
} esp_hidh_cb_event_t;

typedef union {
    struct {
        char name[64];
    } open;
    struct {
        uint8_t *data;
        uint16_t length;
    } input;
} esp_hidh_cb_param_t;

typedef struct {
    void (*callback)(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param);
    void *callback_arg;
    uint32_t event_stack_size;
    int event_prio;
} esp_hidh_config_t;

esp_err_t esp_hidh_init(const esp_hidh_config_t *config);
esp_err_t esp_hidh_register_callback(void (*callback)(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param));

#ifdef __cplusplus
}
#endif
