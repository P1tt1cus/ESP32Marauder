#include "StudioRadio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static bool initialized, started, fail_country, fail_power, fail_channel, mismatch_country, mismatch_channel;
static unsigned country_writes, config_calls;
static wifi_country_t country;
static uint8_t channel = 1;
static wifi_second_chan_t secondary;
static int8_t power = 80;
extern "C" esp_err_t __real_esp_wifi_init(const wifi_init_config_t*) {
    if(initialized) return ESP_ERR_WIFI_INIT_STATE;
    initialized = true; country = {{'P','H',0}, 1, 13, 20, WIFI_COUNTRY_POLICY_AUTO}; return ESP_OK;
}
extern "C" esp_err_t __real_esp_wifi_start() {if(!initialized) return ESP_ERR_WIFI_NOT_INIT; started=true;return ESP_OK;}
extern "C" esp_err_t __real_esp_wifi_stop() {started=false;return ESP_OK;}
extern "C" esp_err_t __real_esp_wifi_deinit() {initialized=started=false;return ESP_OK;}
extern "C" esp_err_t esp_wifi_get_country(wifi_country_t* out) {
    if(!initialized) return ESP_ERR_WIFI_NOT_INIT;
    *out=country; if(mismatch_country) out->nchan=14; return ESP_OK;
}
extern "C" esp_err_t esp_wifi_set_country_code(const char* code, bool auto_policy) {
    assert(!strcmp(code,"AU") && !auto_policy); country_writes++;
    if(fail_country) return ESP_FAIL;
    country={{'A','U',0},1,13,20,WIFI_COUNTRY_POLICY_MANUAL};return ESP_OK;
}
extern "C" esp_err_t __real_esp_wifi_set_channel(uint8_t value,wifi_second_chan_t second) {
    if(!started) return ESP_ERR_WIFI_NOT_STARTED;
    if(fail_channel) return ESP_FAIL;
    channel=value;secondary=second;return ESP_OK;
}
extern "C" esp_err_t esp_wifi_get_channel(uint8_t* out,wifi_second_chan_t* second) {
    if(!started) return ESP_ERR_WIFI_NOT_STARTED;
    *out=mismatch_channel ? 14 : channel;*second=secondary;return ESP_OK;
}
extern "C" esp_err_t __real_esp_wifi_set_max_tx_power(int8_t value) {
    if(!started) return ESP_ERR_WIFI_NOT_STARTED;
    if(fail_power) return ESP_FAIL;
    power=value;return ESP_OK;
}
extern "C" esp_err_t esp_wifi_get_max_tx_power(int8_t* out) {
    if(!started) return ESP_ERR_WIFI_NOT_STARTED;
    *out=power;return ESP_OK;
}
extern "C" esp_err_t __real_esp_wifi_set_config(wifi_interface_t,wifi_config_t*) {config_calls++;return ESP_OK;}
extern "C" const char* esp_err_to_name(esp_err_t e) {return e ? "MOCK_ERROR" : "ESP_OK";}
int main() {
    char report[512]; wifi_init_config_t init{};
    assert(marauder::studioRadioJson(report,sizeof(report)));
    assert(strstr(report,"\"country\":null") && strstr(report,"\"verified\":false"));
    assert(!initialized && !started); // observing never starts a radio
    fail_country=true;
    assert(__wrap_esp_wifi_init(&init)==ESP_FAIL && !initialized);
    fail_country=false; mismatch_country=true;
    assert(__wrap_esp_wifi_init(&init)==ESP_FAIL && !initialized);
    mismatch_country=false;
    assert(__wrap_esp_wifi_init(&init)==ESP_OK);
    const unsigned writes=country_writes;
    assert(__wrap_esp_wifi_start()==ESP_OK && started && marauder::studioRadioRunning());
    assert(country_writes==writes); // don't rewrite country/NVS unnecessarily
    for(int c=1;c<=13;c++) {
        assert(__wrap_esp_wifi_set_channel(c,WIFI_SECOND_CHAN_NONE)==ESP_OK);
        assert(channel==c);
    }
    assert(__wrap_esp_wifi_set_channel(0,WIFI_SECOND_CHAN_NONE)==ESP_ERR_INVALID_ARG);
    assert(__wrap_esp_wifi_set_channel(14,WIFI_SECOND_CHAN_NONE)==ESP_ERR_INVALID_ARG);
    assert(__wrap_esp_wifi_set_channel(255,WIFI_SECOND_CHAN_NONE)==ESP_ERR_INVALID_ARG);
    assert(marauder::studioRadioValidateChannel(256)==ESP_ERR_INVALID_ARG);
    assert(channel==13);
    assert(__wrap_esp_wifi_set_channel(13,WIFI_SECOND_CHAN_ABOVE)==ESP_ERR_INVALID_ARG);
    fail_channel=true;
    assert(__wrap_esp_wifi_set_channel(1,WIFI_SECOND_CHAN_NONE)==ESP_FAIL && channel==13);
    fail_channel=false; mismatch_channel=true;
    assert(__wrap_esp_wifi_set_channel(1,WIFI_SECOND_CHAN_NONE)==ESP_FAIL);
    mismatch_channel=false;
    assert(__wrap_esp_wifi_set_channel(13,WIFI_SECOND_CHAN_NONE)==ESP_OK);
    assert(__wrap_esp_wifi_set_max_tx_power(82)==ESP_OK && power==80);
    assert(__wrap_esp_wifi_set_max_tx_power(72)==ESP_OK && power==72);
    assert(__wrap_esp_wifi_set_max_tx_power(0)==ESP_ERR_INVALID_ARG && power==72);
    wifi_config_t config{}; config.ap.channel=14;
    assert(__wrap_esp_wifi_set_config(WIFI_IF_AP,&config)==ESP_ERR_INVALID_ARG && config_calls==0);
    config.ap.channel=13;
    assert(__wrap_esp_wifi_set_config(WIFI_IF_AP,&config)==ESP_OK && config_calls==1);
    config.sta.channel=0;
    assert(__wrap_esp_wifi_set_config(WIFI_IF_STA,&config)==ESP_OK);
    assert(marauder::studioRadioJson(report,sizeof(report))); puts(report);
    assert(strstr(report,"\"verified\":true") && strstr(report,"\"powerQdbm\":72"));
    power=84;
    assert(marauder::studioRadioJson(report,sizeof(report)) && strstr(report,"\"verified\":false"));
    assert(!marauder::studioRadioJson(report,8));
    assert(__wrap_esp_wifi_stop()==ESP_OK);
    fail_power=true;
    assert(__wrap_esp_wifi_start()==ESP_FAIL && !started && !marauder::studioRadioRunning());
    fail_power=false;
    assert(__wrap_esp_wifi_deinit()==ESP_OK);
    assert(marauder::studioRadioJson(report,sizeof(report)));puts(report);
    assert(strstr(report,"\"powerQdbm\":null"));
    assert(__wrap_esp_wifi_init(&init)==ESP_OK && country_writes>writes);
    assert(__wrap_esp_wifi_start()==ESP_OK);
    assert(marauder::studioRadioJson(report,sizeof(report)) && strstr(report,"\"verified\":true"));
    assert(__wrap_esp_wifi_deinit()==ESP_OK);
    puts("PASS: AU readback, driver lifecycle, channel bounds/errors, TX cap, failure shutdown and honest idle snapshots");
}
