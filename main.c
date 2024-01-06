#include "glad/glad.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

// Vertex Shader source
const char* vertexShaderSource = "#version 460 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";

// Fragment Shader source
const char* fragmentShaderSource = "#version 460 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n"
                                   "}\n\0";

int main()
{
    Display* dpy;
    Window root;
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XVisualInfo* vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    Window win;
    GLXContext glc;
    XWindowAttributes gwa;
    XEvent xev;

    dpy = XOpenDisplay(NULL);

    if (dpy == NULL) {
        printf("\n\tcannot connect to X server\n\n");
        exit(0);
    }

    root = DefaultRootWindow(dpy);
    vi = glXChooseVisual(dpy, 0, att);

    if (vi == NULL) {
        printf("\n\tno appropriate visual found\n\n");
        exit(0);
    } else {
        printf("\n\tvisual %p selected\n", (void*)vi->visualid);
    }

    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    win = XCreateWindow(dpy,      //
        root,                     //
        0,                        //
        0,                        //
        600,                      //
        600,                      //
        0,                        //
        vi->depth,                //
        InputOutput,              //
        vi->visual,               //
        CWColormap | CWEventMask, //
        &swa);

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Basic 2D");

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glXGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Shader Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete shaders; no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data and buffers
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // bottom left
        0.5f, -0.5f, 0.0f,  // bottom right
        0.5f, 0.5f, 0.0f,   // top right
        -0.5f, 0.5f, 0.0f   // top left
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (1) {
        XNextEvent(dpy, &xev);

        if (xev.type == Expose) {
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0, 0.0, 0.0, 1.0);

            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glXSwapBuffers(dpy, win);
        }

        if (xev.type == KeyPress) {
            glXMakeCurrent(dpy, None, NULL);
            glXDestroyContext(dpy, glc);
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);
            exit(0);
        }
    }
    return 0;
}
