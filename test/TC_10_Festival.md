# TC-10 | Festival

## Deskripsi
Menguji mekanisme petak festival: pemilihan properti, penggandaan sewa, batas maksimum, dan durasi.

---

## Test Cases

### TC-10-01 | Mendarat festival → pilih properti → sewa x2
**Input:** Alice punya GRT (sewa dasar M2), mendarat FES, pilih GRT
**Expected:** festivalMult=2, dur=3, sewa GRT jadi M4
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT. Mendarat FES (idx 7 atau 33). Pilih GRT.
Apakah festivalMult GRT = 2 dan dur = 3?
Bob mendarat GRT → apakah bayar M4 (bukan M2)?
Tulis PASS atau FAIL.
Jika FAIL: FestivalTile::onLand() → EffectManager::applyFestival():
  multiplier *= 2; setFestivalDuration(3).
```

### TC-10-02 | Tidak punya properti → tidak ada efek
**Input:** Alice belum punya properti, mendarat FES
**Expected:** Event "tidak ada properti yang bisa diperkuat"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game. Alice mendarat FES sebelum beli properti apapun.
Apakah muncul event "tidak ada properti" dan tidak ada prompt pilihan?
Tulis PASS atau FAIL.
Jika FAIL: di FestivalTile::onLand(), saat candidates.empty():
  engine.pushEvent(..., "Tidak ada properti yang bisa diperkuat."); return;
```

### TC-10-03 | Pilih properti yang sudah festival → mult x2 lagi
**Input:** GRT mult=2, dur=1, Alice mendarat FES, pilih GRT lagi
**Expected:** mult=4, dur di-reset ke 3
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
GRT sudah festival mult=2, dur=1. Alice mendarat FES lagi, pilih GRT.
Apakah mult menjadi 4 dan dur=3?
Tulis PASS atau FAIL.
Jika FAIL: applyFestival() → if (mult < 8) mult *= 2; setFestivalDuration(3).
```

### TC-10-04 | Maksimum festival x8
**Input:** GRT mult=8, Alice mendarat FES, pilih GRT
**Expected:** mult tetap 8, dur di-reset ke 3, event "sudah maksimum"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
GRT mult=8 (festival 3x). Alice mendarat FES lagi, pilih GRT.
Apakah muncul event "efek sudah maksimum" dan mult tetap 8?
Tulis PASS atau FAIL.
Jika FAIL: di applyFestival(): if (mult >= 8) hanya reset durasi, jangan kalikan.
```

### TC-10-05 | Durasi berkurang tiap akhir giliran pemilik
**Input:** GRT fest mult=2, dur=3
**Expected:** Setelah giliran Alice berakhir: dur=2
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Aktifkan festival GRT (dur=3). Lakukan AKHIRI_GILIRAN untuk Alice.
Cek durasi GRT (via CETAK_AKTA GRT). Apakah dur=2?
Tulis PASS atau FAIL.
Jika FAIL: EffectManager::decrementTemporaryEffects() dipanggil di onTurnEnd():
  property.setFestivalDuration(dur - 1).
```

### TC-10-06 | Efek hilang saat durasi = 0
**Input:** GRT fest dur=1, akhir giliran Alice
**Expected:** dur=0, mult kembali ke 1
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
GRT fest dur=1. AKHIRI_GILIRAN Alice.
Apakah dur=0 dan mult=1 (sewa normal kembali)?
Tulis PASS atau FAIL.
Jika FAIL: di decrementTemporaryEffects():
  if (dur - 1 <= 0) { setFestivalDuration(0); setFestivalMultiplier(1); }
```

### TC-10-07 | Durasi dua properti independen
**Input:** GRT mult=2 dur=2, TSK mult=2 dur=3
**Expected:** Setelah 1 giliran: GRT dur=1, TSK dur=2
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Festival di GRT (dur=2) dan TSK (dur=3). AKHIRI_GILIRAN.
Apakah GRT dur=1 dan TSK dur=2 (masing-masing turun 1)?
Tulis PASS atau FAIL.
Jika FAIL: loop di decrementTemporaryEffects() harus iterasi SEMUA properti milik player.
```

### TC-10-08 | Properti MORTGAGED tidak muncul di pilihan
**Input:** Alice punya GRT (OWNED) dan TSK (MORTGAGED), mendarat FES
**Expected:** Hanya GRT yang muncul di daftar pilihan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice gadai TSK. Mendarat FES. Apakah daftar pilihan hanya menampilkan GRT?
Tulis PASS atau FAIL.
Jika FAIL: di FestivalTile::onLand(), filter candidates:
  if (prop && !prop->isMortgaged()) candidates.push_back(prop);
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-10-01 | Festival aktif sewa x2 | ⬜ |
| TC-10-02 | Tidak punya properti | ⬜ |
| TC-10-03 | Pilih yang sudah festival x2 lagi | ⬜ |
| TC-10-04 | Maksimum x8 | ⬜ |
| TC-10-05 | Durasi berkurang tiap giliran | ⬜ |
| TC-10-06 | Efek hilang dur=0 | ⬜ |
| TC-10-07 | Durasi independen per properti | ⬜ |
| TC-10-08 | MORTGAGED tidak di pilihan | ⬜ |

**Total: 8 test cases**
