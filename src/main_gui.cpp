#include "../include/core/GameEngine.hpp"
#include "../include/viewsGUI/SfmlGuiManager.hpp"

#include <iostream>

int main() {
    try {
        GameEngine engine;

        viewsGUI::SfmlGuiManager gui(engine);
        gui.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FATAL GUI] " << e.what() << "\n";
        return 1;
    }
}
