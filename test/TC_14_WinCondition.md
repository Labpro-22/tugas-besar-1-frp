# TC-14 | Kondisi Game Selesai

## Deskripsi
Menguji semua kondisi kemenangan: MAX_TURN (tiebreak), BANKRUPTCY, dan mode bankruptcy-only.

---

## Test Cases

### TC-14-01 | MAX_TURN: game berakhir saat batas tercapai
**Input:** misc.txt MAX_TURN=3, semua pemain main 3 giliran
**Expected:** Game selesai, tampil pemenang
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set MAX_TURN=3 di misc.txt. Mainkan 3 giliran penuh.
Apakah game berakhir setelah semua pemain menyelesaikan giliran ke-3?
Tulis PASS atau FAIL.
Jika FAIL: di checkWinCondition():
  if (maxTurn > 0 && getCurrentTurn() >= maxTurn) endGame();
```

### TC-14-02 | Pemenang = kekayaan terbanyak
**Input:** P1 kekayaan M3000, P2 kekayaan M2000, MAX_TURN tercapai
**Expected:** P1 menang
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
MAX_TURN tercapai. P1 uang M3000, P2 uang M2000.
Apakah P1 dinyatakan pemenang?
Tulis PASS atau FAIL.
Jika FAIL: endGame() sort active players dengan operator> (berdasar getTotalWealth()).
```

### TC-14-03 | Tiebreak properti
**Input:** P1=P2=M2000, P1 punya 4 properti, P2 punya 3
**Expected:** P1 menang karena properti lebih banyak
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
MAX_TURN tercapai. P1 dan P2 uang sama, P1 lebih banyak properti.
Apakah P1 menang?
Tulis PASS atau FAIL.
Jika FAIL: di operator>: if (tw == ow) return countProperties() > other.countProperties().
```

### TC-14-04 | Tiebreak kartu
**Input:** P1=P2 uang+properti sama, P1 punya 3 kartu, P2 punya 1
**Expected:** P1 menang karena lebih banyak kartu
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
MAX_TURN tercapai. Semua sama kecuali kartu.
Apakah P1 menang?
Tulis PASS atau FAIL.
Jika FAIL: di operator>: if (countProperties() == other.countProperties())
  return countCards() > other.countCards().
```

### TC-14-05 | Semua seri → semua menang
**Input:** Semua pemain identik persis: uang, properti, kartu sama
**Expected:** Semua pemain dinyatakan menang (seri)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
MAX_TURN tercapai dengan semua P sama persis.
Apakah event pemenang menyebut semua pemain?
Tulis PASS atau FAIL.
Jika FAIL: di endGame(), loop semua active:
  if (p->getTotalWealth() == top->... && countProperties == top... && countCards == top...) winners.push_back(p).
```

### TC-14-06 | Bankruptcy: 1 pemain tersisa
**Input:** P2 dan P3 bangkrut, hanya P1 tersisa
**Expected:** P1 menang
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
3 pemain. P2 bangkrut, lalu P3 bangkrut.
Apakah game selesai dan P1 menang?
Tulis PASS atau FAIL.
Jika FAIL: checkWinCondition(): if (activeCount <= 1) endGame().
```

### TC-14-07 | MAX_TURN=0 → mode bankruptcy only
**Input:** misc.txt MAX_TURN=0
**Expected:** Game tidak berakhir karena turn, hanya berakhir jika 1 pemain tersisa
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set MAX_TURN=0. Mainkan lebih dari 15 giliran.
Apakah game tidak berakhir meskipun sudah > 15 giliran?
Tulis PASS atau FAIL.
Jika FAIL: di checkWinCondition():
  if (maxTurn > 0 && ...) endGame(); // jangan cek jika maxTurn <= 0.
```

### TC-14-08 | Rekap pemain tampil saat game selesai
**Input:** Game berakhir
**Expected:** Rekap semua pemain: uang, properti, kartu, kekayaan total
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Jalankan game sampai selesai (MAX_TURN atau bankruptcy).
Apakah output menampilkan rekap tiap pemain (uang + properti + kartu)?
Apakah pemenang disebutkan dengan jelas?
Tulis PASS atau FAIL.
Jika FAIL: endGame() → pushEvent() dengan rekap dan nama pemenang.
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-14-01 | MAX_TURN berakhir | ⬜ |
| TC-14-02 | Kekayaan terbanyak menang | ⬜ |
| TC-14-03 | Tiebreak properti | ⬜ |
| TC-14-04 | Tiebreak kartu | ⬜ |
| TC-14-05 | Semua seri | ⬜ |
| TC-14-06 | Bankruptcy 1 tersisa | ⬜ |
| TC-14-07 | MAX_TURN=0 bankruptcy only | ⬜ |
| TC-14-08 | Rekap tampil saat selesai | ⬜ |

**Total: 8 test cases**
