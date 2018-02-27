#pragma once

// Include GLM
#include <glm/glm.hpp>

#include "Tile.h"

class Camera 
{
public:
    Camera();
    virtual ~Camera();

    void setCenter(double x, double y);
    void setZoom(double z);
    void setCutoff(double cutoff);
    void setDimensionsPx(int width, int height);

    glm::mat4 getMvp() const;
    double getCutoff() const;
    Tile::Bounds getBounds() const;
    int getWidthPx() const;
    int getHeightPx() const;

private:
    double m_zoom;
    double m_centerX;
    double m_centerY;
    double m_cutoff;
    int m_width;
    int m_height;
};