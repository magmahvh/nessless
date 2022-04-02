#pragma once 

#include "IClientNetworkable.hpp"
#include "IClientRenderable.hpp"
#include "IClientUnknown.hpp"
#include "IClientThinkable.hpp"

struct SpatializationInfo_t;

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual void Release(void) = 0;
};

#pragma pack(push, 1)
class CCSWeaponInfo { //xSeeker
public:
    char pad_0000[4];
    char* ConsoleName;
    char pad_0008[12];
    int iMaxClip1;
    char pad_0018[12];
    int iMaxClip2;
    char pad_0028[4];
    char* szWorldModel;
    char* szViewModel;
    char* szDropedModel;
    char pad_0038[4];
    char* N00000984;
    char pad_0040[56];
    char* szEmptySound;
    char pad_007C[4];
    char* szBulletType;
    char pad_0084[4];
    char* szHudName;
    char* szWeaponName;
    char pad_0090[60];
    int iWeaponType;
    int iWeaponPrice;
    int iKillAward;
    char* szAnimationPrefex;
    float flCycleTime;
    float flCycleTimeAlt;
    float flTimeToIdle;
    float flIdleInterval;
    bool bFullAuto;
    char pad_00ED[3];
    int iDamage;
    float headshotmultyplrier;
    float flArmorRatio;
    int iBullets;
    float flPenetration;
    float flFlinchVelocityModifierLarge;
    float flFlinchVelocityModifierSmall;
    float flRange;
    float flRangeModifier;
    char pad_0110[28];
    int iCrosshairMinDistance;
    float flMaxPlayerSpeed;
    float flMaxPlayerSpeedAlt;
    char pad_0138[4];
    float flSpread;
    float flSpreadAlt;
    float flInaccuracyCrouch;
    float flInaccuracyCrouchAlt;
    float flInaccuracyStand;
    float flInaccuracyStandAlt;
    float flInaccuracyJumpIntial;
    float flInaccaurcyJumpApex;
    float flInaccuracyJump;
    float flInaccuracyJumpAlt;
    float flInaccuracyLand;
    float flInaccuracyLandAlt;
    float flInaccuracyLadder;
    float flInaccuracyLadderAlt;
    float flInaccuracyFire;
    float flInaccuracyFireAlt;
    float flInaccuracyMove;
    float flInaccuracyMoveAlt;
    float flInaccuracyReload;
    int iRecoilSeed;
    float flRecoilAngle;
    float flRecoilAngleAlt;
    float flRecoilVariance;
    float flRecoilAngleVarianceAlt;
    float flRecoilMagnitude;
    float flRecoilMagnitudeAlt;
    float flRecoilMagnatiudeVeriance;
    float flRecoilMagnatiudeVerianceAlt;
    float flRecoveryTimeCrouch;
    float flRecoveryTimeStand;
    float flRecoveryTimeCrouchFinal;
    float flRecoveryTimeStandFinal;
    int iRecoveryTransititionStartBullet;
    int iRecoveryTransititionEndBullet;
    bool bUnzoomAfterShot;
    char pad_01C1[31];
    char* szWeaponClass;
    char pad_01E4[56];
    float flInaccuracyPitchShift;
    float flInaccuracySoundThreshold;
    float flBotAudibleRange;
    char pad_0228[12];
    bool bHasBurstMode;
};
#pragma pack(pop)

class IWeaponSystem
{
	virtual void unused0() = 0;
	virtual void unused1() = 0;
public:
	virtual CCSWeaponInfo* GetWpnData(unsigned ItemDefinitionIndex) = 0;
};