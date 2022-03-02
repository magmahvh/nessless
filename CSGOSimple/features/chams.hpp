#pragma once

#include "../singleton.hpp"
#include "../valve_sdk/sdk.hpp"

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class Chams
    : public Singleton<Chams>
{
    friend class Singleton<Chams>;


public:
    void OnDrawModelExecute(void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags);

};