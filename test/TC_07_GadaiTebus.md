# TC-07 | Gadai & Tebus

## Deskripsi
Menguji perintah GADAI dan TEBUS beserta semua constraint dan alur jual bangunan.

---

## Test Cases

### TC-07-01 | GADAI properti OWNED tanpa bangunan
**Input:** Alice OWNED GRT tanpa bangunan, `GADAI GRT`
**Expected:** Status → MORTGAGED, uang += mortgageValue (M40)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT. Ketik: GADAI GRT
Apakah status GRT menjadi MORTGAGED? Uang Alice += M40?
Tulis PASS atau FAIL.
Jika FAIL: PropertyManager::mortgageProperty():
  prop.setStatus(MORTGAGED); bank.sendMoney(player, mortgageValue);
```

### TC-07-02 | GADAI dengan bangunan di color group → jual dulu
**Input:** Alice monopoli CK, GRT dan TSK masing-masing 1 rumah, `GADAI GRT`
**Expected:** Prompt jual bangunan CK dulu → jual → lalu gadai
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice bangun 1 rumah di GRT dan TSK. Ketik: GADAI GRT
Apakah muncul prompt "jual semua bangunan di [CK] dulu?"
Ketik y. Apakah bangunan terjual dan GRT tergadai?
Tulis PASS atau FAIL.
Jika FAIL: hasBuildingsInColorGroup() return true →
pushPrompt "jual_bangunan_GRT" → sellAllBuildingsInColorGroup().
```

### TC-07-03 | GADAI properti MORTGAGED → error
**Input:** GRT sudah MORTGAGED
**Expected:** AlreadyMortgagedException
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
GADAI GRT dua kali. Apakah percobaan kedua menghasilkan error AlreadyMortgagedException?
Tulis PASS atau FAIL.
Jika FAIL: di mortgageProperty():
  if (prop.isMortgaged()) throw AlreadyMortgagedException(prop.getCode());
```

### TC-07-04 | GADAI milik orang lain → error
**Input:** GRT milik Bob, Alice coba `GADAI GRT`
**Expected:** NotOwnerException
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob beli GRT. Alice ketik: GADAI GRT
Apakah muncul NotOwnerException?
Tulis PASS atau FAIL.
Jika FAIL: if (prop.getOwner() != &player) throw NotOwnerException(player.getUsername(), prop.getCode());
```

### TC-07-05 | TEBUS properti MORTGAGED
**Input:** GRT MORTGAGED, `TEBUS GRT`
**Expected:** Bayar purchasePrice (M60), status → OWNED
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice GADAI GRT. Lalu: TEBUS GRT
Apakah uang Alice berkurang M60? Status GRT = OWNED?
Tulis PASS atau FAIL.
Jika FAIL: redeemProperty():
  bank.receivePayment(player, prop.getPurchasePrice());
  prop.setStatus(OwnershipStatus::OWNED);
```

### TC-07-06 | TEBUS dengan uang tidak cukup
**Input:** Uang Alice < purchasePrice
**Expected:** Event "uang tidak cukup", return false
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice gadai GRT (dapat M40), uang tersisa kurang dari M60. Ketik: TEBUS GRT
Apakah muncul event "uang tidak cukup untuk menebus"?
Tulis PASS atau FAIL.
Jika FAIL: if (!player.canAfford(redeemCost)) { pushEvent(...); return false; }
```

### TC-07-07 | TEBUS properti OWNED (bukan MORTGAGED) → error
**Input:** GRT status OWNED, `TEBUS GRT`
**Expected:** NotMortgagedException
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT (OWNED). Ketik: TEBUS GRT
Apakah muncul NotMortgagedException?
Tulis PASS atau FAIL.
Jika FAIL: if (!prop.isMortgaged()) throw NotMortgagedException(prop.getCode());
```

### TC-07-08 | MORTGAGED tidak generate sewa
**Input:** GRT MORTGAGED milik Alice, Bob mendarat
**Expected:** Bob tidak bayar sewa
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice gadai GRT. Bob mendarat di GRT.
Apakah Bob tidak membayar sewa? Uang Alice tidak bertambah?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyTile::onLand() atau payRent(): if (prop.isMortgaged()) return;
```

### TC-07-09 | Jual bangunan: nilai = ½ harga beli
**Input:** GRT punya 2 rumah (houseCost=M20), jual bangunan
**Expected:** Alice dapat 2 × M10 = M20 dari jual bangunan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice 2 rumah di GRT. Proses gadai → prompt jual bangunan → y
Apakah Alice mendapat M20 (2 rumah × M10)?
Tulis PASS atau FAIL.
Jika FAIL: getBuildingSellValue() = numHouses * (houseCost / 2).
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-07-01 | GADAI normal | ⬜ |
| TC-07-02 | GADAI dengan bangunan → jual dulu | ⬜ |
| TC-07-03 | GADAI dua kali → error | ⬜ |
| TC-07-04 | GADAI milik orang lain → error | ⬜ |
| TC-07-05 | TEBUS normal | ⬜ |
| TC-07-06 | TEBUS uang kurang | ⬜ |
| TC-07-07 | TEBUS OWNED → error | ⬜ |
| TC-07-08 | MORTGAGED tidak sewa | ⬜ |
| TC-07-09 | Nilai jual bangunan ½ | ⬜ |

**Total: 9 test cases**
