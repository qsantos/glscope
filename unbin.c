#include <stdio.h>

int main() {
    while (1) {
        int byte = fgetc(stdin);
        if (byte == EOF) {
            break;
        }
        printf("%i\n", byte);
    }
    return 0;
}
