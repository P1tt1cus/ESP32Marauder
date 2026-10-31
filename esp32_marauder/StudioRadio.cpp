#include "StudioRadio.h"
#ifdef STUDIO_ZY_ESP32
#ifndef STUDIO_RADIO_WRAPPERS
#error "Studio AU builds require the Wi-Fi linker wrappers in studio-build.yml"
#endif
#include <atomic>
#include <stdio.h>
#include <string.h>

// Linker wrapping covers both the sketch and Arduino's WiFi library lifecycle.
// Country setters are deliberately not wrapped: the SDK's country-code API may
// call the lower-level setter internally. Application setters use Apply below.
extern "C" {
esp_err_t __real_esp_wifi_init(const wifi_init_config_t* config);
esp_err_t __real_esp_wifi_start();
esp_err_t __real_esp_wifi_stop();
esp_err_t __real_esp_wifi_deinit();
esp_err_t __real_esp_wifi_set_channel(uint8_t channel, wifi_second_chan_t secondary);
esp_err_t __real_esp_wifi_set_max_tx_power(int8_t power);
esp_err_t __real_esp_wifi_set_config(wifi_interface_t interface, wifi_config_t* config);
}

namespace {
std::atomic<bool> running{false};
std::atomic<int> apply_error{ESP_ERR_WIFI_NOT_INIT};
std::atomic<int> last_error{ESP_OK};
std::atomic<uint32_t> error_count{0};
esp_err_t record(esp_err_t error) {
    if(error != ESP_OK) { last_error.store(error); error_count.fetch_add(1); }
    return error;
}
bool country_matches(const wifi_country_t& country) {
    return country.cc[0] == 'A' && country.cc[1] == 'U' &&
           country.schan == 1 && country.nchan == 13 &&
           country.policy == WIFI_COUNTRY_POLICY_MANUAL;
}
struct Snapshot {
    wifi_country_t country{};
    uint8_t channel = 0;
    wifi_second_chan_t secondary{};
    int8_t power = 0;
    esp_err_t country_error, channel_error, power_error;
    bool active;
    bool verified;
};
Snapshot snapshot() {
    Snapshot s{};
    s.active = running.load();
    s.country_error = esp_wifi_get_country(&s.country);
    s.channel_error = s.active ? esp_wifi_get_channel(&s.channel, &s.secondary) : ESP_ERR_WIFI_NOT_STARTED;
    s.power_error = s.active ? esp_wifi_get_max_tx_power(&s.power) : ESP_ERR_WIFI_NOT_STARTED;
    s.verified = s.active && s.country_error == ESP_OK && country_matches(s.country) &&
                 s.channel_error == ESP_OK && s.channel >= 1 && s.channel <= 13 &&
                 s.power_error == ESP_OK && s.power >= 8 && s.power <= marauder::kStudioTxPowerQdbm &&
                 apply_error.load() == ESP_OK;
    return s;
}
}

namespace marauder {
bool studioRadioRunning() { return running.load(); }
esp_err_t studioRadioValidateChannel(int channel) {
    return channel >= kStudioFirstChannel && channel <= kStudioLastChannel ? ESP_OK : record(ESP_ERR_INVALID_ARG);
}
esp_err_t studioRadioApply() {
    // Read first to avoid unnecessary country/NVS writes on every start.
    wifi_country_t country{};
    esp_err_t error = esp_wifi_get_country(&country);
    if(error != ESP_OK || !country_matches(country)) {
        error = esp_wifi_set_country_code("AU", false);
        if(error == ESP_OK) error = esp_wifi_get_country(&country);
    }
    if(error == ESP_OK && !country_matches(country)) error = ESP_FAIL;
    apply_error.store(error);
    return record(error);
}

bool studioRadioJson(char* output, size_t size) {
    const Snapshot s = snapshot();
    char country[8] = "null", first[8] = "null", count[8] = "null";
    char power[12] = "null", channel[8] = "null";
    if(s.country_error == ESP_OK) {
        // Only emit well-formed ISO letters; never splice arbitrary driver data into JSON.
        if(s.country.cc[0] >= 'A' && s.country.cc[0] <= 'Z' && s.country.cc[1] >= 'A' && s.country.cc[1] <= 'Z')
            snprintf(country, sizeof(country), "\"%c%c\"", s.country.cc[0], s.country.cc[1]);
        snprintf(first, sizeof(first), "%u", s.country.schan);
        snprintf(count, sizeof(count), "%u", s.country.nchan);
    }
    if(s.power_error == ESP_OK) snprintf(power, sizeof(power), "%d", s.power);
    if(s.channel_error == ESP_OK) snprintf(channel, sizeof(channel), "%u", s.channel);
    const int written = snprintf(output, size,
        "{\"target\":\"AU\",\"limitQdbm\":%d,\"active\":%s,\"verified\":%s,"
        "\"country\":%s,\"first\":%s,\"count\":%s,\"manual\":%s,\"powerQdbm\":%s,\"channel\":%s,"
        "\"countryError\":%d,\"powerError\":%d,\"channelError\":%d,\"applyError\":%d,\"lastError\":%d,\"errors\":%lu}",
        kStudioTxPowerQdbm, s.active ? "true" : "false", s.verified ? "true" : "false",
        country, first, count, s.country_error == ESP_OK ? (s.country.policy == WIFI_COUNTRY_POLICY_MANUAL ? "true" : "false") : "null",
        power, channel, s.country_error, s.power_error, s.channel_error,
        apply_error.load(), last_error.load(), (unsigned long)error_count.load());
    return written >= 0 && static_cast<size_t>(written) < size;
}

void studioRadioPrint() {
    const Snapshot s = snapshot();
    printf("Radio target: AU 1-13\nWiFi: %s\nAU verified: %s\n", s.active ? "active" : "off", s.verified ? "yes" : "no");
    if(s.country_error == ESP_OK)
        printf("Driver country: %.2s\nChannels: %u-%u\nCountry policy: %s\n", s.country.cc,
               s.country.schan, s.country.schan + s.country.nchan - 1,
               s.country.policy == WIFI_COUNTRY_POLICY_MANUAL ? "manual" : "auto");
    else printf("Country unavailable: %s\n", esp_err_to_name(s.country_error));
    if(s.power_error == ESP_OK) printf("Driver TX max: %d.%02d dBm\n", s.power / 4, (s.power % 4) * 25);
    else printf("Driver TX max: unavailable\n");
    printf("Configured cap: 20 dBm\nApply error: %s\nLast radio error: %s\nRadio errors: %lu\n",
           esp_err_to_name(apply_error.load()), esp_err_to_name(last_error.load()), (unsigned long)error_count.load());
}
}

extern "C" esp_err_t __wrap_esp_wifi_init(const wifi_init_config_t* config) {
    esp_err_t error = __real_esp_wifi_init(config);
    if(error == ESP_OK) {
        error = marauder::studioRadioApply();
        if(error != ESP_OK) { __real_esp_wifi_deinit(); running.store(false); }
    }
    return error; // INIT_STATE is an expected ownership probe in CSI, not a radio failure.
}
extern "C" esp_err_t __wrap_esp_wifi_start() {
    esp_err_t error = marauder::studioRadioApply();
    if(error != ESP_OK) return error;
    error = __real_esp_wifi_start();
    if(error != ESP_OK) return record(error);
    running.store(true);
    error = __real_esp_wifi_set_max_tx_power(marauder::kStudioTxPowerQdbm);
    int8_t actual = 0;
    if(error == ESP_OK) error = esp_wifi_get_max_tx_power(&actual);
    if(error == ESP_OK && (actual < 8 || actual > marauder::kStudioTxPowerQdbm)) error = ESP_FAIL;
    if(error != ESP_OK) {
        // Do not leave a newly started radio operating with an unverified power ceiling.
        __real_esp_wifi_stop(); running.store(false);
        apply_error.store(error);
    }
    return record(error);
}
extern "C" esp_err_t __wrap_esp_wifi_stop() {
    esp_err_t error = __real_esp_wifi_stop();
    if(error == ESP_OK || error == ESP_ERR_WIFI_NOT_INIT) running.store(false);
    return error;
}
extern "C" esp_err_t __wrap_esp_wifi_deinit() {
    esp_err_t error = __real_esp_wifi_deinit();
    if(error == ESP_OK || error == ESP_ERR_WIFI_NOT_INIT) {
        running.store(false); apply_error.store(ESP_ERR_WIFI_NOT_INIT);
    }
    return error;
}
extern "C" esp_err_t __wrap_esp_wifi_set_channel(uint8_t channel, wifi_second_chan_t secondary) {
    if(marauder::studioRadioValidateChannel(channel) != ESP_OK) return ESP_ERR_INVALID_ARG;
    if((secondary == WIFI_SECOND_CHAN_ABOVE && channel > 9) ||
       (secondary == WIFI_SECOND_CHAN_BELOW && channel < 5)) return record(ESP_ERR_INVALID_ARG);
    esp_err_t error = __real_esp_wifi_set_channel(channel, secondary);
    uint8_t actual = 0;
    wifi_second_chan_t actual_secondary{};
    if(error == ESP_OK) error = esp_wifi_get_channel(&actual, &actual_secondary);
    if(error == ESP_OK && (actual != channel || actual_secondary != secondary)) error = ESP_FAIL;
    return record(error);
}
extern "C" esp_err_t __wrap_esp_wifi_set_max_tx_power(int8_t power) {
    if(power < 8) return record(ESP_ERR_INVALID_ARG);
    if(power > marauder::kStudioTxPowerQdbm) power = marauder::kStudioTxPowerQdbm;
    esp_err_t error = __real_esp_wifi_set_max_tx_power(power);
    int8_t actual = 0;
    if(error == ESP_OK) error = esp_wifi_get_max_tx_power(&actual);
    if(error == ESP_OK && (actual < 8 || actual > power)) error = ESP_FAIL;
    return record(error);
}
extern "C" esp_err_t __wrap_esp_wifi_set_config(wifi_interface_t interface, wifi_config_t* config) {
    if(!config) return record(ESP_ERR_INVALID_ARG);
    // Zero means automatic/default in these driver configs; explicit channels must be AU-valid.
    unsigned channel = interface == WIFI_IF_AP ? config->ap.channel : config->sta.channel;
    if(channel && marauder::studioRadioValidateChannel(channel) != ESP_OK) return ESP_ERR_INVALID_ARG;
    return record(__real_esp_wifi_set_config(interface, config));
}
#endif
