#include <GL/glew.h>
#include "hello_ibl.h"

#include "gl/error.h"
#include <fmt/core.h>
#include <imgui.h>

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#include "TracyOpenGL.hpp"
#endif

namespace gl
{
void HelloIbl::Init()
{
    sphere_.Init();
    quad_.Init();
    skybox_.Init();

    equiToCubemap_.CreateDefaultProgram(
        "data/shaders/25_hello_ibl/cube.vert",
        "data/shaders/25_hello_ibl/cube.frag");

    skyboxShader_.CreateDefaultProgram(
        "data/shaders/25_hello_ibl/skybox.vert",
        "data/shaders/25_hello_ibl/skybox.frag");
    pbrShader_.CreateDefaultProgram(
        "data/shaders/25_hello_ibl/pbr.vert",
        "data/shaders/25_hello_ibl/pbr.frag");
    irradianceShader_.CreateDefaultProgram(
        "data/shaders/25_hello_ibl/cube.vert",
        "data/shaders/25_hello_ibl/irradiance.frag");
    prefilterShader_.CreateDefaultProgram(
        "data/shaders/25_hello_ibl/cube.vert",
        "data/shaders/25_hello_ibl/prefilter.frag");
    brdfShader_.CreateDefaultProgram(
        "data/shaders/25_hello_ibl/brdf.vert",
        "data/shaders/25_hello_ibl/brdf.frag");

    hdrTexture_.LoadTexture("data/textures/Ridgecrest_Road_Ref.hdr", Texture::SMOOTH | Texture::CLAMP_WRAP | Texture::FLIP_Y);
    flags_ = FIRST_FRAME;

    camera_.Init();
    camera_.position = glm::vec3(0, 0, 30.0f);
    camera_.LookAt(glm::vec3());


    GenerateCubemap();
    GenerateDiffuseIrradiance();
    GeneratePrefilter();
    GenerateLUT();
    glEnable(GL_DEPTH_TEST);

    auto& engine = Engine::GetInstance();
    const auto windowSize = engine.GetWindowSize();
    glViewport(0, 0, windowSize[0], windowSize[1]);
}

void HelloIbl::Update(core::seconds dt)
{
    camera_.Update(dt);

    const auto view = camera_.GetView();
    const auto projection = camera_.GetProjection();
    //Render PBR spheres
    const int nrRows = 7;
    const int nrColumns = 7;
    pbrShader_.Bind();
    pbrShader_.SetInt("enableIrradiance", flags_ & ENABLE_IRRADIANCE);
    pbrShader_.SetInt("enableIblSpecular", flags_ & ENABLE_IBL_SPECULAR);
    pbrShader_.SetInt("enableSchlickRoughness", flags_ & ENABLE_SCHLICK_ROUGHNESS);
    pbrShader_.SetInt("gammaCorrect", true);
    pbrShader_.SetFloat("ao", 1.0f);
    pbrShader_.SetVec3("albedo", baseColor_);
    pbrShader_.SetMat4("view", view);
    pbrShader_.SetVec3("viewPos", camera_.position);
    pbrShader_.SetMat4("projection", projection);
    pbrShader_.SetTexture("irradianceMap", irradianceMap_, 0);
    pbrShader_.SetTexture("prefilterMap", prefilterMap_, 1);
    pbrShader_.SetTexture("brdfLUT", brdfLUTTexture_, 2);
    for (size_t i = 0; i < lights_.size(); i++)
    {
        pbrShader_.SetVec3(fmt::format("lights[{}].position", i), lights_[i].position);
        pbrShader_.SetVec3(fmt::format("lights[{}].color", i), lights_[i].color);

    }
    for (int row = 0; row < nrRows; ++row)
    {
        pbrShader_.SetFloat("metallic", static_cast<float>(row) / static_cast<float>(nrRows - 1));
        for (int col = 0; col < nrColumns; ++col)
        {
            // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
            // on direct lighting.
            pbrShader_.SetFloat("roughness", std::clamp(static_cast<float>(col) / static_cast<float>(nrColumns - 1), 0.05f, 1.0f));

            auto model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(
                static_cast<float>(col - nrColumns / 2) * spacing_,
                static_cast<float>(row - nrRows / 2) * spacing_,
                0.0f
            ));
            pbrShader_.SetMat4("model", model);
            pbrShader_.SetMat4("normalMatrix", glm::transpose(glm::inverse(model)));
            sphere_.Draw();
        }
    }
    //Render skybox
    glDepthFunc(GL_LEQUAL);
    skyboxShader_.Bind();
    skyboxShader_.SetMat4("view", view);
    skyboxShader_.SetMat4("projection", projection);
    skyboxShader_.SetTexture("environmentMap",
        flags_ & SHOW_PREFILTER ? prefilterMap_ : (flags_ & SHOW_IRRADIANCE ? irradianceMap_ : envCubemap_), 0);
    skybox_.Draw();
    glDepthFunc(GL_LESS);
    glCheckError();
}

void HelloIbl::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    hdrTexture_.Destroy();
    sphere_.Destroy();
    quad_.Destroy();
    skybox_.Destroy();

    skyboxShader_.Destroy();
    brdfShader_.Destroy();
    irradianceShader_.Destroy();
    pbrShader_.Destroy();
    prefilterShader_.Destroy();
    equiToCubemap_.Destroy();

    captureFramebuffer_.Destroy();
    prefilterMap_.Destroy();
    envCubemap_.Destroy();
    irradianceMap_.Destroy();
    brdfLUTTexture_.Destroy();
}

void HelloIbl::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloIbl::DrawImGui()
{
    ImGui::Begin("Pbr Program");
    ImGui::ColorPicker3("Base Color", &baseColor_[0]);
    bool showIrradianceMap = flags_ & SHOW_IRRADIANCE;
    if (ImGui::Checkbox("Show Irradiance Map", &showIrradianceMap))
    {
        flags_ = showIrradianceMap ? flags_ | SHOW_IRRADIANCE : flags_ & ~SHOW_IRRADIANCE;
    }
    bool showPrefilterMap = flags_ & SHOW_PREFILTER;
    if (ImGui::Checkbox("Show Prefilter Map", &showPrefilterMap))
    {
        flags_ = showPrefilterMap ? flags_ | SHOW_PREFILTER : flags_ & ~SHOW_PREFILTER;
    }
    bool enableIrradiance = flags_ & ENABLE_IRRADIANCE;
    if (ImGui::Checkbox("Enable Irradiance", &enableIrradiance))
    {
        flags_ = enableIrradiance ? flags_ | ENABLE_IRRADIANCE : flags_ & ~ENABLE_IRRADIANCE;
    }
    bool enableSchlickRoughness = flags_ & ENABLE_SCHLICK_ROUGHNESS;
    if (ImGui::Checkbox("Enable Schlick Roughness", &enableSchlickRoughness))
    {
        flags_ = enableSchlickRoughness ? flags_ | ENABLE_SCHLICK_ROUGHNESS : flags_ & ~ENABLE_SCHLICK_ROUGHNESS;
    }
    bool enableIblSpecular = flags_ & ENABLE_IBL_SPECULAR;
    if (ImGui::Checkbox("Enable Specular IBL", &enableIblSpecular))
    {
        flags_ = enableIblSpecular ? flags_ | ENABLE_IBL_SPECULAR : flags_ & ~ENABLE_IBL_SPECULAR;
    }

    ImGui::End();
}

void HelloIbl::GenerateCubemap()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(genCubemap, "Generate Cubemap", true);
    TracyGpuNamedZone(genCubemapGpu, "Generate Cubemap", true);
#endif
    captureFramebuffer_.SetSize({cubemapFaceSize_.x, cubemapFaceSize_.y});
    captureFramebuffer_.SetType(
        Framebuffer::COLOR_ATTACHMENT_0 |
        Framebuffer::COLOR_CUBEMAP |
        Framebuffer::HDR |
        Framebuffer::DEPTH_RBO);
    captureFramebuffer_.Reload();

    Camera3D captureCamera;
    captureCamera.position = glm::vec3();
    captureCamera.aspect = 1.0f;
    captureCamera.fovY = 90.0f;
    captureCamera.nearPlane = 0.1f;
    captureCamera.farPlane = 10.0f;
    equiToCubemap_.Bind();
    equiToCubemap_.SetTexture("equirectangularMap", hdrTexture_, 0);
    equiToCubemap_.SetMat4("projection", captureCamera.GetProjection());
    glViewport(0, 0, cubemapFaceSize_.x, cubemapFaceSize_.y);
    captureFramebuffer_.Bind();
    for (unsigned int i = 0; i < 6; ++i)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(genCubemapFace, "Generate Cubemap Face", true);
        TracyGpuNamedZone(genCubemapFaceGpu, "Generate Cubemap Face", true);
#endif
        captureCamera.LookAt(viewDirs[i], upDirs[i]);
        equiToCubemap_.SetMat4("view", captureCamera.GetView());
        captureFramebuffer_.ActivateColorFace(i);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox_.Draw();
    }
    envCubemap_.SetName(captureFramebuffer_.MoveColorTexture(0));
    envCubemap_.SetType(GL_TEXTURE_CUBE_MAP);
    Framebuffer::Unbind();
}

void HelloIbl::GenerateDiffuseIrradiance()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(genDiffuse, "Generate Diffuse Irradiance", true);
    TracyGpuNamedZone(genDiffuseGpu, "Generate Diffuse Irradiance", true);
#endif
    captureFramebuffer_.SetSize({irradianceFaceSize_.x, irradianceFaceSize_.y});
    captureFramebuffer_.Reload();

    Camera3D captureCamera;
    captureCamera.position = glm::vec3();
    captureCamera.aspect = 1.0f;
    captureCamera.fovY = 90.0f;
    captureCamera.nearPlane = 0.1f;
    captureCamera.farPlane = 10.0f;

    irradianceShader_.Bind();
    irradianceShader_.SetMat4("projection", captureCamera.GetProjection());
    irradianceShader_.SetTexture("environmentMap", envCubemap_, 0);
    glViewport(0, 0, irradianceFaceSize_.x, irradianceFaceSize_.y);
    captureFramebuffer_.Bind();
    for (int i = 0; i < 6; i++)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(genDiffuseFace, "Generate Diffuse Irradiance Face", true);
        TracyGpuNamedZone(genDiffuseFaceGpu, "Generate Diffuse Irradiance Face", true);
#endif
        captureCamera.LookAt(viewDirs[i], upDirs[i]);
        irradianceShader_.SetMat4("view", captureCamera.GetView());
        captureFramebuffer_.ActivateColorFace(i);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox_.Draw();
    }
    irradianceMap_.SetName(captureFramebuffer_.MoveColorTexture(0));
    irradianceMap_.SetType(GL_TEXTURE_CUBE_MAP);
    Framebuffer::Unbind();
    glCheckError();
}

void HelloIbl::GeneratePrefilter()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(genPrefilter, "Generate Prefilter Env Map", true);
    TracyGpuNamedZone(genPrefilterGpu, "Generate Prefilter Env Map", true);
#endif
    Camera3D captureCamera;
    captureCamera.position = glm::vec3();
    captureCamera.aspect = 1.0f;
    captureCamera.fovY = 90.0f;
    captureCamera.nearPlane = 0.1f;
    captureCamera.farPlane = 10.0f;
    captureFramebuffer_.SetSize({prefilterFaceSize_.x, prefilterFaceSize_.y});
    captureFramebuffer_.Reload();
    captureFramebuffer_.Bind();
    const auto colorBuffer = captureFramebuffer_.GetColorTexture(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, colorBuffer);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // be sure to set minifcation filter to mip_linear 
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glCheckError();
    prefilterShader_.Bind();
    prefilterShader_.SetTexture("environmentMap", envCubemap_, 0);
    prefilterShader_.SetMat4("projection", captureCamera.GetProjection());

    captureFramebuffer_.Bind();
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(genPrefilterMip, "Generate Prefilter Env Map Mip", true);
        TracyGpuNamedZone(genPrefilterMipGpu, "Generate Prefilter Env Map Mip", true);
#endif
        // reisze framebuffer according to mip-level size.
        const unsigned int mipWidth = prefilterFaceSize_.x * std::pow(0.5, mip);
        const unsigned int mipHeight = prefilterFaceSize_.y * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureFramebuffer_.GetDepthRbo());
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mipWidth, mipHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glViewport(0, 0, mipWidth, mipHeight);

        const float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        prefilterShader_.SetFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(genPrefilterFace, "Generate Prefilter Env Map Mip Face", true);
            TracyGpuNamedZone(genPrefilterGpuFace, "Generate Prefilter Env Map Mip Face", true);
#endif
            captureCamera.LookAt(viewDirs[i], upDirs[i]);
            prefilterShader_.SetMat4("view", captureCamera.GetView());
            captureFramebuffer_.ActivateColorFace(i, mip);
            glCheckError();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            skybox_.Draw();
        }
    }
    prefilterMap_.SetName(captureFramebuffer_.MoveColorTexture(0));
    prefilterMap_.SetType(GL_TEXTURE_CUBE_MAP);
    Framebuffer::Unbind();
    glCheckError();
}

void HelloIbl::GenerateLUT()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(genLUT, "Generate LUT", true);
    TracyGpuNamedZone(genLUTGpu, "Generate LUT", true);
#endif
    captureFramebuffer_.SetType(Framebuffer::COLOR_ATTACHMENT_0 | Framebuffer::HDR);
    captureFramebuffer_.SetSize({ lutSize_.x,lutSize_.y });
    captureFramebuffer_.SetChannelCount(2);
    captureFramebuffer_.Reload();
    glDisable(GL_DEPTH_TEST);
    captureFramebuffer_.Bind();
    glViewport(0, 0, lutSize_.x, lutSize_.y);
    brdfShader_.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    quad_.Draw();
    brdfLUTTexture_.SetName(captureFramebuffer_.MoveColorTexture(0));
    glEnable(GL_DEPTH_TEST);
    Framebuffer::Unbind();
}
}
