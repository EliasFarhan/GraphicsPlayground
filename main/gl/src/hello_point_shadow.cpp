//
// Created by efarhan on 21.06.21.
//
#include <GL/glew.h>
#include "hello_point_shadow.h"

namespace gl
{

void HelloPointShadow::Init()
{
    cube_.Init();
    cubeTexture_.LoadTexture("data/textures/brickwall.jpg");

    simpleDepthShader_.CreateDefaultProgram("data/shaders/17_hello_point_shadow/simpleDepth.vert",
                                            "data/shaders/17_hello_point_shadow/simpleDepth.frag");

    cubeShader_.CreateDefaultProgram("data/shaders/17_hello_point_shadow/shadow.vert",
                                     "data/shaders/17_hello_point_shadow/shadow.frag");
    lightCubeShader_.CreateDefaultProgram("data/shaders/17_hello_point_shadow/lamp.vert",
                                          "data/shaders/17_hello_point_shadow/lamp.frag");

    shadowFramebuffer_.SetType(Framebuffer::NO_DRAW | Framebuffer::DEPTH_ATTACHMENT | Framebuffer::DEPTH_CUBEMAP);
    shadowFramebuffer_.SetSize({1024, 1024});
    shadowFramebuffer_.Create();
    camera_.Init();
    camera_.position = glm::vec3 (3.0f);
    camera_.LookAt(glm::vec3());

    glEnable(GL_DEPTH_TEST);

    lightCamera_.position = glm::vec3();
    lightCamera_.farPlane = 50.0f;
    lightCamera_.nearPlane = 0.1f;
    lightCamera_.fovY = 90.0f;
    lightCamera_.aspect = 1.0f;

}

void HelloPointShadow::Update(core::seconds dt)
{
    camera_.Update(dt);
    dt_ += dt.count();
    lightCamera_.position = 4.0f * glm::vec3 (std::sin(dt_), 0.0f, std::sin(dt_));

    shadowFramebuffer_.Bind();
    constexpr std::array lightDirs =
            {
                    glm::vec3(1,0,0),
                    glm::vec3(-1,0,0),
                    glm::vec3(0,1,0),
                    glm::vec3(0,-1,0),
                    glm::vec3(0,0,1),
                    glm::vec3(0,0,-1)
            };
    constexpr std::array lightUps =
            {
                    glm::vec3(0,1,0),
                    glm::vec3(0,1,0),
                    glm::vec3(0,0,-1),
                    glm::vec3(0,0,1),
                    glm::vec3(0,1,0),
                    glm::vec3(0,1,0)
            };
    simpleDepthShader_.Bind();
    simpleDepthShader_.SetVec3("lightPos", lightCamera_.position);
    glViewport(0, 0, shadowFramebuffer_.GetSize().x, shadowFramebuffer_.GetSize().y);
    for (int i = 0; i < 6; i++)
    {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               shadowFramebuffer_.GetDepthTexture(), 0);
        glClear(GL_DEPTH_BUFFER_BIT);

        lightCamera_.LookAt(lightCamera_.position+lightDirs[i], lightUps[i]);
        const auto lightSpaceMatrix = lightCamera_.GetProjection() * lightCamera_.GetView();
        simpleDepthShader_.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        simpleDepthShader_.SetFloat("lightFarPlane", lightCamera_.farPlane);
        simpleDepthShader_.SetVec3("lightDir", lightDirs[i]);
        simpleDepthShader_.SetVec3("lightPos", lightCamera_.position);
        RenderScene(simpleDepthShader_);
    }

    Framebuffer::Unbind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const auto view = camera_.GetView();
    const auto projection = camera_.GetProjection();
    const auto windowSize = Engine::GetInstance().GetWindowSize();
    glViewport(0,0,windowSize[0], windowSize[1]);
    cubeShader_.Bind();
    cubeShader_.SetMat4("view", view);
    cubeShader_.SetMat4("projection", projection);
    cubeShader_.SetVec3("light.lightPos", lightCamera_.position);
    cubeShader_.SetVec3("viewPos", camera_.position);

    cubeShader_.SetFloat("lightFarPlane", lightCamera_.farPlane);
    cubeShader_.SetFloat("bias", bias_);
    //Render the scene with shadow
    cubeShader_.SetTexture("material.texture_diffuse1", cubeTexture_, 0);
    cubeShader_.SetInt("shadowMap", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowFramebuffer_.GetDepthTexture());
    RenderScene(cubeShader_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //Render the white light cube
    lightCubeShader_.Bind();
    lightCubeShader_.SetVec3("lightColor", glm::vec3(1.0f));
    lightCubeShader_.SetMat4("view", view);
    lightCubeShader_.SetMat4("projection", projection);
    auto model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.2f));
    model = glm::translate(model, lightCamera_.position);
    lightCubeShader_.SetMat4("model", model);
    cube_.Draw();
}

void HelloPointShadow::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    cube_.Destroy();
    lightCubeShader_.Destroy();
    shadowFramebuffer_.Destroy();
    cubeShader_.Destroy();
    simpleDepthShader_.Destroy();
    cubeTexture_.Destroy();
}

void HelloPointShadow::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloPointShadow::DrawImGui()
{
    ImGui::Begin("Point Shadow Program");
    ImGui::InputFloat3("Light Pos", &lightCamera_.position[0]);
    ImGui::InputFloat("Shadow Bias", &bias_);
    ImGui::End();
}

void HelloPointShadow::RenderScene(ShaderProgram& program)
{
    auto model = glm::mat4(1.0f);
    program.SetInt("inverseNormals", false);

    for (const auto& transform : transforms_)
    {
        model = glm::mat4 (1.0f);
        model = glm::rotate(model, transform.angle, transform.axis);
        model = glm::scale(model, transform.scale);
        model = glm::translate(model, transform.position);
        program.SetMat4("model", model);
        program.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
        cube_.Draw();
    }

    //Render bigger cube
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(10.0f));
    program.SetMat4("model", model);
    program.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
    program.SetInt("inverseNormals", true);
    cube_.Draw();
}
}