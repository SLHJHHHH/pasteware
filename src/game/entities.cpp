#include "entities.h"
#include "../core/trace.h"

#include <Psapi.h>
#include <cstring>

struct EngineApi
{
    void* ignored[51];
    cl_entity_s* (__cdecl* GetLocalPlayer)();
    void* GetViewModel;
    cl_entity_s* (__cdecl* GetEntityByIndex)(int);
};

namespace
{
    EngineApi* find_engine(HMODULE module)
    {
        MODULEINFO info{};
        if (!module || !GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info)))
            return nullptr;

        auto base = static_cast<const unsigned char*>(info.lpBaseOfDll);
        const auto size = static_cast<size_t>(info.SizeOfImage);
        const char text[] = "sprites/voiceicon.spr";
        uintptr_t string_address = 0;

        for (size_t i = 0; i + sizeof(text) <= size; ++i)
        {
            if (std::memcmp(base + i, text, sizeof(text) - 1) == 0)
            {
                string_address = reinterpret_cast<uintptr_t>(base + i);
                break;
            }
        }

        if (!string_address)
            return nullptr;

        for (size_t i = 0; i + 5 <= size; ++i)
        {
            if (base[i] != 0x68)
                continue;

            uintptr_t value = 0;
            std::memcpy(&value, base + i + 1, sizeof(value));
            if (value == string_address && i + 7 + sizeof(void*) <= size)
                return *reinterpret_cast<EngineApi* const*>(base + i + 7);
        }

        return nullptr;
    }
}

Entities& Entities::instance()
{
    static Entities entities;
    return entities;
}

bool Entities::bind(HMODULE client)
{
    engine_ = find_engine(client);
    return engine_ != nullptr;
}

void Entities::update()
{
    static bool logged = false;
    auto* engine = static_cast<EngineApi*>(engine_);
    if (!engine || !engine->GetEntityByIndex)
        return;

    for (int i = 0; i < 65; ++i)
        entities_[i] = engine->GetEntityByIndex(i);

    auto local = engine->GetLocalPlayer ? engine->GetLocalPlayer() : nullptr;
    local_index_ = 0;
    for (int i = 0; i < 65; ++i)
    {
        if (entities_[i] == local)
        {
            local_index_ = i;
            break;
        }
    }

    if (!logged)
    {
        int count = 0;
        int players = 0;
        for (int i = 0; i < 65; ++i)
        {
            if (entities_[i])
                ++count;
            if (entities_[i] && entities_[i]->player)
                ++players;
        }

        char line[128]{};
        auto local_entity = local_index_ > 0 ? entities_[local_index_] : nullptr;
        sprintf_s(line, "entity update count=%d players=%d local=%d localptr=%p origin=%.1f %.1f %.1f", count, players, local_index_, local_entity, local_entity ? local_entity->origin.x : 0.0f, local_entity ? local_entity->origin.y : 0.0f, local_entity ? local_entity->origin.z : 0.0f);
        Trace::write(line);
        logged = true;
    }
}

void Entities::set(int index, cl_entity_s* entity)
{
    if (index >= 0 && index < static_cast<int>(entities_.size()))
        entities_[index] = entity;
}

cl_entity_s* Entities::get(int index) const
{
    if (index < 0 || index >= static_cast<int>(entities_.size()))
        return nullptr;

    return entities_[index];
}

cl_entity_s* Entities::local() const
{
    return get(local_index_);
}

void Entities::set_local(int index)
{
    local_index_ = index;
}

void Entities::clear()
{
    entities_.fill(nullptr);
    local_index_ = 0;
    engine_ = nullptr;
}
