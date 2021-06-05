#pragma once

#include <vector>
#include <string>
#include <memory>
#include "gl/engine.h"

namespace gl
{
class SampleBrowser : public core::Program
{
public:
    SampleBrowser();

    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    struct Sample
    {
        std::string sampleName;
        std::unique_ptr<Program> sample;

    };
    std::vector<Sample> samples_;
    int currentIndex_ = 0;
};
}