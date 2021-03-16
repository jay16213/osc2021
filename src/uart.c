#include "gpio.h"
#include "aux.h"
#include "utils.h"

char uart_getc() {
    char c;
    while (!(*AUX_MU_LSR & 0x01)) {}
    c = (char)(*AUX_MU_IO);
    return c == '\r'?'\n':c;
}

void uart_putc(char c) {
    while (!(*AUX_MU_LSR & 0x20)) {}
    *AUX_MU_IO = c;
}

void print(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}

void print_int(unsigned long long num) {
    if (!num) {
        uart_putc('0');
        return ;
    }
    int buffer[25];
    int count = 0;
    while (num) {
        buffer[count++] = num%10;
        num/=10;
    }
    for (int i = count-1; i >= 0; i--) {
				switch (buffer[i]) {
					case 0:
							uart_putc('0');
							break;
					case 1:
							uart_putc('1');
							break;
					case 2:
							uart_putc('2');
							break;
					case 3:
							uart_putc('3');
							break;
					case 4:
							uart_putc('4');
							break;
					case 5:
							uart_putc('5');
							break;
					case 6:
							uart_putc('6');
							break;
					case 7:
							uart_putc('7');
							break;
					case 8:
							uart_putc('8');
							break;
					case 9:
							uart_putc('9');
							break;
				}
		}
}
