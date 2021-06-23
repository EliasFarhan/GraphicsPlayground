#pragma once

#include "engine.h"

namespace gl
{
class HelloInstancing : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    enum class InstancingType
    {
        NO_INSTANCING,
        UNIFORM_INSTANCING,
        BUFFER_INSTANCING,
        LENGTH
    };
    void CalculateForce(unsigned long long begin, unsigned long long end);
    void CalculateVelocity(unsigned long long begin, unsigned long long end);
    void CalculatePositions(unsigned long long begin, unsigned long long end);

    InstancingType instancingType_ = InstancingType::NO_INSTANCING;

    sdl::Camera3D camera_;
    Model rockModel_;

    const unsigned long long maxAsteroidNmb_ = 100'000;
    const unsigned long long minAsteroidNmb_ = 1'000;
    const unsigned long long uniformChunkSize_ = 254;
    unsigned long long instanceChunkSize_ = 1'000;
    unsigned long long asteroidNmb_ = 1000;

    ShaderProgram singleDrawShader_;
    ShaderProgram uniformInstancingShader_;
    ShaderProgram vertexInstancingDrawShader_;

    std::vector<glm::vec3> asteroidPositions_;
    std::vector<glm::vec3> asteroidVelocities_;
    std::vector<glm::vec3> asteroidForces_;

    unsigned int instanceVBO_ = 0;

    const float gravityConst = 1000.0f;
    const float centerMass = 1000.0f;
    const float asteroidMass = 1.0f;
    float dt_ = 1.0f;
};
}