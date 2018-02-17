#pragma once

struct GLFWwindow;
typedef unsigned int GLuint;

// Include GLM
#include <glm/glm.hpp>

class Camera;
class Tile;

class Screen 
{
public:
    Screen(const Camera& camera, const Tile& tile);
    virtual ~Screen();
    
    void draw() const;

private:
    const Camera& m_camera;
    const Tile& m_tile;

    GLuint m_vertexArrayId;
    GLuint m_programId;
    GLuint m_matrixId;
    GLuint m_backgroundId;
    GLuint m_cutoffId;
    GLuint m_colorPeriodId;
    GLuint m_textureId;
    GLuint m_colorTextureId;


    GLuint m_vertexBuffer;
    GLuint m_uvBuffer;

    mutable uint64_t m_frameNum = 0;

    GLuint m_colorTexture;

    GLuint renderMandelbrot(double left, double right, double top, double bottom, int maxIt, unsigned int width, unsigned int height);
    GLuint createGradientTexture();
    void setClearColor(float red, float green, float blue, float alpha = 0.0f);
};