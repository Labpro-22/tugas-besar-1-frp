# TC-04 | Kepemilikan Properti

## Deskripsi
Menguji pembelian street, perolehan otomatis railroad/utility, CETAK_AKTA, CETAK_PROPERTI, dan status properti.

---

## Test Cases

### TC-04-01 | Mendarat di street BANK → tawarkan beli
**Input:** ATUR_DADU ke GRT (idx 1)
**Expected:** Tampil akta + prompt "Apakah ingin membeli?"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game 2 pemain. ATUR_DADU 1 0 — pakai ATUR_DADU 1 1 (→ idx 2 = SLM jika config baru).
Apakah muncul info properti dan prompt y/n untuk membeli?
Tulis PASS atau FAIL.
Jika FAIL: periksa PropertyTile::onLand() memanggil propertyManager.offerPurchase()
saat prop.isBank() dan prop.getType() == STREET.
```

### TC-04-02 | Beli street dengan uang cukup → OWNED
**Input:** Prompt beli → input `y`
**Expected:** Uang -= harga beli, status → OWNED, properti masuk daftar milik pemain
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Mendarat di GRT (harga M60). Saat prompt muncul, ketik: y
Apakah uang berkurang M60 (dari M1000 → M940)?
Apakah CETAK_PROPERTI menampilkan GRT OWNED?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyManager::offerPurchase(), setelah jawaban "y":
  bank.receivePayment(buyer, price);
  bank.transferPropertyToPlayer(&prop, buyer);
```

### TC-04-03 | Tolak beli street → lelang
**Input:** Prompt beli → input `n`
**Expected:** Lelang dimulai otomatis
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Mendarat di street, prompt muncul, ketik: n
Apakah event "Properti masuk sistem lelang" muncul?
Apakah alur lelang dimulai (giliran bid/pass muncul)?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyTile::onLand(), saat offerPurchase() return false:
  engine.getAuctionManager().startAuction(prop, &player, true);
```

### TC-04-04 | Uang tidak cukup → lelang langsung
**Input:** Uang < harga beli, mendarat di street BANK
**Expected:** Properti langsung masuk lelang tanpa prompt
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set uang Alice ke M30, lalu mendarat di GRT (harga M60).
Apakah muncul event "Uang tidak cukup" dan langsung masuk lelang (tanpa prompt y/n)?
Tulis PASS atau FAIL.
Jika FAIL: di offerPurchase(), sebelum pushPrompt():
  if (!buyer.canAfford(price)) { pushEvent(...); return false; }
```

### TC-04-05 | Railroad BANK → otomatis milik pemain
**Input:** ATUR_DADU ke GBR (idx 5)
**Expected:** "Stasiun kini menjadi milikmu" tanpa prompt beli
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game, ATUR_DADU ke idx 5 (GBR/CDG).
Apakah event muncul bahwa stasiun langsung milik pemain tanpa prompt?
Apakah CETAK_PROPERTI menampilkan stasiun OWNED?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyTile::onLand(), saat prop.isBank() && type != STREET:
  engine.getPropertyManager().autoAcquire(player, prop);
```

### TC-04-06 | Utility BANK → otomatis milik pemain
**Input:** ATUR_DADU ke PLN (idx 12)
**Expected:** "PLN kini menjadi milikmu" tanpa prompt
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game, ATUR_DADU ke idx 12 (PLN/EDF).
Apakah event menunjukkan utility langsung jadi milik pemain?
Tulis PASS atau FAIL.
Jika FAIL: sama dengan TC-04-05.
```

### TC-04-07 | CETAK_AKTA dengan kode
**Input:** `CETAK_AKTA JKT`
**Expected:** Akta lengkap: nama, harga, gadai, sewa per level, status
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Ketik: CETAK_AKTA JKT (atau LON jika config baru)
Apakah output menampilkan semua field: harga beli, nilai gadai,
sewa L0-L5, harga rumah, harga hotel, dan status (BANK/OWNED)?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand case PRINT_DEED, lengkapi ostringstream dengan
semua field StreetProperty termasuk rentLevels[0..5].
```

### TC-04-08 | CETAK_AKTA tanpa kode → properti di posisi saat ini
**Input:** Alice di atas GRT, ketik `CETAK_AKTA`
**Expected:** Akta GRT muncul
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice di idx 1 (GRT), ketik: CETAK_AKTA
Apakah akta GRT muncul?
Tulis PASS atau FAIL.
Jika FAIL: di case PRINT_DEED, saat cmd.args.empty():
  Tile& tile = getBoard().getTileByIndex(getCurrentPlayer().getPosition());
  if (!tile.isProperty()) throw GameException(...);
```

### TC-04-09 | CETAK_AKTA kode tidak ada → error
**Input:** `CETAK_AKTA XYZ`
**Expected:** GameException: kode tidak ditemukan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Ketik: CETAK_AKTA XYZ
Apakah muncul error "Tile dengan kode XYZ tidak ditemukan"?
Tulis PASS atau FAIL.
Jika FAIL: Board::getTileByCode() harus throw TileNotFoundException(code).
```

### TC-04-10 | CETAK_PROPERTI — tampil daftar dengan status
**Input:** Alice punya GRT (OWNED), TSK (MORTGAGED)
**Expected:** Daftar kedua properti dengan status masing-masing
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Beli GRT (OWNED), gadai TSK (MORTGAGED). Ketik: CETAK_PROPERTI
Apakah GRT muncul sebagai OWNED dan TSK sebagai MORTGAGED?
Apakah total nilai properti ditampilkan?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand case PRINT_PROPERTIES, iterasi owned properties
dan tampilkan status tiap properti dengan getStatus().
```

### TC-04-11 | CETAK_PROPERTI — tidak punya properti
**Input:** `CETAK_PROPERTI` sebelum beli apapun
**Expected:** "Belum memiliki properti apapun"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game, langsung ketik: CETAK_PROPERTI
Apakah muncul pesan "belum memiliki properti"?
Tulis PASS atau FAIL.
Jika FAIL: di case PRINT_PROPERTIES, saat owned.empty():
  result.addEvent(..., "Belum memiliki properti apapun.");
```

### TC-04-12 | Properti MORTGAGED tidak generate sewa
**Input:** Alice punya GRT MORTGAGED, Bob mendarat di GRT
**Expected:** Tidak ada sewa yang dipungut, event "properti digadaikan"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT, lalu GADAI GRT. Bob mendarat di GRT.
Apakah Bob tidak membayar sewa?
Apakah muncul event "properti sedang digadaikan, tidak ada sewa"?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyManager::payRent():
  if (prop.isMortgaged()) { pushEvent(...); return; }
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-04-01 | Mendarat street BANK → prompt | ⬜ |
| TC-04-02 | Beli street → OWNED | ⬜ |
| TC-04-03 | Tolak beli → lelang | ⬜ |
| TC-04-04 | Uang kurang → lelang langsung | ⬜ |
| TC-04-05 | Railroad otomatis | ⬜ |
| TC-04-06 | Utility otomatis | ⬜ |
| TC-04-07 | CETAK_AKTA dengan kode | ⬜ |
| TC-04-08 | CETAK_AKTA tanpa kode | ⬜ |
| TC-04-09 | CETAK_AKTA kode tidak ada | ⬜ |
| TC-04-10 | CETAK_PROPERTI dengan properti | ⬜ |
| TC-04-11 | CETAK_PROPERTI kosong | ⬜ |
| TC-04-12 | MORTGAGED tidak generate sewa | ⬜ |

**Total: 12 test cases**
