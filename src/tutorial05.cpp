// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>



// Include GLFW
#include <glfw3.h>


#include "Screen.h"
#include "Camera.h"
#include "Tile.h"

int main( void )
{


    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    int width = 1024;
    int height = 1024;

    GLFWwindow* window = glfwCreateWindow(width, height, "Tutorial 05 - Textured Cube", NULL, NULL);
    if (window == nullptr) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        throw std::runtime_error("Failed to open GLFW window.");
    }
    glfwMakeContextCurrent(window);



    Camera camera;

    Tile tile1(-2.5, -0.5, -2, 0, 1000);
    Tile tile2(-0.5, 1.5, 0, 2, 1000);
    Screen screen(camera);
    screen.addTile(&tile1);
    screen.addTile(&tile2);








    do{
        screen.draw();

        // Swap buffers
        glfwSwapBuffers(window);

        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

