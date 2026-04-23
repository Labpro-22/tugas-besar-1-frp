# TC-06 | Peningkatan Properti

## Deskripsi
Menguji perintah BANGUN: syarat monopoli, aturan pemerataan, upgrade hotel, dan validasi error.

---

## Test Cases

### TC-06-01 | BANGUN rumah dengan monopoli
**Input:** Alice monopoli CK (GRT+TSK), ketik `BANGUN GRT`
**Expected:** 1 rumah di GRT, uang -= M20 (UPG_RUMAH GRT)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT dan TSK. Ketik: BANGUN GRT
Prompt "bangun rumah? biaya M20" → input y
Apakah GRT punya 1 rumah? Uang Alice berkurang M20?
Tulis PASS atau FAIL.
Jika FAIL: PropertyManager::buildOnProperty() harus cek hasMonopoly() dulu,
lalu prop.buildHouse() dan bank.receivePayment().
```

### TC-06-02 | BANGUN tanpa monopoli → error
**Input:** Alice hanya punya GRT (tidak punya TSK)
**Expected:** GameException: harus monopoli untuk membangun
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice hanya beli GRT. Ketik: BANGUN GRT
Apakah muncul error "harus memonopoli color group CK"?
Tulis PASS atau FAIL.
Jika FAIL: di buildOnProperty():
  if (!hasMonopoly(player, cg)) throw GameException("Harus monopoli [" + cg + "].");
```

### TC-06-03 | Pemerataan: skip level → hanya tampil petak minimum
**Input:** Alice monopoli CK (GRT=0, TSK=0), coba BANGUN TSK sebelum GRT
**Expected:** Hanya GRT yang muncul sebagai pilihan valid (bukan TSK)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT dan TSK. Ketik BANGUN TSK.
Apakah muncul pesan bahwa TSK tidak bisa dibangun dulu (GRT harus dibangun dulu)?
Atau apakah hanya GRT yang muncul di daftar buildable?
Tulis PASS atau FAIL.
Jika FAIL: getBuildableTilesInGroup() hanya kembalikan petak dengan level == minLevel.
```

### TC-06-04 | Pemerataan: selisih max 1
**Input:** GRT=1 rumah, TSK=0 rumah — coba bangun GRT ke-2
**Expected:** Error atau GRT tidak muncul di pilihan (TSK harus dibangun dulu)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
BANGUN GRT (1 rumah). Coba BANGUN GRT lagi.
Apakah muncul error karena TSK belum punya 1 rumah?
Tulis PASS atau FAIL.
Jika FAIL: cek selisih level antar petak: hanya boleh bangun di petak dengan buildingCount == minLevel.
```

### TC-06-05 | Upgrade ke hotel
**Input:** Semua petak CK sudah 4 rumah, BANGUN GRT
**Expected:** Prompt upgrade hotel, GRT jadi hotel, uang -= UPG_HT (M50)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Build GRT dan TSK secara merata sampai keduanya 4 rumah.
Ketik BANGUN GRT → prompt upgrade hotel → input y
Apakah GRT berstatus hotel? Uang berkurang M50?
Tulis PASS atau FAIL.
Jika FAIL: di buildOnProperty(), saat level HOUSE_4 dan semua CG punya 4 rumah:
  prop.buildHotel(); bank.receivePayment(player, prop.getHotelCost());
```

### TC-06-06 | Tidak bisa upgrade hotel jika belum semua 4 rumah
**Input:** GRT=4 rumah, TSK=3 rumah — coba upgrade GRT
**Expected:** GameException: semua petak harus 4 rumah dulu
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Build sampai GRT=4, TSK=3. Ketik BANGUN GRT.
Apakah muncul error "semua petak [CK] harus 4 rumah sebelum hotel"?
Tulis PASS atau FAIL.
Jika FAIL: di buildOnProperty() saat upgradeToHotel, loop semua petak CG:
  for (auto* sp : getColorGroup(cg))
    if (sp->getBuildingLevel() != HOUSE_4) throw GameException(...);
```

### TC-06-07 | Tidak bisa bangun setelah hotel
**Input:** GRT sudah hotel, coba BANGUN GRT
**Expected:** MaxBuildingLevelException
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
GRT sudah hotel. Ketik: BANGUN GRT
Apakah muncul error MaxBuildingLevelException?
Tulis PASS atau FAIL.
Jika FAIL: di buildOnProperty():
  if (prop.getBuildingLevel() == BuildingLevel::HOTEL) throw MaxBuildingLevelException(prop.getCode());
```

### TC-06-08 | Uang tidak cukup bangun
**Input:** Alice uang M5, harga rumah M20
**Expected:** Event "uang tidak cukup", return false
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice monopoli CK, uang M5. Ketik BANGUN GRT → prompt → input y
Apakah muncul event "uang tidak cukup" dan rumah tidak terbangun?
Tulis PASS atau FAIL.
Jika FAIL: di buildOnProperty():
  if (!player.canAfford(cost)) { pushEvent(...); return false; }
```

### TC-06-09 | BANGUN di Railroad → error
**Input:** Alice punya GBR, ketik `BANGUN GBR`
**Expected:** BuildNotAllowedException
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice dapat GBR (railroad). Ketik: BANGUN GBR
Apakah muncul error "BANGUN hanya berlaku untuk properti STREET"?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand case BUILD:
  if (prop.getType() != PropertyType::STREET) throw GameException("BANGUN hanya untuk STREET.");
```

### TC-06-10 | getSellValue termasuk setengah harga bangunan
**Input:** GRT punya 2 rumah (houseCost=20), getSellValue()
**Expected:** 60 + 2×(20/2) = 80
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya GRT 2 rumah. Cek nilai jual di panel likuidasi atau CETAK_PROPERTI.
Apakah nilai jual GRT = M80 (harga beli M60 + 2 rumah × M10)?
Tulis PASS atau FAIL.
Jika FAIL: StreetProperty::getSellValue():
  return purchasePrice + numHouses * (houseCost / 2);
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-06-01 | Bangun rumah monopoli | ⬜ |
| TC-06-02 | Bangun tanpa monopoli | ⬜ |
| TC-06-03 | Pemerataan tampil minimum | ⬜ |
| TC-06-04 | Selisih max 1 | ⬜ |
| TC-06-05 | Upgrade hotel | ⬜ |
| TC-06-06 | Hotel butuh semua 4 rumah | ⬜ |
| TC-06-07 | Sudah hotel → error | ⬜ |
| TC-06-08 | Uang kurang → error | ⬜ |
| TC-06-09 | BANGUN railroad → error | ⬜ |
| TC-06-10 | getSellValue + bangunan | ⬜ |

**Total: 10 test cases**
