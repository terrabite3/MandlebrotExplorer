#include "Tile.h"

#include <GL/glew.h>

#include <cstring>


Tile::Tile(double left, double right, double top, double bottom, int maxIt) :
    m_bounds{ (float)left, (float)right, (float)top, (float)bottom, (float)maxIt },
    m_texture(NULL)
{
    glGenTextures(1, &m_texture);

    // Fill the texture with 0

    glBindTexture(GL_TEXTURE_2D, m_texture);

    auto buffer = new float[TEXTURE_SIZE * TEXTURE_SIZE];
    // IEEE says the pattern for 0.0f is all zeros
    memset(buffer, 0, TEXTURE_SIZE * TEXTURE_SIZE * sizeof(float));

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_FLOAT, buffer);
    delete[] buffer;
    
    // Not sure about this stuff
    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // This can't be GL_NEAREST_MIPMAP_*, but what about GL_LINEAR?
    // ... which requires mipmaps. Generate them automatically.
    //glGenerateMipmap(GL_TEXTURE_2D);


    // Unbind the texture? Is this needed?
    glBindTexture(GL_TEXTURE_2D, 0);

    // Not sure what this would do here, but it's in the example
    glFinish();

}

Tile::~Tile()
{
    glDeleteTextures(1, &m_texture);
}

const Tile::Bounds * Tile::getBounds() const
{
    return &m_bounds;
}

GLuint Tile::getTexture() const
{
    return m_texture;
}

int Tile::getTextureSize() const
{
    return TEXTURE_SIZE;
}

void Tile::getVertexData(GLfloat * buffer) const
{
    // Bottom left
    buffer[0] = (float)m_bounds.left;
    buffer[1] = (float)m_bounds.bottom;
    buffer[2] = 0.0f;
    // Top left
    buffer[3] = (float)m_bounds.left;
    buffer[4] = (float)m_bounds.top;
    buffer[5] = 0.0f;
    // Top right
    buffer[6] = (float)m_bounds.right;
    buffer[7] = (float)m_bounds.top;
    buffer[8] = 0.0f;

    // Bottom left
    buffer[9] = (float)m_bounds.left;
    buffer[10] = (float)m_bounds.bottom;
    buffer[11] = 0.0f;
    // Top right
    buffer[12] = (float)m_bounds.right;
    buffer[13] = (float)m_bounds.top;
    buffer[14] = 0.0f;
    // Bottom right
    buffer[15] = (float)m_bounds.right;
    buffer[16] = (float)m_bounds.bottom;
    buffer[17] = 0.0f;

}

void Tile::getUvData(GLfloat * buffer) const
{
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

    memcpy(buffer, g_uv_buffer_data, sizeof(GLfloat) * 12);
}
