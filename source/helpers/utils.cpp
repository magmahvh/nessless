#include "Utils.hpp"

#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "../valve_sdk/csgostructs.hpp"
#include "Math.hpp"
#include <Psapi.h>


HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;

#define IS_IN_RANGE( value, max, min ) ( value >= max && value <= min )
#define GET_BITS( value ) ( IS_IN_RANGE( value, '0', '9' ) ? ( value - '0' ) : ( ( value & ( ~0x20 ) ) - 'A' + 0xA ) )
#define GET_BYTE( value ) ( GET_BITS( value[0] ) << 4 | GET_BITS( value[1] ) )
#define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))

namespace Utils {
	std::vector<char> HexToBytes(const std::string& hex) {
		std::vector<char> res;

		for (auto i = 0u; i < hex.length(); i += 2) {
			std::string byteString = hex.substr(i, 2);
			char byte = (char)strtol(byteString.c_str(), NULL, 16);
			res.push_back(byte);
		}

		return res;
	}
	std::string BytesToString(unsigned char* data, int len) {
		constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		std::string res(len * 2, ' ');
		for (int i = 0; i < len; ++i) {
			res[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
			res[2 * i + 1] = hexmap[data[i] & 0x0F];
		}
		return res;
	}
	std::vector<std::string> Split(const std::string& str, const char* delim) {
		std::vector<std::string> res;
		char* pTempStr = _strdup(str.c_str());
		char* context = NULL;
		char* pWord = strtok_s(pTempStr, delim, &context);
		while (pWord != NULL) {
			res.push_back(pWord);
			pWord = strtok_s(NULL, delim, &context);
		}

		free(pTempStr);

		return res;
	}

	unsigned int FindInDataMap(datamap_t *pMap, const char *name) {
		while (pMap) {
			for (int i = 0; i<pMap->dataNumFields; i++) {
				if (pMap->dataDesc[i].fieldName == NULL)
					continue;

				if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
					return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

				if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED) {
					if (pMap->dataDesc[i].td) {
						unsigned int offset;

						if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
							return offset;
					}
				}
			}
			pMap = pMap->baseMap;
		}

		return 0;
	}
    /*
     * @brief Create console
     *
     * Create and attach a console window to the current process
     */
    void AttachConsole()
    {
        _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
        _old_err = GetStdHandle(STD_ERROR_HANDLE);
        _old_in  = GetStdHandle(STD_INPUT_HANDLE);

        ::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

        _out     = GetStdHandle(STD_OUTPUT_HANDLE);
        _err     = GetStdHandle(STD_ERROR_HANDLE);
        _in      = GetStdHandle(STD_INPUT_HANDLE);

        SetConsoleMode(_out,
            ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

        SetConsoleMode(_in,
            ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
            ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
    }

    /*
     * @brief Detach console
     *
     * Detach and destroy the attached console
     */
    void DetachConsole()
    {
        if(_out && _err && _in) {
            FreeConsole();

            if(_old_out)
                SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
            if(_old_err)
                SetStdHandle(STD_ERROR_HANDLE, _old_err);
            if(_old_in)
                SetStdHandle(STD_INPUT_HANDLE, _old_in);
        }
    }

    /*
     * @brief Print to console
     *
     * Replacement to printf that works with the newly created console
     */
    bool ConsolePrint(const char* fmt, ...)
    {
        if(!_out) 
            return false;

        char buf[1024];
        va_list va;

        va_start(va, fmt);
        _vsnprintf_s(buf, 1024, fmt, va);
        va_end(va);

        return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
    }

    /*
     * @brief Blocks execution until a key is pressed on the console window
     *
     */
    char ConsoleReadKey()
    {
        if(!_in)
            return false;

        auto key = char{ 0 };
        auto keysread = DWORD{ 0 };

        ReadConsoleA(_in, &key, 1, &keysread, nullptr);

        return key;
    }


    /*
     * @brief Wait for all the given modules to be loaded
     *
     * @param timeout How long to wait
     * @param modules List of modules to wait for
     *
     * @returns See WaitForSingleObject return values.
     */
    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules)
    {
        bool signaled[32] = { 0 };
        bool success = false;

        std::uint32_t totalSlept = 0;

        if(timeout == 0) {
            for(auto& mod : modules) {
                if(GetModuleHandleW(std::data(mod)) == NULL)
                    return WAIT_TIMEOUT;
            }
            return WAIT_OBJECT_0;
        }

        if(timeout < 0)
            timeout = INT32_MAX;

        while(true) {
            for(auto i = 0u; i < modules.size(); ++i) {
                auto& module = *(modules.begin() + i);
                if(!signaled[i] && GetModuleHandleW(std::data(module)) != NULL) {
                    signaled[i] = true;

                    //
                    // Checks if all modules are signaled
                    //
                    bool done = true;
                    for(auto j = 0u; j < modules.size(); ++j) {
                        if(!signaled[j]) {
                            done = false;
                            break;
                        }
                    }
                    if(done) {
                        success = true;
                        goto exit;
                    }
                }
            }
            if(totalSlept > std::uint32_t(timeout)) {
                break;
            }
            Sleep(10);
            totalSlept += 10;
        }

    exit:
        return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }

    /*
     * @brief Scan for a given byte pattern on a module
     *
     * @param module    Base of the module to search
     * @param signature IDA-style byte array pattern
     *
     * @returns Address of the first occurence
     */
    std::uint8_t* PatternScan(void* module, const char* signature)
    {
        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for(auto current = start; current < end; ++current) {
                if(*current == '?') {
                    ++current;
                    if(*current == '?')
                        ++current;
                    bytes.push_back(-1);
                } else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for(auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for(auto j = 0ul; j < s; ++j) {
                if(scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if(found) {
                return &scanBytes[i];
            }
        }
        return nullptr;
    }

    std::uintptr_t Scan(const std::string& image_name, const std::string& signature)
    {
        auto image = GetModuleHandleA(image_name.c_str());

        if (!image)
        {
#ifdef DEBUG
            Win32::Error("GetModuleHandleA failed (L3D451R7::Memory::%s)", __FUNCTION__);
#endif // DEBUG
            return 0u;
        }

        auto image_base = (std::uintptr_t)(image);
        auto image_dos_hdr = (IMAGE_DOS_HEADER*)(image_base);

        if (image_dos_hdr->e_magic != IMAGE_DOS_SIGNATURE)
        {
#ifdef DEBUG
            Win32::Error("IMAGE_DOS_HEADER::e_magic is invalid (L3D451R7::Memory::%s)", __FUNCTION__);
#endif // DEBUG
            return 0u;
        }

        auto image_nt_hdrs = (IMAGE_NT_HEADERS*)(image_base + image_dos_hdr->e_lfanew);

        if (image_nt_hdrs->Signature != IMAGE_NT_SIGNATURE)
        {
#ifdef DEBUG
            Win32::Error("IMAGE_NT_HEADERS::Signature is invalid (L3D451R7::Memory::%s)", __FUNCTION__);
#endif // DEBUG
            return 0u;
        }

        auto scan_begin = (std::uint8_t*)(image_base);
        auto scan_end = (std::uint8_t*)(image_base + image_nt_hdrs->OptionalHeader.SizeOfImage);

        std::uint8_t* scan_result = nullptr;
        std::uint8_t* scan_data = (std::uint8_t*)(signature.c_str());

        for (auto current = scan_begin; current < scan_end; current++)
        {
            if (*(std::uint8_t*)scan_data == '\?' || *current == GET_BYTE(scan_data))
            {
                if (!scan_result)
                    scan_result = current;

                if (!scan_data[2])
                    return (std::uintptr_t)(scan_result);

                scan_data += (*(std::uint16_t*)scan_data == '\?\?' || *(std::uint8_t*)scan_data != '\?') ? 3 : 2;

                if (!*scan_data)
                    return (std::uintptr_t)(scan_result);
            }
            else if (scan_result)
            {
                current = scan_result;
                scan_data = (std::uint8_t*)(signature.c_str());
                scan_result = nullptr;
            }
        }

#ifdef DEBUG
        Win32::Warning("Signature '%s' in module '%s' not found (L3D451R7::Memory::%s)", signature.c_str(), image_name.c_str(), __FUNCTION__);
#endif // DEBUG

        return 0u;
    }

    /*
     * @brief Set player clantag
     *
     * @param tag New clantag
     */
    void SetClantag(const char* tag)
    {
        static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*))PatternScan(GetModuleHandleW(L"engine.dll"), "53 56 57 8B DA 8B F9 FF 15");

        fnClantagChanged(tag, tag);
    }

    /*
     * @brief Set player name
     *
     * @param name New name
     */
    void SetName(const char* name)
    {
        static auto nameConvar = g_CVar->FindVar("name");
        nameConvar->m_fnChangeCallbacks.m_Size = 0;

        // Fix so we can change names how many times we want
        // This code will only run once because of `static`
        static auto do_once = (nameConvar->SetValue("\n���"), true);

        nameConvar->SetValue(name);
    }

    float GetLerpTime() {
        int ud_rate = g_CVar->FindVar("cl_updaterate")->GetInt();
        ConVar* min_ud_rate = g_CVar->FindVar("sv_minupdaterate");
        ConVar* max_ud_rate = g_CVar->FindVar("sv_maxupdaterate");

        if (min_ud_rate && max_ud_rate)
            ud_rate = max_ud_rate->GetInt();

        float ratio = g_CVar->FindVar("cl_interp_ratio")->GetFloat();

        if (ratio == 0)
            ratio = 1.0f;

        float lerp = g_CVar->FindVar("cl_interp")->GetFloat();
        ConVar* c_min_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
        ConVar* c_max_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

        if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
            ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

        return std::max(lerp, (ratio / ud_rate));
    }
}
