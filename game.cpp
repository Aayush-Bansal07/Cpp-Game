#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

float x = 0.0f, z = -5.0f;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) z += 0.05f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) z -= 0.05f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) x -= 0.05f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) x += 0.05f;
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Game", NULL, NULL);
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    glewInit();
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(x, 0.0f, z);

        glBegin(GL_QUADS);
        glColor3f(0.2f, 0.7f, 1.0f);

        // Front
        glVertex3f(-1, -1,  1);
        glVertex3f( 1, -1,  1);
        glVertex3f( 1,  1,  1);
        glVertex3f(-1,  1,  1);

        // Back
        glVertex3f(-1, -1, -1);
        glVertex3f(-1,  1, -1);
        glVertex3f( 1,  1, -1);
        glVertex3f( 1, -1, -1);

        // Left
        glVertex3f(-1, -1, -1);
        glVertex3f(-1, -1,  1);
        glVertex3f(-1,  1,  1);
        glVertex3f(-1,  1, -1);

        // Right
        glVertex3f(1, -1, -1);
        glVertex3f(1,  1, -1);
        glVertex3f(1,  1,  1);
        glVertex3f(1, -1,  1);

        // Top
        glVertex3f(-1, 1, -1);
        glVertex3f(-1, 1,  1);
        glVertex3f( 1, 1,  1);
        glVertex3f( 1, 1, -1);

        // Bottom
        glVertex3f(-1, -1, -1);
        glVertex3f( 1, -1, -1);
        glVertex3f( 1, -1,  1);
        glVertex3f(-1, -1,  1);

        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
