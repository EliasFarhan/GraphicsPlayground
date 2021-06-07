//
// Created by efarhan on 6/6/21.
//

#include "hello_framebuffer.h"
#include <imgui.h>
namespace gl
{

void HelloFramebuffer::Init()
{
    screenFrame_.Init();
    cube_.Init();
    containerTexture_.LoadTexture("data/textures/container.jpg");
    camera_.Init();

    auto* window = Engine::GetInstance().GetWindow();
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    framebuffer_.SetSize({width, height});
    framebuffer_.Create();

    modelShader_.CreateDefaultProgram(
            "data/shaders/10_hello_framebuffer/model.vert",
            "data/shaders/10_hello_framebuffer/model.frag");
    screenShader_.CreateDefaultProgram(
            "data/shaders/10_hello_framebuffer/screen.vert",
            "data/shaders/10_hello_framebuffer/screen.frag");
    screenBlurShader_.CreateDefaultProgram(
            "data/shaders/10_hello_framebuffer/screen.vert",
            "data/shaders/10_hello_framebuffer/screen_blur.frag");
    screenEdgeDetectionShader_.CreateDefaultProgram(
            "data/shaders/10_hello_framebuffer/screen.vert",
            "data/shaders/10_hello_framebuffer/screen_edge_detection.frag");
    screenGrayscaleShader_.CreateDefaultProgram(
            "data/shaders/10_hello_framebuffer/screen.vert",
            "data/shaders/10_hello_framebuffer/screen_grayscale.frag");
    screenInverseShader_.CreateDefaultProgram(
            "data/shaders/10_hello_framebuffer/screen.vert",
            "data/shaders/10_hello_framebuffer/screen_inverse.frag");

    glEnable(GL_DEPTH_TEST);
}

void HelloFramebuffer::Update(core::seconds dt)
{
    camera_.Update(dt);

    framebuffer_.Bind();
    framebuffer_.Clear(glm::vec3());

    //Draw scene
    modelShader_.Bind();
    modelShader_.SetMat4("model", glm::mat4(1.0f));
    modelShader_.SetMat4("view", camera_.GetView());
    modelShader_.SetMat4("projection", camera_.GetProjection());

    modelShader_.SetTexture("texture_diffuse1",containerTexture_, 0);
    cube_.Draw();

    ShaderProgram* currentShader = nullptr;
    switch (postProcessingType_)
    {
        case PostProcessingType::NO_POSTPROCESS:
            currentShader = &screenShader_;
            break;
        case PostProcessingType::INVERSE:
            currentShader = &screenInverseShader_;
            break;
        case PostProcessingType::GRAYSCALE:
            currentShader = &screenGrayscaleShader_;
            break;
        case PostProcessingType::BLUR:
            currentShader = &screenBlurShader_;
            break;
        case PostProcessingType::EDGE_DETECTION:
            currentShader = &screenEdgeDetectionShader_;
            break;
        default:
            break;
    }
    Framebuffer::Unbind();
    currentShader->Bind();
    glDisable(GL_DEPTH_TEST);
    currentShader->SetInt("screenTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer_.GetColorTexture());
    screenFrame_.Draw();
    glEnable(GL_DEPTH_TEST);
}

void HelloFramebuffer::Destroy()
{

    glDisable(GL_DEPTH_TEST);
    screenFrame_.Destroy();
    cube_.Destroy();
    containerTexture_.Destroy();
    framebuffer_.Destroy();
    screenShader_.Destroy();
    screenInverseShader_.Destroy();
    screenGrayscaleShader_.Destroy();
    screenBlurShader_.Destroy();
    screenEdgeDetectionShader_.Destroy();
    modelShader_.Destroy();

}

void HelloFramebuffer::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        framebuffer_.SetSize({event.window.data1, event.window.data2});
        framebuffer_.Reload();
    }
}

void HelloFramebuffer::DrawImGui()
{
    ImGui::Begin("Post Processing");
    const char* postProcessingNames[(int)PostProcessingType::LENGTH] =
            {
                    "No Processing",
                    "Inverse",
                    "Grayscale",
                    "Blur",
                    "Edge Detection"
            };
    int currentIndex = (int)postProcessingType_;
    if(ImGui::Combo("Post Processing Type", &currentIndex, postProcessingNames, (int)PostProcessingType::LENGTH))
    {
        postProcessingType_ = (PostProcessingType)currentIndex;
    }
    ImGui::End();
}
}