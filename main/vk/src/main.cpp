#include <SDL_main.h>

#include "vk/engine.h"
#include <filesystem.h>
#include "hello_texture.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    core::Filesystem filesystem;
    vk::HelloTexture program_;
    vk::Engine engine(program_);
    engine.Run();
    return EXIT_SUCCESS;
}