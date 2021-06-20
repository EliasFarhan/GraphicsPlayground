#include "GL/glew.h"
#include "hello_frustum.h"
#include <random>
#include "imgui.h"

namespace gl
{

void HelloFrustum::Init()
{
    rockModel_.LoadModel("data/model/rock/rock.obj");
    asteroidCulledPositions_.resize(maxAsteroidNmb_);
    asteroidForces_.resize(maxAsteroidNmb_);
    asteroidVelocities_.resize(maxAsteroidNmb_);
    asteroidPositions_.resize(maxAsteroidNmb_);
    //Calculate init pos and velocities
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> radiusDis(20.0f, 300.0f);
    std::uniform_real_distribution<float> angleDis(0.0f, 360.0f);
    for (std::size_t i = 0; i < maxAsteroidNmb_; i++)
    {
        const float radius = radiusDis(gen);
        const float angle = angleDis(gen);
        glm::vec3 position = glm::vec3(0, 0, 1);
        position = glm::angleAxis(glm::radians(angle), glm::vec3(0, 1, 0)) *
                   position;
        position *= radius;
        asteroidPositions_[i] = position;
    }

    vertexInstancingDrawShader_.CreateDefaultProgram(
            "data/shaders/14_hello_frustum/asteroid_vertex_instancing.vert",
            "data/shaders/14_hello_frustum/asteroid.frag");
    screenShader_.CreateDefaultProgram("data/shaders/14_hello_frustum/screen.vert",
                                       "data/shaders/14_hello_frustum/screen.frag");

    camera_.Init();
    camera_.position = glm::vec3(0.0f, 600.0f, -500.0f);
    camera_.farPlane = 1'500.0f;
    camera_.LookAt(glm::vec3());

    overCamera_ = camera_;
    overCamera_.SetAspect({1024, 1024});

    screenPlan_.Init();

    overviewFramebuffer_.SetSize({1024, 1024});
    overviewFramebuffer_.Create();

    const auto& mesh = rockModel_.GetMesh(0);
    glBindVertexArray(mesh.GetVao());
    glGenBuffers(1, &instanceVBO_);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);
    glVertexAttribDivisor(5, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void HelloFrustum::Update(core::seconds dt)
{
    camera_.Update(dt);
    dt_ = dt.count();
    asteroidCulledPositions_.clear();
    CalculateForce(0, asteroidNmb_);
    CalculateVelocity(0, asteroidNmb_);
    CalculatePositions(0, asteroidNmb_);
    Culling(0, asteroidNmb_);

    vertexInstancingDrawShader_.Bind();

    const auto& asteroidMesh = rockModel_.GetMesh(0);
    asteroidMesh.BindTextures(vertexInstancingDrawShader_);
    const auto drawAsteroids = [this, &asteroidMesh]() {
        const auto actualAsteroidNmb = asteroidCulledPositions_.size();

        for (std::size_t chunk = 0; chunk < actualAsteroidNmb / instanceChunkSize_ + 1; chunk++)
        {
            const std::size_t chunkBeginIndex = chunk * instanceChunkSize_;
            const std::size_t chunkEndIndex = std::min(actualAsteroidNmb, (chunk + 1) * instanceChunkSize_);
            if (chunkEndIndex > chunkBeginIndex)
            {
                const std::size_t chunkSize = chunkEndIndex - chunkBeginIndex;

                glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * chunkSize, &asteroidCulledPositions_[chunkBeginIndex],
                             GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(asteroidMesh.GetVao());
                glDrawElementsInstanced(GL_TRIANGLES, asteroidMesh.GetIndicesCount(), GL_UNSIGNED_INT, 0,
                                        chunkSize);
                glBindVertexArray(0);
            }
        }
    };
    // draw in the mini frame
    overviewFramebuffer_.Bind();
    const auto framebufferSize = overviewFramebuffer_.GetSize();
    glViewport(0, 0, framebufferSize.x, framebufferSize.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    vertexInstancingDrawShader_.SetMat4("view", overCamera_.GetView());
    vertexInstancingDrawShader_.SetMat4("projection", overCamera_.GetProjection());
    drawAsteroids();

    Framebuffer::Unbind();
    const auto screenSize = Engine::GetInstance().GetWindowSize();
    glViewport(0, 0, screenSize[0], screenSize[1]);
    vertexInstancingDrawShader_.SetMat4("view", camera_.GetView());
    vertexInstancingDrawShader_.SetMat4("projection", camera_.GetProjection());
    drawAsteroids();

    //Draw the mini view on top left
    glDisable(GL_DEPTH_TEST);
    screenShader_.Bind();
    const float miniMapSize = 0.2f;
    screenShader_.SetVec2("offset", glm::vec2((1.0f - miniMapSize / camera_.aspect), 1.0f - miniMapSize));
    screenShader_.SetVec2("scale", glm::vec2(miniMapSize / camera_.aspect, miniMapSize));

    screenShader_.SetInt("screenTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, overviewFramebuffer_.GetColorTexture());
    screenPlan_.Draw();
    glEnable(GL_DEPTH_TEST);
}

void HelloFrustum::Destroy()
{
    vertexInstancingDrawShader_.Destroy();
    screenShader_.Destroy();
    screenPlan_.Destroy();
    overviewFramebuffer_.Destroy();
    rockModel_.Destroy();
}

void HelloFrustum::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloFrustum::DrawImGui()
{
    ImGui::Begin("Frustum Culling");

    ImGui::SliderScalar("Asteroid Nmb", ImGuiDataType_U64, &asteroidNmb_, &minAsteroidNmb_, &maxAsteroidNmb_);

    const unsigned long minChunkSize = 100;
    const unsigned long maxChunkSize = 10'000;
    ImGui::SliderScalar("Instance Chunk Size", ImGuiDataType_U64, &instanceChunkSize_, &minChunkSize, &maxChunkSize);
    ImGui::LabelText("Asteroid Actual Nmb", "%zu", asteroidCulledPositions_.size());
    ImGui::End();
}

void HelloFrustum::CalculateForce(unsigned long begin, unsigned long end)
{
    const unsigned long endCount = std::min(end, asteroidNmb_);
    for (auto i = begin; i < endCount; i++)
    {
        const auto deltaToCenter = glm::vec3(0.0f) - asteroidPositions_[i];
        const auto r = glm::length(deltaToCenter);
        const auto force = gravityConst * centerMass * asteroidMass / (r * r);
        asteroidForces_[i] = deltaToCenter / r * force;
    }
}

void HelloFrustum::CalculateVelocity(unsigned long begin, unsigned long end)
{
    const size_t endCount = std::min(end, asteroidNmb_);
    for (auto i = begin; i < endCount; i++)
    {
        const auto deltaToCenter = glm::vec3() - asteroidPositions_[i];
        auto velDir = glm::vec3(-deltaToCenter.z, 0.0f, deltaToCenter.x);
        velDir = glm::normalize(velDir);

        const auto speed = std::sqrt(glm::length(asteroidForces_[i]) / asteroidMass * glm::length(deltaToCenter));
        asteroidVelocities_[i] = velDir * speed;
    }
}

void HelloFrustum::CalculatePositions(unsigned long begin, unsigned long end)
{
    const size_t endCount = std::min(end, asteroidNmb_);
    for (auto i = begin; i < endCount; i++)
    {
        asteroidPositions_[i] += asteroidVelocities_[i] * dt_;
    }
}

void HelloFrustum::Culling(unsigned long begin, unsigned long end)
{
    const auto& asteroidMesh = rockModel_.GetMesh(0);
    const auto asteroidRadius = glm::length(asteroidMesh.GetMax() - asteroidMesh.GetMin()) / 2.0f;
    const auto cameraDir = camera_.direction;
    const auto cameraLeftDir = camera_.leftDir;
    const auto cameraUp = camera_.upDir;
    const auto fovX = glm::radians(camera_.GetFovX());

    const auto leftQuaternion = glm::angleAxis(fovX / 2.0f, cameraUp);
    const auto leftNormal = leftQuaternion * cameraLeftDir;
    const auto rightQuaternion = glm::angleAxis(-fovX / 2.0f, cameraUp);
    const auto rightNormal = rightQuaternion * -cameraLeftDir;
    const auto topQuaternion = glm::angleAxis(glm::radians(-camera_.fovY) / 2.0f, cameraLeftDir);
    const auto topNormal = topQuaternion * cameraUp;
    const auto bottomQuaternion = glm::angleAxis(glm::radians(camera_.fovY) / 2.0f, cameraLeftDir);
    const auto bottomNormal = bottomQuaternion * -cameraUp;

    //asteroidCulledPositions_ = asteroidPositions_;
    //return;
    for (auto i = begin; i < end; i++)
    {
        const auto asteroidPos = asteroidPositions_[i];
        //Near
        {
            const auto planePos = camera_.position + cameraDir * camera_.nearPlane;
            const auto asterPos = asteroidPos - planePos;
            const auto v = glm::dot(cameraDir, asterPos);
            if (v < -asteroidRadius)
                continue;
        }
        //Far
        {
            const auto planePos = camera_.position + cameraDir * camera_.farPlane;
            const auto asterPos = asteroidPos - planePos;
            const auto v = glm::dot(-camera_.direction, asterPos);
            if (v < -asteroidRadius)
                continue;
        }
        const auto asterPos = asteroidPos - camera_.position;
        //Left
        {
            const auto v = glm::dot(leftNormal, asterPos);
            if (v > asteroidRadius)
            {
                continue;
            }
        }
        //Right
        {
            const auto v = glm::dot(rightNormal, asterPos);
            if (v > asteroidRadius)
            {
                continue;
            }
        }
        //Top
        {


            const auto v = glm::dot(topNormal, asterPos);
            if (v > asteroidRadius)
            {
                continue;
            }
        }
        //Bottom
        {
            const auto v = glm::dot(bottomNormal, asterPos);
            if (v > asteroidRadius)
            {
                continue;
            }
        }

        asteroidCulledPositions_.push_back(asteroidPositions_[i]);
    }
}
}