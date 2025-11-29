#include "include.h"
#include "ZGL.h"
#include "shader.h"

unsigned int wireframeMode = 1;
double lastKeyPressTime = 0.0;

void framebuffer_size_callback(ZGLwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void clear()
{
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void processInput(ZGLwindow *window)
{
    if (ZGLgetKey(window, VK_ESCAPE) == ZGL_PRESSED)
        ZGLsetWindowShouldClose(window, TRUE);
    if (ZGLgetKey(window, VK_F1) == ZGL_PRESSED)
    {
        double currentTime = ZGLgetTime();
        if (currentTime - lastKeyPressTime > 0.2) // Minimum delay of 0.2 seconds
        {
            if (wireframeMode == 0)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Enable wireframe mode
                wireframeMode++;
            }
            else if (wireframeMode == 1)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Disable wireframe mode
                wireframeMode++;
            }
            else if (wireframeMode == 2)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // point
                wireframeMode = 0;
            }

            lastKeyPressTime = currentTime;
        }
    }
}

int main(int argc, char **argv)
{
    ZGLinit();
    ZGLwindow *window = ZGLcreateWindow(800, 600, "openGL3.3 ZGL Window");
    if (window == NULL)
    {
        MessageBox(NULL, "Failed to create window", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    ZGLmakeContextCurrent(window);
    if (!gladLoadGL())
    {
        MessageBox(NULL, "Failed to initialize GLAD", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ZGLsetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, 800, 600);

    // create shader program
    GLuint shaderProgram = createShader("res/shaders/baseVertex.glsl", "res/shaders/baseFragment.glsl");

    // int numTris = 2;
    float vertices[] =
        {
            -1.0f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f};
    unsigned int indices[] =
        {
            0, 1, 2,
            2, 1, 3};

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // unbind VBO -- do not unbind EBO while VAO is active
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // wire frame mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // main loop
    while (!ZGLwindowShouldClose(window))
    {
        processInput(window);
        clear();
        // render here
        glUseProgram(shaderProgram);
        glUniform1f(glGetUniformLocation(shaderProgram, "u_time"), ZGLgetTime());
        int width, height;
        double m_x, m_y;
        ZGLgetFramebufferSize(window, &width, &height);
        ZGLgetCursorPos(window, &m_x, &m_y);
        glUniform2f(glGetUniformLocation(shaderProgram, "u_resolution"), (float)width, (float)height);
        // printf( "%d %d\n",(int)m_x, (int)m_y );
        glUniform2f(glGetUniformLocation(shaderProgram, "u_mouse"), (float)m_x, (float)m_y);

        glBindVertexArray(VAO);
        // glDrawArrays( GL_TRIANGLES, 0, 6 );
        int numTris = sizeof(indices) / sizeof(int) / 3;
        glDrawElements(GL_TRIANGLES, numTris * 3, GL_UNSIGNED_INT, 0); // numTris*3 -> 3 edges in a triangle

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        ZGLswapBuffers(window);
        ZGLpollEvents();
    }

    ZGLterminate(window);
    return 0;
}
