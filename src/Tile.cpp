#include "Tile.h"

// Include GLEW
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>

Tile::Tile(double left, double right, double top, double bottom, int maxIt, bool smooth) :
    m_left(left),
    m_right(right),
    m_top(top),
    m_bottom(bottom),
    m_maxIt(maxIt),
    m_smooth(smooth),
    m_texture(NULL)
{
}

Tile::~Tile()
{
    glDeleteTextures(1, &m_texture);
}

GLuint Tile::getTexture() const
{
    if (m_texture == NULL)
        render();

    return m_texture;
}

void Tile::getVertexData(GLfloat * buffer) const
{
    // Bottom left
    buffer[0] = (float)m_left;
    buffer[1] = (float)m_bottom;
    buffer[2] = 0.0f;
    // Top left
    buffer[3] = (float)m_left;
    buffer[4] = (float)m_top;
    buffer[5] = 0.0f;
    // Top right
    buffer[6] = (float)m_right;
    buffer[7] = (float)m_top;
    buffer[8] = 0.0f;

    // Bottom left
    buffer[9] = (float)m_left;
    buffer[10] = (float)m_bottom;
    buffer[11] = 0.0f;
    // Top right
    buffer[12] = (float)m_right;
    buffer[13] = (float)m_top;
    buffer[14] = 0.0f;
    // Bottom right
    buffer[15] = (float)m_right;
    buffer[16] = (float)m_bottom;
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


void Tile::render() const
{
    // Check if we have already rendered
    if (m_texture != NULL)
        return;


    int width = TEXTURE_SIZE;
    int height = TEXTURE_SIZE;

    unsigned int bufsize = width * height;
    auto buffer = new float[bufsize];

    if (!m_smooth) {
        // Render the fractal
        for (unsigned py = 0; py < height; ++py) {
            for (unsigned px = 0; px < width; ++px) {

                double x0 = m_left + (px * (m_right - m_left)) / width;
                double y0 = m_top + (py * (m_bottom - m_top)) / height;

                double x = 0;
                double y = 0;

                unsigned i;
                for (i = 0; i < m_maxIt; ++i) {

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
    }
    else
    {
        // Render the fractal
        for (unsigned py = 0; py < height; ++py) {
            for (unsigned px = 0; px < width; ++px) {

                double x0 = m_left + (px * (m_right - m_left)) / width;
                double y0 = m_top + (py * (m_bottom - m_top)) / height;

                double x = 0;
                double y = 0;

                // Source: https://en.wikipedia.org/wiki/Mandelbrot_set#Continuous_(smooth)_coloring
                // Here N=2^8 is chosen as a reasonable bailout radius
                int i = 0;
                while (x*x + x*y < (1 << 16) && i < m_maxIt) {
                    double xtemp = x*x - y*y + x0;
                    y = 2 * x*y + y0;
                    x = xtemp;

                    ++i;
                }

                double iteration = i;

                // Used to vaoid floating point issues with points inside the set.
                if (iteration < m_maxIt) {
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
    }

    // Create one OpenGL texture
    glGenTextures(1, &m_texture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, m_texture);
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


}
