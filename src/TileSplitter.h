#pragma once

#include "Tile.h"
#include "OpenClRenderer.h"
#include "Camera.h"

#include <vector>

class TileSplitter
{
public:
    TileSplitter(const Camera& camera, Tile::Bounds initialTile);

    std::vector<Tile*> getTiles() const;

    void splitAsNeeded();

private:
    const Camera& m_camera;
    std::vector<Tile*> m_tiles;
    OpenClRenderer m_renderer;
};