#pragma once

namespace gl
{
class HelloNormal : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    enum NormalFlags : std::uint8_t
    {
        NONE = 0u,
        ENABLE_NORMAL_MAP = 1u << 0u,
        ENABLE_PLANE = 1u << 1u,
        ENABLE_CUBE = 1u << 2u,
        ENABLE_MODEL = 1u << 3u,
        ENABLE_SPHERE = 1u << 4u,
    };
    ShaderProgram diffuseShader_;
    ShaderProgram normalShader_;
    Texture diffuseTexture_;
    Texture normalTexture_;

    Quad plane_ {glm::vec2(1.0f), glm::vec2()};
    Cuboid cube_{glm::vec3(1.0f), glm::vec3()};
    Sphere sphere_{0.5f, glm::vec3()};
    Model model_;

    sdl::Camera3D camera_;
    glm::vec3 lightPos_ = glm::vec3(3.0f);
    float dt_ = 0.0f;

    std::uint8_t flags_ = ENABLE_CUBE | ENABLE_NORMAL_MAP;
};
}