#include <SDL_main.h>

#include "vk/engine.h"
#include <filesystem.h>
#include "hello_triangle.h"
#include "hello_input_buffer.h"
#include "hello_staging_buffer.h"
#include "hello_index_buffer.h"
#include "hello_uniform.h"
#include "hello_texture.h"
#include "hello_cube.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    core::Filesystem filesystem;
    vk::HelloUniform program_;
    vk::Engine engine(program_);
    engine.Run();
    return EXIT_SUCCESS;
}