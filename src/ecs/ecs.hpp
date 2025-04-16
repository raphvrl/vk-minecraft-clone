#pragma once

#include "core/types.hpp"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

#include "components/component.hpp"
#include "systems/system.hpp"

#include "components/physics/transform_component.hpp"

namespace ecs
{

class ECS
{
public:
    EntityID creatEntity()
    {
        return m_nextEntityID++;
    }

    void destroyEntity(EntityID id)
    {
        m_components.erase(id);
    }

    template<typename T>
    T *addComponent(EntityID id)
    {
        auto &componentMap = m_components[id];
        auto typeIndex = std::type_index(typeid(T));

        auto component = std::make_unique<T>();
        componentMap[typeIndex] = std::move(component);

        return static_cast<T *>(componentMap[typeIndex].get());
    }

    template<typename T>
    T *getComponent(EntityID id)
    {
        auto &componentMap = m_components[id];
        auto typeIndex = std::type_index(typeid(T));

        auto iter = componentMap.find(typeIndex);
        if (iter == componentMap.end()) {
            return nullptr;
        }

        return static_cast<T *>(iter->second.get());
    }

    template<typename T>
    void removeComponent(EntityID id)
    {
        auto &componentMap = m_components[id];
        auto typeIndex = std::type_index(typeid(T));

        componentMap.erase(typeIndex);
    }

    template<typename... Components>
    std::vector<EntityID> view()
    {
        std::vector<EntityID> entities;

        for (auto &[id, componentMap] : m_components) {
            if (hasAllComponents<Components...>(id)) {
                entities.push_back(id);
            }
        }

        return entities;
    }

    void storePositions()
    {
        auto entities = view<ecs::TransformComponent>();

        for (auto &entity : entities) {
            if (auto transform = getComponent<ecs::TransformComponent>(entity)) {
                transform->prevPosition = transform->position;
            }
        }
    }

    void interpolate(f32 alpha)
    {
        auto entities = view<ecs::TransformComponent>();

        for (auto &entity : entities) {
            if (auto transform = getComponent<ecs::TransformComponent>(entity)) {
                transform->renderPosition = glm::mix(
                    transform->prevPosition,
                    transform->position,
                    alpha
                );
            }
        }
    }

private:
    EntityID m_nextEntityID = 0;
    
    using ComponentMap = std::unordered_map<std::type_index,
        std::unique_ptr<Component>>;

    std::unordered_map<EntityID, ComponentMap> m_components;

    template <typename T>
    bool hasAllComponents(EntityID id)
    {
        return getComponent<T>(id) != nullptr;
    }

    template <typename First, typename Second, typename... Rest>
    bool hasAllComponents(EntityID id)
    {
        return getComponent<First>(id) != nullptr &&
            hasAllComponents<Second, Rest...>(id);
    }
};

} // namespace ecs