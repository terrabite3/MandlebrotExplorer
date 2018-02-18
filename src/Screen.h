#pragma once

#include <vector>

struct GLFWwindow;
typedef unsigned int GLuint;

// Include GLM
#include <glm/glm.hpp>

class Camera;
class Tile;

class Screen 
{
public:
    Screen(const Camera& camera);
    virtual ~Screen();
    
    void addTile(Tile* tile);

    void draw() const;

private:
    const Camera& m_camera;
    std::vector<Tile*> m_tiles;

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

    GLuint createGradientTexture();
    void setClearColor(float red, float green, float blue, float alpha = 0.0f);
};