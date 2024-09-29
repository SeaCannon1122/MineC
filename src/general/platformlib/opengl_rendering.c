#include "opengl_rendering.h"

#if defined(_WIN32)

#include <windows.h>
#include <gl/gl.h>


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
    hdc = GetDC(hwnd);

    // Set up pixel format descriptor
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd)); // Zero out the structure
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    // Choose a pixel format
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    // Create a rendering context
    hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);
}

void opengl_exit() {
    wglMakeCurrent(hdc, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(NULL, hdc);
}


#elif defined(__linux__)

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

static Display* display;
static Window window;
static GLXContext glxContext;

int opengl_init() {

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        return -1;
    }

    int screen = DefaultScreen(display);

    Window rootWindow = RootWindow(display, screen);

    int fbAttribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo* visualInfo = glXChooseVisual(display, screen, fbAttribs);
    if (visualInfo == NULL) {
        fprintf(stderr, "Unable to choose a visual\n");
        return -1;
    }

    Colormap colormap = XCreateColormap(display, rootWindow, visualInfo->visual, AllocNone);

    XSetWindowAttributes windowAttribs;
    windowAttribs.colormap = colormap;
    windowAttribs.event_mask = ExposureMask | KeyPressMask;

    window = XCreateWindow(
        display, rootWindow,
        0, 0, 10, 10, 0, visualInfo->depth,
        InputOutput, visualInfo->visual,
        CWColormap | CWEventMask, &windowAttribs
    );

    XMapWindow(display, window);

    glxContext = glXCreateContext(display, visualInfo, NULL, GL_TRUE);
    if (glxContext == NULL) {
        fprintf(stderr, "Unable to create GLX context\n");
        return -1;
    }

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
