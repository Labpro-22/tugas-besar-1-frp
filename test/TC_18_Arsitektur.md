# TC-18 | Arsitektur & OOP

## Deskripsi
Menguji kepatuhan terhadap prinsip OOP, layered architecture, exception handling, operator overloading, generic class, dan STL.

---

## Test Cases

### TC-18-01 | Tidak ada cout/cin di src/core/
**Input:** Grep source code
**Expected:** 0 hasil (kecuali file test)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Jalankan: grep -rn "std::cout\|std::cin\|\bcout\b\|\bcin\b" src/core/ --include="*.cpp" | grep -v "test_"
Apakah hasilnya 0 baris?
Tulis PASS atau FAIL.
Jika FAIL: Untuk setiap file yang masih mengandung cout/cin:
  - Ganti cout dengan engine.pushEvent()
  - Ganti cin dengan engine.pushPrompt() + engine.getPromptAnswer()
  - Lihat pola yang sudah diterapkan di PropertyManager.cpp sebagai referensi
```

### TC-18-02 | Tidak ada cout/cin di src/models/
**Input:** Grep source code
**Expected:** 0 hasil
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Jalankan: grep -rn "std::cout\|std::cin" src/models/ --include="*.cpp"
Apakah hasilnya 0 baris?
Tulis PASS atau FAIL.
Jika FAIL: pindahkan semua output ke pushEvent() di engine, input ke pushPrompt().
```

### TC-18-03 | UI tidak panggil logika game langsung
**Input:** Review GameUI.cpp
**Expected:** GameUI hanya panggil engine.processCommand(), formatter.format(), renderer.render()
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buka src/views/GameUI.cpp.
Apakah tidak ada panggilan langsung ke Player, Property, Bank, atau Manager
selain melalui engine.processCommand() dan engine.getCardManager() (untuk drop kartu)?
Tulis PASS atau FAIL.
Jika FAIL: pindahkan semua logika game keluar dari GameUI ke processCommand().
```

### TC-18-04 | Tile adalah abstract class dengan virtual onLand()
**Input:** Review include/models/Tile.hpp
**Expected:** `virtual void onLand(...) = 0` ada di Tile
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buka include/models/Tile.hpp.
Apakah ada deklarasi "virtual void onLand(Player& player, GameEngine& engine) = 0"?
Tulis PASS atau FAIL.
Jika FAIL: tambahkan = 0 pada deklarasi onLand() di Tile.
```

### TC-18-05 | Semua turunan Tile mengoverride onLand()
**Input:** Cek GoTile, JailTile, PropertyTile, dll.
**Expected:** Semua override onLand() dengan keyword override
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Cek header tiap subclass Tile (GoTile.hpp, PropertyTile.hpp, FestivalTile.hpp, dll.).
Apakah semua deklarasi onLand() menggunakan keyword "override"?
Tulis PASS atau FAIL.
Jika FAIL: tambahkan "override" di semua deklarasi onLand() subclass.
```

### TC-18-06 | Property abstract dengan virtual calculateRent()
**Input:** Review Property.hpp
**Expected:** `virtual int calculateRent(...) const = 0`
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buka include/models/Property.hpp.
Apakah calculateRent() adalah pure virtual function?
Tulis PASS atau FAIL.
Jika FAIL: ubah deklarasi menjadi: virtual int calculateRent(const GameContext&) const = 0;
```

### TC-18-07 | operator+= menambah uang pemain
**Input:** `player += 200`
**Expected:** Uang bertambah 200
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Di test atau debug: Player alice("Alice", 500); alice += 200;
Apakah alice.getMoney() == 700?
Tulis PASS atau FAIL.
Jika FAIL: Player::operator+=(int amount) { addMoney(amount); return *this; }
```

### TC-18-08 | operator-= mengurangi uang pemain
**Input:** `player -= 100`
**Expected:** Uang berkurang 100
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Player alice("Alice", 500); alice -= 100;
Apakah alice.getMoney() == 400?
Tulis PASS atau FAIL.
Jika FAIL: Player::operator-=(int amount) { deductMoney(amount); return *this; }
```

### TC-18-09 | operator> membandingkan total kekayaan
**Input:** P1 kekayaan M2000 vs P2 kekayaan M1500
**Expected:** `P1 > P2` = true
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
P1 uang M2000, P2 uang M1500 (tanpa properti).
Apakah (P1 > P2) = true?
Tulis PASS atau FAIL.
Jika FAIL: bool Player::operator>(const Player& other) const {
  return getTotalWealth() > other.getTotalWealth(); ... }
```

### TC-18-10 | InsufficientFundsException dilempar dengan benar
**Input:** Bayar sewa tapi uang tidak cukup
**Expected:** InsufficientFundsException ter-catch di GameUI, pesan muncul
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buat skenario pemain tidak mampu bayar sewa.
Apakah muncul [ERROR] InsufficientFundsException dengan detail username,
jumlah yang dibutuhkan, dan jumlah yang dimiliki?
Apakah program tidak crash?
Tulis PASS atau FAIL.
Jika FAIL: GameUI::run() catch (GameException& e) sudah ada,
pastikan InsufficientFundsException inherit dari GameException.
```

### TC-18-11 | Deck<T> generic class berfungsi untuk ActionCard dan SkillCard
**Input:** Review Deck.hpp
**Expected:** Template class Deck<T> dipakai untuk chanceDeck, communityDeck, skillDeck
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buka include/models/Deck.hpp.
Apakah Deck adalah template class<T>?
Buka CardManager.hpp, apakah ada Deck<ActionCard> dan Deck<SkillCard>?
Tulis PASS atau FAIL.
Jika FAIL: ubah Deck menjadi template: template<typename T> class Deck { ... };
```

### TC-18-12 | STL vector dipakai untuk daftar pemain
**Input:** Review GameEngine
**Expected:** `vector<Player*> players` dipakai
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buka include/core/GameEngine.hpp.
Apakah players adalah vector<Player*>?
Apakah getActivePlayers() mengembalikan vector<Player*>?
Tulis PASS atau FAIL.
Jika FAIL: ubah tipe players menjadi std::vector<Player*>.
```

### TC-18-13 | STL map dipakai untuk tabel sewa RR/Utility
**Input:** Review RailroadProperty.hpp
**Expected:** `map<int,int> rentByCount` ada
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buka include/models/RailroadProperty.hpp.
Apakah ada field "map<int,int> rentByCount"?
Buka UtilityProperty.hpp, apakah ada "map<int,int> multiplierByCount"?
Tulis PASS atau FAIL.
Jika FAIL: ubah tipe field menjadi std::map<int,int>.
```

### TC-18-14 | Compile -Wall -Wextra tanpa warning
**Input:** `g++ -std=c++17 -Wall -Wextra -I./include ...`
**Expected:** 0 warning, 0 error
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Compile seluruh project: g++ -std=c++17 -Wall -Wextra -I./include $(find src -name "*.cpp") -o nimonspoli
Apakah output kompilasi bersih (0 warning, 0 error)?
Tulis PASS atau FAIL.
Jika FAIL: perbaiki setiap warning:
  - Unused variable: tambahkan (void)varname; atau hapus variabel
  - Missing return: tambahkan return di akhir function
  - Shadow variable: rename variabel lokal
  - Signed/unsigned mismatch: tambahkan cast yang sesuai
```

### TC-18-15 | Program berjalan normal di Linux/WSL
**Input:** Compile dan jalankan di WSL Ubuntu
**Expected:** Program berjalan, game bisa dimainkan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Di WSL/Linux: compile dan jalankan ./nimonspoli
Apakah menu awal muncul? Apakah New Game bisa dimulai?
Tulis PASS atau FAIL.
Jika FAIL: cek apakah ada Windows-specific code (backslash path, CRLF).
Gunakan forward slash dan 
 bukan 
.
```

### TC-18-16 | Tidak ada struct yang dilarang (kecuali Command/CommandResult)
**Input:** Grep source untuk 'struct'
**Expected:** Hanya Command.hpp dan CommandResult.hpp yang berisi struct
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
grep -rn "^struct\|^    struct" src/ include/ --include="*.hpp" --include="*.cpp"
Apakah hanya Command.hpp dan CommandResult.hpp yang menggunakan struct?
Tulis PASS atau FAIL.
Jika FAIL: ubah setiap struct menjadi class dengan private attributes dan public getter/setter.
```

### TC-18-17 | Tidak ada global variable
**Input:** Grep untuk global variable
**Expected:** Tidak ada global variable (kecuali constants)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
grep -n "^[a-zA-Z]" src/**/*.cpp | grep -v "^.*:.*(" | grep -v "^.*://"
Apakah tidak ada definisi variabel global (non-fungsi, non-kelas)?
Tulis PASS atau FAIL.
Jika FAIL: pindahkan setiap global variable menjadi member class yang sesuai.
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-18-01 | No cout/cin di core | ⬜ |
| TC-18-02 | No cout/cin di models | ⬜ |
| TC-18-03 | UI tidak panggil logika langsung | ⬜ |
| TC-18-04 | Tile abstract onLand() pure virtual | ⬜ |
| TC-18-05 | Semua subclass override onLand | ⬜ |
| TC-18-06 | Property abstract calculateRent() | ⬜ |
| TC-18-07 | operator+= uang | ⬜ |
| TC-18-08 | operator-= uang | ⬜ |
| TC-18-09 | operator> kekayaan | ⬜ |
| TC-18-10 | InsufficientFundsException | ⬜ |
| TC-18-11 | Deck<T> generic | ⬜ |
| TC-18-12 | STL vector pemain | ⬜ |
| TC-18-13 | STL map sewa RR/Utility | ⬜ |
| TC-18-14 | Compile -Wall -Wextra bersih | ⬜ |
| TC-18-15 | Berjalan di Linux/WSL | ⬜ |
| TC-18-16 | Tidak ada struct terlarang | ⬜ |
| TC-18-17 | Tidak ada global variable | ⬜ |

**Total: 17 test cases**
