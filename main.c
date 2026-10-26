#include "hil.h"

int main() {
    hil_init();

    while (1) {
        hil_display();
    }
}
