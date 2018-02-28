#include "Tile.h"

#include <GL/glew.h>

#include <cstring>
#include <assert.h>


Tile::Tile(Bounds bounds, int generation):
    m_state(State::INIT),
    m_bounds(bounds),
    m_generation(generation),
    m_texture(NULL),
    m_cachedTexture(nullptr)
{

}

Tile::Tile(double left, double right, double top, double bottom, int maxIt, int generation) :
    m_state(State::INIT),
    m_bounds{ (float)left, (float)right, (float)top, (float)bottom, (float)maxIt },
    m_generation(generation),
    m_texture(NULL),
    m_cachedTexture(nullptr)
{

}

Tile::~Tile()
{
    glDeleteTextures(1, &m_texture);
}

Tile::State Tile::getState() const
{
    return m_state;
}

void Tile::createTexture()
{
    assert(m_state == State::INIT);
    assert(m_texture == NULL);

    auto buffer = new float[TEXTURE_SIZE * TEXTURE_SIZE];
    // IEEE says the pattern for 0.0f is all zeros
    memset(buffer, 0, TEXTURE_SIZE * TEXTURE_SIZE * sizeof(float));

    createTexture(buffer);

    delete[] buffer;
    m_state = State::EMPTY;
}

//void Tile::reloadTexture()
//{
//    assert(m_state == State::UNLOADED);
//    assert(m_texture == NULL);
//    assert(m_cachedTexture != nullptr);
//
//    createTexture(m_cachedTexture);
//
//    m_state = State::ACTIVE;
//}

//void Tile::unloadTexture()
//{
//    assert(m_state == State::ACTIVE || m_state == State::SPLIT);
//
//    // TODO read back the buffer data
//    
//
//
//    //m_state = State::UNLOADED;
//}

void Tile::setRendering()
{
    assert(m_state == State::EMPTY);
    m_state = State::RENDERING;
}

void Tile::setRendered()
{
    assert(m_state == State::RENDERING);
    m_state = State::ACTIVE;
}

Tile::Bounds Tile::getBounds() const
{
    return m_bounds;
}

std::vector<Tile*> Tile::split()
{
    assert(m_state == State::ACTIVE);
    assert(m_children.empty());

    //std::vector<Tile*> newTiles;

    float centerX = (m_bounds.left + m_bounds.right) / 2;
    float centerY = (m_bounds.top + m_bounds.bottom) / 2;

    m_children.emplace_back(new Tile(m_bounds.left, centerX, m_bounds.top, centerY, m_bounds.maxIt, m_generation + 1));
    m_children.emplace_back(new Tile(centerX, m_bounds.right, m_bounds.top, centerY, m_bounds.maxIt, m_generation + 1));
    m_children.emplace_back(new Tile(m_bounds.left, centerX, centerY, m_bounds.bottom, m_bounds.maxIt, m_generation + 1));
    m_children.emplace_back(new Tile(centerX, m_bounds.right, centerY, m_bounds.bottom, m_bounds.maxIt, m_generation + 1));

    m_state = State::SPLIT;

    return m_children;
}

bool Tile::childrenAreRendered() const
{
    if (m_state != State::SPLIT)
        return false;

    for (auto tile : m_children) {
        if (tile->getState() < State::ACTIVE) return false;
    }
    
    return true;
}

GLuint Tile::getTexture() const
{
    assert(m_state >= State::INIT && m_state <= State::SPLIT);

    return m_texture;
}

int Tile::getTextureSize() const
{
    return TEXTURE_SIZE;
}

void Tile::getVertexData(GLfloat * buffer) const
{
    assert(m_state >= State::INIT && m_state <= State::SPLIT);

    // Bottom left
    buffer[0] = (float)m_bounds.left;
    buffer[1] = (float)m_bounds.bottom;
    buffer[2] = m_generation / 10.f;
    // Top left
    buffer[3] = (float)m_bounds.left;
    buffer[4] = (float)m_bounds.top;
    buffer[5] = m_generation / 10.f;
    // Top right
    buffer[6] = (float)m_bounds.right;
    buffer[7] = (float)m_bounds.top;
    buffer[8] = m_generation / 10.f;

    // Bottom left
    buffer[9] = (float)m_bounds.left;
    buffer[10] = (float)m_bounds.bottom;
    buffer[11] = m_generation / 10.f;
    // Top right
    buffer[12] = (float)m_bounds.right;
    buffer[13] = (float)m_bounds.top;
    buffer[14] = m_generation / 10.f;
    // Bottom right
    buffer[15] = (float)m_bounds.right;
    buffer[16] = (float)m_bounds.bottom;
    buffer[17] = m_generation / 10.f;

}

void Tile::getUvData(GLfloat * buffer) const
{
    assert(m_state >= State::INIT && m_state <= State::SPLIT);

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

void Tile::createTexture(float * buffer)
{
    assert(m_state == State::INIT /*|| m_state == State::UNLOADED*/);
    assert(m_texture == NULL);

    glGenTextures(1, &m_texture);

    // Fill the texture with 0

    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_FLOAT, buffer);

    // Not sure about this stuff
    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // This can't be GL_NEAREST_MIPMAP_*, but what about GL_LINEAR?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // ... which requires mipmaps. Generate them automatically.
    //glGenerateMipmap(GL_TEXTURE_2D);


    // Unbind the texture? Is this needed?
    glBindTexture(GL_TEXTURE_2D, 0);

    // Not sure what this would do here, but it's in the example
    glFinish();
}

bool inside(const Tile::Bounds & tile, const Tile::Bounds & view)
{
    auto inRangeLambda = [](float viewMin, float viewMax, float tileMin, float tileMax) {
        assert(viewMin < viewMax);
        assert(tileMin < tileMax);

        // The vertical bars represent view
        // The horizontal lines represent tile
        //  --- |     |         A: tile is entirely below view
        //    --+--   |         B: tile starts below view and ends inside view
        //    --+-----+--       C: tile starts below view and ends above view
        //      | --- |         D: tile starts and ends inside view
        //      |   --+--       E: tile starts inside view and ends above view
        //      |     | ---     F: tile is entirely above view

        // A:
        if (tileMax < viewMin) return false;
        // F:
        if (tileMin > viewMax) return false;
        // B-E:
        return true;
    };

    bool insideX = inRangeLambda(view.left, view.right, tile.left, tile.right);
    bool insideY = inRangeLambda(view.top, view.bottom, tile.top, tile.bottom);

    return insideX && insideY;
}
