# TC-11 | Kartu Kesempatan & Dana Umum

## Deskripsi
Menguji semua jenis kartu Kesempatan (KSP) dan Dana Umum (DNU).

---

## Test Cases

### TC-11-01 | Mendarat KSP → ambil kartu teratas
**Input:** Alice mendarat di KSP (idx 22 atau 36)
**Expected:** Kartu teratas deck KSP diambil dan efeknya dijalankan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice di KSP. Apakah kartu diambil otomatis dan efek dijalankan?
Tulis PASS atau FAIL.
Jika FAIL: CardTile::onLand() → engine.getCardManager().drawChanceCard(player, engine).
```

### TC-11-02 | KSP: Pergi ke stasiun terdekat
**Input:** Alice di idx 0 (GO), kartu "Pergi ke stasiun terdekat"
**Expected:** Alice pindah ke GBR (idx 5, stasiun terdekat searah jarum jam)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice di GO (idx 0). Paksa kartu GoToNearestRailroadCard.
Apakah Alice pindah ke stasiun terdekat (idx 5 = GBR/CDG)?
Tulis PASS atau FAIL.
Jika FAIL: GoToNearestRailroadCard::apply() loop board tiles cari RAILROAD terdekat
  dengan board.distanceTo(current, idx) > 0 minimum.
```

### TC-11-03 | KSP: Pergi stasiun terdekat, lewat GO → gaji
**Input:** Alice di idx 38 (IKN/MOS), kartu "Pergi ke stasiun terdekat"
**Expected:** Alice ke GBR (idx 5), lewat GO → dapat M200
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice di MOS (idx 39). Paksa kartu GoToNearestRailroadCard.
Apakah Alice mendapat gaji GO M200 (karena melewati idx 0)?
Tulis PASS atau FAIL.
Jika FAIL: di GoToNearestRailroadCard::apply():
  if (bestIndex < current) player.addMoney(game.getGoSalary());
```

### TC-11-04 | KSP: Mundur 3 petak
**Input:** Alice di idx 10, kartu "Mundur 3 petak"
**Expected:** Alice pindah ke idx 7
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice di idx 10 (PEN). Paksa kartu MoveBackCard(3).
Apakah Alice pindah ke idx 7 (FES)?
Tulis PASS atau FAIL.
Jika FAIL: MoveBackCard::apply() → player.move(-steps, board.size()).
```

### TC-11-05 | KSP: Masuk Penjara
**Input:** Kartu "Masuk Penjara"
**Expected:** Status JAILED, posisi → PEN (idx 10)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Paksa kartu GoToJailCard. Apakah status pemain = JAILED dan posisi = PEN?
Tulis PASS atau FAIL.
Jika FAIL: GoToJailCard::apply():
  player.setPosition(board.getIndexOf("PEN"));
  player.setStatus(PlayerStatus::JAILED);
```

### TC-11-06 | DNU: Hari ulang tahun → dapat M100 dari tiap pemain
**Input:** 4 pemain, semua uang M500, kartu "Ulang Tahun"
**Expected:** Penerima +M300, tiap pemberi -M100
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
4 pemain, masing-masing M500. P1 mendapat kartu BirthdayCard.
Apakah P1 mendapat M300 (dari P2+P3+P4) dan uang P2,P3,P4 masing-masing = M400?
Tulis PASS atau FAIL.
Jika FAIL: BirthdayCard::apply() loop activePlayers, tiap pemberi deductMoney(100),
penerima addMoney(100).
```

### TC-11-07 | DNU: Ulang tahun, pemberi tidak mampu → bangkrut
**Input:** P2 uang M50, kartu ulang tahun
**Expected:** P2 masuk alur bangkrut ke P1 (penerima)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set P2 uang M50. P1 mendapat BirthdayCard.
Apakah P2 masuk alur bangkrut dengan P1 sebagai kreditor?
Tulis PASS atau FAIL.
Jika FAIL: di BirthdayCard::apply() saat !other->canAfford(amount):
  game.getBankruptcyManager().handleDebt(*other, amount, &player);
```

### TC-11-08 | DNU: Biaya dokter M700
**Input:** Alice uang M1000, kartu "Biaya Dokter"
**Expected:** Uang -= M700
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Paksa DoctorFeeCard. Apakah uang berkurang M700?
Tulis PASS atau FAIL.
Jika FAIL: DoctorFeeCard::apply(): bank.receivePayment atau player.deductMoney(fee).
```

### TC-11-09 | DNU: Biaya dokter tidak mampu → bangkrut bank
**Input:** Alice uang M200 < M700
**Expected:** Bangkrut ke bank
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set Alice uang M200. Paksa DoctorFeeCard.
Apakah bangkrut ke bank (bukan ke pemain lain)?
Tulis PASS atau FAIL.
Jika FAIL: di DoctorFeeCard::apply():
  if (!player.canAfford(fee)) game.getBankruptcyManager().handleDebt(player, fee, nullptr);
```

### TC-11-10 | DNU: Nyaleg → bayar M200 ke tiap pemain
**Input:** 4 pemain, P1 mendapat kartu nyaleg
**Expected:** P1 bayar M200 ke P2, P3, P4 (total M600)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
4 pemain, P1 mendapat ElectionCard. P1 uang M1000.
Apakah P1 bayar M600 total (M200 × 3 pemain)?
Tulis PASS atau FAIL.
Jika FAIL: ElectionCard::apply() loop activePlayers kecuali self:
  player.deductMoney(amountPerPlayer); other.addMoney(amountPerPlayer).
```

### TC-11-11 | Kartu dikocok ulang setelah deck habis
**Input:** Semua kartu KSP ditarik 1 kali
**Expected:** Kartu ke-(n+1) adalah kartu pertama lagi (reshuffle)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Ambil semua kartu KSP (3 kartu di config). Pada pengambilan ke-4,
apakah kartu yang didapat adalah kartu pertama lagi (deck dikocok ulang)?
Tulis PASS atau FAIL.
Jika FAIL: Deck<T>::draw() saat drawPile kosong: kocok ulang discardPile ke drawPile.
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-11-01 | Mendarat KSP → ambil kartu | ⬜ |
| TC-11-02 | Pergi ke stasiun terdekat | ⬜ |
| TC-11-03 | Stasiun terdekat lewat GO | ⬜ |
| TC-11-04 | Mundur 3 petak | ⬜ |
| TC-11-05 | Masuk penjara via kartu | ⬜ |
| TC-11-06 | Ulang tahun M100 × pemain | ⬜ |
| TC-11-07 | Ulang tahun pemberi bangkrut | ⬜ |
| TC-11-08 | Biaya dokter M700 | ⬜ |
| TC-11-09 | Biaya dokter bangkrut | ⬜ |
| TC-11-10 | Nyaleg M200 × pemain | ⬜ |
| TC-11-11 | Reshuffle deck | ⬜ |

**Total: 11 test cases**
