#ifndef PARSE_H
#define PARSE_H

#include <stddef.h>

extern const char* usage_string;

// internal structure for current state of argument parsing
struct arginfo {
    int argc;
    int argi;
    char** argv;
    char* arg;
};

extern struct arginfo arginfo;

void usage(const char* message, ...);
int arg_is(const char* long_name, const char* short_name);
const char* arg_get_str(const char* error_message);
long arg_get_int(void);
unsigned long arg_get_uint(void);

#endif
