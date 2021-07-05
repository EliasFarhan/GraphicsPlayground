#pragma once
#include "engine.h"
#include <glm/vec3.hpp>
#include <gl/texture.h>
#include <gl/vertex_array.h>
#include <gl/shader.h>
#include <gl/framebuffer.h>
#include <gl/camera.h>

namespace gl
{

class HelloIbl : public core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:
	enum IblFlags : std::uint8_t
	{
		NONE = 0u,
		FIRST_FRAME = 1u,
		SHOW_IRRADIANCE = 1u << 1u,
		ENABLE_IRRADIANCE = 1u << 2u,
		SHOW_PREFILTER = 1u << 3u,
		ENABLE_IBL_SPECULAR = 1u << 4u,
		ENABLE_SCHLICK_ROUGHNESS = 1u << 5u,

	};
	struct Light
	{
		glm::vec3 position;
		glm::vec3 color;
	};
	void GenerateCubemap();
	void GenerateDiffuseIrradiance();
	void GeneratePrefilter();
	void GenerateLUT();

	std::array<Light, 4> lights_;
	Texture hdrTexture_;
	Sphere sphere_{ 1.0f, glm::vec3() };
	Cuboid skybox_{ glm::vec3(2.0f), glm::vec3() };
	Quad quad_{ glm::vec2(2.0f), glm::vec2() };

	ShaderProgram equiToCubemap_;
	ShaderProgram skyboxShader_;
	ShaderProgram pbrShader_;
	ShaderProgram irradianceShader_;
	ShaderProgram prefilterShader_;
	ShaderProgram brdfShader_;

	sdl::Camera3D camera_;
	glm::vec3 baseColor_ = { 1.0f,0.5f,0.5f };
	float spacing_ = 2.5f;
	std::uint8_t flags_ = NONE;

	const std::array<glm::vec3, 6> viewDirs =
	{
		{

		glm::vec3(1.0f,  0.0f,  0.0f),
		glm::vec3(-1.0f,  0.0f,  0.0f),
		glm::vec3(0.0f,  1.0f,  0.0f),
		glm::vec3(0.0f, -1.0f,  0.0f),
		glm::vec3(0.0f,  0.0f,  1.0f),
		glm::vec3(0.0f,  0.0f, -1.0f),
		}
	};
	const std::array<glm::vec3, 6> upDirs =
	{
		{
		glm::vec3(0.0f, 1.0f,  0.0f),
		glm::vec3(0.0f, 1.0f,  0.0f),
		glm::vec3(0.0f,  0.0f,  -1.0f),
		glm::vec3(0.0f,  0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f,  0.0f),
		glm::vec3(0.0f, 1.0f,  0.0f),
		}
	};

	Framebuffer captureFramebuffer_;
	Texture envCubemap_;
	Texture irradianceMap_;
	Texture brdfLUTTexture_;
	Texture prefilterMap_;
};

}
