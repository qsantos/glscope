#include "argparse.h"

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* usage_string = "";

struct arginfo arginfo;

void usage(const char* message, ...) {
    if (message != NULL) {
        va_list vargs;
        va_start(vargs, message);
        fprintf(stderr, "Error: ");
        vfprintf(stderr, message, vargs);
        fprintf(stderr, "\n");
        va_end(vargs);
    }

    fprintf(stderr, usage_string, arginfo.argv[0]);
    exit(1);
}

int arg_is(const char* long_name, const char* short_name) {
    if (short_name != NULL) {
        if (strncmp(arginfo.arg, short_name, strlen(short_name)) == 0) {
            return 1;
        }
    }
    if (long_name != NULL) {
        if (strncmp(arginfo.arg, long_name, strlen(long_name)) == 0) {
            return 1;
        }
    }
    return 0;
}

const char* arg_get_str(const char* error_message) {
    // handle --key=value
    const char* equal_sign = strchr(arginfo.arg, '=');
    if (equal_sign != NULL) {
        return equal_sign + 1;
    }

    // get next argument
    arginfo.argi += 1;
    if (arginfo.argi >= arginfo.argc) {
        if (error_message == NULL) {
            usage("the option requires a parameter");
        } else {
            usage(error_message);
        }
    }
    arginfo.arg = arginfo.argv[arginfo.argi];

    return arginfo.arg;
}

long arg_get_int(void) {
    // get next argument
    const char* arg = arg_get_str("the option requires an integer");

    // parse as integer
    char* end_ptr;
    errno = 0;
    long value = strtol(arg, &end_ptr, 0);
    if (end_ptr == arg || *end_ptr != '\0') {
        usage("invalid numeric parameter");
    }
    if (errno != 0) {
        if (value == LONG_MIN) {
            usage("numeric parameter too small");
        } else if (value == LONG_MAX) {
            usage("numeric parameter too large");
        } else {
            usage("unknown error with numeric parameter");
        }
    }

    return value;
}

unsigned long arg_get_uint(void) {
    // get next argument
    const char* arg = arg_get_str("the option requires a non-negative integer");

    // parse as integer
    char* end_ptr;
    errno = 0;
    unsigned long value = strtoul(arg, &end_ptr, 0);
    if (end_ptr == arg || *end_ptr != '\0') {
        usage("invalid non-negative parameter");
    }
    if (errno != 0) {
        if (value == ULONG_MAX) {
            usage("non-negative numeric parameter too large");
        } else {
            usage("unknown error with non-negative numeric parameter");
        }
    }

    return value;
}
