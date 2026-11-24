#include "environment.h"
#include "hil.h"
#include "system_environment.h"
#include <stdlib.h>
#include <string.h>

#define APPLICATIONS_DYNAMIC_RAM_START (void *)0x20060000
#define APPLICATIONS_DYNAMIC_RAM_TOLERANCES 0x10000

int main() {
    // TODO: Remove in real firmware:
    hil_init();
    hil_debug_init();

    // Wait for CDC to attach
    hil_sleep(1000);

    hil_debug_printf("Starting picotinker...\n");
    ENVIRONMENT_IN_ROOT_FUNC_INIT;

    struct AppDefinition *to_load = env_get_app_to_init_to();
    if (to_load == NULL || to_load == (void *)0xCAFE4A11) {
        // TODO: Make this pretty:
        hil_debug_printf("No applications to load!!\n");
        for (;;)
            hil_sleep(1000);
    }

    hil_debug_printf("Starting application (%p) %s...\n", to_load->name,
                     to_load->name);
#ifdef PICO_BOARD
    // Load application RAM (only on PICO!)
    hil_debug_printf("Allocating .data, .bss (%08x bytes)...\n",
                     to_load->ram_size_full);
    void *application_memory =
        malloc(to_load->ram_size_full + APPLICATIONS_DYNAMIC_RAM_TOLERANCES);
    if (application_memory + APPLICATIONS_DYNAMIC_RAM_TOLERANCES <
        APPLICATIONS_DYNAMIC_RAM_START) {
        hil_debug_printf(
            "Memory alignment error during application's loading (expected "
            "beginning of alloc at %p +- %06x, got %p)!\n",
            APPLICATIONS_DYNAMIC_RAM_START, APPLICATIONS_DYNAMIC_RAM_TOLERANCES,
            application_memory);
        for (;;)
            hil_sleep(1000);
    }
    application_memory = APPLICATIONS_DYNAMIC_RAM_START; // Force.

    hil_debug_printf("Transferring .data from %p to %p (%04x bytes)...\n",
                     to_load->ram_base, application_memory,
                     to_load->ram_size_def);
    memcpy(application_memory, to_load->ram_base, to_load->ram_size_def);
    hil_debug_printf("Zeroing .bss from at %p (%04x bytes)...\n",
                     application_memory + to_load->ram_size_def,
                     to_load->ram_size_full - to_load->ram_size_def);
    memset(application_memory + to_load->ram_size_def, 0,
           to_load->ram_size_full - to_load->ram_size_def);
#endif

    // Load the userdata:
    if (to_load->userdata_ptr_loc) {
        *to_load->userdata_ptr_loc = malloc(to_load->userdata_size);
        env_load_userdata(to_load, *to_load->userdata_ptr_loc);
    }

    // Run the app
    hil_debug_printf("All done. Jumping to entry (%p) (->%08x)\n",
                     to_load->entry, *(unsigned int *)to_load->entry);
    to_load->entry();

    hil_debug_printf(
        "An application shouldn't ever exit its main() function!\n");
    env_reset();
}
