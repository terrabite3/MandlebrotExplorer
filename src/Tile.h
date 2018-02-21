#pragma once

typedef unsigned int GLuint;
typedef float GLfloat;

class Tile {
public:
    Tile(double left, double right, double top, double bottom, int maxIt, bool smooth = true);
    virtual ~Tile();

    GLuint getTexture() const;

    // Fill 18 float values, 2 triangles * 3 points * 3 coordinates
    void getVertexData(GLfloat* buffer) const;
    // Fill 12 float values, 2 triangles * 3 points * 2 coordinates
    void getUvData(GLfloat* buffer) const;

private:
    static const int TEXTURE_SIZE = 4096;

    double m_left, m_right, m_top, m_bottom;
    int m_maxIt;
    bool m_smooth;

    mutable GLuint m_texture;

    void render() const;
    void renderCl() const;
};