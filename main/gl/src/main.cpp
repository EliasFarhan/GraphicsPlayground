//
// Created by efarhan on 5/27/21.
//

#include "sample_browser.h"
#include "filesystem.h"
#include "gl/engine.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    core::Filesystem filesystem;
    gl::SampleBrowser program;
    gl::Engine engine(program);
    engine.Run();
    return EXIT_SUCCESS;
}
