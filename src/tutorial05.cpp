// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>



// Include GLFW
#include <glfw3.h>

#include <CL/cl.h>


#include "Screen.h"
#include "Camera.h"
#include "Tile.h"
#include "OpenClRenderer.h"
#include "CpuRenderer.h"

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
    camera.setCenter(-0.743643887037158704752191506114774, -0.131825904205311970493132056385139);
    camera.setZoom(0.7);

    Tile tile1(-2.5, -0.5, -2, 0, 1000);
    Tile tile2(-2.5, -0.5, 0, 2, 1000);
    Tile tile3(-0.5, 1.5, -2, 0, 1000);
    Tile tile4(-0.5, 1.5, 0, 2, 1000);
    Screen screen(camera);
    screen.addTile(&tile1);

    OpenClRenderer renderer;
    renderer.render(tile1);

    screen.addTile(&tile2);
    screen.addTile(&tile3);
    screen.addTile(&tile4);

    CpuRenderer cpu;
    cpu.render(tile2);
    renderer.render(tile3);
    cpu.render(tile4);

    double zoom = 0.5;



    int frameNum = 0;

    do{
        ++frameNum;
        

        screen.setCutoff(frameNum / 100.0);

        zoom *= 1.001;

        camera.setZoom(zoom);


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

