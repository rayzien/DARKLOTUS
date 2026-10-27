#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cmath>
#include <algorithm>
#include <unordered_map>

// Raylib headers
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

namespace DarkLotus {

constexpr const char* GAME_NAME = "DARKLOTUS";
constexpr const char* GAME_VERSION = "0.1.0-Alpha";
constexpr const char* GAME_SUBTITLE = "Tekken-Style 3D Fighting Engine";

constexpr int DEFAULT_SCREEN_WIDTH = 1280;
constexpr int DEFAULT_SCREEN_HEIGHT = 720;
constexpr int TARGET_FPS = 60; // 60 FPS deterministic fighting game logic

enum class GameState {
    INTRO,
    MAIN_MENU,
    CHARACTER_SELECT,
    STAGE_SELECT,
    FIGHT_INTRO,
    FIGHT_ACTIVE,
    ROUND_OVER,
    MATCH_OVER,
    PAUSED,
    SETTINGS_MENU
};

enum class FightResult {
    NONE,
    PLAYER_1_WIN,
    PLAYER_2_WIN,
    DRAW,
    TIME_OUT
};

} // namespace DarkLotus
