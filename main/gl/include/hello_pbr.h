#pragma once
#include <engine.h>
#include <gl/vertex_array.h>
#include <gl/shader.h>
#include <gl/camera.h>

namespace gl
{

class HelloPbr : public core::Program
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
		glm::vec3 position;
		glm::vec3 color;
	};
	std::array<Light, 4> lights_{};
	Sphere sphere_{ 1.0f, glm::vec3() };
	ShaderProgram pbrShader_;
	sdl::Camera3D camera_{};

	glm::vec3 baseColor_ = { 1.0f,0.5f,0.5f };
	float spacing_ = 2.5f;
	bool gammaCorrect_ = true;
};

}
