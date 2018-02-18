#include "Screen.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>

#include "Camera.h"
#include "Tile.h"

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



Screen::Screen(const Camera& camera) :
    m_camera(camera)
{



    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        throw std::runtime_error("Failed to initialize GLEW");
    }

    // Dark blue background
    setClearColor(0.0f, 0.0f, 0.0f);


    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);



    glGenVertexArrays(1, &m_vertexArrayId);
    glBindVertexArray(m_vertexArrayId);


    // Create and compile our GLSL program from the shaders
    m_programId = LoadShaders(TRANSFORM_VERTEX_SHADER, TEXTURE_FRAGMENT_SHADER);


    // Get a handle for our "MVP" uniform
    m_matrixId = glGetUniformLocation(m_programId, "MVP");



    m_backgroundId = glGetUniformLocation(m_programId, "background");
    m_cutoffId = glGetUniformLocation(m_programId, "cutoff");
    m_colorPeriodId = glGetUniformLocation(m_programId, "colorPeriod");





    m_colorTexture = createGradientTexture();

    // Get a handle for our "myTextureSampler" uniform
    m_textureId = glGetUniformLocation(m_programId, "myTextureSampler");

    m_colorTextureId = glGetUniformLocation(m_programId, "colorSampler");

    glGenBuffers(1, &m_vertexBuffer);

    // All Tiles will share the same UV coords
    glGenBuffers(1, &m_uvBuffer);



}

Screen::~Screen()
{
    // Cleanup VBO and shader
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_uvBuffer);
    glDeleteProgram(m_programId);
    glDeleteTextures(1, &m_colorTexture);
    glDeleteVertexArrays(1, &m_vertexArrayId);

}

void Screen::addTile(Tile* tile)
{
    m_tiles.emplace_back(tile);
}

void Screen::draw() const
{
    ++m_frameNum;


    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(m_programId);

    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    auto mvp = m_camera.getMvp();
    glUniformMatrix4fv(m_matrixId, 1, GL_FALSE, &mvp[0][0]);

    // Background black
    glUniform3f(m_backgroundId, 0.f, 0.f, 0.f);
    glUniform1f(m_cutoffId, m_frameNum / 100.f);
    glUniform1f(m_colorPeriodId, 32.f);


    for (auto tile : m_tiles) {


        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tile->getTexture());
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(m_textureId, 0);

        // Now the color texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, m_colorTexture);
        // Set the colorSampler to use Texture Unit 1
        glUniform1i(m_colorTextureId, 1);



        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

        // Fill the vertex buffer
        GLfloat g_vertex_buffer_data[18];
        tile->getVertexData(g_vertex_buffer_data);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

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
        glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);


        // Fill the UV buffer
        GLfloat g_uv_buffer_data[12];
        tile->getUvData(g_uv_buffer_data);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            2,                                // size : U+V => 2
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 2 * 3); // 12*3 indices starting at 0 -> 12 triangles
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);



}





GLuint Screen::createGradientTexture()
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


void Screen::setClearColor(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
}