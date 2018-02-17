#pragma once

struct GLFWwindow;
typedef unsigned int GLuint;

// Include GLM
#include <glm/glm.hpp>

class Screen 
{
public:
    Screen(unsigned width, unsigned height);
    virtual ~Screen();
    
    void draw() const;

private:

    GLuint m_vertexArrayId;
    GLuint m_programId;
    GLuint m_matrixId;
    GLuint m_backgroundId;
    GLuint m_cutoffId;
    GLuint m_colorPeriodId;
    GLuint m_textureId;
    GLuint m_colorTextureId;

    glm::mat4 m_projection;
    glm::mat4 m_view;
    glm::mat4 m_model;
    glm::mat4 m_mvp;

    GLuint m_vertexBuffer;
    GLuint m_uvBuffer;

    mutable uint64_t m_frameNum = 0;

    GLuint m_texture;
    GLuint m_colorTexture;

    GLuint renderMandelbrot(double left, double right, double top, double bottom, int maxIt, unsigned int width, unsigned int height);
    GLuint createGradientTexture();
    void setClearColor(float red, float green, float blue, float alpha = 0.0f);
};