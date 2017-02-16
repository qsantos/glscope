#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/freeglut.h>

#include "argparse.h"

// parsed arguments
static struct {
    unsigned long samples_count;
    int show_old_samples;
    const char* file;
} args = {100000, 1, NULL};

double real_clock() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double) now.tv_sec + (double) now.tv_usec / 1e6;
}

/* Storage of currently displayed samples */
static size_t samples_memory_size;  // amount of memory used to store samples
static float* samples;  // local memory used to store samples
static size_t current_sample = 0;  // this is the index of the last sample + 1
static unsigned int samples_opengl_buffer_id;  // GPU memory used for samples
#define SAMPLE(i) (samples[2*(i) + 1])  // accessing the i-th sample

void displayFunc(void) {
    // compute FPS
    static double previous = 0.;
    static size_t aggregated = 0;
    double now = real_clock();
    if (previous == 0.) {
        previous = now;
    } else {
        aggregated += 1;
        if (aggregated >= 10) {
            double elapsed = (now - previous) / ((double) aggregated);
            double fps = 1 / elapsed;
            printf("%.0f\n", fps);
            aggregated = 0;
            previous = now;
        }
    }

    // reset everything
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // save the number of samples to display *before* transfering them to the GPU
    // new samples may be obtained between glBufferData() and glDrawArrays()
    size_t last_sample = args.show_old_samples ? args.samples_count : current_sample;

    // display graph
    // TODO: transfer only changed data (10M samples = 80MB/frame = 4.8GB/s)
    glBufferData(GL_ARRAY_BUFFER, samples_memory_size, samples, GL_STATIC_DRAW);
    glDrawArrays(GL_LINE_STRIP, 0, last_sample);

    glutSwapBuffers();
    glutPostRedisplay();
}

void reshapeFunc(int width, int height) {
    /* Handle window reshapings (GLUT callback) */
    glViewport(0, 0, width, height);

    // reset projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //// left, right, bottom, top, near far
    //glOrtho(0.f, width, height, 0.f, -1.f, 10.f);
    glOrtho(0.f, 1.f, 0.f, 1.f, -1.f, 10.f);
    glMatrixMode(GL_MODELVIEW);

    glutPostRedisplay();
}

void keyboardFunc(unsigned char k, int x, int y) {
    (void) x;
    (void) y;
    /* Handle key presses (GLUT callback) */

    if (k == 0x1b) {  // escape
        glutLeaveMainLoop();
    }
}

void* incoming_data_loop(void* arg) {
    (void) arg;

    FILE* f;
    if (args.file != NULL) {
        f = fopen(args.file, "r");
        if (f == NULL) {
            usage("could not open file '%s'", args.file);
        }
    } else {
        f = stdin;
    }

    // read file until exhaustion
    while (1) {
        int value = fgetc(f);
        if (value == EOF) {
            break;
        }
        SAMPLE(current_sample) = ((float) value) / 256.f;
        current_sample += 1;
        if (current_sample >= args.samples_count) {
            current_sample -= args.samples_count;
        }
    }

    fclose(f);  // TODO: try and reconnect
    return NULL;
}

static void argparse(int argc, char** argv) {
    usage_string = (
    "Usage: %s [OPTIONS] [FILE]\n"
    "Draw a real-time graph\n"
    "\n"
    "  -h --help          display this help and exit\n"
    "  -x --xlen N        width of the graph in data samples\n"
    "  -c --clear-old     clear screen when wrapping\n"
    "  -k --keep-old      keep old samples on screen when\n"
    );

    size_t positional_arguments_read = 0;
    arginfo.argc = argc;
    arginfo.argv = argv;
    for (arginfo.argi = 1; arginfo.argi < argc; arginfo.argi++) {
        arginfo.arg = argv[arginfo.argi];
        if (arg_is("--help", "-h")) {
            usage(NULL);
        } else if (arg_is("--xlen", "-x")) {
            args.samples_count = arg_get_uint();
        } else if (arg_is("--clear-old", "-c")) {
            args.show_old_samples = 0;
        } else if (arg_is("--keep-old", "-k")) {
            args.show_old_samples = 1;
        } else if (arginfo.arg[0] == '-') {
            usage("unknown option '%s'", arginfo.arg);
        } else if (positional_arguments_read == 0) {
            args.file = arginfo.arg;
            positional_arguments_read += 1;
        } else {
            usage("too many arguments");
        }
    }
}

int main(int argc, char** argv) {
    // arguments
    glutInit(&argc, argv);  // may remove some arguments
    argparse(argc, argv);

    // collect samples into buffer within separate thread
    samples_memory_size = args.samples_count * sizeof(float) * 2;
    samples = malloc(samples_memory_size);
    if (samples == NULL) {
        usage("could not allocate enough memory for `samples`");
    }
    for (size_t i = 0; i < args.samples_count; i += 1) {
        samples[2*i + 0] = (float) i / (float) args.samples_count;
        /* SAMPLE(i)     = .5f;*/
        samples[2*i + 1] = .5f;
    }
    pthread_t incoming_data_thread;
    pthread_create(&incoming_data_thread, NULL, incoming_data_loop, NULL);

    //  GLUT init
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Hello World!");

    // callbacks
    glutDisplayFunc(displayFunc);
    glutKeyboardFunc(keyboardFunc);
    //glutSpecialFunc(specialFunc);
    glutReshapeFunc(reshapeFunc);
    //glutMotionFunc(motionFunc);
    //glutPassiveMotionFunc(passiveMotionFunc);
    //glutMouseFunc(mouseFunc);
    //glutCloseFunc(closeFunc);

    // OpenGL init
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(3);
    glGenBuffers(1, &samples_opengl_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, samples_opengl_buffer_id);
    glVertexPointer(2, GL_FLOAT, 0, NULL);
    glEnableClientState(GL_VERTEX_ARRAY);

    glutMainLoop();

    pthread_cancel(incoming_data_thread);

    return 0;
}
