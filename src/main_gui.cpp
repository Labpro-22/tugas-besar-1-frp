#include "../include/core/CardManager.hpp"
#include "../include/core/EffectManager.hpp"
#include "../include/core/GameEngine.hpp"
#include "../include/viewsGUI/SfmlGuiManager.hpp"

#include <iostream>

int main() {
    try {
        GameEngine engine;
        CardManager cardManager;
        EffectManager effectManager;

        engine.setCardManager(&cardManager);
        engine.setEffectManager(&effectManager);

        viewsGUI::SfmlGuiManager gui(engine);
        gui.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FATAL GUI] " << e.what() << "\n";
        return 1;
    }
}
