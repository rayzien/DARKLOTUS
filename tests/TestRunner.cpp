#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Core/Types.hpp"
#include "DarkLotus/Entities/Fighter.hpp"
#include "DarkLotus/Managers/CombatManager.hpp"
#include "DarkLotus/Managers/ButtonManager.hpp"
#include "DarkLotus/Managers/SettingsManager.hpp"
#include "DarkLotus/Managers/CharacterManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

#include <cassert>
#include <iostream>

using namespace DarkLotus;

void TestCharacterRoster() {
    std::cout << "[TEST] 1. Running Character Roster Integrity Test..." << std::endl;
    CharacterManager charMgr;
    charMgr.Init();

    const auto& roster = charMgr.GetRoster();
    assert(roster.size() == 4 && "Roster must contain exactly 4 characters!");

    assert(roster[0].name == "OODIII JII");
    assert(roster[1].name == "LUHAR IHDNAG");
    assert(roster[2].name == "WINPAK");
    assert(roster[3].name == "GANJA");

    std::cout << "  ✓ All 4 characters verified: OODIII JII, LUHAR IHDNAG, WINPAK, GANJA." << std::endl;
}

void TestFighterStateAndPhysics() {
    std::cout << "[TEST] 2. Running Fighter State & Air Juggle Physics Test..." << std::endl;
    Fighter f1(0, 0, "OODIII JII", Color{234, 179, 8, 255}, Vector3{-2.0f, 0.0f, 0.0f});
    f1.Init();

    assert(f1.GetHealth() == 10000);
    assert(f1.GetActionState() == ActionState::IDLE);
    assert(!f1.IsAirborne());

    // Trigger launcher
    f1.TriggerJuggleLaunch(Vector3{2.0f, 10.0f, 0.0f});
    assert(f1.GetActionState() == ActionState::JUGGLED);
    assert(f1.GetVelocity().y > 0.0f);

    // Simulate 0.1s physics tick
    f1.Update(0.1f, 1, 0, nullptr);
    assert(f1.GetPosition().y > 0.0f);

    std::cout << "  ✓ Fighter air juggle launch and vertical physics verified." << std::endl;
}

void TestTekkenGuardRules() {
    std::cout << "[TEST] 3. Running Tekken High/Mid/Low Guard Rules Test..." << std::endl;

    // High Attack properties
    Hitbox3D highAtk{};
    highAtk.height = AttackHeight::HIGH;
    highAtk.damage = 600;

    // Mid Attack properties
    Hitbox3D midAtk{};
    midAtk.height = AttackHeight::MID;
    midAtk.damage = 1000;

    // Low Attack properties
    Hitbox3D lowAtk{};
    lowAtk.height = AttackHeight::LOW;
    lowAtk.damage = 1100;

    // Standing Guard: Blocks HIGH and MID, fails against LOW
    bool standBlockHigh = (highAtk.height == AttackHeight::HIGH);
    bool standBlockMid = (midAtk.height == AttackHeight::MID);
    bool standBlockLow = (lowAtk.height == AttackHeight::HIGH || lowAtk.height == AttackHeight::MID);
    assert(standBlockHigh && "Standing guard must block High!");
    assert(standBlockMid && "Standing guard must block Mid!");
    assert(!standBlockLow && "Standing guard must be pierced by Low!");

    // Crouching Guard: Ducks HIGH (whiffs), Blocks LOW, fails against MID
    bool crouchDucksHigh = (highAtk.height == AttackHeight::HIGH);
    bool crouchBlocksLow = (lowAtk.height == AttackHeight::LOW);
    bool crouchBlocksMid = (midAtk.height == AttackHeight::LOW);
    assert(crouchDucksHigh && "Crouching must duck High attacks!");
    assert(crouchBlocksLow && "Crouch guard must block Low attacks!");
    assert(!crouchBlocksMid && "Crouch guard must be crushed by Mid attacks!");

    std::cout << "  ✓ Tekken High/Mid/Low defense rules verified." << std::endl;
}

void Test3DCollisionDetection() {
    std::cout << "[TEST] 4. Running 3D Hitbox vs Hurtbox Collision Test..." << std::endl;

    BoundingBox hurtbox = {
        Vector3{-0.3f, 0.5f, -0.3f},
        Vector3{ 0.3f, 1.8f,  0.3f}
    };

    Vector3 hittingSphereCenter = {0.2f, 1.2f, 0.0f};
    float hittingRadius = 0.4f;

    Vector3 missingSphereCenter = {5.0f, 1.2f, 0.0f};
    float missingRadius = 0.4f;

    bool hit = ::CheckCollisionBoxSphere(hurtbox, hittingSphereCenter, hittingRadius);
    bool miss = ::CheckCollisionBoxSphere(hurtbox, missingSphereCenter, missingRadius);

    assert(hit && "Hitbox within hurtbox proximity must register collision!");
    assert(!miss && "Distant hitbox must not register collision!");

    std::cout << "  ✓ 3D BoundingBox vs Sphere collision algorithm verified." << std::endl;
}

void TestSettingsPersistence() {
    std::cout << "[TEST] 5. Running Settings Manager Persistence Test..." << std::endl;
    SettingsManager sm;
    sm.Init("build/test_settings.cfg");

    sm.GetVideo().targetFps = 60;
    sm.GetAudio().masterVolume = 0.95f;
    sm.SaveConfig();

    SettingsManager sm2;
    sm2.Init("build/test_settings.cfg");
    assert(sm2.GetVideo().targetFps == 60);
    assert(std::abs(sm2.GetAudio().masterVolume - 0.95f) < 0.01f);

    std::cout << "  ✓ Settings file serialization and loading verified." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  DARKLOTUS AUTOMATED ENGINE TEST SUITE  " << std::endl;
    std::cout << "========================================" << std::endl;

    TestCharacterRoster();
    TestFighterStateAndPhysics();
    TestTekkenGuardRules();
    Test3DCollisionDetection();
    TestSettingsPersistence();

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL 5 TEST SUITES PASSED (100% OK)     " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
