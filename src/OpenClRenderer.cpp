#include "OpenClRenderer.h"

#include "Tile.h"

#include <GL/glew.h>


// For getting the GL context
// What do if not Windows?
#include <windows.h>

#include <iostream>

const static std::string kernelSourceStr = R"(
__kernel void mandelbrotKernel(
    __global const float *bounds,
    //__global const int *maxIt,
    __write_only image2d_t output
) {
    int width = get_image_width(output);
    int height = get_image_height(output);
    int2 coord = (int2) (get_global_id(0), get_global_id(1));

    float left = bounds[0];
    float right = bounds[1];
    float top = bounds[2];
    float bottom = bounds[3];
    int maxIt = (int)bounds[4];
    
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
		if (it >= maxIt) break;

		xTemp = xSqr - ySqr + x0;
		y = 2 * x * y + y0;
		x = xTemp;

		it++;
	}

	if (it == maxIt) 
        write_imagef(output, coord, it);
	else 
        write_imagef(output, coord, it - log2(log2(sqrt(xSqr + ySqr))));
}
)";

void myassert(cl_uint errorCode) {
    if (errorCode != CL_SUCCESS) {
        throw std::runtime_error("Assertion failed: " + std::to_string(errorCode));
    }
}

void CL_CALLBACK myCallback(const char* errorinfo, const void * private_info_size, size_t cb, void* user_data) {
    std::string message(errorinfo);
    std::cout << message;
}

OpenClRenderer::OpenClRenderer()
{

    cl_int result;

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    m_platform = platforms[0];


    // Create a context
    // What do if not Windows?
    HGLRC hGLRC = wglGetCurrentContext();
    HDC hDC = wglGetCurrentDC();

    cl_context_properties properties[] =
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties)m_platform(),
        CL_GL_CONTEXT_KHR,   (cl_context_properties)hGLRC,
        CL_WGL_HDC_KHR,      (cl_context_properties)hDC,
        0
    };

    std::vector<cl::Device> devices;
    m_platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

    m_context = cl::Context(devices[0], properties, &myCallback);

    m_queue = cl::CommandQueue(m_context, devices[0], 0, &result);
    myassert(result);

    m_program = cl::Program(m_context, kernelSourceStr, true, &result);
    //myassert(result);

    if (result != CL_SUCCESS) {
        char buffer[10240];
        clGetProgramBuildInfo(m_program(), devices[0](), CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        std::cout << buffer;
    }
}

void OpenClRenderer::render(const Tile & tile)
{
    cl_int result;

    // Create the kernel
    cl::Kernel mandelbrotKernel(m_program, "mandelbrotKernel", &result);
    myassert(result);

    cl::Buffer boundsBuffer(m_context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(Tile::Bounds),
        (void*)(tile.getBounds()),
        &result
    );

    result = mandelbrotKernel.setArg(0, boundsBuffer);

    cl::ImageGL textureAsClMem(m_context,
        CL_MEM_WRITE_ONLY,
        GL_TEXTURE_2D,
        0,
        tile.getTexture(),
        &result
    );
    myassert(result);

    result = mandelbrotKernel.setArg(1, textureAsClMem);

    std::vector<cl::Memory> textureVector{ textureAsClMem };
    result = m_queue.enqueueAcquireGLObjects(&textureVector);
    myassert(result);

    result = mandelbrotKernel.setArg(1, textureAsClMem);
    myassert(result);


    result = m_queue.enqueueNDRangeKernel(mandelbrotKernel,
        cl::NullRange,                              // offset
        cl::NDRange(tile.getTextureSize(), tile.getTextureSize())     // global
    );  // local left up to the driver
    myassert(result);

    result = m_queue.enqueueReleaseGLObjects(&textureVector);
    myassert(result);

    result = m_queue.finish();
    myassert(result);

}
