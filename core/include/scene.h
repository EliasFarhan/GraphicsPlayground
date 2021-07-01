#pragma once

#include <entt/entt.hpp>

namespace core
{
struct SceneTree
{
    entt::handle parent;
    entt::handle child;
    entt::handle sibling;
};

}