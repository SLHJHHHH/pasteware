#pragma once

class GameInterfaces
{
public:
    static GameInterfaces& instance();

    bool initialize();
    void clear();

    void* surface() const;
    void* panel() const;
    void* engine_vgui() const;
    void* client_vgui() const;
    void* filesystem() const;
    void* console() const;

private:
    void* surface_ = nullptr;
    void* panel_ = nullptr;
    void* engine_vgui_ = nullptr;
    void* client_vgui_ = nullptr;
    void* filesystem_ = nullptr;
    void* console_ = nullptr;
};
