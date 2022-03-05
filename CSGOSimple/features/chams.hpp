#pragma once

#include "../singleton.hpp"

#include "../valve_sdk/csgostructs.hpp"

class Chams
    : public Singleton<Chams>
{
    friend class Singleton<Chams>;

    Chams();
    ~Chams();

public:
    void OnDrawModelExecute(void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags);
private:
    void OverrideMaterial(bool ignorez, int type, const Color& rgba);

    IMaterial* materialRegular = nullptr;
    IMaterial* materialFlat = nullptr;
    IMaterial* materialGlass = nullptr;
    IMaterial* materialGlow = nullptr;
    IMaterial* materialOverlay = nullptr;
    IMaterial* materialAnimated = nullptr;
};
