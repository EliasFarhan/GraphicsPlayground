#pragma once

#include "engine.h"
#include "glm/vec3.hpp"
#include "gl/vertex_array.h"
#include "gl/shader.h"
#include "gl/camera.h"
#include "gl/framebuffer.h"

namespace gl
{
class HelloBloom : public core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:
	struct Light
	{
		glm::vec3 position_;
		glm::vec3 color_;
	};
	struct Transform
	{
		glm::vec3 position = glm::vec3();
		glm::vec3 scale = glm::vec3(1.0f);
		float angle = 0.0f;
		glm::vec3 axis = glm::vec3(0,1,0);
	};
	Framebuffer hdrFramebuffer_;
	std::array<Framebuffer, 2> pingpongFramebuffers_;
    bool enableBloom_ = false;
	ShaderProgram cubeShader_;
	ShaderProgram lightShader_;
	ShaderProgram blurShader_;
	ShaderProgram bloomShader_;

	Cuboid cube_{ glm::vec3(1.0f), glm::vec3() };
	Texture cubeTexture_;
	const std::array<Transform, 6> cubeTransforms_ {
		{
			{glm::vec3(0.0f, 1.5f, 0.0),glm::vec3(0.5f)},
			{glm::vec3(2.0f, 0.0f, 1.0), glm::vec3(0.5f)},
			{glm::vec3(-1.0f, -1.0f, 2.0),glm::vec3(1.0f),
				60.0f, glm::normalize( glm::vec3(1,0,1))},
			{glm::vec3(0.0f, 2.7f, 4.0), glm::vec3(1.25f),
				23.0f,glm::normalize(glm::vec3(1.0, 0.0, 1.0))},
			{glm::vec3(-2.0f, 1.0f, -3.0),glm::vec3(1.0f),
				124.0f,glm::normalize(glm::vec3(1.0, 0.0, 1.0))},
			{glm::vec3(-3.0f, 0.0f, 0.0),glm::vec3(0.5f)}

		} };
	static constexpr std::array<Light, 4> lights_ = {
		{
			{glm::vec3(0.0f, 0.5f,  1.5f), glm::vec3(5.0f,   5.0f,  5.0f)},
			{glm::vec3(-4.0f, 0.5f, -3.0f),glm::vec3{10.0f,  0.0f,  0.0f}},
			{glm::vec3(3.0f, 0.5f,  1.0f), glm::vec3(0.0f,   0.0f,  15.0f)},
			{glm::vec3(-.8f,  2.4f, -1.0f),glm::vec3(0.0f,   5.0f,  0.0f)}
		}
	};

	sdl::Camera3D camera_;
	int blurAmount_ = 10;
	Quad screenPlane_{ glm::vec2(2.0f), glm::vec2() };
	float exposure_ = 1.0f;
	

};
}
