#pragma once

#include "../singleton.hpp"

#include "../valve_sdk/csgostructs.hpp"

class Chams
    : public Singleton<Chams>
{
    friend class Singleton<Chams>;

    Chams();
    ~Chams();

    void OverrideMaterial(int type, bool ignoreZ, const Color& rgba);

    

    //void OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix);

public:
    void OnDrawModelExecute(void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags);
private:
   // void OverrideMaterial(bool ignorez, int type, const Color& rgba);

    IMaterial* materialRegular = nullptr;
    IMaterial* materialFlat = nullptr;
    IMaterial* materialMetallic = nullptr;
    IMaterial* materialDogtag = nullptr;
    IMaterial* materialGlowArmsrace = nullptr;
    IMaterial* materialEsoGlow = nullptr;
    IMaterial* materialGlass = nullptr;
    IMaterial* materialGlow = nullptr;
    IMaterial* materialOverlay = nullptr;
    IMaterial* materialAnimated = nullptr;
};
