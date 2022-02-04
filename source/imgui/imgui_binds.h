//
// Created by chrshnv on 6/29/2021.
//

#ifndef COUNTER_STRIKE_SOURCE_IMGUI_EXTENDED_H
#define COUNTER_STRIKE_SOURCE_IMGUI_EXTENDED_H
#include "../imgui/imgui.h"
#include <any>
#include <string>

struct keybind_t
{
    int key;
    void* data;
    int bind_mode;
    bool is_active;
    unsigned int next_type;
    const char* config_name;
};

class FnvHash
{
    static const unsigned int FNV_PRIME = 16777619u;
    static const unsigned int OFFSET_BASIS = 2166136261u;
    template <unsigned int N>
    static constexpr unsigned int fnvHashConst(const char (&str)[N], unsigned int I = N)
    {
        return I == 1 ? (OFFSET_BASIS ^ str[0]) * FNV_PRIME : (fnvHashConst(str, I - 1) ^ str[I - 1]) * FNV_PRIME;
    }
    static unsigned int fnvHash(const char* str)
    {
        const size_t length = strlen(str) + 1;
        unsigned int hash = OFFSET_BASIS;
        for (size_t i = 0; i < length; ++i)
        {
            hash ^= *str++;
            hash *= FNV_PRIME;
        }
        return hash;
    }
    struct Wrapper
    {
        Wrapper(const char* str) : str (str) { }
        const char* str;
    };
    unsigned int hash_value;
public:
    // calulate in run-time
    FnvHash(Wrapper wrapper) : hash_value(fnvHash(wrapper.str)) { }
    // calulate in compile-time
    template <unsigned int N>
    constexpr FnvHash(const char (&str)[N]) : hash_value(fnvHashConst(str)) { }
    // output result
    constexpr operator unsigned int() const { return this->hash_value; }
};

const char* const bind_types[] = {
        "hold",
        "toggle"
};

const char* const key_names[] = {
        "none",
        "m1",
        "m2",
        "cancel",
        "m3",
        "m4",
        "m5",
        "none",
        "back",
        "tab",
        "none",
        "none",
        "clear",
        "return",
        "none",
        "none",
        "shift",
        "ctrl",
        "menu",
        "pause",
        "caps",
        "VK_KANA",
        "none",
        "VK_JUNJA",
        "VK_FINAL",
        "VK_KANJI",
        "none",
        "esc",
        "VK_CONVERT",
        "VK_NONCONVERT",
        "VK_ACCEPT",
        "VK_MODECHANGE",
        "space",
        "VK_PRIOR",
        "VK_NEXT",
        "end",
        "home",
        "left",
        "up",
        "right",
        "down",
        "VK_SELECT",
        "print",
        "VK_EXECUTE",
        "VK_SNAPSHOT",
        "insert",
        "del",
        "VK_HELP",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "VK_LWIN",
        "VK_RWIN",
        "VK_APPS",
        "none",
        "VK_SLEEP",
        "VK_NUMPAD0",
        "VK_NUMPAD1",
        "VK_NUMPAD2",
        "VK_NUMPAD3",
        "VK_NUMPAD4",
        "VK_NUMPAD5",
        "VK_NUMPAD6",
        "VK_NUMPAD7",
        "VK_NUMPAD8",
        "VK_NUMPAD9",
        "VK_MULTIPLY",
        "VK_ADD",
        "VK_SEPARATOR",
        "VK_SUBTRACT",
        "VK_DECIMAL",
        "VK_DIVIDE",
        "f1",
        "f2",
        "f3",
        "f4",
        "f5",
        "f6",
        "f7",
        "f8",
        "f9",
        "f10",
        "f11",
        "f12",
        "f13",
        "f14",
        "f15",
        "f16",
        "f17",
        "f18",
        "f19",
        "f20",
        "VK_F21",
        "VK_F22",
        "VK_F23",
        "VK_F24",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "num",
        "VK_SCROLL",
        "VK_OEM_NEC_EQUAL",
        "VK_OEM_FJ_MASSHOU",
        "VK_OEM_FJ_TOUROKU",
        "VK_OEM_FJ_LOYA",
        "VK_OEM_FJ_ROYA",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "none",
        "lshift",
        "rshift",
        "lctrl",
        "rctrl",
        "VK_LMENU",
        "VK_RMENU"
};

#include <vector>
namespace ImGui {
    bool BindCheckbox(const char* name, bool* var, int* bind_var, const char* unique_id);
    void key_bind( const std::string& name,  keybind_t* bind );
    void bind_popup(unsigned int next_type, const char* config_name, int* bind_var);
    extern std::vector<keybind_t> binds;
}

#endif //COUNTER_STRIKE_SOURCE_IMGUI_EXTENDED_H
