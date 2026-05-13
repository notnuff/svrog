#pragma once

#include <list>
#include <memory>
#include <optional>
#include <stdexcept>

#include "i_vk_builder.h"

namespace nuff::renderer {

class BuilderChain;

class ChainLink {
    friend class BuilderChain;

    using ListType = std::list<std::unique_ptr<IVkBuilder>>;
    using Iterator = ListType::iterator;

    ChainLink(Iterator it, ListType& list)
        : m_it(it), m_list(&list) {}

public:
    ChainLink(const ChainLink&) = default;
    ChainLink& operator=(const ChainLink&) = default;

    template <typename BuilderType, typename... Args>
    ChainLink insert_after(Args&&... args) {
        static_assert(std::is_base_of_v<IVkBuilder, BuilderType>,
            "builder must derive from IVkBuilder");

        auto ptr = std::make_unique<BuilderType>(std::forward<Args>(args)...);
        auto nextIt = std::next(m_it);
        auto inserted = m_list->insert(nextIt, std::move(ptr));
        return ChainLink(inserted, *m_list);
    }

    template <typename BuilderType, typename... Args>
    ChainLink insert_before(Args&&... args) {
        static_assert(std::is_base_of_v<IVkBuilder, BuilderType>,
            "builder must derive from IVkBuilder");

        auto ptr = std::make_unique<BuilderType>(std::forward<Args>(args)...);
        auto inserted = m_list->insert(m_it, std::move(ptr));
        return ChainLink(inserted, *m_list);
    }

    ChainLink next() const {
        auto nextIt = std::next(m_it);
        if (nextIt == m_list->end()) {
            throw std::out_of_range("ChainLink::next() called on last element");
        }
        return ChainLink(nextIt, *m_list);
    }

    ChainLink prev() const {
        if (m_it == m_list->begin()) {
            throw std::out_of_range("ChainLink::prev() called on first element");
        }
        return ChainLink(std::prev(m_it), *m_list);
    }

    bool has_next() const { return std::next(m_it) != m_list->end(); }
    bool has_prev() const { return m_it != m_list->begin(); }

    template <typename T>
    T& as() {
        auto* casted = dynamic_cast<T*>(m_it->get());
        if (!casted) {
            throw std::bad_cast();
        }
        return *casted;
    }

    IVkBuilder& operator*() const { return **m_it; }
    IVkBuilder* operator->() const { return m_it->get(); }

private:
    Iterator m_it;
    ListType* m_list;
};


class BuilderChain {
    using ListType = std::list<std::unique_ptr<IVkBuilder>>;

public:
    template <typename BuilderType, typename... Args>
    ChainLink insert_back(Args&&... args) {
        static_assert(std::is_base_of_v<IVkBuilder, BuilderType>,
            "builder must derive from IVkBuilder");

        auto ptr = std::make_unique<BuilderType>(std::forward<Args>(args)...);
        m_builders.push_back(std::move(ptr));
        return ChainLink(std::prev(m_builders.end()), m_builders);
    }

    template <typename BuilderType, typename... Args>
    ChainLink insert_front(Args&&... args) {
        static_assert(std::is_base_of_v<IVkBuilder, BuilderType>,
            "builder must derive from IVkBuilder");

        auto ptr = std::make_unique<BuilderType>(std::forward<Args>(args)...);
        m_builders.push_front(std::move(ptr));
        return ChainLink(m_builders.begin(), m_builders);
    }

    template <typename BuilderType>
    std::optional<ChainLink> get() {
        for (auto it = m_builders.begin(); it != m_builders.end(); ++it) {
            if (dynamic_cast<BuilderType*>(it->get())) {
                return ChainLink(it, m_builders);
            }
        }
        return std::nullopt;
    }

    auto begin() { return m_builders.begin(); }
    auto end() { return m_builders.end(); }
    auto begin() const { return m_builders.begin(); }
    auto end() const { return m_builders.end(); }

private:
    ListType m_builders;
};

} // namespace nuff::renderer

