#include "GL/glew.h"
#include "hello_instancing.h"
#include <random>
#include <imgui.h>

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#include "TracyOpenGL.hpp"
#endif


namespace gl
{

void HelloInstancing::Init()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    asteroidPositions_.resize(maxAsteroidNmb_);
    asteroidForces_.resize(maxAsteroidNmb_);
    asteroidVelocities_.resize(maxAsteroidNmb_);
    //Calculate init pos and velocities
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> radiusDis(100.0f, 200.0f);
    std::uniform_real_distribution<float> angleDis(0.0f, 360.0f);
    for (size_t i = 0; i < maxAsteroidNmb_; i++)
    {
        const float radius = radiusDis(gen);
        const float angle = angleDis(gen);
        glm::vec3 position = glm::vec3(0, 0, 1);
        position = glm::angleAxis(glm::radians(angle), glm::vec3(0, 1, 0)) *
                   position;
        position *= radius;
        asteroidPositions_[i] = position;
    }

    rockModel_.LoadModel("data/model/rock/rock.obj");
    const auto& asteroidMesh = rockModel_.GetMesh(0);

    glBindVertexArray(asteroidMesh.GetVao());
    glGenBuffers(1, &instanceVBO_);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void*) 0);
    glVertexAttribDivisor(5, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    singleDrawShader_.CreateDefaultProgram(
            "data/shaders/13_hello_instancing/asteroid_single.vert",
            "data/shaders/13_hello_instancing/asteroid.frag");
    uniformInstancingShader_.CreateDefaultProgram(
            "data/shaders/13_hello_instancing/asteroid_uniform_instancing.vert",
            "data/shaders/13_hello_instancing/asteroid.frag");
    vertexInstancingDrawShader_.CreateDefaultProgram(
            "data/shaders/13_hello_instancing/asteroid_vertex_instancing.vert",
            "data/shaders/13_hello_instancing/asteroid.frag");
    camera_.Init();
    camera_.position = glm::vec3(0.0f, 500.0f, -500.0f);
    camera_.farPlane = 1'000.0f;
    camera_.LookAt(glm::vec3());
}

void HelloInstancing::Update(core::seconds dt)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(update, "Update", true);
    TracyGpuNamedZone(UpdateGpu, "Update", true);
#endif
    camera_.Update(dt);
    dt_ = dt.count();
    CalculateForce(0, asteroidNmb_);
    CalculateVelocity(0, asteroidNmb_);
    CalculatePositions(0, asteroidNmb_);

    switch (instancingType_)
    {
        case InstancingType::NO_INSTANCING:
        {

            singleDrawShader_.Bind();
            singleDrawShader_.SetMat4("view", camera_.GetView());
            singleDrawShader_.SetMat4("projection", camera_.GetProjection());

            for (size_t i = 0; i < asteroidNmb_; i++)
            {
                singleDrawShader_.SetVec3("position", asteroidPositions_[i]);
                rockModel_.Draw(singleDrawShader_);
            }
            break;
        }
        case InstancingType::UNIFORM_INSTANCING:
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(updateUniform, "Update Uniform Instancing", true);
            TracyGpuNamedZone(UpdateUniformGpu, "Update Uniform Instancing",
                              true);
#endif
            uniformInstancingShader_.Bind();
            const auto& asteroidMesh = rockModel_.GetMesh(0);
            asteroidMesh.BindTextures(uniformInstancingShader_);
            uniformInstancingShader_.SetMat4("view",
                                             camera_.GetView());
            uniformInstancingShader_.SetMat4("projection",
                                             camera_.GetProjection());

            for (std::size_t chunk = 0;
                 chunk < asteroidNmb_ / uniformChunkSize_ + 1; chunk++)
            {
                const size_t chunkBeginIndex = chunk * uniformChunkSize_;
                const size_t chunkEndIndex = std::min(asteroidNmb_,
                                                      static_cast<unsigned long>(chunk + 1) *
                                                      uniformChunkSize_);
                {
#ifdef TRACY_ENABLE
                    ZoneNamedN(uploadUniform, "Upload Uniforms", true);
                    TracyGpuNamedZone(uploadUniformGpu, "Upload Uniforms GPU",
                                      true);
#endif
                    for (size_t index = chunkBeginIndex;
                         index < chunkEndIndex; index++)
                    {
                        const std::string uniformName = "position[" +
                                                        std::to_string(index -
                                                                       chunkBeginIndex) +
                                                        "]";
                        uniformInstancingShader_.SetVec3(uniformName,
                                                         asteroidPositions_[index]);
                    }
                }
                if (chunkEndIndex > chunkBeginIndex)
                {
                    glBindVertexArray(asteroidMesh.GetVao());
                    glDrawElementsInstanced(GL_TRIANGLES,
                                            asteroidMesh.GetIndicesCount(),
                                            GL_UNSIGNED_INT, 0,
                                            chunkEndIndex - chunkBeginIndex);
                    glBindVertexArray(0);
                }
            }
            break;
        }
        case InstancingType::BUFFER_INSTANCING:
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(updateVertex, "Update Vertex Instancing", true);
            TracyGpuNamedZone(updateVertexGpu, "Update Vertex Instancing",
                              true);
#endif
            vertexInstancingDrawShader_.Bind();
            const auto& asteroidMesh = rockModel_.GetMesh(0);
            asteroidMesh.BindTextures(vertexInstancingDrawShader_);
            vertexInstancingDrawShader_.SetMat4("view",
                                                camera_.GetView());
            vertexInstancingDrawShader_.SetMat4("projection",
                                                camera_.GetProjection());

            for (std::size_t chunk = 0;
                 chunk < asteroidNmb_ / instanceChunkSize_ + 1; chunk++)
            {
                const std::size_t chunkBeginIndex = chunk * instanceChunkSize_;
                const std::size_t chunkEndIndex = std::min(asteroidNmb_,
                                                      static_cast<unsigned long>(chunk + 1) *
                                                      instanceChunkSize_);
                if (chunkEndIndex > chunkBeginIndex)
                {

                    const size_t chunkSize = chunkEndIndex - chunkBeginIndex;
                    {
#ifdef TRACY_ENABLE
                        ZoneNamedN(uploadVertex, "Upload Vertex Instancing",
                                   true);
                        TracyGpuNamedZone(uploadVertexGPU,
                                          "Upload Vertex Instancing", true);
#endif
                        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
                        glBufferData(GL_ARRAY_BUFFER,
                                     sizeof(glm::vec3) * chunkSize,
                                     &asteroidPositions_[chunkBeginIndex],
                                     GL_DYNAMIC_DRAW);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    }
                    glBindVertexArray(asteroidMesh.GetVao());
                    glDrawElementsInstanced(GL_TRIANGLES,
                                            asteroidMesh.GetIndicesCount(),
                                            GL_UNSIGNED_INT, 0,
                                            chunkSize);
                    glBindVertexArray(0);
                }
            }

            break;
        }
        default:
            break;
    }
}

void HelloInstancing::Destroy()
{
    rockModel_.Destroy();
    singleDrawShader_.Destroy();
    uniformInstancingShader_.Destroy();
    vertexInstancingDrawShader_.Destroy();
}

void HelloInstancing::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloInstancing::DrawImGui()
{
    ImGui::Begin("Instancing example");
    const char* instancingTypeNames[int(InstancingType::LENGTH)] =
            {
                    "No Instancing",
                    "Uniform Instancing",
                    "Vertex Buffer Instancing"
            };
    int currentItem = int(instancingType_);
    if (ImGui::Combo("Instancing Type", &currentItem, instancingTypeNames,
                     int(InstancingType::LENGTH)))
    {
        instancingType_ = InstancingType(currentItem);
    }
    ImGui::SliderScalar("Asteroid Nmb", ImGuiDataType_U64, &asteroidNmb_,
                        &minAsteroidNmb_, &maxAsteroidNmb_);
    if (instancingType_ == InstancingType::BUFFER_INSTANCING)
    {
        const size_t minChunkSize = 100;
        const size_t maxChunkSize = 10'000;
        ImGui::SliderScalar("Instance Chunk Size", ImGuiDataType_U64,
                            &instanceChunkSize_, &minChunkSize, &maxChunkSize);
    }
    ImGui::End();
}

void HelloInstancing::CalculateForce(unsigned long begin, unsigned long end)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const size_t endCount = std::min(end, asteroidNmb_);
    for (auto i = begin; i < endCount; i++)
    {
        const auto deltaToCenter = glm::vec3(0.0f) - asteroidPositions_[i];
        const auto r = glm::length(deltaToCenter);
        const auto force = gravityConst * centerMass * asteroidMass / (r * r);
        asteroidForces_[i] = deltaToCenter / r * force;
    }
}

void HelloInstancing::CalculateVelocity(unsigned long begin, unsigned long end)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const size_t endCount = std::min(end, asteroidNmb_);
    for (auto i = begin; i < endCount; i++)
    {
        const auto deltaToCenter = glm::vec3(0.0f) - asteroidPositions_[i];
        auto velDir = glm::vec3(-deltaToCenter.z, 0.0f, deltaToCenter.x);
        velDir = glm::normalize(velDir);

        const auto speed = std::sqrt(
                glm::length(asteroidForces_[i]) / asteroidMass *
                glm::length(deltaToCenter));
        asteroidVelocities_[i] = velDir * speed;
    }
}

void HelloInstancing::CalculatePositions(unsigned long begin, unsigned long end)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const size_t endCount = std::min(end, asteroidNmb_);
    for (auto i = begin; i < endCount; i++)
    {
        asteroidPositions_[i] += asteroidVelocities_[i] * dt_;
    }
}
}