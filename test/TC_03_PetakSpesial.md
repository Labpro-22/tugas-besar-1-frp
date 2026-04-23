# TC-03 | Petak Spesial

## Deskripsi
Menguji mekanisme GO (gaji), Penjara (masuk/keluar/denda), Bebas Parkir, dan Pergi ke Penjara.

---

## Test Cases

### TC-03-01 | Berhenti tepat di GO → dapat gaji
**Precondition:** Alice di idx 39 (IKN)
**Input:** `ATUR_DADU 1 0` (tidak valid) — gunakan posisi yang pas, misal idx 36 + ATUR_DADU 2 2
**Expected:** Event "Petak GO — terima M200", uang += 200
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice di idx 36 (GUB), lalu ATUR_DADU 2 2 (total 4 → idx 0 = GO).
Apakah Alice menerima M200 (dari GoTile::onLand)?
Apakah event menyebutkan "berhenti di GO" bukan "lewat GO"?
Tulis PASS atau FAIL.
Jika FAIL: pastikan GoTile::onLand() memanggil engine.getBank().paySalary(player, salary)
dan engine.pushEvent() dengan pesan yang benar.
```

---

### TC-03-02 | Hanya mampir di Penjara (dari dadu normal)
**Precondition:** Alice di idx 7 (FES)
**Input:** `ATUR_DADU 2 1` (total 3 → idx 10 = PEN)
**Expected:** Tidak ada penalti, status Alice tetap ACTIVE
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice di idx 7, ATUR_DADU 2 1 (mendarat di PEN idx 10).
Apakah status Alice tetap ACTIVE (bukan JAILED)?
Apakah tidak ada event penalti?
Tulis PASS atau FAIL.
Jika FAIL: di JailTile::onLand(), pastikan hanya pemain dengan status JAILED yang
diproses, bukan pemain yang landing normal.
```

---

### TC-03-03 | Masuk penjara via PPJ
**Precondition:** Alice di idx 27 (AMS)
**Input:** `ATUR_DADU 2 1` (total 3 → idx 30 = PPJ)
**Expected:** Alice → idx 10 (PEN), status JAILED, giliran berakhir
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice di idx 27, ATUR_DADU 2 1.
Apakah posisi Alice berpindah ke idx 10?
Apakah status Alice = JAILED?
Apakah giliran langsung berpindah ke Bob?
Tulis PASS atau FAIL.
Jika FAIL: periksa GoToJailTile::onLand() memanggil jailTile.sendToJail(player)
yang men-set posisi ke index PEN dan status ke JAILED.
```

---

### TC-03-04 | Masuk penjara via kartu GoToJailCard
**Precondition:** Alice mendarat di KSP
**Input:** Kartu "Masuk Penjara" ditarik
**Expected:** Status JAILED, posisi → PEN
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Atur agar deck KSP hanya berisi GoToJailCard (atau paksa draw kartu tersebut).
Posisikan Alice di idx 22, ATUR_DADU 1 0 -> pakai ATUR_DADU 1 1 (ke KSP idx 22 → debug).
Setelah kartu ditarik, apakah Alice berstatus JAILED di idx 10?
Tulis PASS atau FAIL.
Jika FAIL: periksa GoToJailCard::apply() — set player.setStatus(JAILED),
player.setPosition(board.getIndexOf("PEN")).
```

---

### TC-03-05 | Keluar penjara: BAYAR_DENDA sebelum lempar dadu
**Precondition:** Alice JAILED
**Input:** `BAYAR_DENDA`
**Expected:** Uang -= 50 (JAIL_FINE), status → ACTIVE, bisa LEMPAR_DADU
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buat Alice masuk penjara (via PPJ). Catat uang Alice.
Ketik: BAYAR_DENDA
Apakah uang Alice berkurang M50? Status ACTIVE?
Lalu ketik LEMPAR_DADU — apakah berhasil tanpa error?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand case PAY_JAIL_FINE:
  current.deductMoney(jailFine);
  current.setStatus(PlayerStatus::ACTIVE);
  current.setJailTurns(0);
```

---

### TC-03-06 | Keluar penjara: lempar double
**Precondition:** Alice JAILED, jailTurns = 0
**Input:** `ATUR_DADU 3 3` (double)
**Expected:** Status → ACTIVE, bergerak sesuai dadu, jailTurns di-reset
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice JAILED, ketik: ATUR_DADU 3 3
Apakah muncul "mendapat double, keluar dari penjara"?
Apakah Alice bergerak 6 petak dari PEN (idx 10 → idx 16)?
Tulis PASS atau FAIL.
Jika FAIL: di resolveDiceFlow() saat current.isJailed():
  if (rolledDouble) { current.setStatus(ACTIVE); current.setJailTurns(0); ... }
```

---

### TC-03-07 | Di penjara: gagal double, tidak bergerak
**Precondition:** Alice JAILED, jailTurns = 0
**Input:** `ATUR_DADU 3 4` (bukan double)
**Expected:** Alice tidak bergerak, jailTurns++, giliran berpindah
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice JAILED, ketik: ATUR_DADU 3 4 (non-double)
Apakah Alice tetap di idx 10 (tidak bergerak)?
Apakah jailTurns bertambah 1?
Apakah giliran berpindah ke Bob?
Tulis PASS atau FAIL.
Jika FAIL: di resolveDiceFlow() blok isJailed(), saat !rolledDouble:
  current.incrementJailTurns();
  current.resetConsecutiveDoubles();
  flowResult.append(executeTurn()); return;
```

---

### TC-03-08 | Penjara giliran ke-4: wajib bayar denda
**Precondition:** Alice JAILED, jailTurns = 3
**Input:** `LEMPAR_DADU`
**Expected:** Otomatis deduct M50, status ACTIVE, lalu bergerak
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buat Alice masuk penjara, lalu gagal double 3 kali (jailTurns = 3).
Ketik: LEMPAR_DADU
Apakah uang Alice otomatis berkurang M50 sebelum bergerak?
Apakah ada event "wajib membayar denda pada percobaan ke-4"?
Tulis PASS atau FAIL.
Jika FAIL: di resolveDiceFlow() saat current.getJailTurns() >= 3:
  current.deductMoney(jailFine);
  current.setStatus(PlayerStatus::ACTIVE);
  current.setJailTurns(0);
  // lalu bergerak normal
```

---

### TC-03-09 | BAYAR_DENDA setelah lempar dadu → error (harus ketika gilirannya)
**Precondition:** Alice JAILED, sudah LEMPAR_DADU
**Input:** `BAYAR_DENDA`
**Expected:** `GameException`: harus sebelum lempar dadu
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice JAILED, ketik LEMPAR_DADU (gagal double), lalu ketik BAYAR_DENDA.
Apakah muncul error "BAYAR_DENDA harus dilakukan sebelum melempar dadu"?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand case PAY_JAIL_FINE:
  if (diceRolledThisTurn) throw GameException("BAYAR_DENDA harus sebelum lempar dadu.");
```

---



---

### TC-03-10 | Bebas Parkir: tidak ada aksi
**Precondition:** Alice di dekat idx 20 (BBP)
**Input:** ATUR_DADU ke idx 20
**Expected:** Tidak ada event apapun yang merugikan/menguntungkan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice tepat di depan BBP, ATUR_DADU untuk mendarat di idx 20.
Apakah tidak ada event pajak, sewa, atau aksi lain?
Apakah giliran berlanjut normal?
Tulis PASS atau FAIL.
Jika FAIL: pastikan FreeParkingTile::onLand() kosong atau hanya pushEvent info.
```

---

### TC-03-11 | PPJ: giliran langsung berakhir
**Precondition:** Alice mendarat di PPJ
**Input:** ATUR_DADU ke idx 30
**Expected:** Giliran Alice langsung berakhir, giliran berpindah ke Bob
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice di idx 27, ATUR_DADU 1 2 (→ idx 30 = PPJ).
Setelah mendarat, apakah giliran langsung berpindah ke Bob tanpa Alice bisa input lagi?
Tulis PASS atau FAIL.
Jika FAIL: di GoToJailTile::onLand(), setelah sendToJail() pastikan engine
tidak menunggu input lagi (executeTurn() tidak dipanggil dengan giliran sama).
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-03-01 | Berhenti di atau lewat GO → gaji | ⬜ |
| TC-03-02 | Mampir penjara normal | ⬜ |
| TC-03-03 | Masuk penjara via PPJ | ⬜ |
| TC-03-04 | Masuk penjara via kartu | ⬜ |
| TC-03-05 | Keluar via BAYAR_DENDA | ⬜ |
| TC-03-06 | Keluar via double | ⬜ |
| TC-03-07 | Gagal double, tidak bergerak | ⬜ |
| TC-03-08 | Giliran ke-4 (ketika 3 kali gagal lempar dadu double) wajib bayar | ⬜ |
| TC-03-09 | BAYAR_DENDA harus ketika gilirannya | ⬜ |
| TC-03-10 | Bebas Parkir tidak ada aksi | ⬜ |
| TC-03-11 | PPJ giliran langsung berakhir | ⬜ |

**Total: 11 test cases**
