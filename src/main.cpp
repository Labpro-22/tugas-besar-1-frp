#include "../include/core/GameEngine.hpp"
#include "../include/core/AuctionManager.hpp"
#include "../include/core/BankruptcyManager.hpp"
#include "../include/core/CardManager.hpp"
#include "../include/core/EffectManager.hpp"
#include "../include/core/PropertyManager.hpp"
#include "../include/core/TransactionLogger.hpp"
#include "../include/models/Bank.hpp"
#include "../include/views/GameUI.hpp"

int main() {
	GameEngine engine;
	Bank bank;
	TransactionLogger logger;
	CardManager cardManager;
	EffectManager effectManager;
	PropertyManager propertyManager(engine, bank, logger);
	AuctionManager auctionManager(engine, bank, logger);
	BankruptcyManager bankruptcyManager(engine, bank, logger, propertyManager, auctionManager);

	engine.setBank(&bank);
	engine.setTransactionLogger(&logger);
	engine.setCardManager(&cardManager);
	engine.setEffectManager(&effectManager);
	engine.setPropertyManager(&propertyManager);
	engine.setAuctionManager(&auctionManager);
	engine.setBankruptcyManager(&bankruptcyManager);

	GameUI ui(engine);
	ui.run();
	return 0;
}
