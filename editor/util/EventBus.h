// EventBus.h
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include "Entity.h"
#include "Catalog.h"   // for ComponentType

namespace Supernova::Editor {

    enum class EventType {
        EntityCreated,
        EntityDestroyed,
        ComponentAdded,
        ComponentRemoved,
        ComponentChanged
    };

    // A generic event payload
    struct Event {
        EventType type;
        uint32_t sceneId;
        Entity entity;
        std::vector<std::string> properties;
        ComponentType compType;   // only meaningful for component‐events
    };

    class EventBus {
    public:
        using Handler = std::function<void(const Event&)>;

        // Subscribe to a specific event kind
        void subscribe(EventType type, Handler cb) {
            handlers_[type].push_back(std::move(cb));
        }

        // Broadcast an event to all subscribers of that kind
        void publish(const Event& e) {
            auto it = handlers_.find(e.type);
            if (it == handlers_.end()) return;
            for (auto &h : it->second) h(e);
        }

    private:
        std::unordered_map<EventType, std::vector<Handler>> handlers_;
    };

} // ns Supernova