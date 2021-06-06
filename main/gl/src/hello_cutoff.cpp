#include <hello_cutoff.h>
#include <imgui.h>

namespace gl
{
void HelloCutoff::Init()
{
    plane_.Init();
    cube_.Init();
    cutoffProgram_.CreateDefaultProgram(
        "data/shaders/08_hello_cutoff/cutoff.vert", 
        "data/shaders/08_hello_cutoff/cutoff.frag");
    grassTexture_.LoadTexture("data/textures/grass.png");
    cubeTexture_.LoadTexture("data/textures/container.jpg");
    whiteTexture_.CreateWhiteTexture();
    camera_.Init();
	glEnable(GL_DEPTH_TEST);
}

void HelloCutoff::Update(core::seconds dt)
{
    camera_.Update(dt);

    cutoffProgram_.Bind();
	cutoffProgram_.SetInt("enableCutoff", enableCutoff_);
	cutoffProgram_.SetMat4("view", camera_.GetView());
	cutoffProgram_.SetMat4("projection", camera_.GetProjection());

	//Draw grass
	cutoffProgram_.SetTexture("texture1", grassTexture_, 0);
	constexpr std::array vegetationPositions
	{
		glm::vec3(-1.5f, 0.0f, -0.48f),
		glm::vec3(1.5f, 0.0f, 0.51f),
		glm::vec3(0.0f, 0.0f, 0.7f),
		glm::vec3(-0.3f, 0.0f, -2.3f),
		glm::vec3(0.5f, 0.0f, -0.6f)
	};
	for (const auto& position : vegetationPositions)
	{
		auto model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1, -1, 1));
		model = glm::translate(model, position);

		cutoffProgram_.SetMat4("model", model);
		plane_.Draw();
	}
	//Draw cube
	cutoffProgram_.SetTexture("texture1", cubeTexture_, 0);

	auto model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
	cutoffProgram_.SetMat4("model", model);

	cube_.Draw();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
	cutoffProgram_.SetMat4("model", model);
	cube_.Draw();

	//Draw floor

	cutoffProgram_.SetTexture("texture1", whiteTexture_, 0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, -0.5f, 0));
	model = glm::scale(model, glm::vec3(5.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	cutoffProgram_.SetMat4("model", model);
	plane_.Draw();
}

void HelloCutoff::Destroy()
{

	glDisable(GL_DEPTH_TEST);
    plane_.Destroy();
    cube_.Destroy();
    cutoffProgram_.Destroy();
	cubeTexture_.Destroy();
    whiteTexture_.Destroy();
	grassTexture_.Destroy();
}

void HelloCutoff::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloCutoff::DrawImGui()
{
    ImGui::Begin("Cutoff");
    ImGui::Checkbox("Enable Cutoff", &enableCutoff_);
    ImGui::End();
}
}
