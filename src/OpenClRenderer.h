#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

class Tile;

class OpenClRenderer {
public:
    OpenClRenderer();

    void render(const Tile& tile);

private:
    cl::Platform m_platform;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
};