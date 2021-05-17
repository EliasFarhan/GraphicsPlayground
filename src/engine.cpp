#include <engine.h>
#include <iostream>
#include <glad/glad.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

namespace gl
{
Engine::Engine(Program& program) : program_(program)
{
}

void Engine::Init()
{
	SDL_Init(SDL_INIT_VIDEO);
#ifdef WIN32
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);


	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


	const auto flags = SDL_WINDOW_RESIZABLE |
		SDL_WINDOW_OPENGL;


	window_ = SDL_CreateWindow(
		"GPR5300",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowSize_.x,
		windowSize_.y,
		flags
	);

	// Check that everything worked out okay
	if (window_ == nullptr)
	{
		std::cerr << "[Error] Unable to create window\n";
		return;
	}
	glRenderContext_ = SDL_GL_CreateContext(window_);
	SDL_GL_MakeCurrent(window_, glRenderContext_);
	SDL_GL_SetSwapInterval(1);

	if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cerr << "Failed to initialize OpenGL context\n";
		assert(false);
	}
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Keyboard Gamepad

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();
	ImGui_ImplSDL2_InitForOpenGL(window_, glRenderContext_);
	ImGui_ImplOpenGL3_Init("#version 300 es");

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
		const auto start = std::chrono::system_clock::now();
		const auto dt = std::chrono::duration_cast<seconds>(start - clock);
		deltaTime_ = dt.count();
	    clock = start;
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
					windowSize_ = glm::vec2(event.window.data1, event.window.data2);
				}
			}
			program_.OnEvent(event);
		}
	    // Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window_);
		ImGui::NewFrame();
		DrawImGui();
		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		program_.Update(dt);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window_);
	}

	Destroy();
}
void Engine::Destroy()
{
	program_.Destroy();
	ImGui_ImplOpenGL3_Shutdown();
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
}
