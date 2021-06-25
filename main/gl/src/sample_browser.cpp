
#include "sample_browser.h"

#include "hello_blending.h"
#include "hello_triangle.h"
#include "hello_texture.h"
#include "hello_cube.h"
#include "hello_light.h"
#include "hello_model.h"
#include "hello_cutoff.h"
#include <hello_framebuffer.h>
#include <hello_cubemaps.h>
#include <hello_culling.h>
#include <hello_instancing.h>
#include <hello_frustum.h>
#include <hello_point_shadow.h>
#include <hello_cascaded_shadow.h>
#include <hello_deferred.h>

#include <hello_bloom.h>
#include <hello_hdr.h>
#include <hello_shadow.h>
#include <hello_normal.h>
#include <imgui.h>


namespace gl
{

SampleBrowser::SampleBrowser()
{
    samples_.push_back(
            {"01 Hello Triangle", std::make_unique<HelloTriangle>()});
    samples_.push_back({"02 Hello Texture", std::make_unique<HelloTexture>()});
    samples_.push_back({"03 Hello Cube", std::make_unique<HelloCube>()});
    samples_.push_back({"04 Hello Light", std::make_unique<HelloLight>()});
    samples_.push_back({"07 Hello Model", std::make_unique<HelloModel>()});
    samples_.push_back({"08 Hello Cutoff", std::make_unique<HelloCutoff>()});
    samples_.push_back(
            {"09 Hello Blending", std::make_unique<HelloBlending>()});
    samples_.push_back(
            {"10 Hello Framebuffer", std::make_unique<HelloFramebuffer>()});
    samples_.push_back({"11 Hello Cubemaps", std::make_unique<HelloCubemaps>()});
    samples_.push_back({"12 Hello Culling", std::make_unique<HelloCulling>()});
    samples_.push_back({"13 Hello Instancing", std::make_unique<HelloInstancing>()});
    samples_.push_back({"14 Hello Frustum Culling", std::make_unique<HelloFrustum>()});
    samples_.push_back({"15 Hello Normal", std::make_unique<HelloNormal>()});
    samples_.push_back({"16 Hello Shadow", std::make_unique<HelloShadow>()});
    samples_.push_back({"17 Hello Point Shadow", std::make_unique<HelloPointShadow>()});
    samples_.push_back({"18 Hello Cascaded Shadow", std::make_unique<HelloCascadedShadow>()});
    samples_.push_back({"19 Hello HDR", std::make_unique<HelloHdr>()});
    samples_.push_back({"20 Hello Bloom", std::make_unique<HelloBloom>()});
    samples_.push_back({"21 Hello Deferred", std::make_unique<HelloDeferred>()});
}

void SampleBrowser::Init()
{
    samples_[currentIndex_].sample->Init();
}

void SampleBrowser::Update(core::seconds dt)
{
    samples_[currentIndex_].sample->Update(dt);
}

void SampleBrowser::Destroy()
{
    samples_[currentIndex_].sample->Destroy();
}

void SampleBrowser::OnEvent(SDL_Event& event)
{
    samples_[currentIndex_].sample->OnEvent(event);
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        glViewport(0, 0, event.window.data1, event.window.data2);
    }
}

void SampleBrowser::DrawImGui()
{
    samples_[currentIndex_].sample->DrawImGui();
    ImGui::Begin("Sample Browser");
    if (ImGui::BeginCombo("Current Sample",
                          samples_[currentIndex_].sampleName.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (std::size_t i = 0; i < samples_.size(); i++)
        {
            const bool isSelected = currentIndex_ == i;
            if (ImGui::Selectable(samples_[i].sampleName.c_str(), isSelected))
            {
                samples_[currentIndex_].sample->Destroy();
                samples_[i].sample->Init();
                currentIndex_ = i;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::End();
}

}