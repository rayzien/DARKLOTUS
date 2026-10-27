#include "DarkLotus/Core/GameManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

int main() {
    DarkLotus::Logger::Info("DARKLOTUS", "Starting DARKLOTUS Fighting Game Engine...");

    auto& engine = DarkLotus::GameManager::Instance();
    if (!engine.Init()) {
        DarkLotus::Logger::Error("DARKLOTUS", "Engine initialization failed!");
        return -1;
    }

    engine.Run();
    engine.Shutdown();

    DarkLotus::Logger::Info("DARKLOTUS", "Game closed gracefully.");
    return 0;
}
