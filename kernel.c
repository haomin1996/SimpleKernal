#include <stddef.h>
#include <stdint.h>

static volatile uint16_t* const VIDEO_MEMORY = (volatile uint16_t*)0xB8000;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

static size_t cursor = 0;

static void put_char(char c) {
    if (c == '\n') {
        cursor = (cursor / 80 + 1) * 80;
    } else if (c == '\b') {
        if (cursor > 0) {
            cursor--;
            VIDEO_MEMORY[cursor] = ' ' | 0x0F00;
        }
    } else {
        VIDEO_MEMORY[cursor++] = (uint16_t)c | 0x0F00;
    }
    if (cursor >= 80 * 25) {
        cursor = 0;
    }
}

static void print_string(const char* s) {
    for (size_t i = 0; s[i]; i++) {
        put_char(s[i]);
    }
}

static const char keymap[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
    [0x31] = 'n', [0x32] = 'm', [0x39] = ' ',
};

static char get_key(void) {
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t sc = inb(0x60);
            if (!(sc & 0x80)) { // ignore key releases
                if (sc == 0x1C) return '\n';
                if (sc == 0x0E) return '\b';
                char c = keymap[sc];
                if (c) return c;
            }
        }
    }
}

void kernel_main(void) {
    print_string("SimpleKernel CLI\n> ");
    char input[128];
    size_t pos = 0;
    while (1) {
        char c = get_key();
        if (c == '\n') {
            input[pos] = '\0';
            put_char('\n');
            print_string("You typed: ");
            print_string(input);
            put_char('\n');
            pos = 0;
            print_string("> ");
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                put_char('\b');
            }
        } else {
            if (pos < sizeof(input) - 1) {
                input[pos++] = c;
                put_char(c);
            }
        }
    }
}

