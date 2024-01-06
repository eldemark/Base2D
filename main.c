#include "glad/glad.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


char* load_shader(const char* file_name)
{
    FILE* file = fopen(file_name, "r");
    char* buffer;
    size_t file_size;

    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Get file size using file pointer.
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // Allocate memory for file plus null terminator.
    buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

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

    const char* vert_src = load_shader("shaders/vertex.glsl");
    const char* frag_src = load_shader("shaders/fragment.glsl");

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
    glShaderSource(vertexShader, 1, &vert_src, NULL);
    glCompileShader(vertexShader);

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag_src, NULL);
    glCompileShader(fragmentShader);

    // Shader Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete shaders; no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load texture.

    int width, height, nrChannels;
    unsigned char* data = stbi_load("texture/texture.png", &width, &height, &nrChannels, 0);

    if (!data) {
        printf("Failed to load texture\n");
        exit(1);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    // End load texture.

    // Set up tile and texture.

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

    // End set up tile and texture.

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

    free((char*)vert_src);
    free((char*)frag_src);

    return 0;
}
