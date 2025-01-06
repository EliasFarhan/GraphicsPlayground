#include <GL/glew.h>
#include <hello_pbr.h>
#include <imgui.h>
#include <fmt/core.h>

#include <algorithm>

namespace gl
{
void HelloPbr::Init()
{
	sphere_.Init();
	pbrShader_.CreateDefaultProgram("data/shaders/23_hello_pbr/pbr.vert", "data/shaders/23_hello_pbr/pbr.frag");
	camera_.Init();
	camera_.position = glm::vec3(0,0 ,30.0f);
	camera_.LookAt(glm::vec3());

	lights_ = {
		{
		{glm::vec3(-10.0f,  10.0f, 10.0f), glm::vec3(300.0f, 300.0f, 300.0f)},
		{glm::vec3(10.0f,  10.0f, 10.0f),glm::vec3(300.0f, 300.0f, 300.0f)},
		{glm::vec3(-10.0f, -10.0f, 10.0f),glm::vec3(300.0f, 300.0f, 300.0f)},
		{glm::vec3(10.0f, -10.0f, 10.0f),glm::vec3(300.0f, 300.0f, 300.0f)},
		}
	};
	glEnable(GL_DEPTH_TEST);
}

void HelloPbr::Update(core::seconds dt)
{
	camera_.Update(dt);

	constexpr int nrRows = 7;
	constexpr int nrColumns = 7;
	pbrShader_.Bind();
	pbrShader_.SetInt("gammaCorrect", gammaCorrect_);
	pbrShader_.SetFloat("ao", 1.0f);
	pbrShader_.SetVec3("albedo", baseColor_);
	pbrShader_.SetMat4("view", camera_.GetView());
	pbrShader_.SetVec3("viewPos", camera_.position);
	pbrShader_.SetMat4("projection", camera_.GetProjection());
	for (size_t i = 0; i < lights_.size(); i++)
	{
		pbrShader_.SetVec3(fmt::format("lights[{}].position", i), lights_[i].position);
		pbrShader_.SetVec3(fmt::format("lights[{}].color", i), lights_[i].color);

	}
	for (int row = 0; row < nrRows; ++row)
	{
		pbrShader_.SetFloat("metallic", static_cast<float>(row) / static_cast<float>(nrRows - 1));
		for (int col = 0; col < nrColumns; ++col)
		{
			// we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
			// on direct lighting.
			pbrShader_.SetFloat("roughness", std::clamp(static_cast<float>(col) / static_cast<float>(nrColumns - 1), 0.025f, 1.0f));

			auto model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(
				static_cast<float>(col - nrColumns / 2) * spacing_,
				static_cast<float>(row - nrRows / 2) * spacing_,
				0.0f
			));
			pbrShader_.SetMat4("model", model);
			pbrShader_.SetMat4("normalMatrix", glm::transpose(glm::inverse(model)));
			sphere_.Draw();
		}
	}
}

void HelloPbr::Destroy()
{
	glDisable(GL_DEPTH_TEST);
	sphere_.Destroy();
	pbrShader_.Destroy();
}

void HelloPbr::OnEvent(SDL_Event& event)
{
	camera_.OnEvent(event);
}

void HelloPbr::DrawImGui()
{
	ImGui::Begin("Pbr Program");
	ImGui::ColorPicker3("Base Color", &baseColor_[0]);
	ImGui::Checkbox("Gamma Correct", &gammaCorrect_);
	ImGui::End();
}
}
