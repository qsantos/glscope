#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

static double real_clock() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double) now.tv_sec + (double) now.tv_usec / 1e6;
}

static inline void transfer_one_byte(useconds_t delay) {
    unsigned char byte;
    ssize_t n_read = read(0, &byte, 1);
    if (n_read < 0) {
        exit(0);
    }
    write(1, &byte, 1);
    usleep(delay);
}

static double measure_transfer(useconds_t delay) {
    unsigned n_iterations = 100000 / delay;
    double start = real_clock();
    for (unsigned i = 0; i < n_iterations; i += 1) {
        transfer_one_byte(delay);
    }
    double elapsed = real_clock() - start;
    return n_iterations / elapsed;
}

int main() {
    double target_rate = 1000.;

    useconds_t min_delay = 0;
    useconds_t max_delay = 1024;
    while (1) {
        useconds_t delay = (max_delay + min_delay) / 2;
        double effective_rate = measure_transfer(delay);
        if (effective_rate < target_rate) {
            break;
        }
        max_delay *= 2;
    }

    while (min_delay < max_delay) {
        useconds_t delay = (max_delay + min_delay) / 2;
        double effective_rate = measure_transfer(delay);
        if (effective_rate < target_rate) {
            max_delay = delay;
        } else {
            min_delay = delay;
        }
    }

    while (1) {
        transfer_one_byte(min_delay);
    }

    return 0;
}
