#include <SDL_main.h>

#include "vk/engine.h"
#include "hello_triangle.h"
#include "hello_input_buffer.h"

int main(int argc, char** argv)
{
    core::Filesystem filesystem;
    vk::HelloIndexBuffer program_;
    vk::Engine engine(program_);
    engine.Run();
    return EXIT_SUCCESS;
}