#include <stddef.h>
#include <stdint.h>

static uint16_t* const VIDEO_MEMORY = (uint16_t*)0xB8000;

void kernel_main(void) {
    const char* message = "Hello from SimpleKernel!";
    for (size_t i = 0; message[i] != '\0'; i++) {
        VIDEO_MEMORY[i] = (uint16_t)message[i] | 0x0F00;
    }
    while (1) {
        __asm__("hlt");
    }
}
