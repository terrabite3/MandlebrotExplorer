#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

Camera::Camera() :
    m_zoom(1.0),
    m_centerX(0.0),
    m_centerY(0.0)
{


}

Camera::~Camera()
{
}

void Camera::setCenter(double x, double y)
{
    m_centerX = x;
    m_centerY = y;
}

void Camera::setZoom(double z)
{
    m_zoom = z;
}

mat4 Camera::getMvp() const
{
    // By using an identity projection matrix, we use -1,1 coords
    mat4 projection = glm::mat4(1.f);



    // Camera matrix
    mat4 view = glm::mat4(1.f);

    view = scale(view, vec3((float)m_zoom, (float)m_zoom, 0.0f));

    view = translate(view, vec3((float)-m_centerX, (float)-m_centerY, 0.0f));


    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = projection * view * model; // Remember, matrix multiplication is the other way around

    return mvp;
}
