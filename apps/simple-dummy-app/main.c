#include "../../firmware/hil/environment.h"
#include "../../firmware/hil/hil.h"

PICOTINKER_APP("Dummy Application!");

int main() {
    hil_init();
    hil_debug_init();
    hil_sleep(1000);
    hil_debug_printf("I am a simple app.\n");
    for (;;)
        hil_sleep(1000);
}
