#include <SDL_main.h>

#include "vk/engine.h"
#include "hello_triangle.h"

int main(int argc, char** argv)
{
    core::Filesystem filesystem;
    vk::HelloTriangle program_;
    vk::Engine engine(program_);
    engine.Run();
    return EXIT_SUCCESS;
}