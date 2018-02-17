// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>


const std::string TRANSFORM_VERTEX_SHADER = R"(
#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
}
)";

const std::string TEXTURE_FRAGMENT_SHADER = R"(
#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

uniform vec3 background;

uniform float cutoff;
uniform float colorPeriod;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

uniform sampler1D colorSampler;

void main(){

	float depth = texture( myTextureSampler, UV).r;

	if (depth > cutoff)
		color = background;
	else
		color = texture( colorSampler, depth / colorPeriod ).rgb;


	// Output color = color of the texture at the specified UV
//	color.r = texture( myTextureSampler, UV ).r;
//	color.g = texture( myTextureSampler, UV ).r;
//	color.b = texture( myTextureSampler, UV ).r;
}
)";

#define SMOOTH

GLuint renderMandelbrot(double left, double right, double top, double bottom, int maxIt, unsigned int width, unsigned int height) {



    unsigned int bufsize = width * height;
    auto buffer = new float[bufsize];

#ifndef SMOOTH
    // Render the fractal
    for (unsigned py = 0; py < height; ++py) {
        for (unsigned px = 0; px < width; ++px) {

            double x0 = left + (px * (right - left)) / width;
            double y0 = top + (py * (bottom - top)) / height;

            double x = 0;
            double y = 0;

            unsigned i;
            for (i = 0; i < maxIt; ++i) {

                double xtemp = x*x - y*y + x0;
                y = 2 * x * y + y0;
                x = xtemp;

                double magnitudeSqr = x * x + y * y;

                if (magnitudeSqr > 2 * 2) break;
            }

            //buffer[py * width + px] = (i == maxIt) ? (FLT_MAX) : i / 1.f;
            buffer[py * width + px] = i;

        }
    }
#else

    // Render the fractal
    for (unsigned py = 0; py < height; ++py) {
        for (unsigned px = 0; px < width; ++px) {

            double x0 = left + (px * (right - left)) / width;
            double y0 = top + (py * (bottom - top)) / height;

            double x = 0;
            double y = 0;

            // Source: https://en.wikipedia.org/wiki/Mandelbrot_set#Continuous_(smooth)_coloring
            // Here N=2^8 is chosen as a reasonable bailout radius
            int i = 0;
            while (x*x + x*y < (1 << 16) && i < maxIt) {
                double xtemp = x*x - y*y + x0;
                y = 2 * x*y + y0;
                x = xtemp;

                ++i;
            }

            double iteration = i;

            // Used to vaoid floating point issues with points inside the set.
            if (iteration < maxIt) {
                // sqrt of inner term remove using log simplification rules.
                double log_zn = log(x*x + y*y) / 2;
                double nu = log(log_zn / log(2)) / log(2);
                // Rearranging the potential function.
                // Dividing log_zn by log(2) instead of log(N = 1<<8)
                // because we want the entire palette to range from the 
                // center to radius 2, NOT our bailout radius.
                iteration = iteration + 1 - nu;
            }
            else {
                // No need to change iteration -> shader will do the actual gating
                // Plus, anisotropic filtering will work better if it isn't an extreme value
            }

            //buffer[py * width + px] = (i == maxIt) ? (FLT_MAX) : i / 1.f;
            buffer[py * width + px] = (float)iteration;

        }
    }
#endif

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, buffer);

    // OpenGL has now copied the data. Free our own version
    delete[] buffer;


    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // ... which requires mipmaps. Generate them automatically.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Return the ID of the texture we just created
    return textureID;


}


GLuint createGradientTexture() 
{
    int width = 256 * 6;

    auto buffer = new unsigned char[width * 3];


    // Red to yellow
    for (int i = 0; i < 256; ++i)
    {
        auto pixel = buffer + ((0 * 256) + i) * 3;

        pixel[0] = 255;
        pixel[1] = i;
        pixel[2] = 0;
    }
    // Yellow to green
    for (int i = 0; i < 256; ++i)
    {
        auto pixel = buffer + ((1 * 256) + i) * 3;

        pixel[0] = 255 - i;
        pixel[1] = 255;
        pixel[2] = 0;
    }
    // Green to cyan
    for (int i = 0; i < 256; ++i)
    {
        auto pixel = buffer + ((2 * 256) + i) * 3;

        pixel[0] = 0;
        pixel[1] = 255;
        pixel[2] = i;
    }
    // Cyan to blue
    for (int i = 0; i < 256; ++i)
    {
        auto pixel = buffer + ((3 * 256) + i) * 3;

        pixel[0] = 0;
        pixel[1] = 255 - i;
        pixel[2] = 255;
    }
    // Blue to violet
    for (int i = 0; i < 256; ++i)
    {
        auto pixel = buffer + ((4 * 256) + i) * 3;

        pixel[0] = i;
        pixel[1] = 0;
        pixel[2] = 255;
    }
    // Violet to red
    for (int i = 0; i < 256; ++i)
    {
        auto pixel = buffer + ((5 * 256) + i) * 3;

        pixel[0] = 255;
        pixel[1] = 0;
        pixel[2] = 255 - i;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_1D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // I don't think this is necessary

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, width, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    // Do I need filtering?

    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // ... which requires mipmaps. Generate them automatically.
    glGenerateMipmap(GL_TEXTURE_1D);

    return textureID;
}


int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    int width = 1024; 
    int height = 1024;
    window = glfwCreateWindow(width, height, "Tutorial 05 - Textured Cube", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( TRANSFORM_VERTEX_SHADER, TEXTURE_FRAGMENT_SHADER );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    GLuint backgroundID = glGetUniformLocation(programID, "background");
    GLuint cutoffID = glGetUniformLocation(programID, "cutoff");
    GLuint colorPeriodID = glGetUniformLocation(programID, "colorPeriod");

    // By using an identity projection matrix, we use -1,1 coords
    glm::mat4 Projection = glm::mat4(1.f);

    // Camera matrix
    // Scale by 0.9 so we can see the edge of the texture
    //glm::mat4 View = glm::scale(glm::mat4(1.f), glm::vec3(0.9f, 0.9f, 1.f));
    glm::mat4 View(1.f);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around

    // Load the texture using any two methods
    int texSize = 2048;
    GLuint Texture = renderMandelbrot(-2.5, 1.5, -2, 2, 1000, texSize, texSize);

    GLuint colorTex = createGradientTexture();
    
    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    GLuint colorTexID = glGetUniformLocation(programID, "colorSampler");


    static const GLfloat g_vertex_buffer_data[] = { 
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,

         1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        
    };

    static const GLfloat g_uv_buffer_data[] = { 

        // Top left
        0.f, 1.f,
        // Bottom left
        0.f, 0.f,
        // Bottom right
        1.f, 0.f,
        
        // Top left
        0.f, 1.f,
        // Bottom right
        1.f, 0.f,
        // Top right
        1.f, 1.f,
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);


    int frame = 0;

    do{
        ++frame;


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // Send our transformation to the currently bound shader, 
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Background black
        glUniform3f(backgroundID, 0.f, 0.f, 0.f);
        glUniform1f(cutoffID, frame / 100.f);
        glUniform1f(colorPeriodID, 32.f);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // Now the color texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, colorTex);
        // Set the colorSampler to use Texture Unit 1
        glUniform1i(colorTexID, 1);


        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            2,                                // size : U+V => 2
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

