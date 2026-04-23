# TC-12 | Kartu Kemampuan Spesial

## Deskripsi
Menguji semua 6 jenis Skill Card, aturan penggunaan (1x per giliran, sebelum dadu, max 3 kartu), dan drop kartu ke-4.

---

## Test Cases

### TC-12-01 | Setiap awal giliran dapat 1 kartu skill
**Input:** Giliran baru dimulai
**Expected:** Pemain aktif mendapat 1 kartu acak, event info muncul
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game 2 pemain. Amati event saat giliran pertama dimulai.
Apakah ada event "mendapat 1 kartu kemampuan acak"?
Apakah pemain punya 1 kartu setelah giliran pertama?
Tulis PASS atau FAIL.
Jika FAIL: di GameEngine::executeTurn(), saat giliran beralih:
  cardManager->drawSkillCard(next);
```

### TC-12-02 | Drop kartu ke-4: prompt interaktif
**Input:** Pemain sudah punya 3 kartu, mendapat kartu ke-4
**Expected:** Prompt drop muncul, pemain pilih kartu yang dibuang
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Pastikan pemain sudah punya 3 kartu. Mulai giliran baru (dapat kartu ke-4).
Apakah muncul peringatan "sudah 3 kartu, wajib buang 1"?
Apakah pemain bisa memilih nomor kartu untuk dibuang?
Tulis PASS atau FAIL.
Jika FAIL: GameUI::handlePendingSkillDrop() → cardManager.hasPendingSkillDrop()
→ tampilkan 4 opsi dan baca pilihan user.
```

### TC-12-03 | Hanya 1 kartu per giliran
**Input:** Gunakan kartu ke-1, lalu coba kartu ke-2
**Expected:** Error "sudah gunakan kartu kemampuan giliran ini"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
GUNAKAN_KEMAMPUAN 1. Lalu GUNAKAN_KEMAMPUAN 2.
Apakah muncul error "skill card hanya boleh dipakai sekali per giliran"?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand case USE_SKILL:
  if (current.hasUsedSkillThisTurn()) throw GameException(...).
```

### TC-12-04 | Kartu hanya sebelum lempar dadu
**Input:** Sudah LEMPAR_DADU, lalu GUNAKAN_KEMAMPUAN
**Expected:** Error "kartu hanya boleh dipakai SEBELUM melempar dadu"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
LEMPAR_DADU. Lalu GUNAKAN_KEMAMPUAN 1.
Apakah muncul error "kartu kemampuan hanya bisa digunakan sebelum melempar dadu"?
Tulis PASS atau FAIL.
Jika FAIL: if (diceRolledThisTurn) throw GameException(...).
```

### TC-12-05 | GUNAKAN_KEMAMPUAN tanpa argumen → daftar kartu
**Input:** `GUNAKAN_KEMAMPUAN` tanpa nomor
**Expected:** Daftar kartu di tangan ditampilkan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Pemain punya 2 kartu. Ketik: GUNAKAN_KEMAMPUAN
Apakah daftar kartu muncul (nomor + nama + deskripsi)?
Tulis PASS atau FAIL.
Jika FAIL: di case USE_SKILL, saat cmd.args.empty():
  tampilkan hand cards dan return result tanpa error.
```

### TC-12-06 | MoveCard: maju N petak
**Input:** Alice punya MoveCard(5), gunakan
**Expected:** Alice maju 5 petak, trigger landing
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya MoveCard value=5 (posisi idx 0). Ketik: GUNAKAN_KEMAMPUAN 1
Apakah Alice pindah ke idx 5? Apakah landing di idx 5 di-trigger?
Tulis PASS atau FAIL.
Jika FAIL: MoveCard::apply() → player.move(value, board.size()) → handleLanding.
```

### TC-12-07 | ShieldCard: kebal sewa 1 giliran
**Input:** Alice ShieldCard, mendarat di properti Bob
**Expected:** Sewa tidak dipungut
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice GUNAKAN_KEMAMPUAN ShieldCard. Alice mendarat di properti Bob.
Apakah Alice tidak membayar sewa?
Tulis PASS atau FAIL.
Jika FAIL: di payRent(): if (payer.isShieldActive()) { pushEvent(...); return; }
```

### TC-12-08 | TeleportCard: pindah ke petak mana saja
**Input:** `GUNAKAN_KEMAMPUAN 1 JKT`
**Expected:** Alice pindah ke JKT, trigger landing JKT
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya TeleportCard. Ketik: GUNAKAN_KEMAMPUAN 1 JKT
Apakah Alice pindah ke JKT (idx 37)? Landing di-trigger?
Tulis PASS atau FAIL.
Jika FAIL: TeleportCard::apply() → setTargetCode(target) → player.setPosition(targetIdx).
```

### TC-12-09 | TeleportCard tanpa target → error hint
**Input:** `GUNAKAN_KEMAMPUAN 1` (TeleportCard tanpa target)
**Expected:** Event "target diperlukan" + hint cara pakai
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya TeleportCard. Ketik: GUNAKAN_KEMAMPUAN 1 (tanpa kode petak)
Apakah muncul event warning "kartu TeleportCard membutuhkan TARGET" beserta contoh?
Tulis PASS atau FAIL.
Jika FAIL: di case USE_SKILL, cek cardType == "TeleportCard" && target.empty():
  pushEvent(..., "Target diperlukan. Contoh: GUNAKAN_KEMAMPUAN 1 JKT");
```

### TC-12-10 | LassoCard: tarik lawan di depan
**Input:** Alice idx 5, Bob idx 15, Alice gunakan LassoCard Bob
**Expected:** Bob pindah ke idx 5
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice idx 5, Bob idx 15. Alice: GUNAKAN_KEMAMPUAN 1 Bob
Apakah Bob berpindah ke idx 5?
Tulis PASS atau FAIL.
Jika FAIL: LassoCard::apply() → target.setPosition(player.getPosition()).
```

### TC-12-11 | LassoCard: tidak bisa tarik yang di belakang
**Input:** Alice idx 15, Bob idx 5
**Expected:** GameException: target tidak di depan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice idx 15, Bob idx 5. Alice: GUNAKAN_KEMAMPUAN 1 Bob
Apakah muncul error "target harus berada di depan"?
Tulis PASS atau FAIL.
Jika FAIL: di LassoCard::apply():
  int dist = board.distanceTo(player.getPosition(), target.getPosition());
  if (dist <= 0) throw GameException("Target harus di depan.");
```

### TC-12-12 | DemolitionCard: hancurkan bangunan lawan
**Input:** Bob punya GRT hotel, Alice gunakan DemolitionCard GRT
**Expected:** GRT kembali ke NONE (0 bangunan)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob punya GRT hotel. Alice: GUNAKAN_KEMAMPUAN 1 GRT
Apakah GRT kembali ke 0 bangunan?
Tulis PASS atau FAIL.
Jika FAIL: DemolitionCard::apply() → street.demolishBuildings().
```

### TC-12-13 | DemolitionCard: tidak bisa target properti sendiri
**Input:** Alice punya GRT, coba DemolitionCard GRT
**Expected:** GameException: harus properti lawan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya GRT. Alice: GUNAKAN_KEMAMPUAN 1 GRT
Apakah muncul error "harus properti lawan"?
Tulis PASS atau FAIL.
Jika FAIL: if (!property.getOwner() || property.getOwner() == &player) throw GameException(...).
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-12-01 | Dapat 1 kartu per giliran | ⬜ |
| TC-12-02 | Drop kartu ke-4 interaktif | ⬜ |
| TC-12-03 | 1x per giliran | ⬜ |
| TC-12-04 | Sebelum dadu | ⬜ |
| TC-12-05 | Tanpa arg → daftar kartu | ⬜ |
| TC-12-06 | MoveCard maju N | ⬜ |
| TC-12-07 | ShieldCard kebal sewa | ⬜ |
| TC-12-08 | TeleportCard pindah | ⬜ |
| TC-12-09 | TeleportCard tanpa target | ⬜ |
| TC-12-10 | LassoCard tarik lawan | ⬜ |
| TC-12-11 | LassoCard tidak bisa belakang | ⬜ |
| TC-12-12 | DemolitionCard hancurkan | ⬜ |
| TC-12-13 | DemolitionCard properti sendiri | ⬜ |

**Total: 13 test cases**
