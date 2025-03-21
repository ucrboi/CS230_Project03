#include <GLFW/glfw3.h>
#include <cmath>

// Error callback
void error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error: " << description << std::endl;
}

// Draw a circle using GL_TRIANGLE_FAN
void drawCircle(float cx, float cy, float r, int num_segments)
{
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= num_segments; i++)
    {
        float theta = 2.0f * M_PI * float(i) / float(num_segments);
        float x = r * cos(theta);
        float y = r * sin(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void controls(GLFWwindow *window, float *camScale, float *camX, float *camY, float resetScaleValue, bool *shouldMove)
{
    float panSpeed = *camScale * 0.01f; // Pan speed is proportional to the current zoom
    float zoomSpeed = 1.0f;             // Fixed zoom speed

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        *camX -= panSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        *camX += panSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        *camY += panSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        *camY -= panSpeed;

    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        *camScale -= zoomSpeed;
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        *camScale += zoomSpeed;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        *shouldMove = !*shouldMove;
    }

    // Reset view when "0" is pressed
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        *camX = 0.0f;
        *camY = 0.0f;
        *camScale = resetScaleValue;
    }
    if (*camScale < 1.0f)
        *camScale = 1.0f;

    glClear(GL_COLOR_BUFFER_BIT);

    // Update the projection matrix based on camera parameters
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(*camX - *camScale, *camX + *camScale, *camY - *camScale, *camY + *camScale, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}