# TC-16 | Transaction Logger

## Deskripsi
Menguji pencatatan log transaksi, format entri, perintah CETAK_LOG, dan persistensi di save file.

---

## Test Cases

### TC-16-01 | CETAK_LOG semua tanpa argumen
**Input:** Ada 10 entri log, ketik `CETAK_LOG`
**Expected:** Semua 10 entri ditampilkan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Lakukan 10 aksi berbeda. Ketik: CETAK_LOG
Apakah semua 10 entri ditampilkan?
Tulis PASS atau FAIL.
Jika FAIL: processCommand case PRINT_LOG tanpa args → logger.getAllLogs() → addEvent tiap entri.
```

### TC-16-02 | CETAK_LOG N terakhir
**Input:** 10 entri log, `CETAK_LOG 3`
**Expected:** 3 entri terakhir ditampilkan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Ada 10 entri log. Ketik: CETAK_LOG 3
Apakah hanya 3 entri terakhir yang muncul?
Tulis PASS atau FAIL.
Jika FAIL: logger.getLastN(n) mengembalikan n entri dari akhir vector.
```

### TC-16-03 | Format entri: [Turn N] username | ACTION | detail
**Input:** Alice beli GRT di turn 2
**Expected:** `[Turn 2] Alice | BELI | Beli GARUT (GRT) seharga M60`
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice beli GRT di turn 2. CETAK_LOG 1 (1 terakhir).
Apakah format sesuai "[Turn 2] Alice | BELI | Beli GARUT (GRT) seharga M60"?
Tulis PASS atau FAIL.
Jika FAIL: LogEntry::toString() format:
  "[Turn " + turn + "] " + username + " | " + action + " | " + detail.
```

### TC-16-04 | Log DADU dicatat otomatis
**Input:** Alice ATUR_DADU 3 4
**Expected:** Entri DADU: "Lempar: 3+4=7 → mendarat di ..."
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
ATUR_DADU 3 4. CETAK_LOG 1.
Apakah ada entri DADU dengan detail "3+4=7"?
Tulis PASS atau FAIL.
Jika FAIL: di resolveDiceFlow(), setelah dadu di-set: logger->logDice(username, d1, d2, landedAt).
```

### TC-16-05 | Log SEWA dicatat
**Input:** Bob mendarat di properti Alice, bayar sewa
**Expected:** Entri SEWA di log
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob mendarat di GRT Alice. CETAK_LOG 1.
Apakah ada entri SEWA dengan detail jumlah yang dibayar?
Tulis PASS atau FAIL.
Jika FAIL: PropertyManager::payRent() → logger.logRent(...).
```

### TC-16-06 | Log PAJAK dicatat
**Input:** Alice mendarat PPH, bayar M150
**Expected:** Entri PAJAK: "Bayar PPH flat M150"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice mendarat PPH, pilih flat. CETAK_LOG 1.
Apakah ada entri PAJAK?
Tulis PASS atau FAIL.
Jika FAIL: TaxTile::handlePPH() → engine.getLogger().logTax(username, "PPH flat", taxFlat).
```

### TC-16-07 | Log BANGKRUT dicatat
**Input:** Bob bangkrut
**Expected:** Entri BANGKRUT di log
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob bangkrut. CETAK_LOG 1.
Apakah ada entri BANGKRUT?
Tulis PASS atau FAIL.
Jika FAIL: BankruptcyManager::handleDebt() → logger.logBankruptcy(debtor.getUsername(), creditorName).
```

### TC-16-08 | Log SAVE dan LOAD dicatat
**Input:** SIMPAN dan MUAT
**Expected:** Entri SAVE dan LOAD di log
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
SIMPAN test.txt. CETAK_LOG 1.
Apakah ada entri SAVE? Restart, MUAT, CETAK_LOG 1 → apakah ada entri LOAD?
Tulis PASS atau FAIL.
Jika FAIL: processCommand case SAVE → logger->logSave(username, filename).
         case LOAD → logger->logLoad(username, filename).
```

### TC-16-09 | Log festival dicatat
**Input:** Alice aktivasi festival
**Expected:** Entri FESTIVAL di log
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice mendarat FES, aktivasi festival. CETAK_LOG 1.
Apakah ada entri FESTIVAL dengan detail properti dan sewa sebelum/sesudah?
Tulis PASS atau FAIL.
Jika FAIL: FestivalTile::onLand() → engine.getLogger().logFestival(...).
```

### TC-16-10 | Log tersimpan saat SIMPAN
**Input:** 15 entri log, SIMPAN, restart, MUAT
**Expected:** 15 entri log ter-restore
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Lakukan 15 aksi. SIMPAN. Restart. MUAT. CETAK_LOG.
Apakah semua 15 entri masih ada?
Tulis PASS atau FAIL.
Jika FAIL: GameEngine::createSnapshot() harus include log entries.
applySnapshot() harus restore log via logger->loadLogs(entries).
```

### TC-16-11 | Turn number di entri sesuai giliran
**Input:** Aksi di turn ke-7
**Expected:** Entri memiliki turn=7
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Mainkan 7 giliran. Cek entri log terakhir.
Apakah turn number = 7 pada entri tersebut?
Tulis PASS atau FAIL.
Jika FAIL: logger->setCurrentTurn(turnManager.getTurnNumber()) dipanggil di awal tiap giliran.
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-16-01 | CETAK_LOG semua | ⬜ |
| TC-16-02 | CETAK_LOG N terakhir | ⬜ |
| TC-16-03 | Format entri | ⬜ |
| TC-16-04 | Log DADU | ⬜ |
| TC-16-05 | Log SEWA | ⬜ |
| TC-16-06 | Log PAJAK | ⬜ |
| TC-16-07 | Log BANGKRUT | ⬜ |
| TC-16-08 | Log SAVE/LOAD | ⬜ |
| TC-16-09 | Log FESTIVAL | ⬜ |
| TC-16-10 | Log persist setelah MUAT | ⬜ |
| TC-16-11 | Turn number di entri | ⬜ |

**Total: 11 test cases**
