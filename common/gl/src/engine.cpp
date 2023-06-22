#include <gl/engine.h>
#include <GL/glew.h>
#include <gl/error.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "log.h"

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#include "TracyOpenGL.hpp"
#endif

namespace gl
{

Engine* Engine::instance_ = nullptr;

Engine::Engine(core::Program& program) : program_(program), window_(nullptr),
                                         glRenderContext_(nullptr)
{
    instance_ = this;
}

void Engine::Init()
{
#ifdef TRACY_ENABLE
    ZoneScopedN("Engine Init");
#endif

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);


    // Turn on double buffering with a 24bit Z buffer.
    // You may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


    const auto flags = SDL_WINDOW_RESIZABLE |
                       SDL_WINDOW_OPENGL;


    window_ = SDL_CreateWindow(
            "GL Playground",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            static_cast<int>(windowSize_.x),
            static_cast<int>(windowSize_.y),
            flags
    );

    // Check that everything worked out okay
    if (window_ == nullptr)
    {
        core::LogError("Unable to create window");
        std::terminate();
    }
    glRenderContext_ = SDL_GL_CreateContext(window_);

    SDL_GL_MakeCurrent(window_, glRenderContext_);

    SDL_GL_SetSwapInterval(1);
    if (const auto errorCode = glewInit(); GLEW_OK != errorCode)
    {
        core::LogError("Failed to initialize GLEW");
        std::terminate();
    }
    glCheckError();
#ifdef TRACY_ENABLE
    TracyGpuContext
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Keyboard Gamepad

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();
    ImGui_ImplSDL2_InitForOpenGL(window_, glRenderContext_);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    glCheckError();

    //stbi_set_flip_vertically_on_load(true);
    program_.Init();
}


void Engine::Run()
{
    Init();
    bool isOpen = true;
    std::chrono::time_point<std::chrono::system_clock> clock =
            std::chrono::system_clock::now();
    while (isOpen)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(engineLoop, "Engine Loop", true);
        TracyGpuNamedZone(gpuEngineLoop, "Engine Loop", true);
#endif
        const auto start = std::chrono::system_clock::now();
        const auto dt = std::chrono::duration_cast<core::seconds>(
                start - clock);
        deltaTime_ = dt.count();
        clock = start;
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(eventManagement, "Event Management", true);
            TracyGpuNamedZone(eventManagementGpu, "Event Management", true);
#endif
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                {
                    isOpen = false;
                }

                if (event.type == SDL_WINDOWEVENT)
                {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        windowSize_ = glm::vec2(event.window.data1,
                                                event.window.data2);
                    }
                }
                program_.OnEvent(event);
            }
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window_);
        ImGui::NewFrame();
        DrawImGui();
        ImGui::Render();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();
        program_.Update(dt);
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(imguiRender, "ImGui Render Data", true);
            TracyGpuNamedZone(gpuImguiRender, "ImGui Render Data", true);
#endif
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glCheckError();
        }
#ifdef TRACY_ENABLE
        ZoneNamedN(swapWindow, "Swap Window", true);
        TracyGpuNamedZone(gpuSwapWindow, "Swap Window", true);
#endif
        SDL_GL_SwapWindow(window_);
#ifdef TRACY_ENABLE
        TracyGpuCollect
        FrameMark
#endif
        glCheckError();

    }

    Destroy();
}

void Engine::Destroy()
{
    program_.Destroy();
    ImGui_ImplOpenGL3_Shutdown();
    glCheckError();
    // Delete our OpengL context
    SDL_GL_DeleteContext(glRenderContext_);
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    // Destroy our window
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Engine::DrawImGui()
{
    ImGui::Begin("Engine");
    ImGui::Text("FPS: %f", 1.0f / deltaTime_);
    ImGui::End();
    program_.DrawImGui();
}

std::array<int, 2> Engine::GetWindowSize() const
{
    std::array<int, 2> size{};
    SDL_GetWindowSize(window_, &size[0], &size[1]);
    return size;
}
}
