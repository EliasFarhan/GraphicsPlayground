#pragma once
#include <entt/entity/registry.hpp>


namespace core
{
    class ComponentSystem
    {
    public:
        virtual ~ComponentSystem() = default;
        ComponentSystem(entt::registry& registry);
        virtual void AddEntity(entt::entity newEntity) = 0;
    protected:
        entt::registry& registry_;

    };
}
