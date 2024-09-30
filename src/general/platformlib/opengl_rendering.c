#include "opengl_rendering.h"

#include <stdio.h>
#include <GL/glew.h>
#include <GL/gl.h>


void GLClearError() {

    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line) {
    GLenum error = glGetError();
    while (error) {

        printf("[OpenGL ERROR] (%u): %s %s: %d\n", error, function, file, line);
        return false;
        error = glGetError();
    }

    return true;
}



#if defined(_WIN32)

#include <windows.h>
#include <GL/wglew.h>

typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);

static HDC hdc;
static HGLRC hglrc;

int opengl_init() {
    // Initialize Windows
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HWND hwnd = CreateWindowExA(
        0, "STATIC", "Offscreen OpenGL",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 10, 10,
        NULL, NULL, hInstance, NULL
    );

    // Create a device context
    HDC hdc = GetDC(hwnd);

    // Set up pixel format descriptor
    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
        .cDepthBits = 24,
        .cStencilBits = 8,
    };

    // Choose a pixel format
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    // Create a temporary rendering context
    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

    // Load the wglCreateContextAttribsARB function pointer
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglCreateContextAttribsARB) {
        // Handle the error
        MessageBox(NULL, "wglCreateContextAttribsARB not available!", "Error", MB_OK);
        return -1;
    }

    // Create an OpenGL 3.3 context (or the desired version)
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3, // Major version
        WGL_CONTEXT_MINOR_VERSION_ARB, 3, // Minor version
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // Forward-compatible
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // Core profile WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
        0 // End of attributes
    };

    // Create the OpenGL context
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, NULL, attribs);
    if (!hglrc) {
        // Handle the error
        MessageBox(NULL, "Failed to create OpenGL context!", "Error", MB_OK);
        return -1;
    }

    wglMakeCurrent(hdc, hglrc);
    
    // Delete the temporary context
    wglDeleteContext(tempContext);

    // Initialize GLEW (or another loader)
    glewExperimental = GL_TRUE; // Enable modern OpenGL features
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        // Handle the error
        MessageBox(NULL, (const char*)glewGetErrorString(err), "GLEW Error", MB_OK);
        return -1;
    }

    DestroyWindow(hwnd);

    return 0;
}

void opengl_exit() {
    wglMakeCurrent(hdc, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(NULL, hdc);
}


#elif defined(__linux__)

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <stdio.h>
#include <stdlib.h>

static Display* display;
static GLXContext glxContext;

int opengl_init() {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        return -1;
    }

    int screen = DefaultScreen(display);
    Window rootWindow = RootWindow(display, screen);

    Window window = XCreateSimpleWindow(display, rootWindow,
        10, 10,
        1, 1,
        0, 0,
        0);

    int visual_attribs[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, true,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        None
    };

    int num_fbc = 0;
    GLXFBConfig* fbc = glXChooseFBConfig(display,
        screen,
        visual_attribs, &num_fbc);
    if (!fbc) {
        printf("glXChooseFBConfig() failed\n");
        exit(1);
    }

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = 
        (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

    if (!glXCreateContextAttribsARB) {
        fprintf(stderr, "glXCreateContextAttribsARB not supported\n");
        return -1;
    }

    int contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    glxContext = glXCreateContextAttribsARB(display, fbc[0], NULL, GL_TRUE, contextAttribs);
    if (glxContext == NULL) {
        fprintf(stderr, "Unable to create GLX context\n");
        return -1;
    }

    XMapWindow(display, window);
    glXMakeCurrent(display, window, glxContext);

    XDestroyWindow(display, window);

    return 0;
}

void opengl_exit() {
    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, glxContext);
    XCloseDisplay(display);
}


#endif


GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }

    return shader;
}

// Function to create and compile the shader program
GLuint createShaderProgram(char* vertex_shader_source, char* fragment_shader_source) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragment_shader_source);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}