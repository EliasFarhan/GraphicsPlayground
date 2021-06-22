//
// Created by efarhan on 21.06.21.
//
#include <GL/glew.h>
#include "hello_cascaded_shadow.h"
#include <gl/error.h>
#include <imgui.h>

namespace gl
{

void HelloCascadedShadow::Init()
{
    plane_.Init();
    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");
    simpleDepthShader_.CreateDefaultProgram("data/shaders/18_hello_cascaded_shadow/simple_depth.vert",
                                            "data/shaders/18_hello_cascaded_shadow/simple_depth.frag");
    shadowShader_.CreateDefaultProgram("data/shaders/18_hello_cascaded_shadow/shadow.vert",
                                       "data/shaders/18_hello_cascaded_shadow/shadow.frag");
    screenShader_.CreateDefaultProgram("data/shaders/18_hello_cascaded_shadow/screen.vert",
                                       "data/shaders/18_hello_cascaded_shadow/screen.frag");
    screenPlane_.Init();
    shadowFramebuffer_.SetType(Framebuffer::NO_DRAW | Framebuffer::DEPTH_ATTACHMENT);
    shadowFramebuffer_.SetSize({SHADOW_WIDTH, SHADOW_HEIGHT});
    shadowFramebuffer_.Create();
    shadowMaps_[0] = shadowFramebuffer_.GetDepthTexture();
    glGenTextures(shadowMaps_.size() - 1, &shadowMaps_[1]);
    for (std::size_t i = 1; i < shadowMaps_.size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, shadowMaps_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glCheckError();
    }
    brickwall_.LoadTexture("data/textures/brickwall.jpg");
    whiteTexture_.CreateWhiteTexture();

    camera_.Init();
    camera_.position = glm::vec3(0, 3, -3);
    camera_.LookAt(glm::vec3(0, 0, 1) * camera_.farPlane / 2.0f);
    camera_.farPlane = 100.0f;
    glEnable(GL_DEPTH_TEST);
}

void HelloCascadedShadow::Update(core::seconds dt)
{
    camera_.Update(dt);
    //Shadow passes
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    shadowFramebuffer_.Bind();
    simpleDepthShader_.Bind();
    for (std::size_t i = 0; i < shadowMaps_.size(); i++)
    {
        ShadowPass(i);
    }
    Framebuffer::Unbind();
    const auto windowSize = Engine::GetInstance().GetWindowSize();
    glViewport(0, 0, windowSize[0], windowSize[1]);
    shadowShader_.Bind();
    shadowShader_.SetMat4("view", camera_.GetView());
    shadowShader_.SetMat4("projection", camera_.GetProjection());
    shadowShader_.SetTexture("material.texture_diffuse1", whiteTexture_, 0);
    for (size_t i = 0; i < lights_.size(); i++)
    {
        shadowShader_.SetMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lights_[i].lightSpaceMatrix);
        shadowShader_.SetVec3("lights[" + std::to_string(i) + "].direction", lights_[i].direction);
    }
    shadowShader_.SetVec3("viewPos", camera_.position);
    for (size_t i = 0; i < shadowMaps_.size(); i++)
    {
        shadowShader_.SetTexture("lights[" + std::to_string(i) + "].shadowMap", shadowMaps_[i], i + 3);
    }
    shadowShader_.SetInt("enableCascadeColor", flags_ & ENABLE_CASCADE_COLOR);
    shadowShader_.SetInt("enableDepthColor", flags_ & ENABLE_DEPTH_COLOR);
    shadowShader_.SetFloat("farPlane", camera_.farPlane);
    shadowShader_.SetFloat("bias", shadowBias_);
    shadowShader_.SetFloat("maxNearCascade", camera_.farPlane * cascadedNearRatio_);
    shadowShader_.SetFloat("maxMiddleCascade", camera_.farPlane * cascadedMiddleRatio_);
    RenderScene(shadowShader_);
    if(flags_ & SHOW_DEPTH_TEXTURES)
    {
        glDisable(GL_DEPTH_TEST);
        screenShader_.Bind();
        constexpr float miniMapSize = 1.0f/3.0f;
        for (std::size_t i = 0; i < shadowMaps_.size(); i++)
        {
            screenShader_.SetVec2("offset", glm::vec2((1.0f - miniMapSize / camera_.aspect),
                                                      1.0f - miniMapSize - (2.0f * i * miniMapSize)));
            screenShader_.SetVec2("scale", glm::vec2(miniMapSize / camera_.aspect, miniMapSize));

            screenShader_.SetInt("screenTexture", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, shadowMaps_[i]);
            screenPlane_.Draw();
        }
        glEnable(GL_DEPTH_TEST);
    }
}

void HelloCascadedShadow::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    simpleDepthShader_.Destroy();
    shadowFramebuffer_.Destroy();
    shadowShader_.Destroy();
    screenShader_.Destroy();
    screenPlane_.Destroy();
    plane_.Destroy();
    model_.Destroy();
    whiteTexture_.Destroy();
    brickwall_.Destroy();
    glDeleteTextures(shadowMaps_.size() - 1, &shadowMaps_[1]);
}

void HelloCascadedShadow::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloCascadedShadow::DrawImGui()
{
    ImGui::Begin("Cascaded Shadow Program");

    ImGui::SliderFloat("Near Ratio", &cascadedNearRatio_, 0.0001f, cascadedMiddleRatio_);
    ImGui::SliderFloat("Middle Ratio", &cascadedMiddleRatio_, cascadedNearRatio_, 1.0f);
    if (ImGui::InputFloat3("Light Direction", &lights_[0].direction[0]))
    {
        lights_[1].direction = lights_[2].direction = lights_[0].direction;
    }
    bool enableAabbCascade = flags_ & ENABLE_DEPTH_COLOR;
    if (ImGui::Checkbox("Enable Depth Color", &enableAabbCascade))
    {
        flags_ = enableAabbCascade ? flags_ | ENABLE_DEPTH_COLOR : flags_ & ~ENABLE_DEPTH_COLOR;
    }
    bool enableCascadeColor = flags_ & ENABLE_CASCADE_COLOR;
    if (ImGui::Checkbox("Enable Cascade Color", &enableCascadeColor))
    {
        flags_ = enableCascadeColor ? flags_ | ENABLE_CASCADE_COLOR : flags_ & ~ENABLE_CASCADE_COLOR;
    }
    bool showDepthTextures = flags_ & SHOW_DEPTH_TEXTURES;
    if (ImGui::Checkbox("Show Depth Textures", &showDepthTextures))
    {
        flags_ = showDepthTextures ? flags_ | SHOW_DEPTH_TEXTURES : flags_ & ~SHOW_DEPTH_TEXTURES;
    }
    ImGui::End();
}

Camera2D HelloCascadedShadow::CalculateOrthoLight(float cascadeNear, float cascadeFar, glm::vec3 lightDir) const
{
    Camera2D lightCamera{};
    Camera3D camera = static_cast<Camera3D>(camera_);
    camera.nearPlane = cascadeNear;
    camera.farPlane = cascadeFar;
    lightCamera.position = glm::vec3();
    lightCamera.LookAt(lightDir);

    const auto tanHalfFovY = std::tan(glm::radians(camera_.fovY) / 2.0f);
    const auto tanHalfFovX = std::tan(glm::radians(camera_.GetFovX()) / 2.0f);
    const float nearX = cascadeNear * tanHalfFovX;
    const float nearY = cascadeNear * tanHalfFovY;
    const float farX = cascadeFar * tanHalfFovX;
    const float farY = cascadeFar * tanHalfFovY;

    std::array frustumCorners =
            {
                    // near face
                    camera.position + cascadeNear * camera.direction - camera.leftDir * nearX + camera.upDir * nearY,
                    camera.position + cascadeNear * camera.direction + camera.leftDir * nearX + camera.upDir * nearY,
                    camera.position + cascadeNear * camera.direction - camera.leftDir * nearX - camera.upDir * nearY,
                    camera.position + cascadeNear * camera.direction + camera.leftDir * nearX - camera.upDir * nearY,
                    // far face
                    camera.position + cascadeFar * camera.direction - camera.leftDir * farX + camera.upDir * farY,
                    camera.position + cascadeFar * camera.direction + camera.leftDir * farX + camera.upDir * farY,
                    camera.position + cascadeFar * camera.direction - camera.leftDir * farX - camera.upDir * farY,
                    camera.position + cascadeFar * camera.direction + camera.leftDir * farX - camera.upDir * farY,
            };

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    auto center = glm::vec3();
    for (auto& frustumCorner : frustumCorners)
    {
        center += frustumCorner;
    }
    center /= static_cast<float>(frustumCorners.size());
    lightCamera.position = center;
    const auto lightView = lightCamera.GetView();
    for (auto& frustumCorner : frustumCorners)
    {
        const auto frustumCornersLight = lightView * glm::vec4(frustumCorner, 1.0f);
        minX = std::min(minX, frustumCornersLight.x);
        maxX = std::max(maxX, frustumCornersLight.x);
        minY = std::min(minY, frustumCornersLight.y);
        maxY = std::max(maxY, frustumCornersLight.y);
        minZ = std::min(minZ, frustumCornersLight.z);
        maxZ = std::max(maxZ, frustumCornersLight.z);
    }

    //const auto size = glm::vec2((maxX - minX), (maxY - minY));


    lightCamera.left = minX ;
    lightCamera.right = maxX ;
    lightCamera.bottom = minY ;
    lightCamera.top =  maxY ;
    constexpr float aabbFactor = 2.0f;
    const auto distZ = (maxZ - minZ) / 2.0f;
    lightCamera.nearPlane = minZ + distZ - distZ * aabbFactor;
    lightCamera.farPlane = maxZ - distZ + distZ * aabbFactor;

    //const auto maxSize = std::max(std::max(maxX-minX, maxY-minY), maxZ-minZ);

    return lightCamera;
}

void HelloCascadedShadow::ShadowPass(int cascadeIndex)
{
    const auto cascadeNear = cascadeIndex == 0 ? camera_.nearPlane :
                             cascadeIndex == 1 ? camera_.farPlane * cascadedNearRatio_ :
                             camera_.farPlane * cascadedMiddleRatio_;
    const auto cascadeFar = cascadeIndex == 0 ? camera_.farPlane * cascadedNearRatio_ :
                            cascadeIndex == 1 ? camera_.farPlane * cascadedMiddleRatio_ :
                            camera_.farPlane;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMaps_[cascadeIndex], 0);
    glClear(GL_DEPTH_BUFFER_BIT);


    const auto lightCamera = CalculateOrthoLight(cascadeNear, cascadeFar, lights_[cascadeIndex].direction);
    lights_[cascadeIndex].position = lightCamera.position;
    lights_[cascadeIndex].lightSpaceMatrix =
            lightCamera.GetProjection() * lightCamera.GetView();

    simpleDepthShader_.SetFloat("lightFarPlane", lightCamera.farPlane);
    simpleDepthShader_.SetMat4("lightSpaceMatrix",
                               lights_[cascadeIndex].lightSpaceMatrix);

    RenderScene(simpleDepthShader_);
}

void HelloCascadedShadow::RenderScene(gl::ShaderProgram& shader)
{

    for (int z = 0; z < 5; z++)
    {
        for (int x = -1; x < 2; x++)
        {
            auto model = glm::mat4(1.0f);

            model = glm::translate(model,
                                   glm::vec3(-10.0f * float(x), 0.0f, 10.0f * float(z) + 5.0f));
            model = glm::scale(model, glm::vec3(0.2f));
            shader.SetMat4("model", model);
            shader.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
            model_.Draw(shader);
        }
    }
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, 0, 1) * camera_.farPlane / 2.0f);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(camera_.farPlane));
    shader.SetTexture("texture_diffuse1", whiteTexture_, 0);
    shader.SetMat4("model", model);
    shader.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
    plane_.Draw();
}
}