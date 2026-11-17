#pragma once

#include <third_party/haxsdk/unity/hax_unity.h>

struct PrefabRef : System::Object
{
    using System::Object::Object;

    META("Assembly-CSharp", "", "PrefabRef");

    Unity::GameObject Prefab() { THROW_IF_NULL(); return s_get_Prefab.Call<Unity::GameObject>(*this); }

    // Path usable with Resources / PhotonNetwork.Instantiate. Since the game started to ship
    // prefabs in asset bundles, the raw resourcePath field is not enough for them:
    // the getter adds the "AssetBundles/<bundle>/" prefix.
    System::String ResourcePath() { THROW_IF_NULL(); return s_get_ResourcePath.Call<System::String>(*this); }

    FIELD(prefabName, System::String);
    FIELD(resourcePath, System::String);

private:
    METHOD_WRAPPER(get_Prefab);
    METHOD_WRAPPER(get_ResourcePath);
};

struct MenuCursor : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MenuCursor");

    FIELD(showTimer, float);

    METHOD_WRAPPER(Update);
};

struct PlayerHealth : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerHealth");

    void HealOther(int healAmount, bool effect)
    {
        THROW_IF_NULL();
        s_HealOther.Call<void>(*this, healAmount, effect);
    }

    FIELD(maxHealth, int);
    FIELD(health, int);
    FIELD(renderers, System::List<Unity::MeshRenderer>);

    METHOD_WRAPPER_EX(Hurt, "System.Void(System.Int32,System.Boolean,System.Int32,System.Boolean)");
    METHOD_WRAPPER(Update);
private:
    METHOD_WRAPPER(HealOther);
};

struct PlayerDeathHead;
struct PhysGrabber;
struct PlayerVoiceChat;
struct PlayerTumble;
struct MapToolController;
struct PlayerAvatarVisuals;
struct PlayerAvatar : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerAvatar");

    void PlayerDeath(int enemyIndex)
    {
        THROW_IF_NULL();
        s_PlayerDeath.Call<void>(*this, enemyIndex);
    }

    void Revive(bool revivedByTruck = false)
    {
        THROW_IF_NULL();
        s_Revive.Call<void>(*this, revivedByTruck);
    }

    STATIC_FIELD(instance, PlayerAvatar);
    FIELD(playerHealth, PlayerHealth);
    FIELD(isLocal, bool);
    FIELD(deadSet, bool);
    FIELD(playerDeathHead, PlayerDeathHead);
    FIELD(physGrabber, PhysGrabber);
    FIELD(playerName, System::String);
    FIELD(voiceChat, PlayerVoiceChat);
    FIELD(spawned, bool);
    FIELD(photonView, Unity::Photon::PhotonView);
    FIELD(tumble, PlayerTumble);
    FIELD(upgradeMapPlayerCount, int);
    FIELD(isCrouching, bool);
    FIELD(isDisabled, bool);
    FIELD(spectatePoint, Unity::Transform);
    FIELD(mapToolController, MapToolController);
    FIELD(playerTransform, Unity::Transform);
    FIELD(playerAvatarVisuals, PlayerAvatarVisuals);
    FIELD(voiceChatFetched, bool);

    METHOD_WRAPPER(OnDestroy);
    METHOD_WRAPPER_EX(PlayerDeath, "System.Void(System.Int32)");
    METHOD_WRAPPER(Revive);
};

struct PlayerVoiceChat : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerVoiceChat");

    FIELD(inLobbyMixer, bool);

    void ToggleMixer(bool toggle, bool distorted)
    {
        THROW_IF_NULL();
        s_ToggleMixer.Call<void>(*this, toggle, distorted);
    }

private:
    METHOD_WRAPPER(ToggleMixer);
};

struct PlayerController : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerController");

    STATIC_FIELD(instance, PlayerController);
    FIELD(EnergyCurrent, float);
    FIELD(EnergyStart, float);
    FIELD(MoveSpeed, float);
    FIELD(SprintSpeed, float);
    FIELD(CrouchSpeed, float);
    FIELD(JumpExtra, int);
    FIELD(SprintSpeedUpgrades, float);

    METHOD_WRAPPER(FixedUpdate);
    METHOD_WRAPPER(Update);
};

struct PlayerTumble : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerTumble");

    void TumbleRequest(bool _isTumbling, bool _playerInput) 
    { 
        THROW_IF_NULL(); 
        s_TumbleRequest.Call<void, PlayerTumble, bool, bool>(*this, _isTumbling, _playerInput); 
    }

    FIELD(tumbleLaunch, int);
    FIELD(playerAvatar, PlayerAvatar);

    METHOD_WRAPPER_EX(TumbleRequest, "System.Void(System.Boolean,System.Boolean)");
};

struct FlashlightController : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "FlashlightController");

    FIELD(spotlight, Unity::Light);
    FIELD(baseIntensity, float);

    METHOD_WRAPPER(Update);
};

struct ItemToggle : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemToggle");

    void ToggleItem(bool toggle, int player = -1)
    {
        THROW_IF_NULL();
        s_ToggleItem.Call<void, ItemToggle, bool, int>(*this, toggle, player);
    }

    FIELD(toggleState, bool);
    FIELD(photonView, Unity::Photon::PhotonView);

private:
    METHOD_WRAPPER(ToggleItem);
};

struct ItemAttributes : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemAttributes");

    FIELD(itemAssetName, System::String);
};

struct ItemUpgrade : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemUpgrade");

    FIELD(upgradeDone, bool);
    FIELD(itemToggle, ItemToggle);
    FIELD(itemAttributes, ItemAttributes);
    FIELD(isPlayerUpgrade, bool);
    FIELD(photonView, Unity::Photon::PhotonView);

    METHOD_WRAPPER(Update);
};

struct ItemHealthPack : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemHealthPack");

    FIELD(used, bool);
    FIELD(itemToggle, ItemToggle);
    FIELD(photonView, Unity::Photon::PhotonView);
    FIELD(itemAttributes, ItemAttributes);

    METHOD_WRAPPER(Update);
};

struct SemiFunc
{
    META("Assembly-CSharp", "", "SemiFunc");

    static inline Unity::Camera MainCamera() 
    { 
        return s_MainCamera.Call<Unity::Camera>(); 
    }

    static inline bool OnScreen(const Unity::Vector3& pos, float paddWidth, float paddHeight) 
    {
        return s_OnScreen.Call<bool>(System::Box(pos), paddWidth, paddHeight); 
    }

    static inline bool IsMasterClientOrSingleplayer()
    {
        return s_IsMasterClientOrSingleplayer.Call<bool>();
    }

    static inline bool IsMultiplayer()
    {
        return s_IsMultiplayer.Call<bool>();
    }

    static inline bool IsMasterClient()
    {
        return s_IsMasterClient.Call<bool>();
    }

    static inline System::String PlayerGetSteamID(PlayerAvatar player)
    {
        return s_PlayerGetSteamID.Call<System::String>(player);
    }

    static inline int StatSetRunCurrency(int value)
    {
        return s_StatSetRunCurrency.Call<int, int>(value);
    }

    static inline int PhotonViewIDPlayerAvatarLocal()
    {
        return s_PhotonViewIDPlayerAvatarLocal.Call<int>();
    }

    static inline int LayerMaskGetVisionObstruct()//!
    {
        return s_LayerMaskGetVisionObstruct.Call<System::Boxed<Unity::LayerMask>>().Unbox().m_Mask;
    }

    static inline System::String PlayerGetName(PlayerAvatar player)
    {
        return s_PlayerGetName.Call<System::String>(player);
    }

private:
    METHOD_WRAPPER(OnScreen);
    METHOD_WRAPPER(MainCamera);
    METHOD_WRAPPER(IsMasterClientOrSingleplayer);
    METHOD_WRAPPER(IsMultiplayer);
    METHOD_WRAPPER(IsMasterClient);
    METHOD_WRAPPER(PlayerGetSteamID);
    METHOD_WRAPPER(StatSetRunCurrency);
    METHOD_WRAPPER(PhotonViewIDPlayerAvatarLocal);
    METHOD_WRAPPER(LayerMaskGetVisionObstruct);
    METHOD_WRAPPER(PlayerGetName);
};

struct PhysGrabObjectImpactDetector;
struct PhysGrabObject : Unity::Behaviour
{
    using Unity::Behaviour::Behaviour;

    META("Assembly-CSharp", "", "PhysGrabObject");

    FIELD(grabbedLocal, bool);
    FIELD(centerPoint, Unity::Vector3);
    FIELD(boundingBox, Unity::Vector3);
    FIELD(midPoint, Unity::Vector3);
    FIELD(impactDetector, PhysGrabObjectImpactDetector);

    METHOD_WRAPPER(PhysicsGrabbingManipulation);
};

struct PhysGrabber : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PhysGrabber");

    void PhysGrabOverChargeImpact()
    {
        THROW_IF_NULL();
        s_PhysGrabOverChargeImpact.Call<void, PhysGrabber>(*this);
    }

    FIELD(grabRange, float);
    FIELD(grabStrength, float);
    FIELD(throwStrength, float);
    FIELD(maxDistanceFromPlayer, float);    // how far a held object can be pushed (2.5 m)
    FIELD(pullerDistance, float);           // current distance of the held object
    FIELD(grabbed, bool);
    FIELD(grabState, int);                  // PhysGrabber.GrabState: None, Phys, Heal, Static, Climb

    METHOD_WRAPPER(Update);
    METHOD_WRAPPER(PhysGrabLogic);
    METHOD_WRAPPER_EX(RayCheck, "System.Void(System.Boolean)");
    METHOD_WRAPPER(PhysGrabOverChargeImpact);
    METHOD_WRAPPER_EX(PhysGrabOverCharge, "System.Void(System.Single,System.Single)");
};

// ScriptableObject with the valuables a level can spawn, grouped by volume size.
struct LevelValuables : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "LevelValuables");

    FIELD(tiny, System::List<PrefabRef>);
    FIELD_EX(smallList, "small", System::List<PrefabRef>);   // "small" is a macro in Windows' rpcndr.h
    FIELD(medium, System::List<PrefabRef>);
    FIELD(big, System::List<PrefabRef>);
    FIELD(wide, System::List<PrefabRef>);
    FIELD(tall, System::List<PrefabRef>);
    FIELD(veryTall, System::List<PrefabRef>);
};

struct Level : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "Level");

    FIELD(NarrativeName, System::String);
    FIELD(ValuablePresets, System::List<LevelValuables>);
};

struct RunManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "RunManager");

    void ChangeLevel(bool completedLevel, bool levelFailed, int changeLevelType = 0)
    {
        THROW_IF_NULL();
        s_ChangeLevel.Call<void, RunManager, bool, bool, int>(*this, completedLevel, levelFailed, changeLevelType);
    }

    STATIC_FIELD(instance, RunManager);
    FIELD(levels, System::List<Level>);
    FIELD(levelCurrent, Level);
    FIELD(levelMainMenu, Level);
    FIELD(levelLobbyMenu, Level);
    FIELD(levelShop, System::List<Level>);
    FIELD(previousRunLevel, Level);
    FIELD(levelsCompleted, int);
    FIELD(levelIsShop, bool);

    METHOD_WRAPPER(ChangeLevel);
    METHOD_WRAPPER(SetRunLevel);
};

struct GameDirector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "GameDirector");

    STATIC_FIELD(instance, GameDirector);
    FIELD(MainCamera, Unity::Camera);
    FIELD(currentState, int);
    FIELD(PlayerList, System::List<PlayerAvatar>);
};

struct GameDirector_gameState
{
    META("Assembly-CSharp", "", "GameDirector/gameState");

    ENUM_VALUE(Main);
};

struct SpectateCamera : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "SpectateCamera");

    STATIC_FIELD(instance, SpectateCamera);
    FIELD(headEnergy, float);
    FIELD(headEnergyEnough, bool);    

    METHOD_WRAPPER(HeadEnergyLogic);
};

struct PlayerHealthGrab : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerHealthGrab");
};

struct PlayerAvatarVisuals : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerAvatarVisuals");

    FIELD(playerAvatar, PlayerAvatar);
    FIELD(isMenuAvatar, bool);
    FIELD(meshParent, Unity::GameObject);
    FIELD(animator, Unity::Behaviour);
    FIELD(headSideTransform, Unity::Transform);
    FIELD(legTwistTransform, Unity::Transform);
    FIELD(showSelfOverrideTimer, float);

    METHOD_WRAPPER(Update);
};

struct GameManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "GameManager");

    STATIC_FIELD(instance, GameManager);
    PROPERTY(gameMode, int);
};

struct MapToolController : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MapToolController");

    FIELD(Active, bool);
};

struct EnemyRigidbody : Unity::Behaviour
{
    using Unity::Behaviour::Behaviour;

    META("Assembly-CSharp", "", "EnemyRigidbody");

    FIELD(physGrabObject, PhysGrabObject);
    FIELD(grabShakeReleaseTimer, float);
    FIELD(grabStrengthTimer, float);
    FIELD(grabStrengthTime, float);
    FIELD(grabTimeCurrent, float);

    METHOD_WRAPPER(FixedUpdate);
};

struct EnemyHealth : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "EnemyHealth");

    void Hurt(int damage, const Unity::Vector3& hurtDirection)
    {
        THROW_IF_NULL();
        s_Hurt.Call<void>(*this, damage, System::Box(hurtDirection));
    }

    FIELD(healthCurrent, int);
    FIELD(health, int);
    FIELD(dead, bool);

    METHOD_WRAPPER(Hurt);
};

struct EnemyState
{
    META("Assembly-CSharp", "", "EnemyState");

    ENUM_VALUE(None);
    ENUM_VALUE(Spawn);
    ENUM_VALUE(Despawn);
};

struct EnemyParent;
struct Enemy : Unity::Behaviour
{
    META("Assembly-CSharp", "", "Enemy");

    FIELD(Rigidbody, EnemyRigidbody);
    FIELD(CenterTransform, Unity::Transform);
    FIELD(Health, EnemyHealth);
    FIELD(CurrentState, int);
    FIELD_EX(enemyParent, "EnemyParent", EnemyParent);

    METHOD_WRAPPER(Update);
};

struct EnemyParent : Unity::Behaviour
{
    META("Assembly-CSharp", "", "EnemyParent");

    FIELD_EX(enemy, "Enemy", Enemy);
    FIELD(Spawned, bool);
    FIELD(enemyName, System::String);
};

struct EnemySetup : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "EnemySetup");

    FIELD(spawnObjects, System::List<PrefabRef>);
};

struct EnemyDirector : Unity::Behaviour
{
    using Unity::Behaviour::Behaviour;

    META("Assembly-CSharp", "", "EnemyDirector");

    STATIC_FIELD(instance, EnemyDirector);
    FIELD(enemiesSpawned, System::List<EnemyParent>);
    FIELD(enemiesDifficulty1, System::List<EnemySetup>);
    FIELD(enemiesDifficulty2, System::List<EnemySetup>);
    FIELD(enemiesDifficulty3, System::List<EnemySetup>);
    FIELD(debugNoVision, System::List<System::String>); // steam IDs of players enemies can't see
    FIELD(debugNoGrabMaxTime, bool);
    FIELD(debugEasyGrab, bool);
};

struct RoomVolume : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "RoomVolume");
        
    bool SetExplored()
    {
        THROW_IF_NULL();
        return s_SetExplored.Call<bool, RoomVolume>(*this);
    }

private:
    METHOD_WRAPPER(SetExplored);
};

struct LevelPoint : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "LevelPoint");

    FIELD(Room, RoomVolume);
};

struct LevelGenerator : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "LevelGenerator");

    void EnemySpawn(EnemySetup enemySetup, Unity::Vector3 position)
    {
        THROW_IF_NULL();
        s_EnemySpawn.Call<void>(*this, enemySetup, System::Box(position));
    }

    STATIC_FIELD(Instance, LevelGenerator);
    FIELD(LevelPathPoints, System::List<LevelPoint>);
    FIELD(Generated, bool);

private:
    METHOD_WRAPPER(EnemySpawn);
};

struct EnemySlowMouth : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "EnemySlowMouth");

    FIELD(attachedTimer, float);
};

struct MenuManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MenuManager");

    STATIC_FIELD(instance, MenuManager);
    FIELD(currentMenuPage, Component);
};

struct PhysGrabObjectImpactDetector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PhysGrabObjectImpactDetector");

    void HealLogic(float healAmount, const Unity::Vector3& healingPoint)
    {
        s_HealLogic.Call<void>(*this, healAmount, System::Box(healingPoint));
    }

    FIELD(isValuable, bool);
    FIELD(impactDisabledTimer, float);
    FIELD(photonView, Unity::Photon::PhotonView);

    METHOD_WRAPPER(FixedUpdate);
    METHOD_WRAPPER_EX(Break, "System.Void(System.Single,UnityEngine.Vector3,System.Int32,System.Boolean)");
    METHOD_WRAPPER_EX(DestroyObject, "System.Void(System.Boolean,System.Boolean)");
private:
    METHOD_WRAPPER(HealLogic);
};

struct ItemBattery : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemBattery");

    void SetBatteryLife(int batteryLife)
    {
        THROW_IF_NULL();
        s_SetBatteryLife.Call<void, ItemBattery, int>(*this, batteryLife);
    }

    void BatteryFullPercentChange(int batteryLifeInt, bool charge = false, bool playSound = true)
    {
        THROW_IF_NULL();
        s_BatteryFullPercentChange.Call<void, ItemBattery, int, bool, bool>(*this, batteryLifeInt, charge, playSound);
    }

    FIELD(batteryLife, float);
    FIELD(batteryLifeInt, int);
    FIELD(batteryBars, int);
    FIELD(physGrabObject, PhysGrabObject);

    METHOD_WRAPPER(Update);
    METHOD_WRAPPER(SetBatteryLife);
    METHOD_WRAPPER_EX(BatteryFullPercentChange, "System.Void(System.Int32,System.Boolean,System.Boolean)");
};

// "Value" ScriptableObject: the price range a valuable rolls its value from.
struct ValuePreset : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "Value");

    FIELD(valueMin, float);
    FIELD(valueMax, float);
};

// "Version" ScriptableObject of BuildManager, title is e.g. "v0.5.1".
struct GameVersionAsset : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "Version");

    FIELD(title, System::String);
};

struct BuildManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "BuildManager");

    STATIC_FIELD(instance, BuildManager);
    FIELD(version, GameVersionAsset);
};

struct ValuableObject : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ValuableObject");

    bool Discover(int state)
    {
        THROW_IF_NULL();
        return s_Discover.Call<bool, ValuableObject, int>(*this, state);
    }

    // Rolls the value from the valuable preset (once). The game calls it right after spawning.
    void DollarValueSetLogic()
    {
        THROW_IF_NULL();
        s_DollarValueSetLogic.Call<void, ValuableObject>(*this);
    }

    FIELD(physGrabObject, PhysGrabObject);
    FIELD(dollarValueCurrent, float);
    FIELD(valuePreset, ValuePreset);
    FIELD(dollarValueOverride, int);        // fixed value when not 0

private:
    METHOD_WRAPPER(Discover);
    METHOD_WRAPPER(DollarValueSetLogic);
};

struct CosmeticWorldObjectSetup : System::Object
{
    using System::Object::Object;

    META("Assembly-CSharp", "", "ValuableDirector/CosmeticWorldObjectSetup");

    FIELD(prefab, PrefabRef);
};

struct ValuableDirector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ValuableDirector");

    STATIC_FIELD(instance, ValuableDirector);
    FIELD(valuableList, System::List<ValuableObject>);
    FIELD(cosmeticWorldObjectSetups, System::List<CosmeticWorldObjectSetup>);
};

struct ItemGun : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ItemGun");

    FIELD(gunRandomSpread, float);
    FIELD(physGrabObject, PhysGrabObject);
    FIELD(gunMuzzle, Unity::Transform);
    FIELD(gunRange, float);

    METHOD_WRAPPER(Shoot);
    METHOD_WRAPPER(Update);
};

struct Item : Unity::Object
{
    using Unity::Object::Object;

    META("Assembly-CSharp", "", "Item");

    FIELD(prefab, PrefabRef);
    FIELD(itemName, System::String);
};

struct StatsManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "StatsManager");

    void LoadItemsFromFolder()
    {
        THROW_IF_NULL();
        s_LoadItemsFromFolder.Call<void, StatsManager>(*this);
    }

    STATIC_FIELD(instance, StatsManager);
    FIELD(itemDictionary, System::Dictionary<System::String COMMA Item>);
    METHOD_WRAPPER(LoadItemsFromFolder);
};

struct ExtractionPoint : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "ExtractionPoint");

    FIELD(isLocked, bool);
    FIELD(currentState, int);

    void HaulGoalSet(int value)
    {
        THROW_IF_NULL();
        s_HaulGoalSet.Call<void, ExtractionPoint, int>(*this, value);
    }

    void OnClick()
    {
        THROW_IF_NULL();
        s_OnClick.Call<void, ExtractionPoint>(*this);
    }

private:
    METHOD_WRAPPER(HaulGoalSet);
    METHOD_WRAPPER(OnClick);
};

struct CosmeticWorldObject : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "CosmeticWorldObject");

    FIELD(rarity, int);
};

struct RoundDirector : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "RoundDirector");

    void ExtractionPointsUnlock()
    {
        THROW_IF_NULL();
        s_ExtractionPointsUnlock.Call<void>(*this);
    }

    STATIC_FIELD(instance, RoundDirector);
    FIELD(extractionPointActive, bool);
    FIELD(extractionPointCurrent, ExtractionPoint);
    FIELD(extractionPointList, System::List<Unity::GameObject>);
    FIELD(cosmeticWorldObjects, System::List<CosmeticWorldObject>);

private:
    METHOD_WRAPPER(ExtractionPointsUnlock);
};

struct ExtractionPoint_State
{
    META("Assembly-CSharp", "", "ExtractionPoint/State");

    ENUM_VALUE(Complete);
    ENUM_VALUE(Active);
    ENUM_VALUE(Idle);
};

struct TruckSafetySpawnPoint : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "TruckSafetySpawnPoint");

    STATIC_FIELD(instance, TruckSafetySpawnPoint);
};

struct PlayerDeathHead : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "PlayerDeathHead");

    FIELD(colliders, System::Array<Unity::Collider>);
    FIELD(meshRenderers, System::List<Unity::MeshRenderer>);
    FIELD(eyeRenderers, System::Array<Unity::MeshRenderer>);
};

struct MetaManager : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("Assembly-CSharp", "", "MetaManager");

    bool CosmeticUnlockAll()
    {
        THROW_IF_NULL();
        return s_CosmeticUnlockAll.Call<bool, MetaManager>(*this);
    }

    void Reset()
    {
        THROW_IF_NULL();
        s_Reset.Call<void, MetaManager>(*this);
    }

    STATIC_FIELD(instance, MetaManager);
private:
    METHOD_WRAPPER(CosmeticUnlockAll);
    METHOD_WRAPPER(Reset);
};

struct EventSystem : Unity::MonoBehaviour
{
    using Unity::MonoBehaviour::MonoBehaviour;

    META("UnityEngine.UI", "UnityEngine.EventSystems", "EventSystem");

    METHOD_WRAPPER(Update);
};

struct Cursor
{
    META("UnityEngine.CoreModule", "UnityEngine", "Cursor");
    METHOD_WRAPPER(set_visible);
};

struct PostProcessLayer
{
    META("Unity.Postprocessing.Runtime", "UnityEngine.Rendering.PostProcessing", "PostProcessLayer");
    METHOD_WRAPPER(BuildCommandBuffers);
};

struct Physics
{
    META("UnityEngine.PhysicsModule", "UnityEngine", "Physics");
    METHOD_WRAPPER_EX(Raycast, "System.Boolean(UnityEngine.Vector3,UnityEngine.Vector3,UnityEngine.RaycastHit&,System.Single,System.Int32,UnityEngine.QueryTriggerInteraction)");
};

struct Camera
{
    META("UnityEngine.CoreModule", "UnityEngine", "Camera");
    METHOD_WRAPPER(FireOnPreRender);
};