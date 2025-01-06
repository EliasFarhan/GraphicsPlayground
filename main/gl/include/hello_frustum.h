#pragma once

#include "glm/vec3.hpp"
#include "engine.h"
#include "gl/shader.h"
#include "gl/model.h"
#include "gl/framebuffer.h"
#include "gl/camera.h"

namespace gl
{

class HelloFrustum : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;
    
private:
    void CalculateForce(uint64_t begin, uint64_t end);
    void CalculateVelocity(uint64_t begin, uint64_t end);
    void CalculatePositions(uint64_t begin, uint64_t end);
    void Culling(unsigned long begin, unsigned long end);


    sdl::Camera3D camera_;
    Camera3D overCamera_;

    Model rockModel_;
    static constexpr uint64_t maxAsteroidNmb_ = 100'000;
    static constexpr uint64_t minAsteroidNmb_ = 1'000;
    uint64_t instanceChunkSize_ = 1'000;
    uint64_t asteroidNmb_ = 1000;


    ShaderProgram vertexInstancingDrawShader_;
    ShaderProgram screenShader_;

    std::vector<glm::vec3> asteroidPositions_;
    std::vector<glm::vec3> asteroidVelocities_;
    std::vector<glm::vec3> asteroidForces_;
    /**
     * Used by frustum culling before sending to GPU
     */
    std::vector<glm::vec3> asteroidCulledPositions_;


    unsigned int instanceVBO_ = 0;

    const float gravityConst = 1000.0f;
    const float centerMass = 1000.0f;
    const float asteroidMass = 1.0f;
    float dt_ = 1.0f;

    Framebuffer overviewFramebuffer_;
    Quad screenPlan_{glm::vec2(2.0f), glm::vec2()};
};

}