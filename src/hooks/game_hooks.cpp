#include <src/pch.h>

#include <src/cheat.h>
#include <src/game_classes.h>
#include <src/crash_reporter.h>
#include <src/resource.h>
#include <src/overlay/esp/esp.h>
#include <src/overlay/menu/item_names.h>
#include <src/overlay/menu/valuable_names.h>

#include <algorithm>
#include <strsafe.h>

using namespace System;
using namespace Unity;

namespace Cheat
{
    extern void Hook(void* ptr, void* detour, SafetyHookInline& out, const char* name);
    extern void HookModuleProc(HMODULE module, const char* procName, void* procHook, SafetyHookInline& out);

    static void* Hooked_MonoRuntimeInvoke(void* a1, void* a2, void* a3, void* a4);
    static void Hooked__EventSystem_Update(Unity::EventSystem __this);
    static void Hooked__PlayerHealth_Hurt(PlayerHealth __this, int damage, bool savingGrace, int enemyIndex, bool hurtByHeal);
    static void Hooked__PlayerAvatar_PlayerDeath(PlayerAvatar __this, int enemyIndex);
    static void Hooked__PlayerController_FixedUpdate(PlayerController __this);
    static void Hooked__PlayerController_Update(PlayerController __this);
    static void Hooked__PlayerTumble_TumbleRequest(PlayerTumble __this, bool isTumbling, bool playerInput);
    static void Hooked__FlashlightController_Update(FlashlightController __this);
    static void Hooked__ItemUpgrade_Update(ItemUpgrade __this);
    static void Hooked__ItemHealthPack_Update(ItemHealthPack __this);
    static void Hooked__UpdateLoop();
    static void Hooked__PhysGrabObject_PhysicsGrabbingManipulation(PhysGrabObject __this);
    static void Hooked__PostProcessLayer_BuildCommandBuffers(Unity::PostProcessLayer __this);
    static void Hooked__PlayerAvatarVisuals_Update(PlayerAvatarVisuals __this);
    static void Hooked__PhysGrabber_Update(PhysGrabber __this);
    static void Hooked__EnemySpinny_GetAndSyncRandomRotation(EnemySpinny __this);
    static void Hooked__PhysGrabber_PhysGrabLogic(PhysGrabber __this);
    static void Hooked__PhysGrabber_RayCheck(PhysGrabber __this, bool grab);
    static bool Hooked__Physics_Raycast(void* v1, void* v2, void* v3, float maxDistance, int v5, int v6);
    static void Hooked__PhysGrabObjectImpactDetector_Break(PhysGrabObjectImpactDetector __this, float a1, void* a2, int a3, bool a4);
    static void Hooked__PhysGrabObjectImpactDetector_DestroyObject(PhysGrabObjectImpactDetector __this, bool effects, bool forced);
    static bool Hooked__PhotonNetwork_IsMasterClient();
    static void Hooked__ItemBattery_Update(ItemBattery __this);
    static void Hooked__Camera_FireOnPreRender(Unity::Camera __this);
    static void Hooked__ItemGun_Update(ItemGun __this);
    static void Hooked__RunManager_SetRunLevel(RunManager __this);
    static void Hooked__PhysGrabber_PhysGrabOverCharge(PhysGrabber __this, float _amount, float _multiplier);
    static void Hooked__SpectateCamera_HeadEnergyLogic(SpectateCamera __this);
    static void Hooked__PostLateUpdateLoop();
    static void Hooked__PresentAfterDrawLoop();
    static void Hooked__EnemyRigidbody_FixedUpdate(EnemyRigidbody __this);

    static bool IsInGame();
    static void* GetPlayerLoopPtr(System::Type type, System::Type subType);
    static void Heal(PhysGrabObjectImpactDetector obj, float value);
    static void SpawnItem(Item item);
    static void SpawnCosmeticBox(int rarity);
    static void SpawnValuable(int index);
    static void ParseValuables();
    static void UpdateValuablePrices();
    static void ParseEnemies();
    static void ParseItems();
    static void ParseLevels();
    static void DrawPlayerChams(PlayerAvatar player, Unity::CommandBuffer cb, Unity::Material aliveMat, Unity::Material deadMat);

    void HookMonoRuntimeInvoke()
    {
        HookModuleProc((HMODULE)Hax::Unity::GetUvmHandle(), "mono_runtime_invoke", Hooked_MonoRuntimeInvoke, G->MonoRuntimeInvokeHook);
        ::FlushInstructionCache(::GetCurrentProcess(), nullptr, 0);
        ::Sleep(20);
    }

    void HookGameFuncs()
    {
        Hax::LogDebug(G->Logger, L"Installing game hooks...");

        #define HOOK(ptr, n) Hook(ptr, Hooked__ ## n, G-> ## n ## _Hook, #n)
        HOOK(::EventSystem::s_Update.Address, EventSystem_Update);
        HOOK(PlayerHealth::s_Hurt.Address, PlayerHealth_Hurt);
        HOOK(PlayerAvatar::s_PlayerDeath.Address, PlayerAvatar_PlayerDeath);
        HOOK(PlayerController::s_FixedUpdate.Address, PlayerController_FixedUpdate);
        HOOK(PlayerController::s_Update.Address, PlayerController_Update);
        HOOK(PlayerTumble::s_TumbleRequest.Address, PlayerTumble_TumbleRequest);
        HOOK(FlashlightController::s_Update.Address, FlashlightController_Update);
        HOOK(ItemUpgrade::s_Update.Address, ItemUpgrade_Update);
        HOOK(ItemHealthPack::s_Update.Address, ItemHealthPack_Update);

        HOOK(PhysGrabObject::s_PhysicsGrabbingManipulation.Address, PhysGrabObject_PhysicsGrabbingManipulation);
        HOOK(::PostProcessLayer::s_BuildCommandBuffers.Address, PostProcessLayer_BuildCommandBuffers);
        HOOK(PlayerAvatarVisuals::s_Update.Address, PlayerAvatarVisuals_Update);
        HOOK(PhysGrabber::s_Update.Address, PhysGrabber_Update);
        HOOK(EnemySpinny::s_GetAndSyncRandomRotation.Address, EnemySpinny_GetAndSyncRandomRotation);
        HOOK(PhysGrabber::s_PhysGrabLogic.Address, PhysGrabber_PhysGrabLogic);
        HOOK(PhysGrabber::s_RayCheck.Address, PhysGrabber_RayCheck);
        HOOK(::Physics::s_Raycast.Address, Physics_Raycast);
        HOOK(PhysGrabObjectImpactDetector::s_Break.Address, PhysGrabObjectImpactDetector_Break);
        HOOK(PhysGrabObjectImpactDetector::s_DestroyObject.Address, PhysGrabObjectImpactDetector_DestroyObject);
        //HOOK(::PhotonNetwork::s_IsMasterClient.Address, PhotonNetwork_IsMasterClient);
        HOOK(ItemBattery::s_Update.Address, ItemBattery_Update);
        HOOK(::Camera::s_FireOnPreRender.Address, Camera_FireOnPreRender);
        HOOK(ItemGun::s_Update.Address, ItemGun_Update);
        HOOK(RunManager::s_SetRunLevel.Address, RunManager_SetRunLevel);
        HOOK(PhysGrabber::s_PhysGrabOverCharge.Address, PhysGrabber_PhysGrabOverCharge);
        HOOK(SpectateCamera::s_HeadEnergyLogic.Address, SpectateCamera_HeadEnergyLogic);
        HOOK(EnemyRigidbody::s_FixedUpdate.Address, EnemyRigidbody_FixedUpdate);
        #undef HOOK

        ::FlushInstructionCache(::GetCurrentProcess(), nullptr, 0);
    }

    void HookPlayerLoop()
    {
        HAX_PANIC(G->PlayerLoop.ReadyToHook, &G->Logger, L"LoopSystem not ready");

        Hook(G->PlayerLoop.UpdatePtr, Hooked__UpdateLoop, G->UpdateHook, "LoopSystem.Update");
        Hook(G->PlayerLoop.PostLateUpdatePtr, Hooked__PostLateUpdateLoop, G->PostLateUpdateHook, "LoopSystem.PostLateUpdate");
        Hook(G->PlayerLoop.PresentAfterDrawPtr, Hooked__PresentAfterDrawLoop, G->PresentAfterDrawHook, "LoopSystem.PresentAfterDrawLoop");
    }

    // No CRASH_SCOPE here on purpose: Unity calls almost all managed game code (Update, ...)
    // through mono_runtime_invoke, so every crash inside the game would be blamed on this hook.
    static void* Hooked_MonoRuntimeInvoke(void* a1, void* a2, void* a3, void* a4)
    {
        void* ret = G->MonoRuntimeInvokeHook.unsafe_call<void*>(a1, a2, a3, a4);

        if (G->UnityLoadedEvent != nullptr)
        {
            using Func = const char*(*)(void*);
            static Func mono_method_get_name = (Func)::GetProcAddress((HMODULE)Hax::Unity::GetUvmHandle(), "mono_method_get_name");

            Hax::StringView methodName = mono_method_get_name(a1);
            if (methodName == "Update")
            {
                ::SetEvent(G->UnityLoadedEvent);
                G->UnityLoadedEvent = nullptr;
            }
        }

        return ret;
    }

    static void Hooked__EventSystem_Update(Unity::EventSystem __this)
    {
        CRASH_SCOPE("hook EventSystem.Update");
        try
        {
            if (__this != Unity::EventSystem::GetCurrent())
                return;

            PlayerAvatar avatar = PlayerAvatar::instance();
            PlayerHealth health = avatar ? avatar.playerHealth() : nullptr;

            G->IsClient = !SemiFunc::IsMasterClientOrSingleplayer();
            G->IsInGame = IsInGame();

            if (!G->PlayerLoop.ReadyToHook)
            {
                G->PlayerLoop.UpdatePtr = GetPlayerLoopPtr(Unity::PlayerLoop::Update::typeof(), Unity::PlayerLoop::ScriptRunBehaviourUpdate::typeof());
                Hax::LogDebug(G->Logger, L"m_UpdatePtr ptr = %p", G->PlayerLoop.UpdatePtr);

                G->PlayerLoop.PostLateUpdatePtr = GetPlayerLoopPtr(Unity::PlayerLoop::PostLateUpdate::typeof(), Unity::PlayerLoop::PlayerSendFrameStarted::typeof());
                Hax::LogDebug(G->Logger, L"m_PostLateUpdatePtr ptr = %p", G->PlayerLoop.PostLateUpdatePtr);

                G->PlayerLoop.PresentAfterDrawPtr = GetPlayerLoopPtr(Unity::PlayerLoop::PostLateUpdate::typeof(), Unity::PlayerLoop::PresentAfterDraw::typeof());
                Hax::LogDebug(G->Logger, L"m_PresentAfterDrawPtr ptr = %p", G->PlayerLoop.PresentAfterDrawPtr);

                G->PlayerLoop.ReadyToHook = true;
            }

            if (G->HealToMax)
            {
                G->HealToMax = false;
                if (health)
                    health.HealOther(99999, true);
            }

            if (G->UpgradeToToggle)
            {
                ItemUpgrade item = G->UpgradeToToggle;
                G->UpgradeToToggle = nullptr;

                ItemToggle toggle = item.itemToggle();
                if (toggle)
                {
                    toggle.photonView() = item.photonView();
                    toggle.ToggleItem(true, SemiFunc::PhotonViewIDPlayerAvatarLocal());
                }
            }

            if (G->AidToToggle)
            {
                ItemHealthPack item = G->AidToToggle;
                G->AidToToggle = nullptr;

                ItemToggle toggle = item.itemToggle();
                if (toggle)
                {
                    toggle.photonView() = item.photonView();
                    toggle.ToggleItem(true, SemiFunc::PhotonViewIDPlayerAvatarLocal());
                }
            }

            if (G->EnemiesPool.Empty())
                ParseEnemies();

            if (G->ItemsPool.Empty())
                ParseItems();

            if (G->LevelBans.Empty())
                ParseLevels();

            if (!G->ValuablesPoolReady)
                ParseValuables();
            else
                UpdateValuablePrices();

            if (G->EnemyToSpawn)
            {
                EnemySetup enemy = G->EnemyToSpawn;
                G->EnemyToSpawn = null;

                if (LevelGenerator gen = LevelGenerator::Instance())
                {
                    System::List<LevelPoint> points = gen.LevelPathPoints();
                    if (points != null && points.Count() > 0)
                    {
                        LevelPoint point = points[0];
                        if (point)
                            gen.EnemySpawn(enemy, point.GetTransform().GetPosition());
                    }
                }
            }

            if (G->CommandBuffer == null)
            {
                G->CommandBuffer = CommandBuffer::New();
                System::GCHandle::Alloc(G->CommandBuffer, true);
            }

            if (G->Bundle == null)
            {
                auto ptr = Hax::Gui::GetResourceData((Hax::Handle)G->Handle, IDR_BUNDLE1, L"BUNDLE");

                auto bundleRaw = Array<Char>::CreateInstance(ptr.Size());

                memcpy(bundleRaw.begin(), ptr.Data(), ptr.Size());
                G->Bundle = AssetBundle::LoadFromMemory(bundleRaw);
                G->Bundle.SetHideFlags(HideFlags::DontUnloadUnusedAsset);

                Hax::LogDebug(G->Logger, L"Bundle loaded");
            }

            if (!G->ItemsChamsMat && G->Bundle)
            {
                Shader shader = As<Shader>(G->Bundle.LoadAsset(String::New("assets/myshader.shader"), Shader::typeof()));

                G->ItemsChamsMat = Material::New(shader);
                G->ItemsChamsMat.SetHideFlags(HideFlags::DontUnloadUnusedAsset);
                G->ItemsChamsMat.SetColor(Color(1.f, 1.f, 0.9f, 1.f));
                GCHandle::Alloc(G->ItemsChamsMat, true);
                Hax::LogDebug(G->Logger, L"Items chams mat loaded");

                G->PlayerChamsMat = Material::New(shader);
                G->PlayerChamsMat.SetHideFlags(HideFlags::DontUnloadUnusedAsset);
                G->PlayerChamsMat.SetColor(Color::green());
                GCHandle::Alloc(G->PlayerChamsMat, true);
                Hax::LogDebug(G->Logger, L"Player chams mat loaded");

                G->HeadChamsMat = Material::New(shader);
                G->HeadChamsMat.SetHideFlags(HideFlags::DontUnloadUnusedAsset);
                G->HeadChamsMat.SetColor(Color(0x54 / 255.f, 0x54 / 255.f, 0x54 / 255.f, 1.f));
                GCHandle::Alloc(G->HeadChamsMat, true);
                Hax::LogDebug(G->Logger, L"Head chams mat loaded");
            }

            if (G->SetToZero)
            {
                G->SetToZero = false;
                if (ValuableDirector dir = ValuableDirector::instance())
                {
                    for (ValuableObject obj : dir.valuableList())
                    {
                        if (obj)
                            Heal(obj.physGrabObject().impactDetector(), -999999.f);
                    }
                }
            }

            if (G->SetToMax)
            {
                G->SetToMax = false;
                if (ValuableDirector dir = ValuableDirector::instance())
                {
                    for (ValuableObject obj : dir.valuableList())
                    {
                        if (obj)
                            Heal(obj.physGrabObject().impactDetector(), 999999.f);
                    }
                }
            }

            if (G->ItemToSpawn)
            {
                Item item = G->ItemToSpawn;
                G->ItemToSpawn = nullptr;
                SpawnItem(item);
            }

            if (G->ValuableToSpawn >= 0)
            {
                int index = G->ValuableToSpawn;
                G->ValuableToSpawn = -1;
                SpawnValuable(index);
            }

            if (G->ForceNextLevel)
            {
                G->ForceNextLevel = false;
                if (RunManager manager = RunManager::instance())
                    manager.ChangeLevel(true, false);
            }

            if (G->ActivateNextPoint)
            {
                G->ActivateNextPoint = false;
                if (RoundDirector dir = RoundDirector::instance())
                {
                    for (Unity::GameObject go : dir.extractionPointList())
                    {
                        ExtractionPoint point = go.GetComponent<ExtractionPoint>();
                        if (point && point.currentState() == ExtractionPoint_State::Idle() && !point.isLocked())
                        {
                            point.OnClick();
                            break;
                        }
                    }
                }
            }

            if (G->TeleportToTruck)
            {
                G->TeleportToTruck = false;
                if (avatar && !avatar.deadSet())
                    avatar.playerTransform().SetPosition(TruckSafetySpawnPoint::instance().GetTransform().GetPosition());
            }

            if (G->PlayerToTumble)
            {
                PlayerAvatar player = G->PlayerToTumble;
                G->PlayerToTumble = nullptr;
                if (player && !player.isDisabled())
                    player.tumble().TumbleRequest(true, false);
            }

            if (G->PlayerToKill)
            {
                PlayerAvatar player = G->PlayerToKill;
                G->PlayerToKill = nullptr;
                if (player && !player.deadSet() && !G->IsClient)
                    player.PlayerDeath(-1);
            }

            if (G->PlayerToRevive)
            {
                PlayerAvatar player = G->PlayerToRevive;
                G->PlayerToRevive = nullptr;
                if (player && player.deadSet() && !G->IsClient)
                    player.Revive(false);
            }

            PlayerVoiceChat chat = avatar ? avatar.voiceChat() : nullptr;
            if (chat && avatar.voiceChatFetched() && G->ChatPrefered != ChatPref::None)
            {
                bool isDeadPrefered = G->ChatPrefered == ChatPref::Dead;
                if (chat.inLobbyMixer() != isDeadPrefered)
                {
                    chat.ToggleMixer(isDeadPrefered, false);
                    Hax::LogDebug(G->Logger, L"Chat changed");
                }
                G->ChatPrefered = ChatPref::None;
            }

            if (G->UnlockAllCosmetic && MetaManager::instance())
            {
                G->UnlockAllCosmetic = false;
                MetaManager::instance().CosmeticUnlockAll();
            }

            if (G->ResetAllCosmetic && MetaManager::instance())
            {
                G->ResetAllCosmetic = false;
                MetaManager::instance().Reset();
            }

            if (G->RarityToSpawn > 0)
            {
                int rarity = G->RarityToSpawn;
                G->RarityToSpawn = 0;
                SpawnCosmeticBox(rarity - 1);
            }
        }
        catch (Exception& ex)
        {
            String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->EventSystem_Update_Hook.unsafe_call<void, Unity::EventSystem>(__this);
    }

    static void Hooked__PlayerHealth_Hurt(PlayerHealth __this, int damage, bool savingGrace, int enemyIndex, bool hurtByHeal)
    {
        CRASH_SCOPE("hook PlayerHealth.Hurt");
        if (G->Godmode)
            return;

        G->PlayerHealth_Hurt_Hook.unsafe_call<void, PlayerHealth, int, bool, int, bool>(__this, damage, savingGrace, enemyIndex, hurtByHeal);
    }

    static void Hooked__PlayerAvatar_PlayerDeath(PlayerAvatar __this, int enemyIndex)
    {
        CRASH_SCOPE("hook PlayerAvatar.PlayerDeath");
        if (G->Godmode && __this.isLocal())
            return;

        G->PlayerAvatar_PlayerDeath_Hook.unsafe_call<void, PlayerAvatar, int>(__this, enemyIndex);
    }

    static void Hooked__PlayerController_FixedUpdate(PlayerController __this)
    {
        CRASH_SCOPE("hook PlayerController.FixedUpdate");
        try
        {
            float& walkSpeed = __this.MoveSpeed();
            float& sprintSpeed = __this.SprintSpeed();
            float& crouchSpeed = __this.CrouchSpeed();

            float backup[3] = { walkSpeed, sprintSpeed, crouchSpeed };

            if (G->InfStamina)
                __this.EnergyCurrent() = __this.EnergyStart();

            walkSpeed *= (float)G->Accel.Walking;
            sprintSpeed *= (float)G->Accel.Sprinting;
            crouchSpeed *= (float)G->Accel.Crouching;

            G->PlayerController_FixedUpdate_Hook.unsafe_call<void, PlayerController>(__this);

            walkSpeed = backup[0];
            sprintSpeed = backup[1];
            crouchSpeed = backup[2];
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");

            G->PlayerController_FixedUpdate_Hook.unsafe_call<void, PlayerController>(__this);
        }
    }

    static void Hooked__PlayerController_Update(PlayerController __this)
    {
        CRASH_SCOPE("hook PlayerController.Update");
        int& jumps = __this.JumpExtra();
        int cachedJumps = jumps;
        if (G->InfJumps)
            jumps = 9999;

        G->PlayerController_Update_Hook.unsafe_call<void, PlayerController>(__this);

        jumps = cachedJumps;
    }

    static void Hooked__PlayerTumble_TumbleRequest(PlayerTumble __this, bool isTumbling, bool playerInput)
    {
        CRASH_SCOPE("hook PlayerTumble.TumbleRequest");
        try
        {
            if (G->NoTumble && !playerInput)
            {
                PlayerAvatar avatar = __this.playerAvatar();
                if (avatar && avatar.isLocal())
                    return;
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PlayerTumble_TumbleRequest_Hook.unsafe_call<void, PlayerTumble, bool, bool>(__this, isTumbling, playerInput);
    }

    static void Hooked__FlashlightController_Update(FlashlightController __this)
    {
        CRASH_SCOPE("hook FlashlightController.Update");
        try
        {
            PlayerAvatar avatar = PlayerAvatar::instance();

            bool wasCrouching = false;
            if (G->Flashlight.InCrouch && avatar)
            {
                wasCrouching = avatar.isCrouching();
                avatar.isCrouching() = false;
            }

            G->FlashlightController_Update_Hook.unsafe_call<void, FlashlightController>(__this);

            if (G->Flashlight.InCrouch && avatar)
                avatar.isCrouching() = wasCrouching;

            Unity::Light spotLight = __this.spotlight();
            if ((float)G->Flashlight.Angle != spotLight.GetSpotAngle())
            {
                spotLight.SetSpotAngle((float)G->Flashlight.Angle);
            }

            spotLight.SetIntensity(spotLight.GetIntensity() * (G->Flashlight.Intensity / 10.f));
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");

            G->FlashlightController_Update_Hook.unsafe_call<void, FlashlightController>(__this);
        }
    }

    static UpgradeData* GetUpgradeInfoFromName(System::String name)
    {
        for (int i = 0; i < (int)UpgradeType::N; ++i)
        {
            if (name.ToHaxView().EndsWith(G->UpgradesData[i].InternalNameEnd))
                return &G->UpgradesData[i];
        }
        return nullptr;
    }

    static void Hooked__ItemUpgrade_Update(ItemUpgrade __this)
    {
        CRASH_SCOPE("hook ItemUpgrade.Update");
        try
        {
            G->UpgradesPool.GetBack().PushBack(__this);

            if (G->AutoUseUpgrs)
            {
                UpgradeData* data = GetUpgradeInfoFromName(__this.itemAttributes().itemAssetName());
                if (data != nullptr && data->AutoUse)
                {
                    ItemToggle toggle = __this.itemToggle();
                    if (toggle)
                    {
                        toggle.photonView() = __this.photonView();
                        toggle.ToggleItem(true, SemiFunc::PhotonViewIDPlayerAvatarLocal());
                    }
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->ItemUpgrade_Update_Hook.unsafe_call<void, ItemUpgrade>(__this);
    }

    static void Hooked__ItemHealthPack_Update(ItemHealthPack __this)
    {
        CRASH_SCOPE("hook ItemHealthPack.Update");
        try
        {
            RunManager runManager = RunManager::instance();
            if (runManager && !runManager.levelIsShop())
                G->AidsPool.GetBack().PushBack(__this);
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->ItemHealthPack_Update_Hook.unsafe_call<void, ItemHealthPack>(__this);
    }

    static void Hooked__UpdateLoop()
    {
        CRASH_SCOPE("hook PlayerLoop.Update");
        G->UpgradesPool.GetBack().Clear();
        G->AidsPool.GetBack().Clear();

        G->UpdateHook.unsafe_call<void>();

        G->UpgradesPool.RefreshSpare();
        G->AidsPool.RefreshSpare();
    }

    static void SetThirdPerson()
    {
        static String playerTag = null;
        if (playerTag == null)
        {
            playerTag = System::String::New("Player");
            GCHandle::Alloc(playerTag, true);
        }

        PlayerAvatar avatar = PlayerAvatar::instance();

        float dist = 3.f;
        Transform normalTransformPivot = avatar.spectatePoint();
        static int layerMask = SemiFunc::LayerMaskGetVisionObstruct();
        auto hits = Unity::Physics::SphereCastAll(normalTransformPivot.GetPosition(), 0.1f, normalTransformPivot.GetForward() * -1.f, 3.f, layerMask);
        bool isMapOpen = avatar.mapToolController() && avatar.mapToolController().Active();
        if (hits != null && !isMapOpen)
        {
            for (RaycastHit& hit : hits)
            {
                Transform transform = hit.GetTransform();
                if (!transform.GetComponent<PlayerHealthGrab>() && !transform.GetGameObject().CompareTag(playerTag) && !transform.GetComponent<PlayerTumble>() && !transform.GetComponent<EnemyRigidbody>())
                    dist = Hax::Min(Hax::Max(1.f, hit.m_Distance), 3.f);
            }
        }
        else if (isMapOpen)
            dist = 1.f;

        SemiFunc::MainCamera().GetTransform().SetLocalPosition(Unity::Vector3(0.f, 0.f, -dist));
    }

    static bool ShouldDrawEsp()
    {
        return G->IsInGame && MenuManager::instance() && !MenuManager::instance().currentMenuPage();
    }

    static float prevPlane;
    static float prevFov;
    static void Hooked__PostLateUpdateLoop()
    {
        CRASH_SCOPE("hook PlayerLoop.PostLateUpdate");
        try
        {
            Unity::Camera mainCam = SemiFunc::MainCamera();
            PlayerAvatar avatar = PlayerAvatar::instance();
            if (mainCam && G->IsInGame && avatar)
            {
                if (G->BetterVision)
                {
                    prevPlane = mainCam.GetFarClipPlane();
                    if (prevPlane < 32.f)
                        mainCam.SetFarClipPlane(32.f);
                }

                prevFov = mainCam.GetFieldOfView();
                if (prevFov < (float)G->Fov)
                    mainCam.SetFieldOfView((float)G->Fov);

                if (G->ThirdPerson && !SpectateCamera::instance())
                    SetThirdPerson();

                if (ShouldDrawEsp())
                {
                    G->PixelHeight = (float)mainCam.GetPixelHeight();
                    G->PixelWidth = (float)mainCam.GetPixelWidth();
                    G->ScreenHeight = (float)Unity::Screen::GetHeight();
                    G->ScreenWidth = (float)Unity::Screen::GetWidth();

                    if (G->EnemiesEsp)
                    {
                        auto& back = G->EnemiesEspBuffer.GetBack();
                        back.Clear();
                        if (EnemyDirector dir = EnemyDirector::instance())
                        {
                            for (EnemyParent parent : dir.enemiesSpawned())
                            {
                                Enemy enemy = parent.enemy();
                                if (enemy && parent.Spawned())
                                {
                                    Hax::Optional<EnemyEspData> data = ParseEnemyEspData(enemy);
                                    if (data.HasValue())
                                        back.PushBack(*data);
                                }
                            }
                        }
                        G->EnemiesEspBuffer.RefreshSpare();
                    }

                    if (G->ValuablesEsp)
                    {
                        auto& back = G->ValuablesEspBuffer.GetBack();
                        back.Clear();
                        if (ValuableDirector dir = ValuableDirector::instance())
                        {
                            for (ValuableObject obj : dir.valuableList())
                            {
                                Hax::Optional<ValuableEspData> data = ParseValuableEspData(obj);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        G->ValuablesEspBuffer.RefreshSpare();
                    }

                    if (G->ExtrPointsEsp)
                    {
                        auto& back = G->ExtrPointsEspBuffer.GetBack();
                        back.Clear();
                        if (RoundDirector dir = RoundDirector::instance())
                        {
                            for (Unity::GameObject go : dir.extractionPointList())
                            {
                                Hax::Optional<ExtrPointEspData> data = ParseExtrPointEspData(go);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        G->ExtrPointsEspBuffer.RefreshSpare();
                    }

                    if (G->TruckEsp)
                    {
                        auto& back = G->TruckEspBuffer.GetBack();
                        back.Clear();
                        if (TruckSafetySpawnPoint truck = TruckSafetySpawnPoint::instance())
                        {
                            Hax::Optional<TruckEspData> data = ParseTruckEspData(truck);
                            if (data.HasValue())
                                back.PushBack(*data);
                        }
                        G->TruckEspBuffer.RefreshSpare();
                    }

                    if (G->PlayersEsp)
                    {
                        auto& back = G->PlayersEspBuffer.GetBack();
                        back.Clear();
                        if (GameDirector dir = GameDirector::instance())
                        {
                            for (PlayerAvatar avatar : dir.PlayerList())
                            {
                                Hax::Optional<PlayerEspData> data = ParsePlayerEspData(avatar);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        G->PlayersEspBuffer.RefreshSpare();
                    }

                    if (G->CosmeticBoxesEsp)
                    {
                        auto& back = G->CosmeticBoxesEspBuffer.GetBack();
                        back.Clear();
                        if (RoundDirector dir = RoundDirector::instance())
                        {
                            for (CosmeticWorldObject box : dir.cosmeticWorldObjects())
                            {
                                Hax::Optional<CosmeticBoxEspData> data = ParseCosmeticBoxEspData(box);
                                if (data.HasValue())
                                    back.PushBack(*data);
                            }
                        }
                        G->CosmeticBoxesEspBuffer.RefreshSpare();
                    }
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PostLateUpdateHook.unsafe_call<void>();
    }

    static void Hooked__PresentAfterDrawLoop()
    {
        CRASH_SCOPE("hook PlayerLoop.PresentAfterDraw");
        try
        {
            Unity::Camera mainCam = SemiFunc::MainCamera();
            if (mainCam && G->IsInGame)
            {
                if (G->BetterVision)
                    mainCam.SetFarClipPlane(prevPlane);

                mainCam.SetFieldOfView(prevFov);

                if (G->ThirdPerson && !SpectateCamera::instance())
                    mainCam.GetTransform().SetLocalPosition(Unity::Vector3::zero());
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PresentAfterDrawHook.unsafe_call<void>();
    }

    static void Hooked__PhysGrabObject_PhysicsGrabbingManipulation(PhysGrabObject __this)
    {
        CRASH_SCOPE("hook PhysGrabObject.PhysicsGrabbingManipulation");
        try
        {
            if (PlayerAvatar avatar = PlayerAvatar::instance())
            {
                if (PhysGrabber grabber = avatar.physGrabber())
                {
                    float backup = grabber.grabStrength();
                    if (G->EasyGrab && backup < 20.f)
                        grabber.grabStrength() = 20.f;

                    G->PhysGrabObject_PhysicsGrabbingManipulation_Hook.unsafe_call<void, PhysGrabObject>(__this);

                    grabber.grabStrength() = backup;
                    return;
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PhysGrabObject_PhysicsGrabbingManipulation_Hook.unsafe_call<void, PhysGrabObject>(__this);
    }

    static void Hooked__PostProcessLayer_BuildCommandBuffers(Unity::PostProcessLayer __this)
    {
        CRASH_SCOPE("hook PostProcessLayer.BuildCommandBuffers");
        try
        {
            if (G->BetterVision && G->IsInGame)
            {
                __this.m_LegacyCmdBufferBeforeReflections().Clear();
                __this.m_LegacyCmdBufferBeforeLighting().Clear();
                __this.m_LegacyCmdBufferOpaque().Clear();
                __this.m_LegacyCmdBuffer().Clear();
                return;
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PostProcessLayer_BuildCommandBuffers_Hook.unsafe_call<void, Unity::PostProcessLayer>(__this);
    }

    void Hooked__PlayerAvatarVisuals_Update(PlayerAvatarVisuals __this)
    {
        CRASH_SCOPE("hook PlayerAvatarVisuals.Update");
        try
        {
            if (GameManager gm = GameManager::instance())
            {
                PlayerAvatar avatar = __this.playerAvatar();
                if (G->ThirdPerson && avatar && !__this.isMenuAvatar() && avatar.isLocal())
                {
                    __this.showSelfOverrideTimer() = Hax::Max(0.1f, __this.showSelfOverrideTimer());
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PlayerAvatarVisuals_Update_Hook.unsafe_call<void, PlayerAvatarVisuals>(__this);
    }

    static constexpr float kUnlimitedGrabRange = 999999.f;
    static constexpr float kDefaultMaxPushDistance = 2.5f;  // PhysGrabber.maxDistanceFromPlayer
    static constexpr int kGrabStateClimb = 4;               // PhysGrabber.GrabState.Climb

    // PhysGrabber.Update clamps the held object distance (mouse wheel push / pull) to
    // Min(grabRange, maxDistanceFromPlayer) before RayCheck and PhysGrabLogic run, so both limits
    // are lifted for the whole Update of the local player's grabber. Range upgrades are applied
    // outside Update (coroutine / RPC), restoring the fields afterwards does not lose them.
    // Not while climbing (holding on to a wall): the climb rope length is capped by grabRange.
    static void Hooked__PhysGrabber_Update(PhysGrabber __this)
    {
        CRASH_SCOPE("hook PhysGrabber.Update");

        bool lifted = false;
        float savedRange = 0.f;
        float savedMaxDistance = 0.f;
        float distanceBefore = 0.f;

        try
        {
            PlayerAvatar avatar = PlayerAvatar::instance();
            if (G->UnlimGrabRange && avatar && __this == avatar.physGrabber() && __this.grabState() != kGrabStateClimb)
            {
                savedRange = __this.grabRange();
                savedMaxDistance = __this.maxDistanceFromPlayer();
                distanceBefore = __this.pullerDistance();

                __this.grabRange() = kUnlimitedGrabRange;
                __this.maxDistanceFromPlayer() = kUnlimitedGrabRange;
                lifted = true;
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->PhysGrabber_Update_Hook.unsafe_call<void, PhysGrabber>(__this);

        if (!lifted)
            return;

        try
        {
            // One mouse wheel step moves the object by 0.2 m. Past the normal limit the step grows
            // with the distance, otherwise pushing something 30 m away takes ~140 wheel steps.
            float& distance = __this.pullerDistance();
            const float delta = distance - distanceBefore;
            const bool wheelStep = Hax::Abs(Hax::Abs(delta) - 0.2f) < 0.001f;
            if (wheelStep && __this.grabbed() && distanceBefore >= kDefaultMaxPushDistance)
                distance = Hax::Max(kDefaultMaxPushDistance, distanceBefore + delta * (distanceBefore / kDefaultMaxPushDistance));

            __this.grabRange() = savedRange;
            __this.maxDistanceFromPlayer() = savedMaxDistance;
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }
    }

    // EnemySpinny.SliceBorders / SliceColours (v0.5.2): white 22-67, green 157-202, yellow 247-292
    // (green also 337-22). A margin keeps the pointer away from the slice edges.
    static bool RouletteSliceRange(int result, float& from, float& to)
    {
        switch (result)
        {
        case Roulette_White:    from = 22.f;  to = 67.f;  break;
        case Roulette_Green:    from = 157.f; to = 202.f; break;
        case Roulette_Yellow:   from = 247.f; to = 292.f; break;
        default:                return false;
        }
        from += 6.f;
        to -= 6.f;
        return true;
    }

    // The host's game picks the stop angle (Random.Range(0, 360)) and sends it to everyone with
    // GetAndSyncRandomRotationRPC; clients only accept it from the host. With a forced result the
    // angle is picked inside the chosen slice and sent the same way.
    static void Hooked__EnemySpinny_GetAndSyncRandomRotation(EnemySpinny __this)
    {
        CRASH_SCOPE("hook EnemySpinny.GetAndSyncRandomRotation");

        try
        {
            float from = 0.f, to = 0.f;
            if (!G->IsClient && RouletteSliceRange(G->RouletteResult, from, to))
            {
                const float angle = from + (to - from) * ((float)rand() / (float)RAND_MAX);
                const int minSpins = __this.minExtraSpins();
                const int maxSpins = Hax::Max(minSpins, __this.maxExtraSpins());
                const int spins = minSpins + rand() % (maxSpins - minSpins + 1);

                if (SemiFunc::IsMultiplayer())
                {
                    Array<System::Object> params = { System::Int32(spins), System::Single(angle) };
                    __this.photonView().RPC(String::New("GetAndSyncRandomRotationRPC"), Photon::RpcTarget::All, params);
                }
                else
                {
                    __this.extraSpins() = spins;
                    __this.targetAngleDegrees() = angle;
                }

                Hax::LogDebug(G->Logger, L"Roulette forced to %d (angle %d)", G->RouletteResult, (int)angle);
                return;
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->EnemySpinny_GetAndSyncRandomRotation_Hook.unsafe_call<void, EnemySpinny>(__this);
    }

    static void Hooked__PhysGrabber_PhysGrabLogic(PhysGrabber __this)
    {
        CRASH_SCOPE("hook PhysGrabber.PhysGrabLogic");
        float& range = __this.grabRange();
        float cached = range;
        PlayerAvatar playerAvatar = PlayerAvatar::instance();

        if (G->UnlimGrabRange && playerAvatar && __this == playerAvatar.physGrabber())
            range = 999999.f;

        G->PhysGrabber_PhysGrabLogic_Hook.unsafe_call<void, PhysGrabber>(__this);

        range = cached;
    }

    static bool g_RaycastMaxDist;
    static void Hooked__PhysGrabber_RayCheck(PhysGrabber __this, bool grab)
    {
        CRASH_SCOPE("hook PhysGrabber.RayCheck");
        float& range = __this.grabRange();
        float cached = range;
        PlayerAvatar playerAvatar = PlayerAvatar::instance();

        if (G->UnlimGrabRange && playerAvatar && __this == playerAvatar.physGrabber()) 
        {
            range = 999999.f;
            g_RaycastMaxDist = true;
        }

        G->PhysGrabber_RayCheck_Hook.unsafe_call<void, PhysGrabber, bool>(__this, grab);

        range = cached;
        g_RaycastMaxDist = false;
    }

    static bool Hooked__Physics_Raycast(void* v1, void* v2, void* v3, float maxDistance, int v5, int v6)
    {
        CRASH_SCOPE("hook Physics.Raycast");
        if (g_RaycastMaxDist)
            maxDistance = 999999.f;

        return G->Physics_Raycast_Hook.unsafe_call<bool, void*, void*, void*, float, int, int>(v1, v2, v3, maxDistance, v5, v6);
    }

    static void Hooked__PhysGrabObjectImpactDetector_Break(PhysGrabObjectImpactDetector __this, float a1, void* a2, int a3, bool a4)
    {
        CRASH_SCOPE("hook PhysGrabObjectImpactDetector.Break");
        if (G->Unbreakable && __this.isValuable())
            return;

        G->PhysGrabObjectImpactDetector_Break_Hook.unsafe_call<void, PhysGrabObjectImpactDetector, float, void*, int, bool>(__this, a1, a2, a3, a4);
    }

    static void Hooked__PhysGrabObjectImpactDetector_DestroyObject(PhysGrabObjectImpactDetector __this, bool effects, bool forced)
    {
        CRASH_SCOPE("hook PhysGrabObjectImpactDetector.DestroyObject");
        if (G->Unbreakable && __this.isValuable())
            return;

        G->PhysGrabObjectImpactDetector_DestroyObject_Hook.unsafe_call<void, PhysGrabObjectImpactDetector, bool, bool>(__this, effects, forced);
    }

    static bool g_PretendMaster;
    static bool Hooked__PhotonNetwork_IsMasterClient()
    {
        CRASH_SCOPE("hook PhotonNetwork.IsMasterClient");
        if (g_PretendMaster)
        {
            g_PretendMaster = false;
            return true;
        }
        return G->PhotonNetwork_IsMasterClient_Hook.unsafe_call<bool>();
    }

    static void Hooked__ItemBattery_Update(ItemBattery __this)
    {
        CRASH_SCOPE("hook ItemBattery.Update");
        try
        {
            // Since v0.5 the host keeps the charge in batteryLife (0..100) and recomputes
            // batteryLifeInt (the bars guns check before shooting) from it every frame, so only
            // raising batteryLifeInt is undone right away. Fill batteryLife, then sync the bars
            // silently (no charge sound after every shot); clients get it through the game's RPC.
            if (G->InfBattery && !G->IsClient && __this.physGrabObject() && __this.physGrabObject().grabbedLocal()
                && (__this.batteryLife() < 100.f || __this.batteryLifeInt() < __this.batteryBars()))
            {
                __this.batteryLife() = 100.f;
                __this.BatteryFullPercentChange(__this.batteryBars(), false, false);
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->ItemBattery_Update_Hook.unsafe_call<void, ItemBattery>(__this);
    }

    static void Hooked__Camera_FireOnPreRender(Unity::Camera __this)
    {
        CRASH_SCOPE("hook Camera.FireOnPreRender");
        try
        {
            if (IsInGame() && __this == GameDirector::instance().MainCamera() && G->ItemsChamsMat != null && G->CommandBuffer != null)
            {
                G->CommandBuffer.Clear();

                static Unity::Camera s_PrevCam = null;
                if (__this && __this != s_PrevCam)
                {
                    s_PrevCam = __this;
                    __this.AddCommandBuffer(Unity::CameraEvent::AfterEverything, G->CommandBuffer);
                }

                if (G->ValuablesChams && ValuableDirector::instance())
                {
                    for (ValuableObject obj : ValuableDirector::instance().valuableList())
                    {
                        if (obj && obj.GetEnabled())
                        {
                            for (Unity::MeshRenderer renderer : obj.GetTransform().GetChild(0).GetComponentsInChildren<Unity::MeshRenderer>(true))
                            {
                                G->CommandBuffer.DrawRenderer(renderer, G->ItemsChamsMat);
                            }
                        }
                    }
                }

                if (G->PlayersChams && GameDirector::instance())
                {
                    for (PlayerAvatar player : GameDirector::instance().PlayerList())
                    {
                        DrawPlayerChams(player, G->CommandBuffer, G->PlayerChamsMat, G->HeadChamsMat);
                    }
                }
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->Camera_FireOnPreRender_Hook.unsafe_call<void, Unity::Camera>(__this);
    }

    static void Hooked__ItemGun_Update(ItemGun __this)
    {
        CRASH_SCOPE("hook ItemGun.Update");
        try
        {
            Unity::LineRenderer laser = __this.GetComponent<Unity::LineRenderer>();

            if (__this.physGrabObject().grabbedLocal() && G->UseLaser)
            {
                if (!laser)
                {
                    laser = __this.GetGameObject().AddComponent<Unity::LineRenderer>();
                    laser.SetStartWidth(0.02f);
                    laser.SetEndWidth(0.02f);
                    laser.SetSortingOrder(1);
                    Unity::Material mat = Unity::Material::New(Unity::Shader::Find(System::String::New("Sprites/Default")));
                    mat.SetColor(Unity::Color::red());
                    laser.SetMaterial(mat);
                    laser.SetPositionCount(2);
                }

                Unity::Transform transform = __this.gunMuzzle();
                laser.SetEnabled(true);
                laser.SetPosition(0, transform.GetPosition());
                laser.SetPosition(1, transform.GetPosition() + transform.GetForward() * __this.gunRange());
            }
            else if (laser)
            {
                laser.SetEnabled(false);
            }
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->ItemGun_Update_Hook.unsafe_call<void, ItemGun>(__this);
    }

    static void Hooked__RunManager_SetRunLevel(RunManager __this)
    {
        CRASH_SCOPE("hook RunManager.SetRunLevel");
        try
        {
            if (G->LevelBans.Size() > 0)
            {
                List<Level> levels = RunManager::instance().levels();

                LevelBan* levelBan = &G->LevelBans[0];
                Level prevLevel = __this.previousRunLevel();

                while (!levelBan->Allowed || ((G->LevelBans.Size() - G->TotalBans) > 1 && prevLevel && levels[levelBan->Index] == prevLevel))
                    levelBan = &G->LevelBans[rand() % G->LevelBans.Size()];

                __this.levelCurrent() = levels[levelBan->Index];
                return;
            }
        }
        catch (Exception& ex)
        {
            String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        G->RunManager_SetRunLevel_Hook.unsafe_call<void, RunManager>(__this);
    }

    static void Hooked__PhysGrabber_PhysGrabOverCharge(PhysGrabber __this, float _amount, float _multiplier)
    {
        CRASH_SCOPE("hook PhysGrabber.PhysGrabOverCharge");
        if (G->NoOvercharge)
            return;

        G->PhysGrabber_PhysGrabOverCharge_Hook.unsafe_call<void, PhysGrabber, float, float>(__this, _amount, _multiplier);
    }

    static void Hooked__SpectateCamera_HeadEnergyLogic(SpectateCamera __this)
    {
        CRASH_SCOPE("hook SpectateCamera.HeadEnergyLogic");
        if (G->MaxHeadBattery)
        {
            __this.headEnergy() = 9999.f;
            __this.headEnergyEnough() = true;
        }

        G->SpectateCamera_HeadEnergyLogic_Hook.unsafe_call<void, SpectateCamera>(__this);
    }

    static void Hooked__EnemyRigidbody_FixedUpdate(EnemyRigidbody __this)
    {
        CRASH_SCOPE("hook EnemyRigidbody.FixedUpdate");
        if (G->NoGrabMaxTime && SemiFunc::IsMasterClientOrSingleplayer())
        {
            __this.grabShakeReleaseTimer() = 0.f;
            __this.grabTimeCurrent() = 0.f;
        }

        G->EnemyRigidbody_FixedUpdate_Hook.unsafe_call<void, EnemyRigidbody>(__this);
    }

    static bool IsInGame()
    {
        RunManager runManager = RunManager::instance();
        GameDirector gameDirector = GameDirector::instance();
        return runManager && PlayerAvatar::instance() && gameDirector
            && runManager.levelCurrent() != runManager.levelLobbyMenu()
            && runManager.levelCurrent() != runManager.levelMainMenu()
            && gameDirector.currentState() == GameDirector_gameState::Main();
    }

    static void* GetPlayerLoopPtr(System::Type type, System::Type subType)
    {
        auto mainLoop = LowLevel::PlayerLoop::GetDefaultPlayerLoop();
        for (PlayerLoopSystem& loop : mainLoop.m_SubSystemList)
        {
            if (loop.m_Type != type)
                continue;

            for (PlayerLoopSystem& subLoop : loop.m_SubSystemList)
                if (subLoop.m_Type == subType)
                    return *subLoop.m_UpdateFunction;
        }
        return nullptr;
    }

    static void Heal(PhysGrabObjectImpactDetector obj, float value)
    {
        if (SemiFunc::IsMultiplayer())
        {
            Array<System::Object> params = { Single(value), Vector3() };
            obj.photonView().RPC(String::New("HealRPC"), Photon::RpcTarget::All, params);
        }
        else
        {
            obj.HealLogic(value, Vector3::zero());
        }
    }

    static void SpawnItem(Item item)
    {
        Unity::Camera camera = SemiFunc::MainCamera();
        if (!camera)
            return;

        if (!item)
            return;

        Unity::Transform transform = camera.GetTransform();
        Unity::Vector3 pos = transform.GetPosition() + transform.GetForward() * 2.f - transform.GetUp();

        if (SemiFunc::IsMasterClient())
            Photon::PhotonNetwork::InstantiateRoomObject(item.prefab().ResourcePath(), pos, Unity::Quaternion::identity());
        if (!SemiFunc::IsMultiplayer())
            Unity::Object::Instantiate<Unity::GameObject>(item.prefab().Prefab(), pos, Unity::Quaternion::identity());
    }

    static void SpawnCosmeticBox(int rarity)
    {
        Unity::Camera camera = SemiFunc::MainCamera();
        if (!camera)
            return;

        ValuableDirector dir = ValuableDirector::instance();
        if (!dir)
            return;

        auto setups = dir.cosmeticWorldObjectSetups();
        if (setups == null || rarity < 1 || rarity >= setups.Count())
            return;

        CosmeticWorldObjectSetup setup = setups[rarity];
        if (setup == null || setup.prefab() == null)
            return;

        Transform transform = camera.GetTransform();
        Vector3 pos = transform.GetPosition() + transform.GetForward() * 2.f - transform.GetUp();

        if (GameManager::instance().gameMode() == 0)
            Unity::Object::Instantiate<Unity::GameObject>(setup.prefab().Prefab(), pos, Quaternion::identity());
        else
            Photon::PhotonNetwork::InstantiateRoomObject(setup.prefab().ResourcePath(), pos, Quaternion::identity());
    }

    // Same as the game's "/spawn valuable" debug command: PhotonNetwork.InstantiateRoomObject with
    // the prefab resource path in multiplayer, Object.Instantiate in singleplayer, then
    // ValuableObject.DollarValueSetLogic. ValuableObject.Start registers it in ValuableDirector and
    // sends its value to the other players. Every physics object is indestructible for 5 seconds
    // after it spawns (PhysGrabObjectImpactDetector.indestructibleSpawnTimer).
    static void SpawnValuable(int index)
    {
        if (G->IsClient || !G->ValuablesPoolReady || index < 0 || (size_t)index >= G->ValuablesPool.Size())
            return;

        const ValuableSpawnEntry& entry = G->ValuablesPool[index];
        PrefabRef ref = entry.Ref;
        if (ref == null)
            return;

        Unity::Camera camera = SemiFunc::MainCamera();
        if (!camera)
            return;

        // In front of the player at eye height (looking up or down does not matter): big
        // valuables a bit further away so they don't spawn inside the player.
        Unity::Transform cameraTransform = camera.GetTransform();
        Unity::Vector3 forward = cameraTransform.GetForward();
        forward.Y = 0.f;
        if (forward == Unity::Vector3::zero())
            forward = Unity::Vector3(0.f, 0.f, 1.f);
        forward = Unity::Vector3::Normalized(forward);

        const bool large = entry.Size >= ValuableSize::Big;
        Unity::Vector3 pos = cameraTransform.GetPosition() + forward * (large ? 3.f : 1.8f);
        Unity::Quaternion rot = Unity::Quaternion::identity();

        Unity::GameObject spawned = null;
        if (GameManager::instance().gameMode() == 0)
        {
            Unity::GameObject prefab = ref.Prefab();
            if (prefab)
                spawned = Unity::Object::Instantiate<Unity::GameObject>(prefab, pos, rot);
        }
        else
        {
            System::String path = ref.ResourcePath();
            if (path != null && path.GetLength() > 0)
                spawned = Photon::PhotonNetwork::InstantiateRoomObject(path, pos, rot);
        }

        if (!spawned)
        {
            Hax::LogError(G->Logger, L"Valuable %.*ls was not spawned", (int)entry.Name.Length(), entry.Name.Data());
            return;
        }

        ValuableObject valuable = spawned.GetComponent<ValuableObject>();
        if (valuable)
            valuable.DollarValueSetLogic();

        Hax::LogDebug(G->Logger, L"Valuable spawned %.*ls", (int)entry.Name.Length(), entry.Name.Data());
    }

    // ---------------------------------------------------------------- valuable prices
    //
    // The price range of a valuable lives in its prefab (ValuableObject.valuePreset, or a fixed
    // dollarValueOverride), not in the level lists. Reading it means loading the prefab, so it is
    // done once: in the background (Resources.LoadAsync, a few at a time) and saved to
    // haxsdk_valuable_prices.txt. Later launches read the file and load nothing; only valuables
    // missing from it (game update) are read again, all of them if the game version changed.

    static constexpr wchar_t kPriceCacheFile[] = L"haxsdk_valuable_prices.txt";

    static void GetGameVersion(char* out, size_t outSize)
    {
        ::StringCchCopyA(out, outSize, "unknown");
        try
        {
            BuildManager build = BuildManager::instance();
            if (!build)
                return;
            GameVersionAsset version = build.version();
            if (!version)
                return;
            System::String title = version.title();
            if (title != null && title.GetLength() > 0)
            {
                int n = ::WideCharToMultiByte(CP_UTF8, 0, title.begin(), title.GetLength(), out, (int)outSize - 1, nullptr, nullptr);
                out[n > 0 ? n : 0] = 0;
            }
        }
        catch (System::Exception&)
        {
        }
    }

    // File format (UTF-8): "version=<game version>", then one "<min> <max> <name>" line per valuable.
    static void LoadPriceCache(Hax::Vector<ValuableSpawnEntry>& pool)
    {
        HANDLE file = ::CreateFileW(kPriceCacheFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            return;

        static char s_Buffer[256 * 1024];
        DWORD read = 0;
        ::ReadFile(file, s_Buffer, sizeof(s_Buffer) - 1, &read, nullptr);
        ::CloseHandle(file);
        s_Buffer[read] = 0;

        char version[64];
        GetGameVersion(version, sizeof(version));

        int loaded = 0;
        bool versionOk = false;
        char* context = nullptr;
        for (char* line = ::strtok_s(s_Buffer, "\r\n", &context); line != nullptr; line = ::strtok_s(nullptr, "\r\n", &context))
        {
            if (::strncmp(line, "version=", 8) == 0)
            {
                versionOk = ::strcmp(line + 8, version) == 0;
                continue;
            }
            if (!versionOk)
                break; // other game version: read everything again

            int priceMin = 0, priceMax = 0, nameOffset = 0;
            if (::sscanf_s(line, "%d %d %n", &priceMin, &priceMax, &nameOffset) < 2 || nameOffset <= 0)
                continue;

            wchar_t name[128];
            int n = ::MultiByteToWideChar(CP_UTF8, 0, line + nameOffset, -1, name, _countof(name));
            if (n <= 1)
                continue;

            Hax::WStringView nameView(name, (size_t)(n - 1));
            for (ValuableSpawnEntry& entry : pool)
            {
                if (entry.Name == nameView)
                {
                    entry.PriceMin = (float)priceMin;
                    entry.PriceMax = (float)priceMax;
                    entry.PriceKnown = true;
                    ++loaded;
                    break;
                }
            }
        }

        Hax::LogDebug(G->Logger, L"Valuable prices: %d taken from %ls", loaded, kPriceCacheFile);
    }

    static void SavePriceCache()
    {
        HANDLE file = ::CreateFileW(kPriceCacheFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            return;

        char version[64];
        GetGameVersion(version, sizeof(version));

        char line[512];
        DWORD written = 0;
        ::StringCchPrintfA(line, _countof(line), "version=%s\r\n", version);
        ::WriteFile(file, line, (DWORD)::strlen(line), &written, nullptr);

        for (const ValuableSpawnEntry& entry : G->ValuablesPool)
        {
            if (!entry.PriceKnown)
                continue;

            char name[256];
            int n = ::WideCharToMultiByte(CP_UTF8, 0, entry.Name.Data(), (int)entry.Name.Length(), name, (int)sizeof(name) - 1, nullptr, nullptr);
            name[n > 0 ? n : 0] = 0;

            ::StringCchPrintfA(line, _countof(line), "%d %d %s\r\n", (int)entry.PriceMin, (int)entry.PriceMax, name);
            ::WriteFile(file, line, (DWORD)::strlen(line), &written, nullptr);
        }
        ::CloseHandle(file);
    }

    // Price range from a loaded valuable prefab, rounded to hundreds like DollarValueSetLogic.
    static bool ReadValuablePrice(ValuableSpawnEntry& entry, Unity::Object asset)
    {
        Unity::GameObject prefab = As<Unity::GameObject>(asset);
        if (!prefab)
            return false;

        ValuableObject valuable = prefab.GetComponent<ValuableObject>();
        if (!valuable)
            return false;

        float priceMin = 0.f, priceMax = 0.f;
        if (int fixedValue = valuable.dollarValueOverride(); fixedValue != 0)
        {
            priceMin = priceMax = (float)fixedValue;
        }
        else
        {
            ValuePreset preset = valuable.valuePreset();
            if (!preset)
                return false;
            priceMin = preset.valueMin();
            priceMax = preset.valueMax();
        }

        auto round100 = [](float v) { return (float)(int)(v / 100.f + 0.5f) * 100.f; };
        entry.PriceMin = round100(priceMin);
        entry.PriceMax = round100(Hax::Max(priceMin, priceMax));
        entry.PriceKnown = true;
        return true;
    }

    // Runs every frame on the game thread until every price is known.
    static void UpdateValuablePrices()
    {
        static constexpr int kSlots = 4;
        static Unity::ResourceRequest s_Requests[kSlots];
        static void* s_Handles[kSlots];
        static int s_Index[kSlots] = { -1, -1, -1, -1 };
        static size_t s_Next = 0;
        static int s_Read = 0;
        static bool s_Done = false;

        if (s_Done || !G->ValuablesPoolReady)
            return;

        Hax::Vector<ValuableSpawnEntry>& pool = G->ValuablesPool;

        // Without a GC handle that can be freed again the request object can't be kept between
        // frames: load one prefab per frame synchronously (asking for the asset finishes the load).
        // Only happens until the prices are cached in the file.
        static const bool s_Async = System::GCHandle::FreeIsSafe();
        if (!s_Async)
        {
            while (s_Next < pool.Size() && pool[s_Next].PriceKnown)
                ++s_Next;

            if (s_Next < pool.Size())
            {
                const int index = (int)s_Next++;
                try
                {
                    System::String path = pool[index].Ref.ResourcePath();
                    if (path != null && path.GetLength() > 0)
                    {
                        Unity::ResourceRequest request = Unity::Resources::LoadAsync(path);
                        if (request != null && ReadValuablePrice(pool[index], request.GetAsset()))
                            ++s_Read;
                    }
                }
                catch (System::Exception&)
                {
                }
                return;
            }

            s_Done = true;
            Hax::LogDebug(G->Logger, L"Valuable prices: %d read from the game (one per frame)", s_Read);
            if (s_Read > 0)
                SavePriceCache();
            return;
        }

        bool busy = false;

        for (int slot = 0; slot < kSlots; ++slot)
        {
            if (s_Index[slot] >= 0)
            {
                bool finished = true;
                try
                {
                    if (!s_Requests[slot].GetIsDone())
                        finished = false;
                    else if (ReadValuablePrice(pool[s_Index[slot]], s_Requests[slot].GetAsset()))
                        ++s_Read;
                }
                catch (System::Exception&)
                {
                }

                if (!finished)
                {
                    busy = true;
                    continue;
                }

                System::GCHandle(s_Handles[slot]).Free();
                s_Requests[slot] = Unity::ResourceRequest();
                s_Index[slot] = -1;
            }

            while (s_Next < pool.Size() && pool[s_Next].PriceKnown)
                ++s_Next;
            if (s_Next >= pool.Size())
                continue;

            const int index = (int)s_Next++;
            try
            {
                System::String path = pool[index].Ref.ResourcePath();
                if (path != null && path.GetLength() > 0)
                {
                    Unity::ResourceRequest request = Unity::Resources::LoadAsync(path);
                    if (request != null)
                    {
                        s_Handles[slot] = System::GCHandle::Alloc(request, false).Handle;
                        s_Requests[slot] = request;
                        s_Index[slot] = index;
                        busy = true;
                    }
                }
            }
            catch (System::Exception& ex)
            {
                System::String message = ex.Message();
                Hax::LogError(G->Logger, L"%d: %ls", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
            }
        }

        if (!busy && s_Next >= pool.Size())
        {
            s_Done = true;
            Hax::LogDebug(G->Logger, L"Valuable prices: %d read from the game", s_Read);
            if (s_Read > 0)
                SavePriceCache();
        }
    }

    // All valuables of all levels (RunManager.levels -> Level.ValuablePresets -> size lists), the
    // list the game's "/spawn valuable" debug command uses. Vanilla valuables are loaded from
    // Resources, so valuables of other levels can be spawned too.
    static void ParseValuables()
    {
        RunManager manager = RunManager::instance();
        if (!manager)
            return;

        System::List<Level> levels = manager.levels();
        if (levels == null)
            return;

        Hax::Vector<ValuableSpawnEntry> pool;
        static constexpr Hax::WStringView kPrefix = L"Valuable ";

        for (int i = 0; i < levels.Count(); ++i)
        {
            Level level = levels[i];
            if (!level)
                continue;

            System::List<LevelValuables> presets = level.ValuablePresets();
            if (presets == null)
                continue;

            for (LevelValuables preset : presets)
            {
                if (!preset)
                    continue;

                System::List<PrefabRef> groups[(int)ValuableSize::N] =
                {
                    preset.tiny(), preset.smallList(), preset.medium(), preset.big(),
                    preset.wide(), preset.tall(), preset.veryTall()
                };

                for (int size = 0; size < (int)ValuableSize::N; ++size)
                {
                    if (groups[size] == null)
                        continue;

                    for (PrefabRef ref : groups[size])
                    {
                        if (ref == null)
                            continue;

                        System::String resourcePath = ref.resourcePath();
                        System::String prefabName = ref.prefabName();
                        if (resourcePath == null || resourcePath.GetLength() == 0 || prefabName == null || prefabName.GetLength() == 0)
                            continue;

                        Hax::WStringView name = prefabName.ToHaxView();
                        if (name.StartsWith(kPrefix) && name.Length() > kPrefix.Length())
                            name = name.Substr(kPrefix.Length());

                        bool known = false;
                        for (const ValuableSpawnEntry& entry : pool)
                        {
                            if (entry.Name == name)
                            {
                                known = true;
                                break;
                            }
                        }
                        if (known)
                            continue;

                        ValuableNameParts ru = TranslateValuableName(name);

                        ValuableSpawnEntry entry{};
                        entry.Name = name;
                        entry.NameRu = ru.NameRu;
                        entry.ThemeRu = ru.ThemeRu;
                        entry.Ref = ref;
                        entry.Size = (ValuableSize)size;
                        pool.PushBack(entry);
                    }
                }
            }
        }

        if (pool.Empty())
            return;

        for (const ValuableSpawnEntry& entry : pool)
            Hax::LogDebug(G->Logger, L"Valuable parsed %.*ls (size %d)", (int)entry.Name.Length(), entry.Name.Data(), (int)entry.Size);

        LoadPriceCache(pool);

        // Menu order for each language: by the shown name, then by the theme.
        for (int lang = 0; lang < Lang_Count; ++lang)
        {
            Hax::Vector<int>& order = G->ValuablesOrder[lang];
            order.Clear();
            for (size_t i = 0; i < pool.Size(); ++i)
                order.PushBack((int)i);

            std::sort(order.begin(), order.end(), [&pool, lang](int a, int b)
            {
                const ValuableSpawnEntry& ea = pool[a];
                const ValuableSpawnEntry& eb = pool[b];
                if (lang != Lang_Ru)
                    return MenuNameLess(ea.Name, eb.Name);
                if (MenuNameLess(ea.NameRu, eb.NameRu))
                    return true;
                if (MenuNameLess(eb.NameRu, ea.NameRu))
                    return false;
                return MenuNameLess(ea.ThemeRu, eb.ThemeRu);
            });
        }

        // The menu (render thread) only reads the pool after it is complete.
        G->ValuablesPool = std::move(pool);
        G->ValuablesPoolReady = true;
    }

    static void ParseEnemies()
    {
        EnemyDirector dir = EnemyDirector::instance();
        if (!G->EnemiesPool.Empty() || !dir)
            return;

        Hax::LogDebug(G->Logger, L"Parsing enemies");
        //G->LoadRequests.reserve(50);

        List<EnemySetup> setups[3] = {dir.enemiesDifficulty1(), dir.enemiesDifficulty2(), dir.enemiesDifficulty3()};
        for (size_t i = 0; i < 3; ++i)
        {
            for (EnemySetup setup : setups[i])
            {
                if (!setup)
                    continue;

                String name = setup.GetName();
                if (name.Contains(L"Bang") || name.Contains(L"Gnome"))
                    continue;

                for (PrefabRef ref : setup.spawnObjects())
                {
                    if (ref == null)
                        continue;

                    String prefabName = ref.prefabName();
                    if (prefabName == null || prefabName.GetLength() == 0)
                        continue;

                    String resourcePath = ref.resourcePath();
                    if (resourcePath != null && resourcePath.GetLength() > 0)
                        Unity::Resources::LoadAsync(resourcePath);

                    // Prefabs are named "Enemy - <Name>"; keep the whole name if the game changes that.
                    static constexpr Hax::WStringView kEnemyPrefix = L"Enemy - ";
                    Hax::WStringView enemyName = prefabName.StartsWith(kEnemyPrefix) && prefabName.GetLength() > (int)kEnemyPrefix.Length()
                        ? Hax::WStringView(prefabName.begin() + kEnemyPrefix.Length())
                        : Hax::WStringView(prefabName.begin());

                    EnemySetup& entry = G->EnemiesPool.FindOrAdd(enemyName);
                    if (!entry)
                    {
                        entry = setup;
                        GCHandle::Alloc(name, true);
                        Hax::LogDebug(G->Logger, L"Enemy parsed %ls", enemyName.begin());
                    }
                }
            }
        }
    }

    static void ParseItems()
    {
        if (StatsManager manager = StatsManager::instance())
        {
            auto itemDict = manager.itemDictionary();
            int nItems = itemDict.Count();

            if (itemDict.Count() == 0)
                manager.LoadItemsFromFolder();

            for (auto& entry : itemDict)
            {
                if (!entry.Value)
                    continue;

                System::String itemName = entry.Value.itemName();
                if (itemName == null || itemName.GetLength() == 0)
                    continue;

                Hax::WStringView name = itemName.ToHaxView();

                if (!G->ItemsPool.Contains(name))
                {
                    G->ItemsPool.Insert(name, entry.Value);

                    PrefabRef prefab = entry.Value.prefab();
                    if (prefab != null)
                    {
                        System::String resourcePath = prefab.resourcePath();
                        if (resourcePath != null && resourcePath.GetLength() > 0)
                            Resources::LoadAsync(resourcePath);
                    }

                    Hax::LogDebug(G->Logger, L"Item parsed %s", name.begin());
                }
            }
        }
    }

    static void ParseLevels()
    {
        if (RunManager manager = RunManager::instance())
        {
            Hax::LogDebug(G->Logger, L"Parsing levels");

            System::List<Level> levels = manager.levels();
            G->LevelBans.Reserve(levels.Count());

            for (int i = 0; i < levels.Count(); ++i)
            {
                if (!levels[i])
                    continue;

                System::String levelName = levels[i].NarrativeName();
                if (levelName == null)
                    continue;

                LevelBan ban = {levelName.ToHaxView(), i, true};
                G->LevelBans.PushBack(ban);

                GCHandle::Alloc(levelName, true);
                Hax::LogDebug(G->Logger, L"Parsed level %s", levelName.begin());
            }

            std::sort(G->LevelBans.begin(), G->LevelBans.end(), [](const LevelBan& o1, const LevelBan& o2) { return o1.Name < o2.Name; });
        }
    }

    static void DrawPlayerChams(PlayerAvatar player, Unity::CommandBuffer cb, Unity::Material aliveMat, Unity::Material deadMat)
    {
        if (cb == null || !aliveMat || !deadMat || !player || player.isLocal())
            return;

        if (player.deadSet())
        {
            PlayerDeathHead head = player.playerDeathHead();
            if (head && head.GetEnabled())
            {
                for (Unity::MeshRenderer r : head.meshRenderers())
                    cb.DrawRenderer(r, deadMat);
            }
            return;
        }

        if (!player.isDisabled())
        {
            for (Unity::MeshRenderer renderer : player.playerHealth().renderers())
            {
                if (renderer && renderer.GetEnabled())
                    cb.DrawRenderer(renderer, aliveMat);
            }
        }
    }
}