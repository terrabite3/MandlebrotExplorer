#pragma once

// Include GLM
#include <glm/glm.hpp>

class Camera 
{
public:
    Camera();
    virtual ~Camera();

    void setCenter(double x, double y);
    void setZoom(double z);

    glm::mat4 getMvp() const;


private:
    double m_zoom;
    double m_centerX;
    double m_centerY;


};