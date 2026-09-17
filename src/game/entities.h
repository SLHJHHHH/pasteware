#pragma once

#include "types.h"

#include <array>

class Entities
{
public:
    static Entities& instance();

    bool bind(HMODULE client);
    void update();
    void set(int index, cl_entity_s* entity);
    cl_entity_s* get(int index) const;
    cl_entity_s* local() const;
    void set_local(int index);
    void clear();

private:
    std::array<cl_entity_s*, 65> entities_{};
    int local_index_ = 0;
    void* engine_ = nullptr;
};
