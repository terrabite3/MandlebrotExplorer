#include "Tile.h"

// Include GLEW
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <iostream>
#include <string>

// For getting the GL context
// What do if not Windows?
#include <windows.h>


void myassert(cl_uint errorCode) {
    if (errorCode != CL_SUCCESS) {
        throw std::runtime_error("Assertion failed: " + std::to_string(errorCode));
    }
}

const static std::string kernelSourceStr = R"(
__kernel void mandelbrotKernel(
    __global const float *bounds,
    __global const int *maxIt,
    __write_only image2d_t output
) {
    int width = get_image_width(output);
    int height = get_image_height(output);
    int2 coord = (int2) (get_global_id(0), get_global_id(1));

    float left = bounds[0];
    float right = bounds[1];
    float top = bounds[2];
    float bottom = bounds[3];
    
    float x0 = left + (coord.x * (right - left)) / width;
    float y0 = top + (coord.y * (bottom - top)) / height;

	float x = 0;
	float y = 0;
	float xSqr;
	float ySqr;
	float xTemp = 0;
	int it = 0;

	while (true) {
		xSqr = x * x;
		ySqr = y * y;
		if (xSqr + ySqr > 4) break;
		if (it >= *maxIt) break;

		xTemp = xSqr - ySqr + x0;
		y = 2 * x * y + y0;
		x = xTemp;

		it++;
	}

	if (it == *maxIt) 
        write_imagef(output, coord, it);
	else 
        write_imagef(output, coord, it - log2(log2(sqrt(xSqr + ySqr))));
}
)";


Tile::Tile(double left, double right, double top, double bottom, int maxIt, bool smooth) :
    m_left(left),
    m_right(right),
    m_top(top),
    m_bottom(bottom),
    m_maxIt(maxIt),
    m_smooth(smooth),
    m_texture(NULL)
{
}

Tile::~Tile()
{
    glDeleteTextures(1, &m_texture);
}

GLuint Tile::getTexture() const
{
    static bool first = true;
    if (m_texture == NULL) {
        if (first) {
            renderCl();
            first = false;
        }
        else {
            render();
        }
    }

    return m_texture;
}

void Tile::getVertexData(GLfloat * buffer) const
{
    // Bottom left
    buffer[0] = (float)m_left;
    buffer[1] = (float)m_bottom;
    buffer[2] = 0.0f;
    // Top left
    buffer[3] = (float)m_left;
    buffer[4] = (float)m_top;
    buffer[5] = 0.0f;
    // Top right
    buffer[6] = (float)m_right;
    buffer[7] = (float)m_top;
    buffer[8] = 0.0f;

    // Bottom left
    buffer[9] = (float)m_left;
    buffer[10] = (float)m_bottom;
    buffer[11] = 0.0f;
    // Top right
    buffer[12] = (float)m_right;
    buffer[13] = (float)m_top;
    buffer[14] = 0.0f;
    // Bottom right
    buffer[15] = (float)m_right;
    buffer[16] = (float)m_bottom;
    buffer[17] = 0.0f;

}

void Tile::getUvData(GLfloat * buffer) const
{
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


void Tile::render() const
{
    // Check if we have already rendered
    if (m_texture != NULL)
        return;


    int width = TEXTURE_SIZE;
    int height = TEXTURE_SIZE;

    unsigned int bufsize = width * height;
    auto buffer = new float[bufsize];

    if (!m_smooth) {
        // Render the fractal
        for (unsigned py = 0; py < height; ++py) {
            for (unsigned px = 0; px < width; ++px) {

                double x0 = m_left + (px * (m_right - m_left)) / width;
                double y0 = m_top + (py * (m_bottom - m_top)) / height;

                double x = 0;
                double y = 0;

                unsigned i;
                for (i = 0; i < m_maxIt; ++i) {

                    double xtemp = x*x - y*y + x0;
                    y = 2 * x * y + y0;
                    x = xtemp;

                    double magnitudeSqr = x * x + y * y;

                    if (magnitudeSqr > 2 * 2) break;
                }

                //buffer[py * width + px] = (i == maxIt) ? (FLT_MAX) : i / 1.f;
                buffer[py * width + px] = i;

            }
        }
    }
    else
    {
        // Render the fractal
        for (unsigned py = 0; py < height; ++py) {
            for (unsigned px = 0; px < width; ++px) {

                double x0 = m_left + (px * (m_right - m_left)) / width;
                double y0 = m_top + (py * (m_bottom - m_top)) / height;

                double x = 0;
                double y = 0;

                // Source: https://en.wikipedia.org/wiki/Mandelbrot_set#Continuous_(smooth)_coloring
                // Here N=2^8 is chosen as a reasonable bailout radius
                int i = 0;
                while (x*x + x*y < (1 << 16) && i < m_maxIt) {
                    double xtemp = x*x - y*y + x0;
                    y = 2 * x*y + y0;
                    x = xtemp;

                    ++i;
                }

                double iteration = i;

                // Used to vaoid floating point issues with points inside the set.
                if (iteration < m_maxIt) {
                    // sqrt of inner term remove using log simplification rules.
                    double log_zn = log(x*x + y*y) / 2;
                    double nu = log(log_zn / log(2)) / log(2);
                    // Rearranging the potential function.
                    // Dividing log_zn by log(2) instead of log(N = 1<<8)
                    // because we want the entire palette to range from the 
                    // center to radius 2, NOT our bailout radius.
                    iteration = iteration + 1 - nu;
                }
                else {
                    // No need to change iteration -> shader will do the actual gating
                    // Plus, anisotropic filtering will work better if it isn't an extreme value
                }

                //buffer[py * width + px] = (i == maxIt) ? (FLT_MAX) : i / 1.f;
                buffer[py * width + px] = (float)iteration;

            }
        }
    }

    // Create one OpenGL texture
    glGenTextures(1, &m_texture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, buffer);

    // OpenGL has now copied the data. Free our own version
    delete[] buffer;


    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // ... which requires mipmaps. Generate them automatically.
    glGenerateMipmap(GL_TEXTURE_2D);


}

void CL_CALLBACK myCallback(const char* errorinfo, const void * private_info_size, size_t cb, void* user_data) {
    std::string message(errorinfo);
    std::cout << message;
}


void Tile::renderCl() const
{
    // Generate the OpenGL texture to be shared with OpenCL
    glGenTextures(1, &m_texture);

    glBindTexture(GL_TEXTURE_2D, m_texture);



    // Generate some test data so we know when CL finishes rendering
    auto buffer = new float[TEXTURE_SIZE * TEXTURE_SIZE];

    for (int py = 0; py < TEXTURE_SIZE; ++py) {
        for (int px = 0; px < TEXTURE_SIZE; ++px) {
            // Make a checkerboard pattern
            if ((px & 0x40) ^ (py & 0x40)) {
                buffer[py * TEXTURE_SIZE + px] = 1.f;
            }
            else {
                buffer[py * TEXTURE_SIZE + px] = 2.f;
            }
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_FLOAT, buffer);
    delete[] buffer;


    // Not sure about this stuff
    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // This can't be GL_NEAREST_MIPMAP_*, but what about GL_LINEAR?
    // ... which requires mipmaps. Generate them automatically.
    //glGenerateMipmap(GL_TEXTURE_2D);


    // Unbind the texture? Is this needed?
    glBindTexture(GL_TEXTURE_2D, 0);
    // Not sure what this would do here, but it's in the example
    glFinish();





    cl_int result;

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    auto platform = platforms[0];

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

    // Create a context
    // What do if not Windows?
    HGLRC hGLRC = wglGetCurrentContext();
    HDC hDC = wglGetCurrentDC();

    cl_context_properties properties[] =
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
        CL_GL_CONTEXT_KHR,   (cl_context_properties)hGLRC,
        CL_WGL_HDC_KHR,      (cl_context_properties)hDC,
        0
    };

    cl::Context context(devices[0], properties, &myCallback);

    cl::CommandQueue commandQueue(context, devices[0], 0, &result);
    myassert(result);

    cl::Program module(context, kernelSourceStr, true, &result);
    myassert(result);

    // Create the kernel
    cl::Kernel mandelbrotKernel(module, "mandelbrotKernel", &result);
    myassert(result);


    float bounds[] = {
        (float)m_left,
        (float)m_right,
        (float)m_top,
        (float)m_bottom,
    };

    int maxIt = m_maxIt;

    cl::Buffer boundsBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(bounds), bounds, &result);
    myassert(result);
    cl::Buffer maxItBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(maxIt), &maxIt, &result);
    myassert(result);


    result = mandelbrotKernel.setArg(0, boundsBuffer);
    myassert(result);
    
    result = mandelbrotKernel.setArg(1, maxItBuffer);
    myassert(result);


    cl::ImageGL textureAsClMem(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, m_texture, &result);
    myassert(result);

    std::vector<cl::Memory> textureVector{ textureAsClMem };
    result = commandQueue.enqueueAcquireGLObjects(&textureVector);
    myassert(result);

    result = mandelbrotKernel.setArg(2, textureAsClMem);
    myassert(result);


    result = commandQueue.enqueueNDRangeKernel(mandelbrotKernel,
        cl::NullRange,                              // offset
        cl::NDRange(TEXTURE_SIZE, TEXTURE_SIZE)     // global
    );  // local left up to the driver
    myassert(result);

    result = commandQueue.enqueueReleaseGLObjects(&textureVector);
    myassert(result);

    result = commandQueue.finish();
    myassert(result);

}
