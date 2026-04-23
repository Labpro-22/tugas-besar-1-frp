# TC-01 | Inisialisasi & Konfigurasi

## Deskripsi
Menguji pembacaan file config, inisialisasi game baru, dan load game dari save file.

---

## Prompt AI — Jalankan test ini

```
Kamu adalah test runner untuk program Nimonspoli (C++ CLI).
Jalankan setiap test case di bawah secara berurutan.
Untuk setiap test, jalankan program dengan input yang diberikan,
amati output, lalu tulis hasil: ✅ PASS atau ❌ FAIL beserta alasan.
Jika FAIL, tulis juga perintah perbaikan yang harus dilakukan.
```

---

## Test Cases

### TC-01-01 | Baca property.txt — 28 properti ter-load
| Field | Value |
|-------|-------|
| **Precondition** | File `config/property.txt` ada dan valid |
| **Input** | Jalankan program → New Game → 2 pemain → `CETAK_AKTA GRT` |
| **Expected** | Akta Garut muncul: harga beli 60, gadai 40, sewa L0=2 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan: ./nimonspoli
Input sequence:
  NEW
  2
  Alice
  Bob
  CETAK_AKTA GRT

Apakah output menampilkan akta GARUT dengan harga beli M60, nilai gadai M40,
sewa unimproved M2? Tulis PASS atau FAIL.
Jika FAIL: periksa fungsi ConfigLoader::loadProperties(), pastikan kolom
HARGA_LAHAN, NILAI_GADAI, dan RENT_L0 dibaca dengan benar dari property.txt.
```

---

### TC-01-02 | Baca railroad.txt — tabel sewa ter-load
| Field | Value |
|-------|-------|
| **Precondition** | File `config/railroad.txt` ada dan valid |
| **Input** | `CETAK_AKTA GBR` setelah New Game |
| **Expected** | Akta Gambir menampilkan tabel sewa: 1 RR=25, 2 RR=50, 3 RR=100, 4 RR=200 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Setelah New Game, jalankan: CETAK_AKTA GBR
Apakah output menampilkan tabel sewa railroad dengan nilai 1→25, 2→50, 3→100, 4→200?
Tulis PASS atau FAIL.
Jika FAIL: periksa ConfigLoader::loadRailroadConfig(), pastikan map<int,int>
diisi dengan benar. Cek RailroadProperty constructor menerima rentByCount.
```

---

### TC-01-03 | Baca utility.txt — multiplier ter-load
| Field | Value |
|-------|-------|
| **Precondition** | File `config/utility.txt` ada |
| **Input** | `CETAK_AKTA PLN` setelah New Game |
| **Expected** | Akta PLN menampilkan multiplier: 1 utility=×4, 2 utility=×10 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Setelah New Game, jalankan: CETAK_AKTA PLN
Apakah output menampilkan multiplier utility 1→×4 dan 2→×10?
Tulis PASS atau FAIL.
Jika FAIL: periksa ConfigLoader::loadUtilityConfig() dan UtilityProperty constructor.
```

---

### TC-01-04 | Baca tax.txt — PPH_FLAT=150, PPH_PCT=10, PBM=200
| Field | Value |
|-------|-------|
| **Precondition** | File `config/tax.txt` ada |
| **Input** | New Game → set posisi ke PPH → lempar dadu → pilih opsi flat |
| **Expected** | Potongan pajak flat = M150 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Setelah New Game (2 pemain), set posisi pemain ke PPH (idx 4):
  ATUR_DADU sesuai untuk mendarat di PPH
  Saat prompt pilihan: input "1" (flat)
Apakah uang berkurang tepat M150?
Tulis PASS atau FAIL.
Jika FAIL: periksa ConfigLoader::loadTaxConfig() dan TaxTile::handlePPH(),
pastikan flatAmount dan percentage dibaca dari config, bukan hardcode.
```

---

### TC-01-05 | Baca special.txt — GO_SALARY=200, JAIL_FINE=50
| Field | Value |
|-------|-------|
| **Precondition** | File `config/special.txt` ada |
| **Input** | New Game → lewat GO → cek uang bertambah 200 |
| **Expected** | Uang pemain bertambah 200 saat lewat GO |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
New Game (2 pemain), catat uang awal Alice (M1000).
Atur posisi Alice di idx 38 (IKN), lalu ATUR_DADU 2 2 (total 4 → idx 2, lewat GO).
Apakah uang Alice bertambah M200 (menjadi M1200 sebelum aksi lain)?
Tulis PASS atau FAIL.
Jika FAIL: periksa GameEngine::awardPassGoSalary() dan GoTile::onLand(),
pastikan nilai dari config dipakai bukan hardcode.
```

---

### TC-01-06 | Baca misc.txt — MAX_TURN=15, SALDO_AWAL=1000
| Field | Value |
|-------|-------|
| **Precondition** | File `config/misc.txt` ada |
| **Input** | New Game → cek saldo awal pemain |
| **Expected** | Setiap pemain mulai dengan M1000 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan New Game dengan 4 pemain.
Setelah game mulai, amati status bar atau jalankan CETAK_PROPERTI.
Apakah saldo awal tiap pemain tepat M1000?
Tulis PASS atau FAIL.
Jika FAIL: periksa ConfigLoader::loadMiscConfig() dan GameEngine::startNewGame(),
pastikan initialBalance diambil dari miscConfig.getInitialBalance().
```

---

### TC-01-07 | File config tidak ada → gunakan nilai default
| Field | Value |
|-------|-------|
| **Precondition** | Hapus/rename satu file config |
| **Input** | Jalankan program tanpa `config/misc.txt` |
| **Expected** | Program tidak crash, pakai default: MAX_TURN=15, SALDO_AWAL=1500 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Rename config/misc.txt menjadi config/misc.txt.bak, lalu jalankan program.
Apakah program tetap jalan dan menggunakan nilai default (tidak crash)?
Tulis PASS atau FAIL.
Jika FAIL: tambahkan try-catch di ConfigLoader untuk setiap file config,
kembalikan nilai default jika file tidak ditemukan:
  MiscConfig default: MAX_TURN=15, SALDO_AWAL=1500
  SpecialConfig default: GO_SALARY=200, JAIL_FINE=50
  TaxConfig default: PPH_FLAT=150, PPH_PCT=10, PBM=200
```

---

### TC-01-08 | New Game — 2 pemain valid
| Field | Value |
|-------|-------|
| **Precondition** | Program baru |
| **Input** | NEW → 2 → Alice → Bob |
| **Expected** | 2 pemain dibuat, saldo M1000 masing-masing, urutan giliran diacak |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan: ./nimonspoli
Input: NEW, 2, Alice, Bob
Apakah program menampilkan konfirmasi 2 pemain berhasil dibuat
dan giliran pertama dimulai (salah satu dari Alice atau Bob)?
Tulis PASS atau FAIL.
Jika FAIL: periksa GameEngine::startNewGame(), pastikan loop pembuatan Player
berjalan nPlayers kali dan TurnManager::initializeOrder() dipanggil.
```

---

### TC-01-09 | New Game — 4 pemain valid
| Field | Value |
|-------|-------|
| **Precondition** | Program baru |
| **Input** | NEW → 4 → A → B → C → D |
| **Expected** | 4 pemain dibuat, urutan giliran diacak |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan: ./nimonspoli → NEW → 4 → A → B → C → D
Apakah 4 pemain berhasil dibuat dan giliran pertama dimulai?
Apakah urutan giliran tidak selalu A-B-C-D (diacak)?
Jalankan 3x dan catat urutan pertama tiap run.
Tulis PASS atau FAIL.
Jika FAIL (urutan selalu sama): periksa TurnManager::initializeOrder(),
pastikan std::shuffle dipakai dengan seed random_device.
```

---

### TC-01-10 | New Game — 1 pemain → error
| Field | Value |
|-------|-------|
| **Precondition** | Program baru |
| **Input** | NEW → 1 → Solo |
| **Expected** | GameException: jumlah pemain minimal 2 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan: ./nimonspoli → NEW → 1 → Solo
Apakah muncul pesan error "jumlah pemain harus 2 sampai 4" (atau serupa)?
Apakah program tidak crash dan kembali ke menu?
Tulis PASS atau FAIL.
Jika FAIL: tambahkan validasi di GameEngine::startNewGame():
  if (nPlayers < 2 || nPlayers > 4)
    throw GameException("Jumlah pemain harus 2 sampai 4.");
```

---

### TC-01-11 | New Game — 5 pemain → error
| Field | Value |
|-------|-------|
| **Precondition** | Program baru |
| **Input** | NEW → 5 → A B C D E |
| **Expected** | GameException: jumlah pemain maksimal 4 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan: ./nimonspoli → NEW → 5
Apakah muncul pesan error sebelum input nama?
Tulis PASS atau FAIL.
Jika FAIL: sama dengan TC-01-10, validasi nPlayers > 4.
```

---

### TC-01-12 | New Game — username kosong → default P1, P2
| Field | Value |
|-------|-------|
| **Precondition** | Program baru |
| **Input** | NEW → 2 → (enter kosong) → (enter kosong) |
| **Expected** | Pemain bernama P1 dan P2 |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Jalankan New Game 2 pemain, tekan Enter tanpa mengetik nama.
Apakah pemain dinamai P1 dan P2 secara default?
Tulis PASS atau FAIL.
Jika FAIL: di GameUI::bootstrapIfNeeded(), tambahkan:
  if (name.empty()) name = "P" + std::to_string(i + 1);
```

---

### TC-01-13 | Load Game — restore state lengkap
| Field | Value |
|-------|-------|
| **Precondition** | Ada file save `test_save.txt` yang valid |
| **Input** | LOAD → test_save.txt |
| **Expected** | Uang, posisi, properti, status pemain ter-restore |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Buat save file dulu: jalankan game, lakukan beberapa aksi, SIMPAN test_save.txt.
Lalu restart program → LOAD → test_save.txt.
Apakah semua state ter-restore: uang tiap pemain, posisi, properti yang dimiliki,
giliran aktif saat disimpan?
Tulis PASS atau FAIL.
Jika FAIL: periksa SaveLoadManager::loadInto() dan GameEngine::applySnapshot(),
pastikan semua field di SavedPlayerState dan SavedPropertyState di-restore.
```

---

### TC-01-14 | Load Game saat game berjalan → error
| Field | Value |
|-------|-------|
| **Precondition** | Game sudah berjalan |
| **Input** | MUAT test_save.txt (saat giliran berlangsung) |
| **Expected** | GameException: MUAT hanya sebelum game mulai |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Mulai game, lalu ketik: MUAT test_save.txt
Apakah muncul error "[ERROR] MUAT hanya tersedia sebelum permainan dimulai"?
Tulis PASS atau FAIL.
Jika FAIL: di GameEngine::processCommand() case LOAD, tambahkan:
  if (!players.empty()) throw GameException("MUAT hanya sebelum permainan dimulai.");
```

---

### TC-01-15 | Load Game — file tidak ada → error
| Field | Value |
|-------|-------|
| **Precondition** | Game belum mulai |
| **Input** | LOAD → tidak_ada.txt |
| **Expected** | Error: file tidak ditemukan |
| **Status** | ⬜ BELUM DITEST |

**AI Prompt:**
```
Di menu awal pilih LOAD, ketik nama file yang tidak ada: tidak_ada.txt
Apakah muncul pesan error file tidak ditemukan (bukan crash)?
Tulis PASS atau FAIL.
Jika FAIL: di SaveLoadManager::loadInto(), pastikan std::ifstream dicek:
  if (!file.is_open()) throw ConfigFileNotFoundException(filename);
```

---

## Ringkasan Hasil

| TC | Deskripsi | Status |
|----|-----------|--------|
| TC-01-01 | Baca property.txt | ⬜ |
| TC-01-02 | Baca railroad.txt | ⬜ |
| TC-01-03 | Baca utility.txt | ⬜ |
| TC-01-04 | Baca tax.txt | ⬜ |
| TC-01-05 | Baca special.txt | ⬜ |
| TC-01-06 | Baca misc.txt | ⬜ |
| TC-01-07 | Config tidak ada → default | ⬜ |
| TC-01-08 | New Game 2 pemain | ⬜ |
| TC-01-09 | New Game 4 pemain, urutan acak | ⬜ |
| TC-01-10 | New Game 1 pemain → error | ⬜ |
| TC-01-11 | New Game 5 pemain → error | ⬜ |
| TC-01-12 | Username kosong → default | ⬜ |
| TC-01-13 | Load Game restore state | ⬜ |
| TC-01-14 | Load saat game berjalan → error | ⬜ |
| TC-01-15 | Load file tidak ada → error | ⬜ |

**Total: 15 test cases**
