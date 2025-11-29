#include "ZGL.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// WGL extension typedefs
typedef HGLRC(WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL(WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);

// Globals
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
static int ZGLINITIALIZED = 0;
static LARGE_INTEGER ZGL_frequency;
static LARGE_INTEGER ZGL_startTime;

// ----------------------------
// Window procedure
// ----------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ZGLwindow *window = (ZGLwindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!window)
        return DefWindowProc(hWnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_KEYDOWN:
        window->keys[wParam] = 1;
        break;
    case WM_KEYUP:
        window->keys[wParam] = 0;
        break;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        window->width = width;
        window->height = height;

        if (window->framebufferSizeCallback)
            window->framebufferSizeCallback(window, width, height);
    }
    break;
    case WM_CLOSE:
        window->shouldClose = 1;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ----------------------------
// ZGLinit (one-time initialization)
// ----------------------------
int ZGLinit(void)
{
    if (ZGLINITIALIZED)
        return 1;
    HINSTANCE hInst = GetModuleHandle(NULL);

    // Register window class

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ZGLwindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create a small visible dummy window to get WGL extensions
    HWND dummyWnd = CreateWindowEx(0, "ZGLwindowClass", "Dummy", WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 1, 1,
                                   NULL, NULL, hInst, NULL);
    ShowWindow(dummyWnd, SW_SHOW);
    UpdateWindow(dummyWnd);

    HDC dummyDC = GetDC(dummyWnd);

    // Set simple pixel format
    PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1};
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pf = ChoosePixelFormat(dummyDC, &pfd);
    SetPixelFormat(dummyDC, pf, &pfd);

    // Create dummy context
    HGLRC dummyRC = wglCreateContext(dummyDC);
    if (!dummyRC)
    {
        MessageBox(NULL, "Failed to create dummy OpenGL context", "Error", MB_ICONERROR);
        return 0;
    }
    if (!wglMakeCurrent(dummyDC, dummyRC))
    {
        MessageBox(NULL, "Failed to make dummy context current", "Error", MB_ICONERROR);
        return 0;
    }

    // Load WGL extensions
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
        wglGetProcAddress("wglChoosePixelFormatARB");

    if (!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
    {
        MessageBox(NULL, "WGL_ARB_create_context not supported", "Error", MB_ICONERROR);
        return 0;
    }

    // Cleanup dummy
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(dummyRC);
    ReleaseDC(dummyWnd, dummyDC);
    DestroyWindow(dummyWnd);


    // Time
    QueryPerformanceFrequency(&ZGL_frequency);
    QueryPerformanceCounter(&ZGL_startTime);

    ZGLINITIALIZED = 1;
    return 1;
}

// ----------------------------
// Make context current
// ----------------------------
void ZGLmakeContextCurrent(ZGLwindow *window)
{
    if (window)
    {
        wglMakeCurrent(window->hDC, window->hRC);
    }
}

// ----------------------------
// Window creation
// ----------------------------
ZGLwindow *ZGLcreateWindow(int width, int height, const char *title)
{
    if (!ZGLINITIALIZED)
    {
        MessageBox(NULL, "ZGL not initialized! Call ZGLinit first", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }
    HINSTANCE hInst = GetModuleHandle(NULL);
    // Create window
    const int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
    HWND hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "ZGLwindowClass",
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x, y, width, height,
        NULL, NULL, hInst, NULL);

    if (!hWnd)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }

    // Allocate window struct
    ZGLwindow *window = (ZGLwindow *)malloc(sizeof(ZGLwindow));
    if (!window)
        return NULL;
    memset(window, 0, sizeof(ZGLwindow));

    window->hWnd = hWnd;
    window->width = width;
    window->height = height;
    window->shouldClose = 0;
    memset(window->keys, 0, sizeof(window->keys));
    window->framebufferSizeCallback = NULL;

    // Store pointer in Win32 user data
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);

    // Device context
    HDC dc = GetDC(hWnd);

    int pixelAttribs[] = {
        0x2001, 1,      // WGL_DRAW_TO_WINDOW_ARB
        0x2010, 1,      // WGL_SUPPORT_OPENGL_ARB
        0x2011, 1,      // WGL_DOUBLE_BUFFER_ARB
        0x2013, 0x202B, // WGL_PIXEL_TYPE_ARB = RGBA
        0x2014, 24,     // WGL_COLOR_BITS_ARB
        0x2022, 24,     // WGL_DEPTH_BITS_ARB
        0x2023, 8,      // WGL_STENCIL_BITS_ARB
        0};

    int pf;
    UINT numFormats;
    if (!wglChoosePixelFormatARB(dc, pixelAttribs, NULL, 1, &pf, &numFormats) || numFormats == 0)
    {
        MessageBox(NULL, "wglChoosePixelFormatARB failed", "Error", MB_ICONERROR);
        return NULL;
    }

    PIXELFORMATDESCRIPTOR pdf = {0};
    DescribePixelFormat(dc, pf, sizeof(pdf), &pdf);
    SetPixelFormat(dc, pf, &pdf);

    window->hDC = dc;

    // Create OpenGL 3.3 core context
    int contextAttribs[] = {
        0x2091, 3,      // WGL_CONTEXT_MAJOR_VERSION_ARB
        0x2092, 3,      // WGL_CONTEXT_MINOR_VERSION_ARB
        0x9126, 0x0001, // WGL_CONTEXT_PROFILE_MASK_ARB (core)
        0};

    HGLRC glrc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
    if (!glrc)
    {
        MessageBox(NULL, "Failed to create OpenGL 3.3 context", "Error", MB_ICONERROR);
        return NULL;
    }

    if (!wglMakeCurrent(dc, glrc))
    {
        MessageBox(NULL, "Failed to make context current", "Error", MB_ICONERROR);
        return NULL;
    }

    window->hRC = glrc;

    return window;
}

// ----------------------------
// Termination
// ----------------------------
void ZGLterminate(ZGLwindow *window)
{
    if (!window)
        return;

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(window->hRC);
    ReleaseDC(window->hWnd, window->hDC);
    DestroyWindow(window->hWnd);
    free(window);
}

// ----------------------------
// Window helpers
// ----------------------------
int ZGLwindowShouldClose(ZGLwindow *window)
{
    return window->shouldClose;
}

void ZGLsetWindowShouldClose(ZGLwindow *window, bool flag)
{
    window->shouldClose = flag;
}

ZGLkeyState ZGLgetKey(ZGLwindow *window, int key)
{
    return window->keys[key] ? ZGL_PRESSED : ZGL_RELEASED;
}

void ZGLsetFramebufferSizeCallback(ZGLwindow *window, void (*callback)(ZGLwindow *window, int, int))
{
    window->framebufferSizeCallback = callback;
}

void ZGLswapBuffers(ZGLwindow *window)
{
    SwapBuffers(window->hDC);
}

void ZGLpollEvents(void)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void ZGLgetFramebufferSize(ZGLwindow *window, int *width, int *height)
{
    *width = window->width;
    *height = window->height;
}
int ZGLgetCursorPos(ZGLwindow *window, double *x, double *y)
{
    POINT p;
    if (!GetCursorPos(&p))
        return 0;
    if (!ScreenToClient(window->hWnd, &p))
        return 0;

    if (x)
        *x = (double)p.x;
    if (y)
        *y = (double)p.y;

    return 1;
}
double ZGLgetTime(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - ZGL_startTime.QuadPart) / (double)ZGL_frequency.QuadPart;
}