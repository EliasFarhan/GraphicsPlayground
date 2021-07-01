#pragma once
#include <set>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "component.h"

namespace core
{
struct Position
{
    glm::vec3 position;
};

struct Scale
{
    glm::vec3 scale;
};

struct Rotation
{
    glm::quat quaternion;
};

struct Transform
{
    glm::mat4 model;
};

class TransformSystem : public ComponentSystem
{
public:
    using ComponentSystem::ComponentSystem;
    void AddEntity(entt::entity newEntity) override;
    void SetPosition(entt::entity entity, glm::vec3 position);
    void SetScale(entt::entity entity, glm::vec3 scale);
    void SetEulerAngles(entt::entity entity, glm::vec3 eulerAngles);
    [[nodiscard]] glm::vec3 GetPosition(entt::entity entity) const;
    [[nodiscard]] glm::vec3 GetScale(entt::entity entity) const;
    [[nodiscard]] glm::vec3 GetEulerAngles(entt::entity entity) const;

    [[nodiscard]] glm::quat GetQuaternion(entt::entity value) const;
    void UpdateTransforms();
private:
    std::set<entt::entity> dirtyEntities_;
};
}
