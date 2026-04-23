# TC-02 | Dadu & Pergerakan

## Deskripsi
Menguji mekanisme lempar dadu (acak & manual), double, triple double, dan pergerakan bidak di papan.

---

## Test Cases

### TC-02-01 | LEMPAR_DADU — hasil acak 2–12
| Field | Value |
|-------|-------|
| **Precondition** | Game berjalan, giliran pemain aktif |
| **Input** | `LEMPAR_DADU` |
| **Expected** | Output: `X + Y = Z` dengan X,Y ∈ [1,6] dan Z = X+Y |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan New Game 2 pemain. Pada giliran Alice, ketik: LEMPAR_DADU
Apakah output menampilkan format "X + Y = Z" dengan nilai valid (1-6 per dadu)?
Apakah bidak Alice berpindah Z petak dari posisi semula?
Tulis PASS atau FAIL.
Jika FAIL: periksa Dice::rollRandom() menggunakan std::uniform_int_distribution(1,6).
Periksa GameEngine::moveCurrentPlayer() menghitung posisi baru dengan benar.
```

---

### TC-02-02 | LEMPAR_DADU dua kali tanpa double → error
| Field | Value |
|-------|-------|
| **Precondition** | Sudah lempar dadu sekali, hasilnya bukan double |
| **Input** | `LEMPAR_DADU` kedua kali |
| **Expected** | `[ERROR] LEMPAR_DADU hanya boleh 1x per giliran` |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan LEMPAR_DADU, pastikan hasilnya bukan double.
Lalu jalankan LEMPAR_DADU lagi.
Apakah muncul error "LEMPAR_DADU hanya boleh 1x per giliran"?
Tulis PASS atau FAIL.
Jika FAIL: periksa flag diceRolledThisTurn di GameEngine.
Di case ROLL_DICE: if (diceRolledThisTurn && !extraRollAllowedThisTurn) throw GameException(...).
```

---

### TC-02-03 | ATUR_DADU — set manual valid
| Field | Value |
|-------|-------|
| **Precondition** | Giliran pemain aktif |
| **Input** | `ATUR_DADU 3 4` |
| **Expected** | Output: `3 + 4 = 7`, bidak maju 7 petak |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
New Game 2 pemain. Pada giliran Alice (posisi awal idx 0 / GO):
  ATUR_DADU 3 4
Apakah output menunjukkan "3 + 4 = 7" dan Alice berada di idx 7 (FES)?
Tulis PASS atau FAIL.
Jika FAIL: periksa Dice::setManual(d1, d2) dan pastikan getTotal() mengembalikan d1+d2.
```

---

### TC-02-04 | ATUR_DADU — nilai 0 → error
| Field | Value |
|-------|-------|
| **Precondition** | Giliran pemain aktif |
| **Input** | `ATUR_DADU 0 3` |
| **Expected** | `InvalidDiceValueException`: nilai dadu harus 1-6 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Ketik: ATUR_DADU 0 3
Apakah muncul error InvalidDiceValueException atau pesan "nilai dadu harus 1-6"?
Tulis PASS atau FAIL.
Jika FAIL: di Dice::setManual(), tambahkan:
  if (d1 < 1 || d1 > 6) throw InvalidDiceValueException(d1, 6);
  if (d2 < 1 || d2 > 6) throw InvalidDiceValueException(d2, 6);
```

---

### TC-02-05 | ATUR_DADU — nilai 7 → error
| Field | Value |
|-------|-------|
| **Precondition** | Giliran pemain aktif |
| **Input** | `ATUR_DADU 7 2` |
| **Expected** | `InvalidDiceValueException` |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Ketik: ATUR_DADU 7 2
Apakah muncul error yang sama seperti TC-02-04?
Tulis PASS atau FAIL.
Jika FAIL: sama dengan TC-02-04, validasi range [1,6].
```

---

### TC-02-06 | ATUR_DADU — hanya 1 argumen → error
| Field | Value |
|-------|-------|
| **Precondition** | Giliran pemain aktif |
| **Input** | `ATUR_DADU 3` |
| **Expected** | `GameException`: ATUR_DADU membutuhkan 2 argumen |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Ketik: ATUR_DADU 3
Apakah muncul error "ATUR_DADU membutuhkan 2 argumen angka"?
Tulis PASS atau FAIL.
Jika FAIL: di processCommand case SET_DICE:
  if (cmd.args.size() < 2) throw GameException("ATUR_DADU membutuhkan 2 argumen angka.");
```

---

### TC-02-07 | ATUR_DADU — argumen bukan angka → error
| Field | Value |
|-------|-------|
| **Precondition** | Giliran pemain aktif |
| **Input** | `ATUR_DADU abc xyz` |
| **Expected** | `GameException`: argumen harus berupa angka |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Ketik: ATUR_DADU abc xyz
Apakah muncul error "Argumen ATUR_DADU harus berupa angka bulat"?
Tulis PASS atau FAIL.
Jika FAIL: tambahkan try-catch di stoi() pada case SET_DICE.
```

---

### TC-02-08 | Double 1x → giliran tambahan
| Field | Value |
|-------|-------|
| **Precondition** | Giliran pemain aktif |
| **Input** | `ATUR_DADU 3 3` |
| **Expected** | Muncul event "Double — mendapat giliran tambahan", bisa LEMPAR_DADU lagi |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
New Game 2 pemain. Ketik: ATUR_DADU 3 3
Apakah muncul pesan giliran tambahan karena double?
Apakah bisa mengetik LEMPAR_DADU lagi tanpa error?
Tulis PASS atau FAIL.
Jika FAIL: periksa resolveDiceFlow() di processCommand — saat rolledDouble=true,
pastikan extraRollAllowedThisTurn=true dan tidak langsung memanggil executeTurn().
```

---

### TC-02-09 | Double 2x berturut → giliran tambahan lagi
| Field | Value |
|-------|-------|
| **Precondition** | Sudah double 1x (consecutiveDoubles=1) |
| **Input** | `ATUR_DADU 4 4` (double ke-2) |
| **Expected** | Mendapat giliran tambahan ke-2 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
ATUR_DADU 3 3 (double ke-1) → lalu ATUR_DADU 4 4 (double ke-2)
Apakah muncul pesan giliran tambahan ke-2?
Apakah consecutiveDoubles = 2 (bisa dicek via debug atau observasi)?
Tulis PASS atau FAIL.
Jika FAIL: periksa Player::incrementConsecutiveDoubles() dipanggil tiap double,
dan extraRollAllowedThisTurn di-set ulang setelah giliran tambahan.
```

---

### TC-02-10 | Triple double → masuk penjara
| Field | Value |
|-------|-------|
| **Precondition** | consecutiveDoubles = 2 |
| **Input** | `ATUR_DADU 5 5` (double ke-3) |
| **Expected** | Bidak pindah ke PEN (idx 10), status JAILED, giliran berakhir |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Setelah 2x double, ketik: ATUR_DADU 5 5
Apakah muncul pesan "Triple Double — masuk penjara"?
Apakah posisi pemain = idx 10 (PEN) dan status = JAILED?
Apakah giliran langsung berpindah ke pemain berikutnya?
Tulis PASS atau FAIL.
Jika FAIL: di resolveDiceFlow(), saat consecutiveDoubles >= 3:
  current.setPosition(board->getIndexOf("PEN"));
  current.setStatus(PlayerStatus::JAILED);
  current.resetConsecutiveDoubles();
  flowResult.append(executeTurn()); return;
```

---

### TC-02-11 | consecutiveDoubles di-reset awal giliran
| Field | Value |
|-------|-------|
| **Precondition** | Pemain double 1x, lalu AKHIRI_GILIRAN |
| **Input** | Giliran berikutnya dimulai |
| **Expected** | consecutiveDoubles = 0 pada awal giliran baru |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
ATUR_DADU 3 3 (double) → aksi → AKHIRI_GILIRAN → giliran P2 → AKHIRI_GILIRAN
→ kembali giliran P1 → ATUR_DADU 3 3 lagi
Apakah double ke-2 dihitung sebagai double pertama (bukan ke-3)?
Apakah tidak langsung masuk penjara?
Tulis PASS atau FAIL.
Jika FAIL: periksa EffectManager::onTurnStart() — pastikan
player.resetConsecutiveDoubles() dipanggil di awal setiap giliran baru.
```

---

### TC-02-12 | Lewat GO → dapat gaji
| Field | Value |
|-------|-------|
| **Precondition** | Alice di idx 38 (IKN) |
| **Input** | `ATUR_DADU 2 2` (total 4, posisi baru idx 2 = DNU, lewat GO) |
| **Expected** | Event "Lewat GO — terima M200", uang Alice += 200 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Set Alice ke posisi IKN (idx 39): gunakan ATUR_DADU hingga Alice di idx 39.
Lalu: ATUR_DADU 2 2
Apakah muncul event "Lewat GO" dan uang Alice bertambah M200?
Apakah posisi Alice = idx 3 (DNU)?
Tulis PASS atau FAIL.
Jika FAIL: di GameEngine::moveCurrentPlayer(), pastikan:
  if (player.getPosition() < oldPos && !landedOnGoToJail)
    awardPassGoSalary(player);
```

---

### TC-02-13 | PPJ → tidak dapat gaji lewat GO
| Field | Value |
|-------|-------|
| **Precondition** | Alice dekat PPJ, sehingga dengan dadu tertentu mendarat di PPJ (idx 30) dan seolah melewati GO |
| **Input** | ATUR_DADU ke PPJ |
| **Expected** | Status JAILED, TIDAK dapat gaji GO |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Posisikan Alice di idx 28 (PKB), lalu ATUR_DADU 1 1 (total 2 → idx 30 = PPJ).
Apakah Alice tidak mendapat gaji M200 meski "melewati" GO dalam hitungan posisi?
Apakah status Alice = JAILED dan posisi = idx 10 (PEN)?
Tulis PASS atau FAIL.
Jika FAIL: di moveCurrentPlayer(), tambahkan flag:
  const bool landedOnGoToJail = (landing.getCode() == "PPJ");
  if (crossedGo && !landedOnGoToJail) awardPassGoSalary(player);
```

---

### TC-02-14 | Posisi wrap around papan
| Field | Value |
|-------|-------|
| **Precondition** | Alice di idx 38 |
| **Input** | `ATUR_DADU 3 3` (total 6, new idx = (38+6)%40 = 4) |
| **Expected** | Alice di idx 4 = PPH |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Posisikan Alice di IKN (idx 39), lalu ATUR_DADU 2 3 (total 5 → (39+5)%40 = 4).
Apakah Alice mendarat di PPH (idx 4)?
Tulis PASS atau FAIL.
Jika FAIL: periksa Player::move() menggunakan: position = (position + steps + boardSize) % boardSize;
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-02-01 | LEMPAR_DADU acak valid | ⬜ |
| TC-02-02 | Lempar 2x tanpa double → error | ⬜ |
| TC-02-03 | ATUR_DADU valid | ⬜ |
| TC-02-04 | Nilai dadu 0 → error | ⬜ |
| TC-02-05 | Nilai dadu 7 → error | ⬜ |
| TC-02-06 | 1 argumen → error | ⬜ |
| TC-02-07 | Argumen non-angka → error | ⬜ |
| TC-02-08 | Double 1x → giliran tambahan | ⬜ |
| TC-02-09 | Double 2x → giliran tambahan | ⬜ |
| TC-02-10 | Triple double → penjara | ⬜ |
| TC-02-11 | Reset consecutiveDoubles | ⬜ |
| TC-02-12 | Lewat GO → gaji | ⬜ |
| TC-02-13 | PPJ tidak dapat gaji GO | ⬜ |
| TC-02-14 | Wrap around papan | ⬜ |

**Total: 14 test cases**
