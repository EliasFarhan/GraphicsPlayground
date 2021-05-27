//
// Created by efarhan on 5/27/21.
//

#include "sample_browser.h"
#include "filesystem.h"
#include "gl/engine.h"

int main(int argc, char** argv)
{
    core::Filesystem filesystem;
    gl::SampleBrowser program;
    gl::Engine engine(program);
    engine.Run();
    return EXIT_SUCCESS;
}
