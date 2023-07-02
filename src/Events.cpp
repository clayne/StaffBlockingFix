#include "Events.h"

namespace Events {
    InputEventHandler* InputEventHandler::GetSingleton() {
        static InputEventHandler singleton;
        return std::addressof(singleton);
    }

    RE::BSEventNotifyControl InputEventHandler::ProcessEvent(RE::InputEvent* const* a_event,
                                                             RE::BSTEventSource<RE::InputEvent*>* a_eventSource) {
        if (!a_event) return RE::BSEventNotifyControl::kContinue;

        if (const auto ui = RE::UI::GetSingleton();
            !ui->GameIsPaused() && !ui->IsApplicationMenuOpen() && !ui->IsItemMenuOpen()
            && !ui->IsMenuOpen(RE::InterfaceStrings::GetSingleton()->dialogueMenu)) {
            if (const auto player = RE::PlayerCharacter::GetSingleton()) {
                if (const auto control_map = RE::ControlMap::GetSingleton(); control_map->IsFightingControlsEnabled()) {
                    if (const auto player_controls = RE::PlayerControls::GetSingleton();
                        player_controls->attackBlockHandler->inputEventHandlingEnabled) {
                        if (const auto player_state = player->AsActorState();
                            player_state->GetWeaponState() == RE::WEAPON_STATE::kDrawn
                            && player_state->GetSitSleepState() == RE::SIT_SLEEP_STATE::kNormal
                            && player_state->GetKnockState() == RE::KNOCK_STATE_ENUM::kNormal
                            && player_state->GetFlyState() == RE::FLY_STATE::kNone
                            && !player->IsInKillMove()) {
                            if (const auto player_ai = player->GetActorRuntimeData().currentProcess) {
                                const auto equipped_left = player_ai->GetEquippedLeftHand();
                                const auto equipped_right = player_ai->GetEquippedRightHand();

                                if (!equipped_left && equipped_right) {
                                    const auto right_weap = equipped_right->As<RE::TESObjectWEAP>();
                                    if (right_weap->IsStaff()) {
                                        auto is_blocking = false;
                                        if (player->GetGraphVariableBool("IsBlocking"sv, is_blocking)) {
                                            for (auto e = *a_event; e != nullptr; e = e->next) {
                                                if (const auto button_event = e->AsButtonEvent()) {
                                                    const auto device = button_event->GetDevice();
                                                    if (button_event->GetIDCode() == control_map->GetMappedKey(
                                                            RE::UserEvents::GetSingleton()->leftAttack, device)) {
                                                        if (button_event->IsHeld()) {
                                                            logger::info("Block held");
                                                            right_weap->weaponData.animationType =
                                                                RE::WEAPON_TYPE::kTwoHandAxe;
                                                            RE::DebugNotification("Set staff anim type to two handed");
                                                        }
                                                        if (button_event->IsUp()) {
                                                            logger::info("Block up");
                                                            right_weap->weaponData.animationType =
                                                                RE::WEAPON_TYPE::kStaff;
                                                            RE::DebugNotification("Reset staff anim type to staff");
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void InputEventHandler::Register() {
        const auto manager = RE::BSInputDeviceManager::GetSingleton();
        manager->AddEventSink(GetSingleton());
        logger::info("Registered input event handler");
    }
}
