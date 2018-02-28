#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <utility>

class Tile;

class OpenClRenderer {
public:
    OpenClRenderer();

    void render(Tile* tile);

    void checkPendingRenders();

private:
    cl::Platform m_platform;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;

    std::vector<std::pair<Tile*, cl::Event>> m_pendingRenders;
};