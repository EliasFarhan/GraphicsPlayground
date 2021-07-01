#include <transform.h>
#include <entt/entt.hpp>

namespace core
{
void TransformSystem::AddEntity(entt::entity newEntity)
{
    registry_.emplace<Position>(newEntity, glm::vec3());
    registry_.emplace<Scale>(newEntity, glm::vec3(1.0f));
    registry_.emplace<Rotation>(newEntity, glm::quat(1,0,0,0));
    registry_.emplace<Transform>(newEntity, glm::mat4(1.0f));
}

void TransformSystem::SetPosition(entt::entity entity, glm::vec3 position)
{
    dirtyEntities_.emplace(entity);
    registry_.patch<Position>(entity, [&position](auto& pos)
    {
        pos.position = position;
    });
}

void TransformSystem::SetScale(entt::entity entity, glm::vec3 scale)
{
    dirtyEntities_.emplace(entity);
    registry_.patch<Scale>(entity, [&scale](auto& s)
        {
            s.scale = scale;
        });
}

void TransformSystem::SetEulerAngles(entt::entity entity, glm::vec3 eulerAngles)
{
    dirtyEntities_.emplace(entity);
    const auto quat = glm::quat(eulerAngles);
    registry_.patch<Rotation>(entity, [&quat](auto& rotation)
        {
            rotation.quaternion = quat;
        });
}

glm::vec3 TransformSystem::GetPosition(entt::entity entity) const
{
    return registry_.get<Position>(entity).position;
}

glm::vec3 TransformSystem::GetScale(entt::entity entity) const
{
    return registry_.get<Scale>(entity).scale;
}

glm::vec3 TransformSystem::GetEulerAngles(entt::entity entity) const
{
    return glm::eulerAngles(GetQuaternion(entity));
}

glm::quat TransformSystem::GetQuaternion(entt::entity entity) const
{
    return registry_.get<Rotation>(entity).quaternion;
}

void TransformSystem::UpdateTransforms()
{
    for(auto entity: dirtyEntities_)
    {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, GetPosition(entity));
        model = glm::scale(model, GetScale(entity));
        model = glm::mat4_cast(GetQuaternion(entity)) * model;
        registry_.patch<Transform>(entity, [&model](auto& t)
            {
                t.model = model;
            });
    }
    dirtyEntities_.clear();
}
}
