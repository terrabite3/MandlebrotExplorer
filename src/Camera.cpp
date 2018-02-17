#include "Camera.h"

Camera::Camera() :
    m_zoom(1.0),
    m_centerX(0.0),
    m_centerY(0.0)
{


}

Camera::~Camera()
{
}

glm::mat4 Camera::getMvp() const
{
    // By using an identity projection matrix, we use -1,1 coords
    glm::mat4 m_projection = glm::mat4(1.f);



    // Camera matrix
    // Scale by 0.9 so we can see the edge of the texture
    //glm::mat4 View = glm::scale(glm::mat4(1.f), glm::vec3(0.9f, 0.9f, 1.f));
    glm::mat4 m_view = glm::mat4(1.f);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 m_model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 m_mvp = m_projection * m_view * m_model; // Remember, matrix multiplication is the other way around

    return m_mvp;
}
