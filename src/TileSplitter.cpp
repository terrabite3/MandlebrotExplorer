#include "TileSplitter.h"

#include <iostream>

TileSplitter::TileSplitter(const Camera& camera, Tile::Bounds initialTile) :
    m_camera(camera)
{
    m_tiles.emplace_back(new Tile(initialTile));
    m_renderer.render(*m_tiles[0]);
}

std::vector<Tile*> TileSplitter::getTiles() const
{
    return m_tiles;
}

void TileSplitter::splitAsNeeded()
{
    // Remove any tiles with all children done rendering
    // TODO

    const auto viewBounds = m_camera.getBounds();

    // Split any tiles that are too close to pixelated
    std::vector<Tile*> newTiles;

    for (auto tile : m_tiles) {

        const auto tileBounds = tile->getBounds();

        bool tileInside = inside(tileBounds, viewBounds);
        float pixelSize = (tileBounds.right - tileBounds.left) / 
            (viewBounds.right - viewBounds.left) * 
            m_camera.getWidthPx() / tile->getTextureSize();


        if (!tile->isSplit() && tileInside && pixelSize > 0.5) {
            std::cout << "Splitting\n";

            auto splitTiles = tile->split();
            for (auto splitTile : splitTiles) {
                newTiles.emplace_back(splitTile);
            }
        }
    }

    //for (auto it = m_tiles.begin(); it < m_tiles.end; ++it) {
    for (auto it = std::begin(m_tiles); it != std::end(m_tiles); /*Nothing*/)
    {
        auto tile = *it;

        if (tile->isSplit()) {
            it = m_tiles.erase(it);
            delete tile;
        }
        else {
            ++it;
        }
    }

    for (auto newTile : newTiles) {
        m_renderer.render(*newTile);
        m_tiles.emplace_back(newTile);
    }
}
