#pragma once

class Feature
{
public:
    virtual ~Feature() = default;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void update() = 0;
    virtual void render() = 0;

    void set_enabled(bool value) { enabled_ = value; }
    bool enabled() const { return enabled_; }

private:
    bool enabled_ = true;
};
