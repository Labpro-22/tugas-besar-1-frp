# TC-17 | Operasi & Perintah CLI

## Deskripsi
Menguji semua perintah CLI: CETAK_PAPAN, BANTUAN, KELUAR, AKHIRI_GILIRAN, dan validasi input.

---

## Test Cases

### TC-17-01 | CETAK_PAPAN menampilkan papan 11×11
**Input:** `CETAK_PAPAN`
**Expected:** Grid ASCII 11×11 dengan 40 petak, posisi pemain terlihat
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game 2 pemain. Ketik: CETAK_PAPAN
Apakah papan 11×11 muncul dengan semua 40 petak?
Apakah posisi pemain (P1/P2) terlihat di cell yang benar?
Tulis PASS atau FAIL.
Jika FAIL: BoardRenderer::render() harus iterasi kPerimeter 11×11 dan
buildCellStatus() menampilkan "(N)" untuk posisi pemain.
```

### TC-17-02 | CETAK_PAPAN: warna ANSI sesuai color group
**Input:** `CETAK_PAPAN`
**Expected:** Street COKLAT berwarna coklat, HIJAU hijau, dll.
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
CETAK_PAPAN. Apakah cell GRT/TSK berwarna coklat (ANSI coklat)?
Apakah cell BDG/DEN/MTR berwarna hijau?
Tulis PASS atau FAIL (perlu terminal yang mendukung ANSI color).
Jika FAIL: BoardRenderer::buildCellLabel() → untuk color group "CK" gunakan AnsiTheme::tileBrown().
```

### TC-17-03 | CETAK_PAPAN: tampilkan bangunan
**Input:** GRT punya 2 rumah, tampilkan papan
**Expected:** Cell GRT menampilkan simbol 2 rumah (⌂⌂ atau ^^)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Build 2 rumah di GRT. CETAK_PAPAN.
Apakah ada indikasi 2 rumah di cell GRT (simbol atau teks)?
Tulis PASS atau FAIL.
Jika FAIL: BoardRenderer::buildCellStatus() untuk STREET:
  int houses = sp->getBuildingCount(); append simbol sesuai count.
```

### TC-17-04 | CETAK_PAPAN: tampilkan TURN / MAX_TURN di tengah
**Input:** Turn ke-5, MAX_TURN=15
**Expected:** Panel tengah menampilkan "TURN 5 / 15"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Mainkan 5 giliran. CETAK_PAPAN.
Apakah panel tengah menampilkan "TURN 5 / 15"?
Tulis PASS atau FAIL.
Jika FAIL: BoardRenderer::render() → turnStr = "TURN " + turn + " / " + maxTurn.
```

### TC-17-05 | BANTUAN menampilkan semua perintah
**Input:** `BANTUAN`
**Expected:** Daftar semua perintah yang tersedia
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Ketik: BANTUAN
Apakah output mencakup: LEMPAR_DADU, ATUR_DADU, CETAK_PAPAN, CETAK_AKTA,
CETAK_PROPERTI, GADAI, TEBUS, BANGUN, GUNAKAN_KEMAMPUAN, BAYAR_DENDA,
CETAK_LOG, SIMPAN, AKHIRI_GILIRAN, KELUAR?
Tulis PASS atau FAIL.
Jika FAIL: update string dalam case HELP di processCommand.
```

### TC-17-06 | KELUAR menutup program
**Input:** `KELUAR`
**Expected:** Program berhenti
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Ketik: KELUAR
Apakah program menampilkan "Sampai jumpa!" dan berhenti?
Tulis PASS atau FAIL.
Jika FAIL: di GameUI::run() case EXIT: std::cout << "Sampai jumpa!"; break;
```

### TC-17-07 | AKHIRI_GILIRAN pindah ke pemain berikutnya
**Input:** `AKHIRI_GILIRAN` sebelum lempar dadu
**Expected:** Giliran berpindah ke pemain berikutnya
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Giliran Alice. Ketik: AKHIRI_GILIRAN
Apakah event "Sekarang giliran Bob" muncul?
Tulis PASS atau FAIL.
Jika FAIL: case END_TURN → executeTurn() → turnManager.nextPlayer().
```

### TC-17-08 | Perintah tidak dikenal → error
**Input:** `ASDFGH`
**Expected:** GameException: perintah tidak dikenali
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Ketik: ASDFGH
Apakah muncul [ERROR] perintah tidak dikenali?
Apakah program tidak crash?
Tulis PASS atau FAIL.
Jika FAIL: case UNKNOWN → throw GameException("Perintah tidak dikenali: " + cmd.raw).
```

### TC-17-09 | Perintah sebelum game mulai → error
**Input:** `LEMPAR_DADU` sebelum New/Load game
**Expected:** Error: game belum dimulai
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Tanpa memulai game, ketik: LEMPAR_DADU
Apakah muncul error "Game belum dimulai"?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand(): if (players.empty() && cmd.type != START_GAME && ...)
  throw GameException("Game belum dimulai.");
```

### TC-17-10 | Status bar menampilkan info giliran
**Input:** Setiap setelah command
**Expected:** Status bar: giliran pemain, uang, turn ke berapa
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Jalankan beberapa command. Apakah setelah tiap command muncul status bar
"[ Giliran: Alice | Uang: M... | Turn: N/MAX ]"?
Tulis PASS atau FAIL.
Jika FAIL: GameUI::printStatusBar() dipanggil setelah tiap command di run().
```

### TC-17-11 | Status bar menampilkan info penjara
**Input:** Pemain JAILED
**Expected:** Status bar mencantumkan "DIPENJARA (percobaan X/4)"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buat Alice masuk penjara. Amati status bar.
Apakah status bar menampilkan info penjara dan hint command?
Tulis PASS atau FAIL.
Jika FAIL: di printStatusBar(): if (cur.isJailed()) tambahkan info penjara ke status bar.
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-17-01 | CETAK_PAPAN 11×11 | ⬜ |
| TC-17-02 | Warna ANSI color group | ⬜ |
| TC-17-03 | Tampilkan bangunan | ⬜ |
| TC-17-04 | TURN/MAX_TURN di tengah | ⬜ |
| TC-17-05 | BANTUAN daftar lengkap | ⬜ |
| TC-17-06 | KELUAR program berhenti | ⬜ |
| TC-17-07 | AKHIRI_GILIRAN pindah | ⬜ |
| TC-17-08 | Perintah tidak dikenal | ⬜ |
| TC-17-09 | Perintah sebelum game | ⬜ |
| TC-17-10 | Status bar info giliran | ⬜ |
| TC-17-11 | Status bar info penjara | ⬜ |

**Total: 11 test cases**
