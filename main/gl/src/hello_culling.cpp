#include "hello_culling.h"

namespace gl
{


void HelloCulling::Init()
{
    camera_.Init();
    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");
    modelShader_.CreateDefaultProgram(
            "data/shaders/12_hello_culling/model.vert",
            "data/shaders/12_hello_culling/model.frag");
    cube_.Init();
    cubeTexture_.LoadTexture("data/textures/brickwall.ktx");
    glEnable(GL_DEPTH_TEST);
}

void HelloCulling::Update(core::seconds dt)
{
    camera_.Update(dt);
    if (flags_ & CULLING)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(flags_ & BACK_CULLING ? GL_BACK : GL_FRONT);
        glFrontFace(flags_ & CCW ? GL_CCW : GL_CW);
    }
    modelShader_.Bind();
    modelShader_.SetMat4("view", camera_.GetView());
    modelShader_.SetMat4("projection", camera_.GetProjection());
    auto model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.1f));
    modelShader_.SetMat4("model", model);
    modelShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
    model_.Draw(modelShader_);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f,0,0));
    modelShader_.SetMat4("model", model);
    modelShader_.SetTexture("texture_diffuse1", cubeTexture_, 0);
    cube_.Draw();

    if (flags_ & CULLING)
    {
        glDisable(GL_CULL_FACE);
    }
}

void HelloCulling::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    cube_.Destroy();
    cubeTexture_.Destroy();
    model_.Destroy();
    modelShader_.Destroy();
}

void HelloCulling::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloCulling::DrawImGui()
{
    ImGui::Begin("Culling Program");
    bool isCulling = flags_ & CULLING;
    if (ImGui::Checkbox("Culling", &isCulling))
    {
        flags_ = isCulling ? flags_ | CULLING : flags_ & ~CULLING;
    }
    bool isBack = flags_ & BACK_CULLING;
    if (ImGui::Checkbox("Back", &isBack))
    {
        flags_ = isBack ? flags_ | BACK_CULLING : flags_ & ~BACK_CULLING;
    }
    bool isCcw = flags_ & CCW;
    if (ImGui::Checkbox("CCW", &isCcw))
    {
        flags_ = isCcw ? flags_ | CCW : flags_ & ~CCW;
    }
    ImGui::End();
}
}