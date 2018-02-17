#pragma once

// Include GLM
#include <glm/glm.hpp>

class Camera 
{
public:
    Camera();
    virtual ~Camera();

    glm::mat4 getMvp() const;


private:
    double m_zoom;
    double m_centerX;
    double m_centerY;


};