#include "../../include/utils/Saveloadmanager.hpp"
#include "../../include/utils/GameException.hpp"
#include "../../include/core/GameEngine.hpp"

#include <fstream>
#include <sstream>





// Private helpers
void SaveLoadManager::validatePath(const string& filename) const {
    if (filename.empty()) {
        throw SaveLoadException("Nama file tidak boleh kosong");
    }
    // Ekstensi yang didukung: .txt
    const bool txtExt = filename.size() >= 4 &&
                        filename.substr(filename.size() - 4) == ".txt";
    if (!txtExt) {
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

void SaveLoadManager::save(const GameEngine& engine,
                            const string& filename) const {
    const GameSnapshot snapshot = engine.createSnapshot();
    save(snapshot, filename);
}


void SaveLoadManager::save(const GameSnapshot& snapshot,
                            const string& filename) const {
    validatePath(filename);

    // SaveLoadManager hanya urus file; formatter sepenuhnya di serializer.
    writeFile(filename, serializer.serialize(snapshot));
}


// Load


GameSnapshot SaveLoadManager::load(const string& filename) const {
    validatePath(filename);
    const string content = readFile(filename);
    return serializer.deserialize(content);
}

void SaveLoadManager::loadInto(GameEngine& engine,
                               const string& filename) const {
    const GameSnapshot snapshot = load(filename);
    engine.applySnapshot(snapshot);
}
