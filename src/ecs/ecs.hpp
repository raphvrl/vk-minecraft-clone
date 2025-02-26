#pragma once

#include "core/types.hpp"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

#include "components/component.hpp"
#include "systems/system.hpp"

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

    template <typename T, typename... Args>
    T *addComponent(EntityID id, Args &&...args)
    {
        auto &componentMap = m_components[id];
        auto typeIndex = std::type_index(typeid(T));

        if (componentMap.find(typeIndex) != componentMap.end()) {
            return nullptr;
        }

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T *componentPtr = component.get();

        componentMap[typeIndex] = std::move(component);

        return componentPtr;
    }

    template <typename T>
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

    template <typename T>
    void removeComponent(EntityID id)
    {
        auto &componentMap = m_components[id];
        auto typeIndex = std::type_index(typeid(T));

        componentMap.erase(typeIndex);
    }

    template <typename T, typename... Args>
    void addSystem(Args &&...args)
    {
        m_systems.push_back(std::make_unique<T>(
            this,
            std::forward<Args>(args)...)
        );
    }

    template <typename... Components>
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

    void update(f32 dt)
    {
        for (auto &system : m_systems) {
            system->tick(dt);
        }
    }

private:
    EntityID m_nextEntityID = 0;
    
    using ComponentMap = std::unordered_map<std::type_index,
        std::unique_ptr<Component>>;

    std::unordered_map<EntityID, ComponentMap> m_components;
    std::vector<std::unique_ptr<System>> m_systems;

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