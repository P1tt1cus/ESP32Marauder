#pragma once
#include <stdint.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_ARG 258
#define ESP_ERR_WIFI_NOT_INIT 12289
#define ESP_ERR_WIFI_NOT_STARTED 12290
#define ESP_ERR_WIFI_INIT_STATE 12291
typedef enum {WIFI_COUNTRY_POLICY_AUTO, WIFI_COUNTRY_POLICY_MANUAL} wifi_country_policy_t;
typedef struct {char cc[3]; uint8_t schan, nchan; int8_t max_tx_power; wifi_country_policy_t policy;} wifi_country_t;
typedef struct {int unused;} wifi_init_config_t;
typedef enum {WIFI_SECOND_CHAN_NONE, WIFI_SECOND_CHAN_ABOVE, WIFI_SECOND_CHAN_BELOW} wifi_second_chan_t;
typedef enum {WIFI_IF_STA, WIFI_IF_AP} wifi_interface_t;
typedef union {struct {uint8_t channel;} ap; struct {uint8_t channel;} sta;} wifi_config_t;
extern "C" {
esp_err_t esp_wifi_set_country_code(const char*, bool);
esp_err_t esp_wifi_get_country(wifi_country_t*);
esp_err_t esp_wifi_get_channel(uint8_t*, wifi_second_chan_t*);
esp_err_t esp_wifi_get_max_tx_power(int8_t*);
const char* esp_err_to_name(esp_err_t);
esp_err_t __wrap_esp_wifi_init(const wifi_init_config_t*);
esp_err_t __wrap_esp_wifi_start();
esp_err_t __wrap_esp_wifi_stop();
esp_err_t __wrap_esp_wifi_deinit();
esp_err_t __wrap_esp_wifi_set_channel(uint8_t, wifi_second_chan_t);
esp_err_t __wrap_esp_wifi_set_max_tx_power(int8_t);
esp_err_t __wrap_esp_wifi_set_config(wifi_interface_t, wifi_config_t*);
}
