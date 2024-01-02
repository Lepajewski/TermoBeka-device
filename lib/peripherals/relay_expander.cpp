#include "global_config.h"
#include "logger.h"

#include "relay_expander.h"


const char * const TAG = "RelayExpander";


RelayExpander::RelayExpander() {}

RelayExpander::~RelayExpander() {
    this->end();
}

void RelayExpander::setup_relays() {
    for (auto &r : this->relays) {
        if (r.setup() != ESP_OK) {
            TB_LOGE(TAG, "fail to setup relay");
            return;
        }
        r.toggle_off();
    }
}

Relay *RelayExpander::lookup_relay(RelayType type) {
    for (auto &i : this->relays) {
        if (type == i.get_type()) {
            return &i;
        }
    }

    return NULL;
}

void RelayExpander::begin() {
    this->controller.begin();

    this->setup_relays();
}

void RelayExpander::end() {
    this->controller.end();
}

void RelayExpander::relay_on(RelayType type) {
    Relay *relay = this->lookup_relay(type);

    if (relay == NULL) {
        return;
    }

    relay->toggle_on();
}
void RelayExpander::relay_off(RelayType type) {
    Relay *relay = this->lookup_relay(type);

    if (relay == NULL) {
        return;
    }

    relay->toggle_off();
}
