#ifndef ZGL_H
#define ZGL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
//#include <GL/gl.h>
#include <glad/glad.h>
#include <stddef.h> // for NULL

typedef unsigned char bool;

// Key state
typedef enum ZGLkeyState {
    ZGL_RELEASED = 0,
    ZGL_PRESSED
} ZGLkeyState;

// Forward declaration of struct
typedef struct ZGLwindow ZGLwindow;

// ZGLwindow struct
struct ZGLwindow {
    HWND hWnd;                       // Win32 window handle
    HDC hDC;                          // Device context
    HGLRC hRC;                        // OpenGL rendering context

    int width;
    int height;
    int shouldClose;

    int keys[256];                    // keyboard state
    void (*framebufferSizeCallback)(ZGLwindow* window, int width, int height);
};

// initialization
int ZGLinit(void);

// Make context current
void ZGLmakeContextCurrent(ZGLwindow* window);

// Window creation / destruction
ZGLwindow* ZGLcreateWindow(int width, int height, const char* title);
void ZGLterminate(ZGLwindow* window);

// Main loop helpers
int ZGLwindowShouldClose(ZGLwindow* window);
void ZGLsetWindowShouldClose(ZGLwindow* window, bool flag);
ZGLkeyState ZGLgetKey(ZGLwindow* window, int key);
void ZGLsetFramebufferSizeCallback(ZGLwindow* window, void (*callback)(ZGLwindow* window, int width, int height));
void ZGLswapBuffers(ZGLwindow* window);
void ZGLpollEvents(void);

#ifdef __cplusplus
}
#endif

#endif // ZGL_H
