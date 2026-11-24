#include "environment.h"

#include "hardware/flash.h"
#include "pico.h"
#include "hardware/watchdog.h"
#include <string.h>

struct AppDefinition *const volatile APPLICATIONS_LINKED_LIST_ROOT =
    (void *)0xCAFE4A11;

void env_reset() {
    watchdog_enable(1, 1);

    for (;;) {
        // Wait for the inevitable :)
    }
}

struct AppDefinition *env_get_installed_apps() {
    return APPLICATIONS_LINKED_LIST_ROOT;
}
void hil_debug_printf(const char *, ...);

volatile struct AppDefinition *volatile __uninitialized_ram(env_app_init_override);
struct AppDefinition *env_get_app_to_init_to() {
    if (watchdog_caused_reboot()) {
        return (struct AppDefinition *) env_app_init_override;
    } else {
        return APPLICATIONS_LINKED_LIST_ROOT;
    }
}

void env_commit_userdata(struct AppDefinition *this_app, void *userdata) {
    flash_range_erase((uint32_t)this_app->userdata_base,
                      this_app->userdata_size);
    flash_range_program((uint32_t)this_app->userdata_base, userdata,
                        this_app->userdata_size);
}

void env_load_userdata(struct AppDefinition *this_app, void *userdata) {
    // Reading from flash is simple
    memcpy(userdata, this_app->userdata_base, this_app->userdata_size);
}

void env_load_application(struct AppDefinition *app) {
    env_app_init_override = app;
    env_reset();
}
