#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/freeglut.h>

double real_clock() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double) now.tv_sec + (double) now.tv_usec / 1e6;
}

#define a_data 100000
static float data[a_data];
static size_t n_data = 0;

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
    for (size_t i = 0; i < a_data; i += 1) {
        float x = (float) i / a_data;
        float y = data[i];
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

    while (1) {
        int value = fgetc(stdin);
        if (value == EOF) {
            break;
        }
        data[n_data] = ((float) value) / 256.f;
        n_data += 1;
        if (n_data >= a_data) {
            n_data -= a_data;
        }
    }

    fclose(stdin);  // TODO: try and reconnect
    return NULL;
}

int main(int argc, char** argv) {
    for (size_t i = 0; i < a_data; i += 1) {
        data[i] = .5f;
    }

    pthread_t incoming_data_thread;
    pthread_create(&incoming_data_thread, NULL, incoming_data_loop, NULL);

    //  GLUT init
    glutInit(&argc, argv);
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
