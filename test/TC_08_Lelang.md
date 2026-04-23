# TC-08 | Lelang

## Deskripsi
Menguji mekanisme lelang: urutan, BID/PASS, kondisi selesai, minimal 1 bid, dan integrasi bangkrut.

---

## Test Cases

### TC-08-01 | Lelang dipicu saat pemain tolak beli
**Input:** Pemain mendarat di street BANK → input `n`
**Expected:** Lelang dimulai dari pemain berikutnya
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
New Game 4 pemain. P1 mendarat di GRT → input n.
Apakah lelang dimulai? Apakah giliran lelang dimulai dari P2 (bukan P1)?
Tulis PASS atau FAIL.
Jika FAIL: di PropertyTile::onLand() setelah offerPurchase() return false:
  engine.getAuctionManager().startAuction(prop, &player, false);
```

### TC-08-02 | Urutan lelang dari pemain setelah trigger
**Input:** 4 pemain, P1 trigger lelang
**Expected:** Urutan: P2 → P3 → P4 → P1
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
P1 (giliran pertama) tolak beli. Catat urutan prompt giliran lelang.
Apakah urutan P2, P3, P4, P1?
Tulis PASS atau FAIL.
Jika FAIL: AuctionManager::buildAuctionOrder() mulai dari index triggerIdx+1.
```

### TC-08-03 | BID valid
**Input:** Giliran P2 di lelang, ketik `BID 50`
**Expected:** highestBid = 50, highestBidder = P2
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Saat giliran P2 di lelang, ketik: BID 50
Apakah output menampilkan "Penawaran tertinggi: M50 (P2)"?
Tulis PASS atau FAIL.
Jika FAIL: di processTurn(), parse "BID 50" → bidAmount=50, update highestBid.
```

### TC-08-04 | BID harus lebih tinggi dari bid sebelumnya
**Input:** highestBid = 100, ketik `BID 100`
**Expected:** Event "bid terlalu rendah"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Saat highestBid = 100, ketik: BID 100
Apakah muncul event "bid harus lebih tinggi dari M100"?
Tulis PASS atau FAIL.
Jika FAIL: if (bidAmount <= highestBid) { pushEvent(...); return false; }
```

### TC-08-05 | BID melebihi uang → error
**Input:** P3 uang M80, ketik `BID 100`
**Expected:** Event "uang tidak cukup untuk bid M100"
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
P3 uang M80. Saat giliran P3, ketik: BID 100
Apakah muncul event "uang tidak cukup"?
Tulis PASS atau FAIL.
Jika FAIL: if (!current.canAfford(bidAmount)) { pushEvent(...); return false; }
```

### TC-08-06 | Lelang selesai setelah (n-1) PASS berturut
**Input:** 4 pemain, P2 bid M50, P3+P4+P1 PASS
**Expected:** Lelang selesai, P2 menang dengan M50
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
P2 BID 50, P3 PASS, P4 PASS, P1 PASS.
Apakah lelang selesai? Apakah P2 mendapat properti dengan bayar M50?
Tulis PASS atau FAIL.
Jika FAIL: consecutivePasses >= (totalActive-1) && atLeastOneBid → return true (done).
```

### TC-08-07 | Minimal 1 bid: tidak boleh semua PASS
**Input:** Semua pemain PASS tanpa ada yang bid
**Expected:** Pemain terakhir dipaksa bid
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Semua pemain pilih PASS berturut-turut tanpa ada BID.
Apakah saat semua sudah PASS, pemain yang "terakhir tidak pass" dipaksa bid?
Apakah muncul pesan "[INFO] Kamu WAJIB bid minimal sekali!"?
Tulis PASS atau FAIL.
Jika FAIL: mustBid = (!atLeastOneBid && consecutivePasses == totalActive - 1).
```

### TC-08-08 | Pemenang bayar bid, properti berpindah
**Input:** P3 menang lelang dengan bid M150
**Expected:** P3 uang -= 150, properti → OWNED P3
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Selesaikan lelang dengan P3 menang bid M150. Catat uang P3 sebelum.
Apakah uang P3 berkurang M150 dan properti ter-OWNED P3?
Tulis PASS atau FAIL.
Jika FAIL: di finalizeAuction():
  bank.receivePayment(*highestBidder, highestBid);
  bank.transferPropertyToPlayer(auctionedProp, *highestBidder);
```

### TC-08-09 | Giliran lelang tidak mempengaruhi durasi festival
**Input:** Festival aktif (dur=2), lelang berlangsung 5 ronde
**Expected:** Festival dur tetap 2 setelah lelang selesai
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Aktifkan festival di suatu properti (dur=3). Jalankan proses lelang beberapa ronde.
Setelah lelang selesai, cek durasi festival (via CETAK_AKTA properti tersebut).
Apakah durasi festival tidak berubah selama lelang?
Tulis PASS atau FAIL.
Jika FAIL: pastikan EffectManager::onTurnEnd() tidak dipanggil selama ronde lelang.
```

### TC-08-10 | Pemain bangkrut dilewati dalam lelang
**Input:** P2 bangkrut, lelang dimulai
**Expected:** Urutan lelang melompati P2
**Status:** ⬜ BELUM DITEST

**AI Prompt:**
```
Buat P2 bangkrut. Jalankan lelang.
Apakah P2 tidak muncul dalam urutan giliran lelang?
Tulis PASS atau FAIL.
Jika FAIL: di AuctionManager::startAuction() dan buildAuctionOrder():
  allActive = engine.getActivePlayers() → hanya pemain tidak bankrupt.
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-08-01 | Lelang dipicu tolak beli | ⬜ |
| TC-08-02 | Urutan dari pemain berikutnya | ⬜ |
| TC-08-03 | BID valid | ⬜ |
| TC-08-04 | BID tidak lebih tinggi → error | ⬜ |
| TC-08-05 | BID melebihi uang → error | ⬜ |
| TC-08-06 | Selesai setelah (n-1) PASS | ⬜ |
| TC-08-07 | Minimal 1 bid | ⬜ |
| TC-08-08 | Pemenang bayar dan dapat properti | ⬜ |
| TC-08-09 | Tidak pengaruhi durasi festival | ⬜ |
| TC-08-10 | Pemain bangkrut dilewati | ⬜ |

**Total: 10 test cases**
