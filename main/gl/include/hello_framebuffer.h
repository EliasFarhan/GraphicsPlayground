#pragma once

#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"
#include "gl/camera.h"
#include "gl/model.h"
#include "gl/framebuffer.h"

namespace gl
{
class HelloFramebuffer : public core::Program
{
public:

    enum class PostProcessingType
    {
        NO_POSTPROCESS,
        INVERSE,
        GRAYSCALE,
        BLUR,
        EDGE_DETECTION,
        LENGTH
    };

    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    sdl::Camera3D camera_{};
    Cuboid cube_{glm::vec3(1.0f), glm::vec3()};
    Texture containerTexture_{};

    Quad screenFrame_{glm::vec2(2.0f), glm::vec2()};

    ShaderProgram screenShader_;
    ShaderProgram screenInverseShader_;
    ShaderProgram screenGrayscaleShader_;
    ShaderProgram screenBlurShader_;
    ShaderProgram screenEdgeDetectionShader_;
    ShaderProgram modelShader_;

    Framebuffer framebuffer_;

    PostProcessingType postProcessingType_ = PostProcessingType::NO_POSTPROCESS;



};
}