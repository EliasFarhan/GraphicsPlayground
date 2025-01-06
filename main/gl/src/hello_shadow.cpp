#include "GL/glew.h"
#include "hello_shadow.h"
#include "imgui.h"
#include "gl/error.h"
#ifdef TRACY_ENABLE

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#endif
namespace gl
{

void HelloShadow::Init()
{
    cube_.Init();
    floor_.Init();
    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");
    floorTexture_.LoadTexture("data/textures/brickwall.jpg");

    camera_.Init();
    camera_.position = glm::vec3(0,3,3);
    camera_.LookAt(glm::vec3());

    depthCamera_.SetExtends(glm::vec2(4.0f));
    depthCamera_.position = light_.lightPos;
    depthCamera_.LookAt(light_.lightPos + light_.lightDir);

    simpleDepthShader_.CreateDefaultProgram(
            "data/shaders/16_hello_shadow/simple_depth.vert",
            "data/shaders/16_hello_shadow/simple_depth.frag");
    modelShader_.CreateDefaultProgram(
            "data/shaders/16_hello_shadow/shadow.vert",
            "data/shaders/16_hello_shadow/shadow.frag");

    shadowFramebuffer_.SetType(Framebuffer::NO_DRAW | Framebuffer::DEPTH_ATTACHMENT);
    shadowFramebuffer_.SetSize({SHADOW_WIDTH, SHADOW_HEIGHT});
    shadowFramebuffer_.Create();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void HelloShadow::Update(core::seconds dt)
{
    camera_.Update(dt);
    glCullFace(GL_BACK);
    const auto lightView = depthCamera_.GetView();
    const auto lightProjection = depthCamera_.GetProjection();
    const auto lightSpaceMatrix = lightProjection * lightView;
    if (flags_ & ENABLE_SHADOW)
    {
        //Render depth buffer from light
        shadowFramebuffer_.Bind();
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader_.Bind();

        simpleDepthShader_.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        if (flags_ & ENABLE_PETER_PANNING)
        {
            glCullFace(GL_FRONT);
        }
        RenderScene(simpleDepthShader_);
        if (flags_ & ENABLE_PETER_PANNING)
        {
            glCullFace(GL_BACK);
        }
        //Render scene with shadow
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCheckError();
        const auto windowSize = Engine::GetInstance().GetWindowSize();
        glViewport(0, 0, windowSize[0], windowSize[1]);
    }
    modelShader_.Bind();
    modelShader_.SetMat4("view", camera_.GetView());
    modelShader_.SetMat4("projection", camera_.GetProjection());
    modelShader_.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    modelShader_.SetTexture("shadowMap", shadowFramebuffer_.GetDepthTexture(), 3);
    modelShader_.SetInt("enableBias", flags_ & ENABLE_BIAS);
    modelShader_.SetInt("enableShadow", flags_ & ENABLE_SHADOW);
    modelShader_.SetInt("enableOverSampling", flags_ & ENABLE_OVER_SAMPLING);
    modelShader_.SetInt("enablePcf", flags_ & ENABLE_PCF);
    modelShader_.SetVec3("viewPos", camera_.position);
    modelShader_.SetFloat("bias", shadowBias_);
    modelShader_.SetVec3("light.lightDir", light_.lightDir);
    RenderScene(modelShader_);
}

void HelloShadow::Destroy()
{

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    cube_.Destroy();
    floor_.Destroy();
    model_.Destroy();

    floorTexture_.Destroy();
    shadowFramebuffer_.Destroy();
    modelShader_.Destroy();
    simpleDepthShader_.Destroy();
}

void HelloShadow::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloShadow::DrawImGui()
{
    ImGui::Begin("Shadow Program");
    if (ImGui::InputFloat3("Light Pos", &light_.lightPos[0]))
    {
        depthCamera_.position = light_.lightPos;
    }
    bool enableShadow = flags_ & ENABLE_SHADOW;
    if (ImGui::Checkbox("Enable Shadow", &enableShadow))
    {
        flags_ = enableShadow ? flags_ | ENABLE_SHADOW : flags_ & ~ENABLE_SHADOW;
    }
    bool enableBias = flags_ & ENABLE_BIAS;
    if (ImGui::Checkbox("Enable Bias", &enableBias))
    {
        flags_ = enableBias ? flags_ | ENABLE_BIAS : flags_ & ~ENABLE_BIAS;
    }
    ImGui::InputFloat("Shadow Bias", &shadowBias_, 0, 0, "%.6f");
    bool enablePeterPanning = flags_ & ENABLE_PETER_PANNING;
    if (ImGui::Checkbox("Peter Panning", &enablePeterPanning))
    {
        flags_ = enablePeterPanning ? flags_ | ENABLE_PETER_PANNING : flags_ & ~ENABLE_PETER_PANNING;
    }
    bool enableOverSampling = flags_ & ENABLE_OVER_SAMPLING;
    if (ImGui::Checkbox("Over Sampling", &enableOverSampling))
    {
        flags_ = enableOverSampling ? flags_ | ENABLE_OVER_SAMPLING : flags_ & ~ENABLE_OVER_SAMPLING;
    }
    glm::vec2 depthCameraSize = glm::vec2(depthCamera_.right, depthCamera_.top);
    if (ImGui::InputFloat2("Depthmap Size", &depthCameraSize[0]))
    {
        depthCamera_.SetExtends(depthCameraSize);
    }
    bool enablePcf = flags_ & ENABLE_PCF;
    if (ImGui::Checkbox("Enable PCF", &enablePcf))
    {
        flags_ = enablePcf ? flags_ | ENABLE_PCF : flags_ & ~ENABLE_PCF;
    }

    ImGui::End();
}

void HelloShadow::RenderScene(ShaderProgram& shader)
{
    //Render model
    auto model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.1f));
    model = glm::translate(model, glm::vec3(0, 0, 5.0f));
    shader.SetMat4("model", model);
    shader.SetMat4("transposeInverseModel",
                   glm::transpose(glm::inverse(model)));
    model_.Draw(shader);

    //Render floor
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
    shader.SetMat4("model", model);
    shader.SetMat4("transposeInverseModel",
                   glm::transpose(glm::inverse(model)));
    shader.SetTexture("texture_diffuse1", floorTexture_, 0);
    floor_.Draw();

    //Render cubes
    for (const auto& transform : cubeTransforms_)
    {
        model = glm::mat4(1.0f);
        model = glm::rotate(model, transform.angle, transform.axis);
        model = glm::scale(model, transform.scale);
        model = glm::translate(model, transform.position);
        shader.SetMat4("model", model);
        shader.SetMat4("transposeInverseModel",
                       glm::transpose(glm::inverse(model)));
        cube_.Draw();
    }
}
}