#pragma once

#include "DarkLotus/Core/Common.hpp"

namespace DarkLotus {

// Tekken 4-button fighting system
enum InputButtons : unsigned int {
    BTN_NONE       = 0,
    BTN_UP         = 1 << 0,  // Sidestep Up (Z-)
    BTN_DOWN       = 1 << 1,  // Sidestep Down (Z+)
    BTN_LEFT       = 1 << 2,  // Retreat / Block (for P1 facing right)
    BTN_RIGHT      = 1 << 3,  // Advance (for P1 facing right)
    BTN_LP         = 1 << 4,  // 1: Left Punch (Square / X)
    BTN_RP         = 1 << 5,  // 2: Right Punch (Triangle / Y)
    BTN_LK         = 1 << 6,  // 3: Left Kick (X / A)
    BTN_RK         = 1 << 7,  // 4: Right Kick (Circle / B)
    BTN_HEAT_RAGE  = 1 << 8,  // R1 / RB: Heat Burst or Rage Art (1+2+3+4)
    BTN_THROW      = 1 << 9,  // 1+3 or 2+4 Throw
    BTN_BLOCK      = 1 << 10  // Guard button (manual or directional)
};

// Hit levels follow standard Tekken rules:
// - HIGH: Duckable by crouching, blocked standing.
// - MID: Must block standing, hits crouching opponents (counter to ducking).
// - LOW: Must block crouching (duck block), beats standing guard.
// - SPECIAL_MID: Can be blocked standing or crouching.
// - UNBLOCKABLE: Cannot be guarded.
// - THROW: High/Mid throw breakable with buttons.
enum class AttackHeight {
    HIGH,
    MID,
    LOW,
    SPECIAL_MID,
    UNBLOCKABLE,
    THROW
};

// Martial arts attack type categories for procedural bone articulation
enum class AttackType {
    NONE,
    PUNCH_LEFT,            // 1: Left Jab / Lead Hook (Xbox X / U / Z)
    PUNCH_RIGHT,           // 2: Right Straight / Cross / Body Blow (Xbox Y / I / X)
    KICK_LEFT,             // 3: Left Kick / Front Snap Kick / Shin Kick (Xbox A / J / C)
    KICK_RIGHT,            // 4: Right Kick / High Roundhouse / Knee Smash (Xbox B / K / V)
    UPPERCUT_LAUNCHER,     // d+2: Electric Wind God Fist / Jet Upper / Rising Elbow / Thrusting Demon
    LOW_SWEEP,             // d+3: Mishima Hell Sweep / Snake Edge / Dragon Sweep
    DUAL_STRIKE,           // 1+2: Demon Breath Palm / Mach Punch / Tetsuzanko (Xbox RB)
    FLYING_DIVE_DROPKICK,  // 3+4: Raiden Torpedo Dive / Kangaroo Dropkick / Can-Can Kicks (Xbox RT)
    THROW_SLAM,            // 1+3: Command Grab / Body Slam (Xbox LT)
    RAGE_ART               // Heat Burst / Rage Art Cinematic Finisher (Xbox LB)
};

// Fighter stance / posture states
enum class StanceState {
    STAND,
    CROUCH,
    JUMP,
    SIDESTEP_LEFT,
    SIDESTEP_RIGHT,
    WHILE_RISING,
    DOWN_FACE_UP,
    DOWN_FACE_DOWN
};

// Action / animation state
enum class ActionState {
    IDLE,
    WALK_FORWARD,
    WALK_BACK,
    SIDESTEP_LEFT,
    SIDESTEP_RIGHT,
    CROUCH_IDLE,
    CROUCH_WALK,
    JUMP_UP,
    ATTACKING,
    BLOCK_STUN,
    HIT_STUN,
    JUGGLED,      // Air juggle state (Tekken float combos)
    WALL_SPLATTED,// Tekken wall collision
    KNOCKED_DOWN, // Grounded state
    WAKEUP_ROLL,
    RAGE_ART_CINEMATIC,
    VICTORY_POSE
};

// 3D Sphere / Capsule for Hitbox and Hurtbox collision
struct Hitbox3D {
    Vector3 offset;       // Relative to fighter root position
    float radius;         // Sphere radius
    bool active;          // Is hitbox currently active
    int damage;           // Base damage
    AttackHeight height;  // High, Mid, Low
    AttackType type{AttackType::NONE}; // Martial arts attack motion category
    int startupFrames;    // Frames before hitbox becomes active
    int activeFrames;     // Duration hitbox stays alive
    int recoveryFrames;   // Frames to recover after attack
    int hitAdvantage;     // Frame advantage on hit (+/-)
    int blockAdvantage;   // Frame advantage on block (+/-)
    Vector3 knockback;    // Launch / pushback impulse vector
    bool isLauncher;      // Does it launch opponent for juggle combo?
    bool causesSplat;     // Does it cause wall splat?
    std::string moveName; // Descriptive name (e.g. "1-2 Jab", "Wind God Fist")
};

struct Hurtbox3D {
    std::string boneName; // Bone attached to (Head, Chest, Pelvis, Limbs)
    Vector3 offset;
    float radius;
    float height;         // If capsule
};

} // namespace DarkLotus
