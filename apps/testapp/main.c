#include "../../firmware/hil/environment.h"
#include "../../firmware/hil/hil.h"
#include <stdlib.h>
#include <string.h>

PICOTINKER_APP("Test Application!");

void super_advanced_fps_meter_draw() {
    static volatile int start = 0;

    uint8_t *buffer = hil_get_display_buffer();

    buffer[start * 3 + 0] = 0xFF;
    buffer[start * 3 + 1] = 0xFF;
    buffer[start * 3 + 2] = 0xFF;

    start += 1;
    if (start == 240 * 240) {
        start = 0;
        memset(buffer, 0, SCREEN_SIZE_X * SCREEN_SIZE_Y * 3);
    }
}

int my_test_global = 10;

int main() {
    hil_init();
    hil_debug_init();
    hil_sleep(1000);
    hil_debug_printf(
        "Hello, the answer to life, universe and everything is %d\n", 42);

    int *x = malloc(4);
    hil_debug_printf("My value A is %p\n", super_advanced_fps_meter_draw);
    *x = 10;
    hil_debug_printf("My value B is %p\n", *x);

    hil_debug_printf("My value BPTR is %p\n", malloc(1000));

    struct AppDefinition *c_app = env_get_installed_apps(), *last_app = NULL;
    hil_debug_printf("All apps' root = %p\n", c_app);
    while (c_app != NULL) {
        hil_debug_printf("- App: %s %p\n", c_app->name, c_app->next);
        last_app = c_app;
        c_app = c_app->next;
    }

    hil_debug_printf("That's all the apps.\n");

    while (1) {
        hil_debug_printf("x %d\n", my_test_global);
        super_advanced_fps_meter_draw();
        hil_display();
        if (my_test_global++ == 100) {
            hil_debug_printf("Switching apps...\n");
            env_load_application(last_app);
        }
    }
}
