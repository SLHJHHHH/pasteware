#pragma once

#include <string>
#include <vector>

class Menu
{
public:
    static Menu& instance();

    void initialize();
    void toggle();
    void set_open(bool value);
    void select(std::size_t index);

    bool open() const;
    std::size_t selected() const;
    const std::vector<std::string>& tabs() const;

private:
    bool open_ = false;
    std::size_t selected_ = 0;
    std::vector<std::string> tabs_;
};
