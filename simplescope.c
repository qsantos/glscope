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
    unsigned long xlen;
    int show_old_samples;
    const char* file;
} args = {100000, 1, NULL};

double real_clock() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double) now.tv_sec + (double) now.tv_usec / 1e6;
}

static float* samples;
static size_t current_sample = 0;

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

    /* Draw the scene */

    /*
    // pack data
    size_t n_points = 1024;
    float data_buffer[n_points*2];
    for (size_t i = 0; i < n_points; i += 1) {
        float x = ((float) i) * 2.f * M_PI / 1000.f;
        float y = sinf(x);
        data_buffer[2*i + 0] = x;
        data_buffer[2*i + 1] = y;
    }

    // fill new buffer
    unsigned int buffer_id;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);;
    glBufferData(GL_ARRAY_BUFFER, n_points*2*sizeof(float), data_buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glVertexPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, n_points);
    glDisableClientState(GL_VERTEX_ARRAY);
    */

    glBegin(GL_LINE_STRIP);
    size_t last_sample = args.show_old_samples ? args.xlen : current_sample;
    for (size_t i = 0; i < last_sample; i += 1) {
        float x = (float) i / (float) args.xlen;
        float y = samples[i];
        glVertex2f(x, y);
    }
    glEnd();

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
        samples[current_sample] = ((float) value) / 256.f;
        current_sample += 1;
        if (current_sample >= args.xlen) {
            current_sample -= args.xlen;
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
            args.xlen = arg_get_uint();
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
    samples = malloc(args.xlen * sizeof(float));
    if (samples == NULL) {
        usage("could not allocate enough memory for `samples`");
    }
    for (size_t i = 0; i < args.xlen; i += 1) {
        samples[i] = .5f;
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

    glutMainLoop();

    pthread_cancel(incoming_data_thread);

    return 0;
}
