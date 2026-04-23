# TC-13 | Kebangkrutan & Likuidasi

## Deskripsi
Menguji alur kebangkrutan lengkap: deteksi, panel likuidasi, bangkrut ke pemain, dan bangkrut ke bank.

---

## Test Cases

### TC-13-01 | Panel likuidasi muncul jika bisa lunasi
**Input:** Bob uang M50, punya GRT (nilai jual M60), sewa M100
**Expected:** Panel likuidasi muncul (bukan langsung bangkrut)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob uang M50, punya GRT. Bob tidak mampu bayar sewa M100.
maxLiquidation = M50 + M60 = M110 >= M100 → harus panel likuidasi.
Apakah panel jual/gadai muncul?
Tulis PASS atau FAIL.
Jika FAIL: BankruptcyManager::handleDebt() → canLiquidate() → runLiquidationPanel().
```

### TC-13-02 | Jual properti di panel → dapat uang
**Input:** Panel muncul, pilih JUAL GRT
**Expected:** GRT kembali ke bank, Bob dapat M60
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Di panel likuidasi, pilih JUAL GRT.
Apakah GRT berstatus BANK dan Bob menerima M60?
Tulis PASS atau FAIL.
Jika FAIL: di runLiquidationPanel(), aksi "JUAL":
  propertyManager.sellPropertyToBank(player, *prop);
```

### TC-13-03 | Gadai properti di panel → dapat mortgageValue
**Input:** Panel muncul, pilih GADAI GRT
**Expected:** GRT MORTGAGED, Bob dapat M40 (mortgageValue)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Di panel, pilih GADAI GRT.
Apakah GRT MORTGAGED dan Bob dapat M40?
Tulis PASS atau FAIL.
Jika FAIL: aksi "GADAI" di panel → propertyManager.mortgageProperty(player, *prop).
```

### TC-13-04 | Setelah likuidasi cukup → tidak jadi bangkrut
**Input:** Bob jual GRT, uang cukup bayar kewajiban
**Expected:** Kewajiban dibayar, Bob tetap ACTIVE
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob jual GRT di panel. Apakah setelah kewajiban terpenuhi, Bob tetap ACTIVE?
Apakah muncul event "kewajiban terpenuhi" dan bukan event bangkrut?
Tulis PASS atau FAIL.
Jika FAIL: di handleDebt(), setelah runLiquidationPanel():
  if (debtor.getMoney() >= obligation) { bayar kewajiban; return; }
```

### TC-13-05 | Bangkrut ke pemain: aset ke kreditor
**Input:** Bob tidak bisa lunasi, bangkrut ke Alice
**Expected:** Semua uang + properti Bob → Alice
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob bangkrut ke Alice. Catat aset Bob sebelumnya.
Apakah semua aset Bob (uang + properti) berpindah ke Alice?
Tulis PASS atau FAIL.
Jika FAIL: transferAssetsToCreditor(): loop owned properties,
debtor.removeProperty(prop); prop.setOwner(&creditor); creditor.addProperty(prop).
```

### TC-13-06 | Bangkrut ke pemain: properti MORTGAGED ikut dialihkan
**Input:** Bob punya GRT MORTGAGED, bangkrut ke Alice
**Expected:** Alice mendapat GRT dalam kondisi MORTGAGED
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob gadai GRT. Bob bangkrut ke Alice.
Apakah Alice mendapat GRT dalam status MORTGAGED?
Tulis PASS atau FAIL.
Jika FAIL: transferAssetsToCreditor() tidak mengubah status properti saat dialihkan.
```

### TC-13-07 | Bangkrut ke bank: uang hilang
**Input:** Bob bangkrut karena pajak (bangkrut ke bank)
**Expected:** Uang Bob hilang, tidak ke pemain manapun
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob bangkrut ke bank (karena pajak). Catat uang semua pemain sebelum.
Apakah uang Bob = 0 setelah bangkrut? Apakah uang pemain lain tidak bertambah?
Tulis PASS atau FAIL.
Jika FAIL: returnAssetsToBank(): debtor.deductMoney(debtor.getMoney()) tanpa transfer ke siapapun.
```

### TC-13-08 | Bangkrut ke bank: properti dilelang
**Input:** Bob punya 3 properti, bangkrut ke bank
**Expected:** 3 properti dilelang satu per satu
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob punya GRT, TSK, GBR. Bob bangkrut ke bank.
Apakah 3 lelang terjadi (satu per properti)?
Tulis PASS atau FAIL.
Jika FAIL: auctionAllBankProperties() loop toAuction → startAuction() per properti.
```

### TC-13-09 | Bangkrut ke bank: bangunan dihancurkan
**Input:** Bob punya GRT hotel, bangkrut ke bank
**Expected:** GRT kembali ke NONE sebelum dilelang
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob punya GRT hotel. Bob bangkrut ke bank.
Apakah GRT level = NONE saat masuk lelang?
Tulis PASS atau FAIL.
Jika FAIL: returnAssetsToBank() untuk STREET: street.demolishBuildings() sebelum bank.reclaim().
```

### TC-13-10 | Pemain bangkrut → status BANKRUPT, keluar giliran
**Input:** Bob dinyatakan bangkrut
**Expected:** Status Bob = BANKRUPT, Bob tidak ikut giliran berikutnya
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Bob bangkrut. Apakah status Bob = BANKRUPT?
Apakah giliran berikutnya melewati Bob langsung ke pemain lain?
Tulis PASS atau FAIL.
Jika FAIL: debtor.setStatus(PlayerStatus::BANKRUPT);
engine.getTurnManager().removePlayer(currentIndex).
```

### TC-13-11 | 1 pemain tersisa → game selesai
**Input:** 2 pemain, P2 bangkrut
**Expected:** P1 menang, game selesai
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
2 pemain. P2 bangkrut.
Apakah game langsung selesai dan P1 dinyatakan menang?
Tulis PASS atau FAIL.
Jika FAIL: di BankruptcyManager setelah bangkrut, checkWinCondition() dipanggil:
  saat activeCount <= 1 → endGame().
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-13-01 | Panel likuidasi muncul | ⬜ |
| TC-13-02 | Jual properti di panel | ⬜ |
| TC-13-03 | Gadai properti di panel | ⬜ |
| TC-13-04 | Setelah likuidasi tidak bangkrut | ⬜ |
| TC-13-05 | Bangkrut ke pemain: aset dialihkan | ⬜ |
| TC-13-06 | MORTGAGED ikut dialihkan | ⬜ |
| TC-13-07 | Bangkrut ke bank: uang hilang | ⬜ |
| TC-13-08 | Bangkrut ke bank: properti dilelang | ⬜ |
| TC-13-09 | Bangunan dihancurkan sebelum lelang | ⬜ |
| TC-13-10 | Status BANKRUPT, keluar giliran | ⬜ |
| TC-13-11 | 1 pemain tersisa → game selesai | ⬜ |

**Total: 11 test cases**
