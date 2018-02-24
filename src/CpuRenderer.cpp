#include "CpuRenderer.h"

#include <cmath>

#include <GL/glew.h>

void CpuRenderer::render(Tile & tile)
{
    int width = tile.getTextureSize();
    int height = tile.getTextureSize();

    auto buffer = new float[width * height];

    Tile::Bounds bounds = *tile.getBounds();
    int maxIt = bounds.maxIt;
    
    // Render the fractal
    for (unsigned py = 0; py < height; ++py) {
        for (unsigned px = 0; px < width; ++px) {

            double x0 = bounds.left + (px * (bounds.right - bounds.left)) / width;
            double y0 = bounds.top + (py * (bounds.bottom - bounds.top)) / height;

            double x = 0;
            double y = 0;

            // Source: https://en.wikipedia.org/wiki/Mandelbrot_set#Continuous_(smooth)_coloring
            // Here N=2^8 is chosen as a reasonable bailout radius
            int i = 0;
            while (x*x + x*y < (1 << 16) && i < maxIt) {
                double xtemp = x*x - y*y + x0;
                y = 2 * x*y + y0;
                x = xtemp;

                ++i;
            }

            double iteration = i;

            // Used to vaoid floating point issues with points inside the set.
            if (iteration < maxIt) {
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

    glBindTexture(GL_TEXTURE_2D, tile.getTexture());

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, buffer);
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
}
