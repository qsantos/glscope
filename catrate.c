#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>

int main() {
    while (1) {
        int byte = fgetc(stdin);
        if (byte == EOF) {
            break;
        }
        fputc(byte, stdout);
        fflush(stdout);
        usleep(100);
    }
    return 0;
}
