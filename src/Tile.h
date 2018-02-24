#pragma once

typedef unsigned int GLuint;
typedef float GLfloat;


class Tile {
public:

    struct Bounds {
        float left;
        float right;
        float top;
        float bottom;
        float maxIt;
    };


    Tile(double left, double right, double top, double bottom, int maxIt);
    virtual ~Tile();

    const Bounds* getBounds() const;

    GLuint getTexture() const;

    int getTextureSize() const;

    // Fill 18 float values, 2 triangles * 3 points * 3 coordinates
    void getVertexData(GLfloat* buffer) const;
    // Fill 12 float values, 2 triangles * 3 points * 2 coordinates
    void getUvData(GLfloat* buffer) const;

private:
    static const int TEXTURE_SIZE = 4096;

    Bounds m_bounds;

    mutable GLuint m_texture;

};