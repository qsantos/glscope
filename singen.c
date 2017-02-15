#define _DEFAULT_SOURCE
#include <stdio.h>
#include <math.h>
#include <unistd.h>

unsigned char float2byte(float x) {
    /* Map float range [-1, 1] to integer range [0, 255] */
    unsigned int r = (unsigned int) (x * 128.f + 128.f);
    if (r == 256) {
        r -= 1;
    }
    return (unsigned char) r;
}

int main() {
    unsigned long long i = 0;
    while (1) {
        float x = (float) i * .01f;
        unsigned char value = float2byte(sinf(x));
        fputc(value, stdout);
        fflush(stdout);
        usleep(1000);
        i += 1;
    }

    return 0;
}
