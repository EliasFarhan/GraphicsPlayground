#include <SDL_main.h>

#include "vk/engine.h"
#include "hello_triangle.h"
#include "hello_input_buffer.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    core::Filesystem filesystem;
    vk::HelloUniform program_;
    vk::Engine engine(program_);
    engine.Run();
    return EXIT_SUCCESS;
}