//
// Created by efarhan on 6/5/21.
//

#include "gl/camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/core.h>

#include "log.h"

namespace gl
{

glm::mat4 Camera::GetView() const
{
    return glm::lookAt(position, position + direction, upDir);
}

void Camera::LookAt(glm::vec3 target, glm::vec3 lookUp)
{
    direction = glm::normalize(target-position);
    leftDir = glm::normalize(glm::cross(direction, lookUp));
    upDir = glm::normalize(glm::cross(direction, leftDir));
}

void Camera::Rotate(glm::vec3 eulerAngle)
{
    const auto quaternion = glm::quat(eulerAngle);
    direction = quaternion * direction;
    LookAt(position + direction);
}

glm::mat4 Camera2D::GetProjection() const
{
    return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
}

void Camera2D::SetExtends(glm::vec2 size)
{
    left = -size.x / 2.0f;
    right = size.x / 2.0f;
    top = size.y / 2.0f;
    bottom = -size.y / 2.0f;
}

glm::mat4 Camera3D::GetProjection() const
{
    return glm::perspective(fovY, aspect, nearPlane, farPlane);
}

void Camera3D::SetAspect(glm::vec2 size)
{
    aspect = size.x / size.y;
}

float Camera3D::GetFovX() const
{
    return fovY * aspect;
}

namespace sdl
{
void Camera3D::Init()
{
    position = cameraOriginPos;
    LookAt(glm::vec3());
}

void Camera3D::Update(core::seconds dt)
{

    //Checking if keys are down
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    cameraMovement_ =
            keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A] ?
            cameraMovement_ | LEFT :
            cameraMovement_ & ~LEFT;
    cameraMovement_ =
            keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D] ?
            cameraMovement_ | RIGHT :
            cameraMovement_ & ~RIGHT;
    cameraMovement_ =
            keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W] ?
            cameraMovement_ | UP :
            cameraMovement_ & ~UP;
    cameraMovement_ =
            keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S] ?
            cameraMovement_ | DOWN :
            cameraMovement_ & ~DOWN;
    cameraMovement_ =
            keys[SDL_SCANCODE_LSHIFT] ?
            cameraMovement_ | ACCELERATE :
            cameraMovement_ & ~ACCELERATE;
    glm::vec2 cameraMove{};
    if (cameraMovement_ & LEFT)
    {
        cameraMove.x -= 1.0f * dt.count();
    }
    if (cameraMovement_ & RIGHT)
    {
        cameraMove.x += 1.0f * dt.count();
    }
    if (cameraMovement_ & UP)
    {
        cameraMove.y += 1.0f * dt.count();
    }
    if (cameraMovement_ & DOWN)
    {
        cameraMove.y -= 1.0f * dt.count();
    }

    if (SDL_IsGameController(0))
    {
        auto* controller = SDL_GameControllerOpen(0);
        const auto move = glm::vec2(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX),
                                -SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY)) * (1.0f/std::numeric_limits<short>::max());
        if (glm::dot(move, move) > 0.1f)
        {
            cameraMove += move * dt.count() * cameraSpeed_;
        }
        const short rightX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        const short rightY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);

        auto rotate = glm::vec2 (rightX, rightY) * (1.0f / std::numeric_limits<short>::max());
        if (glm::dot(rotate, rotate) > 0.1f)
        {
            rotate = rotate * cameraRotationSpeed_ * dt.count();
            Rotate(glm::vec3 (rotate.y,-rotate.x,0.0f)) ;
        }
        SDL_GameControllerClose(controller);
    }

    position +=
            (-leftDir * cameraMove.x -
             -direction * cameraMove.y) *
            (cameraMovement_ & ACCELERATE ? cameraFast_ : cameraSpeed_);

    const auto mouseState = SDL_GetMouseState(nullptr, nullptr);
    cameraMovement_ = mouseState & SDL_BUTTON(3) ?
                      cameraMovement_ | MOUSE_MOVE :
                      cameraMovement_ & ~MOUSE_MOVE;
    //core::LogDebug(fmt::format("Mouse movements {} {} clicked on button: {}", mouseMotion_.x, mouseMotion_.y, cameraMovement_));
    if (cameraMovement_ & MOUSE_MOVE && glm::dot(mouseMotion_, mouseMotion_) > 0.001f)
    {
        const auto rotate = glm::vec2(mouseMotion_.x, mouseMotion_.y)  * cameraRotationSpeed_ * dt.count();
        Rotate(glm::vec3 (rotate.y,-rotate.x,0.0f));
        mouseMotion_ = glm::vec2();
    }
}

void Camera3D::Destroy()
{

}

void Camera3D::OnEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_WINDOWEVENT:
    {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            SetAspect(glm::vec2(event.window.data1, event.window.data2));
        }
        break;
    }
    case SDL_MOUSEMOTION:
    {
        mouseMotion_ = glm::vec2(event.motion.xrel, event.motion.yrel) / mouseMotionRatio_;
        break;
    }
    case SDL_FINGERDOWN:
        break;
    case SDL_FINGERUP:
        break;
    case SDL_FINGERMOTION:
        break;
    default:
        break;
    }
}
}
}