#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include "engine.h"

namespace gl
{
struct Camera
{
    virtual ~Camera() = default;

    glm::vec3 position;
    glm::vec3 direction = glm::vec3(0, 0, 1), leftDir = glm::vec3(-1, 0, 0), upDir = glm::vec3(0, 1, 0);
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    void Rotate(glm::vec3 eulerAngle);
    glm::mat4 GetView() const;
    void LookAt(glm::vec3 target, glm::vec3 lookUp = glm::vec3(0,-1,0));
    virtual glm::mat4 GetProjection() const = 0;
};

struct Camera2D : Camera
{
    float right = 0.0f, left = 0.0f, top = 0.0f, bottom = 0.0f;

    glm::mat4 GetProjection() const override;

    void SetExtends(glm::vec2 size);
};

struct Camera3D : Camera
{
    float aspect = 1.0f;
    /**
     * \brief Field of view with angle in degrees
     */
    float fovY = 45.0f;

    glm::mat4 GetProjection() const override;

    void SetAspect(glm::vec2 size);

    [[nodiscard]] float GetFovX() const;

};

namespace sdl
{
class Camera3D : public gl::Camera3D, public core::SystemInterface, public core::SdlEventSystemInterface
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(const SDL_Event& event) override;

protected:
    enum CameraMovement : std::uint8_t
    {
        NONE = 0u,
        UP = 1u << 0u,
        DOWN = 1u << 1u,
        LEFT = 1u << 2u,
        RIGHT = 1u << 3u,
        DISABLE = 1u << 4u,
        ACCELERATE = 1u << 5u,
        MOUSE_MOVE = 1u << 6u
    };
    static constexpr float cameraSpeed_ = 3.0f;
    static constexpr float cameraRotationSpeed_{ 10.0f };
    static constexpr float cameraFast_ = 7.0f;
    std::uint8_t cameraMovement_ = NONE;
    static constexpr  glm::vec3 cameraOriginPos{0.0f, 3.0f, -3.0f};
    const glm::quat cameraOriginAngles = glm::quat(glm::vec3(
            -45.0f, 0.0f, 0.0f));
    glm::vec2 mouseMotion_{};

    static constexpr float mouseMotionRatio_ = 25.0f;

};
}
}