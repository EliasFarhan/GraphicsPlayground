#pragma once

#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"
#include "gl/camera.h"
#include "gl/model.h"

namespace gl
{
class HelloCubemaps : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    enum class ModelRenderMode
    {
        NONE,
        REFLECTION,
        REFRACTION,
        LENGTH
    };

    enum class TextureExtension
    {
        NONE,
        KTX,
        DDS
    };

    Cuboid skyboxCube_{glm::vec3(2.0f), glm::vec3()};
    ShaderProgram skyboxShader_;
    Texture skyboxTexture_;
    Texture ktxTexture_;
    Texture ddsTexture_;

    ShaderProgram modelShader_;
    ShaderProgram modelReflectionShader_;
    ShaderProgram modelRefractionShader_;
    Model model_;
    Cuboid cube_{glm::vec3(1.0f), glm::vec3()};
    Texture cubeTexture_;

    sdl::Camera3D camera_{};
    ModelRenderMode currentRenderMode_ = ModelRenderMode::NONE;
    float reflectionValue_ = 1.0f;
    float refractionValue_ = 1.0f;
    float refractiveIndex_ = 1.52f;

    TextureExtension textureExtension_ = TextureExtension::NONE;

};
}