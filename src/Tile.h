#pragma once

typedef unsigned int GLuint;
typedef float GLfloat;

#include <vector>

class Tile {
public:
    enum class State {
        INIT,       // The Tile object has been created, nothing more
        EMPTY,      // The texture has been allocated on the GPU
        RENDERING,  // OpenCL is rendering the texture
        ACTIVE,     // The texture is being shown, but is not necessarily in view
        SPLIT,      // The texture has been split into four smaller textures, but is still active
                    // After the children are done rendering, it can be unloaded
        //UNLOADED,   // The texture has been removed from the GPU and cached in the heap
    };

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

    State getState() const;
    
    // Allocates an empty texture on the GPU
    // INIT -> EMPTY
    void createTexture();

    //// Allocates the texture on the GPU and loads the cached data
    //// UNLOADED -> ACTIVE
    //void reloadTexture();

    //// Caches the texture data on the heap and deallocates the texture on the GPU
    //void unloadTexture();

    void setRendering();
    void setRendered();

    Bounds getBounds() const;

    // Splits the tile into four new tiles
    // ACTIVE -> SPLIT
    std::vector<Tile*> split();
    
    bool childrenAreRendered() const;

    GLuint getTexture() const;

    int getTextureSize() const;

    // Fill 18 float values, 2 triangles * 3 points * 3 coordinates
    void getVertexData(GLfloat* buffer) const;
    // Fill 12 float values, 2 triangles * 3 points * 2 coordinates
    void getUvData(GLfloat* buffer) const;

private:
    static const int TEXTURE_SIZE = 4096;

    State m_state;
    Bounds m_bounds;
    int m_generation;
    mutable GLuint m_texture;
    float* m_cachedTexture;
    std::vector<Tile*> m_children;

    void createTexture(float* buffer);

};

bool inside(const Tile::Bounds& view, const Tile::Bounds& tile);