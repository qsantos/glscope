#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "argparse.h"

// parsed arguments
static struct {
    long unsigned int period;
} args = {1000};

unsigned char float2byte(float x) {
    /* Map float range [-1, 1] to integer range [0, 255] */
    unsigned int r = (unsigned int) (x * 128.f + 128.f);
    if (r == 256) {
        r -= 1;
    }
    return (unsigned char) r;
}

static void argparse(int argc, char** argv) {
    usage_string = (
    "Usage: %s PERIOD\n"
    "Generate data for a sine wave. PERIOD is the number of samples to\n"
    "generate for each period.\n"
    "\n"
    "  -h --help          display this help and exit\n"
    );

    size_t positional_arguments_read = 0;
    arginfo.argc = argc;
    arginfo.argv = argv;
    for (arginfo.argi = 1; arginfo.argi < argc; arginfo.argi++) {
        arginfo.arg = argv[arginfo.argi];
        if (arg_is("--help", "-h")) {
            usage(NULL);
        } else if (arginfo.arg[0] == '-') {
            usage("unknown option '%s'", arginfo.arg);
        } else if (positional_arguments_read == 0) {
            args.period = strtoul(arginfo.arg, NULL, 0);
            positional_arguments_read += 1;
        } else {
            usage("too many arguments");
        }
    }
    if (positional_arguments_read < 1) {
        usage("too few arguments");
    }
}

int main(int argc, char** argv) {
    argparse(argc, argv);

    size_t n_points = args.period;
    while (n_points < (1<<15)) {
        n_points *= 2;
    }
    unsigned char* points = malloc(n_points);
    if (points == NULL) {
        usage("could not allocate enough memory for `points`");
    }
    for (unsigned int i = 0; i < n_points; i += 1) {
        float x = (float) i / (float) args.period * 6.283185307179586f;
        unsigned char value = float2byte(sinf(x));
        points[i] = value;
    }

    while (1) {
        fwrite(points, 1, n_points, stdout);
        fflush(stdout);
    }

    free(points);
    return 0;
}
