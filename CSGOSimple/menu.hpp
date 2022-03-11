#pragma once

#include <string>
#include "singleton.hpp"
#include "imgui/imgui.h"

struct IDirect3DDevice9;

class Menu
    : public Singleton<Menu>
{
public:
    float public_alpha = 0;
    ImFont* weaponfont;

    void Initialize();
    void Shutdown();

    void OnDeviceLost();
    void OnDeviceReset();

    void Render();
    void Toggle();

    bool IsVisible() const { return _visible; }

private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;
};