# TC-15 | Save & Load

## Deskripsi
Menguji fitur SIMPAN dan MUAT: format file, restore state, dan validasi constraint.

---

## Test Cases

### TC-15-01 | SIMPAN berhasil di awal giliran
**Input:** Awal giliran (belum aksi apapun), `SIMPAN test.txt`
**Expected:** File test.txt terbuat, event "berhasil disimpan"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game. Di awal giliran pertama (sebelum apapun): SIMPAN test.txt
Apakah file test.txt terbuat dan ada event sukses?
Tulis PASS atau FAIL.
Jika FAIL: processCommand case SAVE → SaveLoadManager::save(*this, filename).
```

### TC-15-02 | SIMPAN setelah lempar dadu → error
**Input:** Sudah LEMPAR_DADU, lalu `SIMPAN test.txt`
**Expected:** GameException: hanya bisa simpan di awal giliran
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
LEMPAR_DADU. Lalu: SIMPAN test.txt
Apakah muncul error "SIMPAN hanya boleh di awal giliran sebelum aksi apapun"?
Tulis PASS atau FAIL.
Jika FAIL: if (turnActionTaken) throw GameException("SIMPAN hanya di awal giliran.");
```

### TC-15-03 | Format file: baris 1 = TURN MAX_TURN
**Input:** SIMPAN saat turn ke-5 dengan MAX_TURN=15
**Expected:** Baris pertama file = `5 15`
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Mainkan 5 giliran. SIMPAN file.txt. Buka file.txt.
Apakah baris pertama = "5 15"?
Tulis PASS atau FAIL.
Jika FAIL: GameStateSerializer harus tulis turnNumber dan maxTurn di baris pertama.
```

### TC-15-04 | Format file: state pemain (username, uang, posisi, status)
**Input:** SIMPAN setelah beberapa aksi
**Expected:** Setiap baris pemain: `Username Uang KodePetak Status`
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice uang M800 di posisi JKT (idx 37), status ACTIVE.
SIMPAN. Buka file, cek baris Alice.
Apakah = "Alice 800 JKT ACTIVE"?
Tulis PASS atau FAIL.
Jika FAIL: SavedPlayerState serialisasi: username money positionCode status.
```

### TC-15-05 | Format file: state properti (kode, jenis, pemilik, status, fmult, fdur, bangunan)
**Input:** GRT milik Alice, OWNED, 2 rumah, festival mult=2 dur=1
**Expected:** Baris properti: `GRT street Alice OWNED 2 1 2`
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
GRT: Alice, OWNED, 2 rumah, festival mult=2 dur=1. SIMPAN. Cek file.
Apakah ada baris "GRT street Alice OWNED 2 1 2"?
Tulis PASS atau FAIL.
Jika FAIL: SavedPropertyState serialisasi semua field termasuk festivalMult, festivalDur, buildings.
```

### TC-15-06 | MUAT restore uang pemain
**Input:** Alice uang M750 saat simpan
**Expected:** Setelah MUAT, Alice uang M750
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice uang M750. SIMPAN. Restart. MUAT.
Apakah uang Alice = M750?
Tulis PASS atau FAIL.
Jika FAIL: applySnapshot() → player.setMoney(saved.getMoney()) — periksa konstruktor Player.
```

### TC-15-07 | MUAT restore posisi pemain
**Input:** Alice di JKT (idx 37) saat simpan
**Expected:** Setelah MUAT, Alice di JKT
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice di JKT. SIMPAN. Restart. MUAT.
Apakah posisi Alice = idx 37 (JKT)?
Tulis PASS atau FAIL.
Jika FAIL: player.setPosition(board.getIndexOf(saved.getPositionCode())).
```

### TC-15-08 | MUAT restore status JAILED
**Input:** Alice JAILED, jailTurns=2 saat simpan
**Expected:** Setelah MUAT, Alice JAILED dengan jailTurns=2
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice masuk penjara, gagal double 2x (jailTurns=2). SIMPAN. Restart. MUAT.
Apakah Alice masih JAILED dengan jailTurns=2?
Tulis PASS atau FAIL.
Jika FAIL: status "JAILED_2" di-parse kembali: extractJailTurns("JAILED_2") = 2.
```

### TC-15-09 | MUAT restore kepemilikan properti
**Input:** Alice punya GRT (OWNED), Bob punya TSK (MORTGAGED)
**Expected:** Setelah MUAT, kepemilikan tetap
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya GRT, Bob punya TSK (gadai). SIMPAN. Restart. MUAT.
Apakah GRT masih milik Alice (OWNED) dan TSK milik Bob (MORTGAGED)?
Tulis PASS atau FAIL.
Jika FAIL: applySnapshot() untuk properti: set owner dan status berdasarkan savedProp.
```

### TC-15-10 | MUAT restore festival state
**Input:** GRT mult=4 dur=2 saat simpan
**Expected:** Setelah MUAT, GRT mult=4 dur=2
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Aktifkan festival GRT 2x (mult=4, dur=2). SIMPAN. Restart. MUAT.
Apakah GRT mult=4 dan dur=2?
Tulis PASS atau FAIL.
Jika FAIL: prop.setFestivalMultiplier(savedProp.getFestivalMult());
prop.setFestivalDuration(savedProp.getFestivalDur()).
```

### TC-15-11 | MUAT restore urutan giliran
**Input:** Giliran aktif P3 saat simpan (dari urutan P2→P3→P4→P1)
**Expected:** Setelah MUAT, giliran dimulai dari P3
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
4 pemain, simpan saat giliran P3. Restart. MUAT.
Apakah giliran dimulai dari P3?
Tulis PASS atau FAIL.
Jika FAIL: applySnapshot() → turnManager.restoreState(restoredOrder, activeOrderIndex, ...).
```

### TC-15-12 | MUAT file rusak → error
**Input:** File save dengan format salah
**Expected:** SaveLoadException dengan lokasi error
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buat file save dengan format salah (hapus satu baris). Coba MUAT.
Apakah muncul SaveLoadException atau ConfigParseException (bukan crash)?
Tulis PASS atau FAIL.
Jika FAIL: wrap semua parsing di try-catch, re-throw SaveLoadException dengan pesan informatif.
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-15-01 | SIMPAN di awal giliran | ⬜ |
| TC-15-02 | SIMPAN setelah dadu → error | ⬜ |
| TC-15-03 | Format: baris 1 TURN MAX_TURN | ⬜ |
| TC-15-04 | Format: state pemain | ⬜ |
| TC-15-05 | Format: state properti | ⬜ |
| TC-15-06 | MUAT restore uang | ⬜ |
| TC-15-07 | MUAT restore posisi | ⬜ |
| TC-15-08 | MUAT restore JAILED | ⬜ |
| TC-15-09 | MUAT restore kepemilikan | ⬜ |
| TC-15-10 | MUAT restore festival | ⬜ |
| TC-15-11 | MUAT restore urutan giliran | ⬜ |
| TC-15-12 | MUAT file rusak → error | ⬜ |

**Total: 12 test cases**
