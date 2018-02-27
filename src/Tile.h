#pragma once

typedef unsigned int GLuint;
typedef float GLfloat;

#include <vector>

class Tile {
public:

    struct Bounds {
        float left;
        float right;
        float top;
        float bottom;
        float maxIt;
    };

    explicit Tile(Bounds bounds, int generation = 0);
    Tile(double left, double right, double top, double bottom, int maxIt, int generation = 0);
    virtual ~Tile();

    Bounds getBounds() const;

    std::vector<Tile*> split();
    bool isSplit() const;

    GLuint getTexture() const;

    int getTextureSize() const;

    // Fill 18 float values, 2 triangles * 3 points * 3 coordinates
    void getVertexData(GLfloat* buffer) const;
    // Fill 12 float values, 2 triangles * 3 points * 2 coordinates
    void getUvData(GLfloat* buffer) const;

private:
    static const int TEXTURE_SIZE = 4096;

    Bounds m_bounds;
    int m_generation;
    bool m_split;
    mutable GLuint m_texture;

    GLuint createTexture();

};

bool inside(const Tile::Bounds& view, const Tile::Bounds& tile);