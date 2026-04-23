# TC-05 | Sewa & Monopoli

## Deskripsi
Menguji perhitungan sewa street (dasar, monopoli, bangunan), railroad, utility, festival, dan shield.

---

## Test Cases

### TC-05-01 | Sewa dasar tanpa monopoli (Street L0)
**Input:** Bob mendarat di GRT milik Alice (Alice hanya punya GRT, bukan TSK)
**Expected:** Bob bayar M2 (RENT_L0 dari config)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT (jangan beli TSK). Bob mendarat di GRT.
Apakah Bob membayar M2 dan uang Alice bertambah M2?
Tulis PASS atau FAIL.
Jika FAIL: periksa StreetProperty::calculateRent() saat buildingLevel=NONE
dan tidak monopoli: return rentLevels[0].
```

### TC-05-02 | Sewa x2 saat monopoli, 0 bangunan
**Input:** Alice punya GRT + TSK (monopoli COKLAT), Bob mendarat di GRT
**Expected:** Bob bayar M4 (2 × RENT_L0 = 2 × 2)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT dan TSK. Bob mendarat di GRT.
Apakah Bob membayar M4 (bukan M2)?
Tulis PASS atau FAIL.
Jika FAIL: di calculateRent(), saat buildingLevel=NONE dan hasMonopoly:
  base = rentLevels[0] * 2;
```

### TC-05-03 | Sewa level 1 rumah
**Input:** GRT punya 1 rumah, Bob mendarat
**Expected:** Bob bayar RENT_L1 = M10
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice monopoli CK, bangun 1 rumah di GRT dan TSK (pemerataan).
Bob mendarat di GRT. Apakah Bob bayar M10?
Tulis PASS atau FAIL.
Jika FAIL: calculateRent() dengan lvl=1 mengembalikan rentLevels[1].
```

### TC-05-04 | Sewa level hotel (L5)
**Input:** GRT punya hotel
**Expected:** Bob bayar M250 (RENT_L5 GRT dari config)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice monopoli CK, bangun sampai hotel di semua petak CK.
Bob mendarat di GRT. Apakah Bob bayar M250?
Tulis PASS atau FAIL.
Jika FAIL: calculateRent() dengan lvl=5 (HOTEL) mengembalikan rentLevels[5].
```

### TC-05-05 | Railroad — 1 RR = M25
**Input:** Alice punya GBR saja, Bob mendarat di GBR
**Expected:** Bob bayar M25
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice dapatkan GBR (idx 5). Bob mendarat di GBR.
Apakah Bob bayar M25?
Tulis PASS atau FAIL.
Jika FAIL: RailroadProperty::calculateRent() hitung jumlah RR milik owner,
lookup di rentByCount[1] = 25.
```

### TC-05-06 | Railroad — 4 RR = M200
**Input:** Alice punya semua 4 railroad
**Expected:** Bob mendarat di salah satu → bayar M200
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice dapatkan semua 4 stasiun. Bob mendarat di GBR.
Apakah Bob bayar M200?
Tulis PASS atau FAIL.
Jika FAIL: rentByCount[4] = 200 di config railroad.txt.
```

### TC-05-07 | Utility — 1 utility, dadu total = 7
**Input:** Alice punya PLN, Bob mendarat di PLN dengan dadu 3+4=7
**Expected:** Bob bayar 7 × 4 = M28
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice dapatkan PLN. Bob: ATUR_DADU 3 4 (mendarat di PLN).
Apakah Bob bayar M28?
Tulis PASS atau FAIL.
Jika FAIL: UtilityProperty::calculateRent(): ctx.getDiceTotal() * multiplierByCount[1].
```

### TC-05-08 | Utility — 2 utility, dadu = 7
**Input:** Alice punya PLN + PAM, Bob mendarat di PLN
**Expected:** Bob bayar 7 × 10 = M70
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya PLN dan PAM. Bob ATUR_DADU 3 4, mendarat di PLN.
Apakah Bob bayar M70?
Tulis PASS atau FAIL.
Jika FAIL: multiplierByCount[2] = 10, pastikan count utility dihitung dari semua milik owner.
```

### TC-05-09 | Sewa × festival multiplier
**Input:** GRT festival mult=2, Bob mendarat
**Expected:** Bob bayar sewa_normal × 2
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice aktivasi festival di GRT (mult=2). Bob mendarat.
Misal sewa normal M4 (monopoli, 0 bangunan), apakah Bob bayar M8?
Tulis PASS atau FAIL.
Jika FAIL: calculateRent() akhirnya kalikan: return base * festivalMultiplier.
```

### TC-05-10 | ShieldCard: sewa di-skip
**Input:** Bob punya ShieldCard aktif, mendarat di properti Alice
**Expected:** Tidak ada sewa
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob aktifkan ShieldCard. Bob mendarat di GRT milik Alice.
Apakah muncul event "Shield melindungi" dan Bob tidak membayar?
Apakah uang Alice tidak bertambah?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyManager::payRent():
  if (payer.isShieldActive()) { pushEvent(...shield...); return; }
```

### TC-05-11 | Tidak mampu bayar sewa → bangkrut
**Input:** Bob uang M10, sewa M600
**Expected:** Trigger alur kebangkrutan (BankruptcyManager::handleDebt)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set Bob uang ke M10. Bob mendarat di properti mahal Alice.
Apakah alur bangkrut dimulai (panel likuidasi atau langsung bangkrut)?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyTile::onLand(), saat !payer.canAfford(rent):
  engine.getBankruptcyManager().handleDebt(payer, rent, owner);
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-05-01 | Sewa dasar tanpa monopoli | ⬜ |
| TC-05-02 | Monopoli x2 | ⬜ |
| TC-05-03 | Sewa L1 rumah | ⬜ |
| TC-05-04 | Sewa hotel | ⬜ |
| TC-05-05 | Railroad 1 RR | ⬜ |
| TC-05-06 | Railroad 4 RR | ⬜ |
| TC-05-07 | Utility 1, dadu 7 | ⬜ |
| TC-05-08 | Utility 2, dadu 7 | ⬜ |
| TC-05-09 | Sewa × festival | ⬜ |
| TC-05-10 | Shield skip sewa | ⬜ |
| TC-05-11 | Tidak mampu → bangkrut | ⬜ |

**Total: 11 test cases**
