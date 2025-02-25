#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>

const int NUM_SEGMENTS = 100; // Higher means a smoother circle

void drawCircle(float cx, float cy, float r)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); // Center point
    for (int i = 0; i <= NUM_SEGMENTS; i++)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(NUM_SEGMENTS);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL Circle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        return -1;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(1.0f, 1.0f, 1.0f);  // Set circle color to black
        drawCircle(0.0f, 0.0f, 0.5f); // Centered at (0,0) with radius 0.5

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
