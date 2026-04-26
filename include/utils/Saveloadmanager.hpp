#pragma once

#include <string>
#include "Gamestateserializer.hpp"
using namespace std;

class GameEngine;


/*
Bertanggung jawab membuka/menutup file .txt dan menjembatani antara
GameEngine dengan Gamestateserializer.

Single Responsibility:
  - SaveLoadManager  → urusan FILE (buka, baca, tulis, tutup, validasi path)
  - Gamestateserializer → urusan FORMAT (encode/decode isi file)
*/

class SaveLoadManager {
    private:
        Gamestateserializer serializer;

        // Validasi: ekstensi harus .txt, path tidak boleh kosong
        void validatePath(const std::string& filename) const;

        // Baca seluruh isi file ke dalam string
        std::string readFile(const std::string& filename) const;

        // Tulis string ke file (overwrite)
        void writeFile(const std::string& filename, const std::string& content) const;

    public:
        SaveLoadManager() = default;

        /*
        Serialisasi seluruh state engine lalu tulis ke file save
        Hanya boleh dipanggil di awal giliran (sebelum aksi apapun)
        TODO: uncomment saat GameEngine tersedia penuh
        void save(const GameEngine& engine, const std::string& filename) const;
        Versi stub: terima snapshot langsung (untuk cicilan sebelum GameEngine penuh)
        */
        void save(const GameEngine& engine, const std::string& filename) const;

        // Compatibility overload: caller sudah punya snapshot.
        void save(const GameSnapshot& snapshot, const std::string& filename) const;

        // ── Load ──────────────────────────────────────────────────────────────────
        // Baca file lalu kembalikan GameSnapshot; GameEngine yang apply ke state-nya
        GameSnapshot load(const std::string& filename) const;

        // Helper agar alur load lebih ringkas dari sisi engine caller.
        void loadInto(GameEngine& engine, const std::string& filename) const;

        // Apply snapshot ke GameEngine (dipanggil setelah load())
        // TODO: implementasikan saat Player, Board, Property dari Orang 1 & 2 tersedia
        // void applySnapshot(const GameSnapshot& snapshot, GameEngine& engine) const;
};
