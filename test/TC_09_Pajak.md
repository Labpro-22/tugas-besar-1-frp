# TC-09 | Pajak PPH & PBM

## Deskripsi
Menguji petak Pajak Penghasilan (PPH) dan Pajak Barang Mewah (PBM), termasuk pilihan flat vs persentase dan kondisi bangkrut.

---

## Test Cases

### TC-09-01 | PPH: pilih flat M150
**Input:** Mendarat PPH → prompt muncul → input `1`
**Expected:** Uang -= M150
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice di PPH (idx 4). Saat prompt pilihan 1/2 muncul, ketik: 1
Apakah uang Alice berkurang M150 (PPH_FLAT dari config)?
Tulis PASS atau FAIL.
Jika FAIL: TaxTile::handlePPH() pilihan 1: bank.receivePayment(player, flatAmount).
```

### TC-09-02 | PPH: pilih persentase 10%
**Input:** Alice uang=M500, properti total=M200, pilih `2`
**Expected:** Total kekayaan=M700, pajak=M70 (10%), uang -= M70
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice uang M500, punya GRT (harga beli M60). Mendarat PPH, pilih: 2
Apakah total kekayaan dihitung = uang + harga beli properti = M560?
Apakah pajak = M56 (10% × 560)?
Tulis PASS atau FAIL.
Jika FAIL: calculateWealth() = player.getMoney() + sum(prop.getPurchasePrice()) + bangunan.
```

### TC-09-03 | PPH: pilih sebelum tampil kekayaan
**Input:** Mendarat PPH
**Expected:** Prompt "pilih 1 atau 2" muncul SEBELUM detail kekayaan ditampilkan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Mendarat di PPH. Amati urutan output.
Apakah prompt "pilih 1=flat / 2=persentase" muncul SEBELUM angka total kekayaan?
Tulis PASS atau FAIL.
Jika FAIL: di TaxTile::handlePPH(), pushPrompt() harus dipanggil sebelum pushEvent() yang menampilkan kekayaan.
```

### TC-09-04 | PPH: pilih flat tapi tidak mampu → bangkrut
**Input:** Uang Alice M80 < PPH_FLAT M150, pilih `1`
**Expected:** Langsung bangkrut ke bank tanpa tawari opsi lain
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set uang Alice M80. Mendarat PPH, pilih: 1
Apakah langsung masuk alur bangkrut tanpa ditawari opsi 2?
Tulis PASS atau FAIL.
Jika FAIL: saat choice==1 && !player.canAfford(taxFlat):
  engine.getBankruptcyManager().handleDebt(player, taxFlat, nullptr); return;
  // Tidak ada tawaran opsi 2
```

### TC-09-05 | PPH: hitung kekayaan termasuk properti digadai
**Input:** Alice uang M500, GRT MORTGAGED (harga beli M60)
**Expected:** Kekayaan = M500 + M60 = M560 (properti digadai tetap dihitung)
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice gadai GRT. Mendarat PPH, pilih 2.
Apakah GRT (harga beli M60) tetap masuk perhitungan kekayaan meski MORTGAGED?
Tulis PASS atau FAIL.
Jika FAIL: calculateWealth() iterasi semua owned properties tanpa cek isMortgaged().
```

### TC-09-06 | PPH: hitung kekayaan termasuk harga bangunan
**Input:** Alice GRT + 2 rumah (houseCost=M20)
**Expected:** Kekayaan += 2 × M20 = M40 dari bangunan
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice punya GRT 2 rumah. Mendarat PPH, pilih 2.
Apakah kekayaan termasuk 2×M20 = M40 dari bangunan?
Tulis PASS atau FAIL.
Jika FAIL: di calculateWealth(), untuk STREET: wealth += street.getBuildingSellValue() * 2.
```

### TC-09-07 | PBM: potong langsung tanpa pilihan
**Input:** Mendarat PBM
**Expected:** Uang -= M200 (PBM_FLAT) langsung tanpa prompt
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Posisikan Alice di idx 38 (PBM). Apakah uang langsung berkurang M200 tanpa prompt?
Tulis PASS atau FAIL.
Jika FAIL: TaxTile::handlePBM() langsung pushEvent dan bank.receivePayment(player, flatAmount).
```

### TC-09-08 | PBM: tidak mampu bayar → bangkrut
**Input:** Uang Alice M100 < PBM_FLAT M200
**Expected:** Bangkrut ke bank
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Set uang Alice M100. Mendarat PBM.
Apakah alur bangkrut ke bank dimulai?
Tulis PASS atau FAIL.
Jika FAIL: di handlePBM(): if (!canAfford(flatAmount)) handleDebt(player, flatAmount, nullptr);
```

### TC-09-09 | ShieldCard melindungi dari PPH
**Input:** Alice ShieldCard aktif, mendarat PPH
**Expected:** Pajak tidak dipotong
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice aktifkan ShieldCard. Mendarat PPH.
Apakah muncul event "Shield melindungi dari PPH" dan uang tidak berkurang?
Tulis PASS atau FAIL.
Jika FAIL: di handlePPH(): if (player.isShieldActive()) { pushEvent(...); return; }
```

### TC-09-10 | ShieldCard melindungi dari PBM
**Input:** Alice ShieldCard aktif, mendarat PBM
**Expected:** Pajak tidak dipotong
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Alice aktifkan ShieldCard. Mendarat PBM.
Apakah uang tidak berkurang?
Tulis PASS atau FAIL.
Jika FAIL: di handlePBM(): if (player.isShieldActive()) { pushEvent(...); return; }
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-09-01 | PPH pilih flat | ⬜ |
| TC-09-02 | PPH pilih persentase | ⬜ |
| TC-09-03 | Pilih sebelum tampil kekayaan | ⬜ |
| TC-09-04 | Flat tidak mampu → bangkrut | ⬜ |
| TC-09-05 | Kekayaan termasuk MORTGAGED | ⬜ |
| TC-09-06 | Kekayaan termasuk bangunan | ⬜ |
| TC-09-07 | PBM potong langsung | ⬜ |
| TC-09-08 | PBM tidak mampu → bangkrut | ⬜ |
| TC-09-09 | Shield lindungi PPH | ⬜ |
| TC-09-10 | Shield lindungi PBM | ⬜ |

**Total: 10 test cases**
