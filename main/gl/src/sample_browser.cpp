//
// Created by efarhan on 5/27/21.
//

#include "sample_browser.h"
#include "hello_triangle.h"
#include "hello_texture.h"
#include "hello_cube.h"
#include "hello_light.h"

#include "imgui.h"

namespace gl
{

SampleBrowser::SampleBrowser()
{
    samples_.push_back({"01 Hello Triangle", std::make_unique<HelloTriangle>()});
    samples_.push_back({"02 Hello Texture", std::make_unique<HelloTexture>()});
    samples_.push_back({"03 Hello Cube", std::make_unique<HelloCube>()});
    samples_.push_back({"04 Hello Light", std::make_unique<HelloLight>()});
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
    if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        glViewport(0,0,event.window.data1, event.window.data2);
    }
}

void SampleBrowser::DrawImGui()
{
    samples_[currentIndex_].sample->DrawImGui();
    ImGui::Begin("Sample Browser");
    if (ImGui::BeginCombo("Current Sample",
                          samples_[currentIndex_].sampleName.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (int i = 0; i < samples_.size(); i++)
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