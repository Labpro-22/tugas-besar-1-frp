#include "../../include/utils/Saveloadmanager.hpp"
#include "../../include/utils/GameException.hpp"

#include <fstream>
#include <sstream>





// Private helpers
void SaveLoadManager::validatePath(const string& filename) const {
    if (filename.empty()) {
        throw SaveLoadException("Nama file tidak boleh kosong");
    }
    // Ekstensi harus .txt
    if (filename.size() < 4 ||
        filename.substr(filename.size() - 4) != ".txt") {
        throw SaveLoadException("File save harus berekstensi .txt: " + filename);
    }
}

string SaveLoadManager::readFile(const string& filename) const {
    ifstream file(filename);
    if (!file.is_open()) {
        throw ConfigFileNotFoundException(filename);
    }
    ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void SaveLoadManager::writeFile(const string& filename,
                                 const string& content) const {
    ofstream file(filename);
    if (!file.is_open()) {
        throw SaveLoadException("Tidak dapat membuka file untuk ditulis: " + filename);
    }
    file << content;
    if (file.fail()) {
        throw SaveLoadException("Gagal menulis ke file: " + filename);
    }
}


// Save


void SaveLoadManager::save(const GameSnapshot& snapshot,
                            const string& filename) const {
    validatePath(filename);

    // Susun konten dari snapshot menggunakan serializer
    ostringstream content;

    content << serializer.serializeHeader(snapshot.getCurrentTurn(),
                                          snapshot.getMaxTurn(),
                                          snapshot.getNumPlayers())
            << "\n";

    for (const auto& player : snapshot.getPlayers()) {
        content << serializer.serializePlayer(player) << "\n";
    }

    content << serializer.serializeTurnOrder(snapshot.getTurnOrder(),
                                              snapshot.getActivePlayer())
            << "\n";

    content << serializer.serializeProperties(snapshot.getProperties()) << "\n";
    content << serializer.serializeDeck(snapshot.getDeck())             << "\n";
    content << serializer.serializeLog(snapshot.getLog());

    writeFile(filename, content.str());
}


// Load


GameSnapshot SaveLoadManager::load(const string& filename) const {
    validatePath(filename);
    const string content = readFile(filename);
    return serializer.deserialize(content);
}