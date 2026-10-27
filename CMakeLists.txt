cmake_minimum_required(VERSION 3.16)
project(DarkLotus LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-missing-field-initializers)

# Source files
set(SOURCES
    src/main.cpp
    src/Core/GameManager.cpp
    src/Managers/ClickManager.cpp
    src/Managers/ButtonManager.cpp
    src/Managers/SettingsManager.cpp
    src/Managers/GraphicsManager.cpp
    src/Managers/CharacterManager.cpp
    src/Managers/AnimationManager.cpp
    src/Managers/UIManager.cpp
    src/Managers/CombatManager.cpp
    src/Managers/SoundManager.cpp
    src/Managers/StageManager.cpp
    src/Entities/Fighter.cpp
    src/Utils/Logger.cpp
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib/include
)

# Link directories
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib/lib
)

# Executable target
add_executable(DarkLotus ${SOURCES})

# Target libraries
target_link_libraries(DarkLotus PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib/lib/libraylib.a
    m
    pthread
    dl
    rt
    X11
)

# Output directory
set_target_properties(DarkLotus PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
