#include "../include/core/GameEngine.hpp"
#include "../include/views/GameUI.hpp"

int main() {
	GameEngine engine;
	GameUI ui(engine);
	ui.run();
	return 0;
}
