#include <stdio.h>
#include <stdlib.h>
#include <math.h>

unsigned char float2byte(float x) {
    /* Map float range [-1, 1] to integer range [0, 255] */
    unsigned int r = (unsigned int) (x * 128.f + 128.f);
    if (r == 256) {
        r -= 1;
    }
    return (unsigned char) r;
}

int main(int argc, char** argv) {
    unsigned int period = 100000;
    unsigned char* points = malloc(period);
    if (points == NULL) {
        fprintf(stderr, "could not allocate enough memory for `points`");
        return 1;
    }
    for (unsigned int i = 0; i < 1000; i += 1) {
        float x = (float) i / (float) period * 6.283185307179586f;
        unsigned char value = float2byte(sinf(x));
        points[i] = value;
    }

    while (1) {
        fwrite(points, 1, period, stdout);
        fflush(stdout);
    }

    free(points);
    return 0;
}
