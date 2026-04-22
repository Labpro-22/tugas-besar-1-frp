# Nimonspoli — GitHub Copilot Context Prompt

> **Cara pakai:** Paste seluruh file ini sebagai konteks ke GitHub Copilot Chat
> (`Ctrl+I` → `#file:NIMONSPOLI_COPILOT_PROMPT.md`) sebelum meminta Copilot
> menulis, melengkapi, atau mereview kode.  Copilot akan menggunakan dokumen
> ini sebagai sumber kebenaran tunggal untuk semua logika, command, constraint,
> dan konvensi arsitektur proyek.

---

## 1. Gambaran Umum Proyek

Nimonspoli adalah permainan papan berbasis teks (CLI) yang ditulis dalam **C++17**
dan dijalankan di **Linux / WSL**.  Arsitekturnya mengikuti **Layered Architecture**
tiga lapis:

| Lapis | Kelas Utama | Tanggung Jawab |
|---|---|---|
| **UI Layer** | `GameUI`, `CommandParser`, `BoardRenderer`, `UiFormatter`, `AnsiTheme` | Semua I/O terminal — membaca input, menampilkan output. **Tidak boleh ada logika game di sini.** |
| **Core Layer** | `GameEngine`, `TurnManager`, `Dice`, `PropertyManager`, `AuctionManager`, `BankruptcyManager`, `CardManager`, `EffectManager`, `TransactionLogger` | Semua aturan permainan, kalkulasi, dan manajemen state. |
| **Data Access Layer** | `ConfigLoader`, `SaveLoadManager`, `GameStateSerializer` | Membaca file config dan file save/load. |

**Prinsip desain yang wajib diikuti Copilot saat membuat kode baru:**
- Single Responsibility — satu kelas satu tanggung jawab.
- Liskov Substitution — child class harus bisa menggantikan parent.
- Tidak ada `struct` — gunakan `class` dengan `private` atribut + getter/setter.
- Tidak ada God Class, tidak ada paradigma prosedural yang dibungkus kelas.
- Tidak ada `using namespace std` di header.
- Tidak ada library selain STL dan pustaka standar C++.
- Tidak ada `dynamic_cast` berlebihan; gunakan polimorfisme virtual.
- Nama variabel, method, dan kelas harus deskriptif (bukan `a`, `b`, `cnt`).
- Setiap method tidak boleh terlalu panjang — pecah ke helper jika perlu.

---

## 2. Papan Permainan

Papan terdiri dari **40 petak** membentuk jalur melingkar (searah jarum jam).
Indeks petak dimulai dari 0 (GO) hingga 39 (IKN).

### 2.1 Daftar Petak

| Idx | Kode | Nama | Tipe | Warna/Kategori |
|-----|------|------|------|----------------|
| 0 | GO | Petak Mulai | Spesial | — |
| 1 | GRT | Garut | Street | Coklat (CK) |
| 2 | DNU | Dana Umum | Kartu | — |
| 3 | TSK | Tasikmalaya | Street | Coklat (CK) |
| 4 | PPH | Pajak Penghasilan | Pajak | — |
| 5 | GBR | Stasiun Gambir | Railroad | — |
| 6 | BGR | Bogor | Street | Biru Muda (BM) |
| 7 | FES | Festival | Festival | — |
| 8 | DPK | Depok | Street | Biru Muda (BM) |
| 9 | BKS | Bekasi | Street | Biru Muda (BM) |
| 10 | PEN | Penjara | Spesial | — |
| 11 | MGL | Magelang | Street | Pink (PK) |
| 12 | PLN | PLN | Utility | Abu-abu (AB) |
| 13 | SOL | Solo | Street | Pink (PK) |
| 14 | YOG | Yogyakarta | Street | Pink (PK) |
| 15 | STB | Stasiun Bandung | Railroad | — |
| 16 | MAL | Malang | Street | Orange (OR) |
| 17 | DNU | Dana Umum | Kartu | — |
| 18 | SMG | Semarang | Street | Orange (OR) |
| 19 | SBY | Surabaya | Street | Orange (OR) |
| 20 | BBP | Bebas Parkir | Spesial | — |
| 21 | MKS | Makassar | Street | Merah (MR) |
| 22 | KSP | Kesempatan | Kartu | — |
| 23 | BLP | Balikpapan | Street | Merah (MR) |
| 24 | MND | Manado | Street | Merah (MR) |
| 25 | TUG | Stasiun Tugu | Railroad | — |
| 26 | PLB | Palembang | Street | Kuning (KN) |
| 27 | PKB | Pekanbaru | Street | Kuning (KN) |
| 28 | PAM | PAM | Utility | Abu-abu (AB) |
| 29 | MED | Medan | Street | Kuning (KN) |
| 30 | PPJ | Pergi ke Penjara | Spesial | — |
| 31 | BDG | Bandung | Street | Hijau (HJ) |
| 32 | DEN | Denpasar | Street | Hijau (HJ) |
| 33 | FES | Festival | Festival | — |
| 34 | MTR | Mataram | Street | Hijau (HJ) |
| 35 | GUB | Stasiun Gubeng | Railroad | — |
| 36 | KSP | Kesempatan | Kartu | — |
| 37 | JKT | Jakarta | Street | Biru Tua (BT) |
| 38 | PBM | Pajak Barang Mewah | Pajak | — |
| 39 | IKN | Ibu Kota Nusantara | Street | Biru Tua (BT) |

### 2.2 Color Group Street

| Kode | Warna | Anggota |
|------|-------|---------|
| CK | Coklat | GRT, TSK |
| BM | Biru Muda | BGR, DPK, BKS |
| PK | Pink | MGL, SOL, YOG |
| OR | Orange | MAL, SMG, SBY |
| MR | Merah | MKS, BLP, MND |
| KN | Kuning | PLB, PKB, MED |
| HJ | Hijau | BDG, DEN, MTR |
| BT | Biru Tua | JKT, IKN |

---

## 3. Aturan Logika Permainan

### 3.1 Alur Program Utama

1. Program membaca file konfigurasi dari `config/`.
2. Menu awal: pilih **New Game** atau **Load Game**.
   - New Game: input jumlah pemain (2–4), lalu username tiap pemain. Urutan giliran diacak.
   - Load Game: hanya tersedia sebelum permainan dimulai; memuat state dari file `.txt`.
3. Satu **turn** = semua pemain bermain satu kali (Pemain 1 → 2 → … → N).
4. Pada setiap giliran seorang pemain, ia dapat menjalankan perintah dari daftar di Seksi 4.
5. Permainan berakhir sesuai kondisi di Seksi 3.10.

### 3.2 Dadu

- Dua dadu bersisi 6 (2d6). Bisa `LEMPAR_DADU` (acak) atau `ATUR_DADU X Y` (manual).
- Pemain maju searah jarum jam sebanyak total angka kedua dadu.
- **Double** (kedua dadu sama): pemain mendapat giliran tambahan setelah menyelesaikan aksi.
- **Triple double** (double tiga kali berturut-turut dalam satu giliran): pemain langsung masuk penjara, giliran berakhir, bidak tidak digerakkan berdasarkan dadu ketiga.
- `consecutiveDoubles` di-reset setiap awal giliran.

### 3.3 Petak Spesial

| Petak | Mekanisme |
|-------|-----------|
| **GO (idx 0)** | Berhenti tepat di GO **atau melewatinya** → pemain menerima gaji GO (dari config). Pengecualian: pemain yang dikirim ke penjara via PPJ tidak mendapat gaji meski melewati GO. |
| **Penjara (idx 10) — Hanya Mampir** | Tiba dari lemparan dadu normal → tidak ada penalti. |
| **Penjara (idx 10) — Dalam Penjara** | Status `JAILED`. Pilihan keluar: (a) bayar denda sebelum lempar dadu, (b) pakai kartu Bebas Penjara, (c) lempar double (maks 3 percobaan). Pada giliran ke-4, **wajib** bayar denda. |
| **Bebas Parkir (idx 20)** | Tidak ada aksi. |
| **Pergi ke Penjara (idx 30)** | Bidak pindah ke idx 10, status `JAILED`, giliran langsung berakhir, tidak dapat gaji GO. |

### 3.4 Properti — Status

| Status | Keterangan |
|--------|-----------|
| `BANK` | Belum dimiliki siapapun. |
| `OWNED` | Dimiliki pemain; sewa berlaku. |
| `MORTGAGED` | Dimiliki pemain tapi digadai; sewa **tidak** berlaku. |

### 3.5 Kepemilikan Properti

- **Street (BANK)**: tampilkan akta, tawarkan beli. Jika pemain tolak atau tidak mampu → lelang otomatis.
- **Railroad (BANK)**: pemain pertama yang mendarat langsung memiliki secara otomatis — **tanpa beli, tanpa lelang**.
- **Utility (BANK)**: sama dengan Railroad — otomatis milik pemain pertama yang mendarat.

### 3.6 Sewa

| Jenis | Perhitungan |
|-------|-------------|
| **Street — tidak monopoli** | Sewa dasar (level 0 dari config). |
| **Street — monopoli, 0 bangunan** | 2× sewa dasar. |
| **Street — ada bangunan** | Sewa sesuai tabel konfigurasi (level 1–5). |
| **Street — festival aktif** | Sewa terkini × `festivalMultiplier`. |
| **Railroad** | Sesuai tabel config berdasarkan jumlah Railroad yang dimiliki pemilik (1→M25, 2→M50, 3→M100, 4→M200). |
| **Utility** | `diceTotal × multiplier` (1 utility→×4, 2 utility→×10). |
| **MORTGAGED** | Tidak ada sewa. |

Jika pemain tidak mampu bayar sewa → masuk alur kebangkrutan (Seksi 3.9).

### 3.7 Peningkatan Properti (BANGUN)

- Hanya untuk **Street** yang telah **dimonopoli** (semua Street satu color group milik pemain).
- Urutan: rumah level 1 → 2 → 3 → 4 → hotel (level 5).
- Maksimum **4 rumah** per petak, lalu upgrade ke **1 hotel**.
- Hotel adalah tingkat tertinggi; tidak bisa dibangun lagi.
- **Pemerataan wajib**: selisih jumlah rumah antar petak dalam satu color group ≤ 1 setiap saat.
- Stok bangunan tidak terbatas.

### 3.8 Gadai & Tebus

- **Gadai**: properti harus `OWNED` dan **tidak ada bangunan** di color group yang sama.
  - Jika masih ada bangunan di color group → semua bangunan di color group itu harus dijual ke Bank (½ harga beli) terlebih dahulu, bisa dalam satu rangkaian aksi `GADAI`.
  - Pemain menerima uang sebesar `mortgageValue` dari config; status → `MORTGAGED`.
- **Tebus**: bayar `purchasePrice` penuh ke Bank; status kembali → `OWNED`.
- Properti `MORTGAGED` tidak bisa langsung dijual ke Bank; harus ditebus dulu.

### 3.9 Lelang

1. Dipicu saat: (a) pemain menolak beli Street, (b) pemain tidak mampu beli, (c) semua properti pemain bangkrut-ke-Bank dilelang satu per satu.
2. Urutan lelang mulai dari pemain **setelah** pemicu lelang, sesuai urutan giliran.
3. Setiap giliran lelang: `PASS` atau `BID <jumlah>`.
4. Bid pertama minimal M0; setiap bid berikutnya **harus lebih tinggi** dari bid sebelumnya.
5. Lelang berakhir jika terjadi `PASS` berturut-turut sebanyak **(jumlah pemain aktif − 1)**.
6. **Minimal satu pemain harus bid** — jika semua pass tanpa ada bid, pemain terakhir yang tidak melakukan pass **wajib** bid.
7. Pemain tidak bisa bid melebihi uang yang dimiliki.
8. Giliran lelang **tidak** dihitung sebagai giliran permainan dan tidak mempengaruhi durasi efek apapun.

### 3.10 Pajak

| Petak | Aturan |
|-------|--------|
| **PPH (idx 4)** | Pemain pilih **sebelum menghitung kekayaan**: (a) bayar flat dari config, atau (b) bayar X% dari total kekayaan. Total kekayaan = uang tunai + harga beli semua properti (termasuk yang digadai) + harga beli semua bangunan. Jika pilih flat tapi tidak mampu → langsung bangkrut. |
| **PBM (idx 38)** | Bayar nilai flat dari config langsung ke Bank, tanpa pilihan. |

Jika tidak mampu bayar pajak → bangkrut dengan kreditor Bank.

### 3.11 Festival

- Mendarat di FES → pilih satu properti milik sendiri.
- Sewa properti itu ×2 selama **3 giliran pemain tersebut**.
- Memilih properti yang **sudah kena festival**: multiplier ×2 lagi (maks 8×), durasi di-reset ke 3.
- Urutan maksimum: 1× → 2× → 4× → 8× (tiga kali penggandaan; setelah itu hanya reset durasi).
- Durasi berkurang 1 setiap akhir giliran pemilik properti.

### 3.12 Kartu Kesempatan & Dana Umum

Mendarat di petak KSP/DNU → ambil kartu teratas, jalankan efek, kartu dikocok kembali.

**Kartu Kesempatan:**
1. Pergi ke stasiun terdekat.
2. Mundur 3 petak.
3. Masuk Penjara.

**Kartu Dana Umum:**
1. Hari ulang tahun — terima M100 dari setiap pemain lain.
2. Biaya dokter — bayar M700 ke Bank.
3. Mau nyaleg — bayar M200 ke setiap pemain lain.

### 3.13 Kartu Kemampuan Spesial

**Aturan umum:**
- Maks **1 kartu per giliran**, hanya boleh dipakai **sebelum melempar dadu**.
- Setiap awal giliran, semua pemain mendapat 1 kartu acak.
- Maks **3 kartu di tangan**. Jika dapat kartu ke-4 → wajib buang 1 (pilih sendiri).
- Setelah dipakai → masuk discard pile. Jika deck habis → kocok ulang discard.

**Jenis kartu:**

| Kartu | Jumlah | Efek |
|-------|--------|------|
| `MoveCard` | 4 | Maju sejumlah langkah (acak saat diterima). |
| `DiscountCard` | 3 | Diskon % acak saat diterima; berlaku **1 giliran**. |
| `ShieldCard` | 2 | Kebal sewa dan sanksi selama **1 giliran**. |
| `TeleportCard` | 2 | Pindah ke petak mana pun di papan. |
| `LassoCard` | 2 | Tarik satu pemain lawan yang ada di depan ke posisi kita. |
| `DemolitionCard` | 2 | Hancurkan bangunan di properti lawan yang dipilih. |

### 3.14 Kebangkrutan

**Urutan penanganan:**
1. Pemain tidak mampu bayar kewajiban (sewa/pajak/efek kartu).
2. Sistem hitung `maxLiquidation` = uang tunai + nilai jual semua properti yang belum digadai.
3. Jika `maxLiquidation >= kewajiban` → pemain **wajib** likuidasi:
   - Opsi: jual properti ke Bank (harga beli + ½ harga bangunan) **atau** gadai properti.
4. Jika setelah likuidasi penuh masih tidak cukup → **BANGKRUT**.

**Pengambilalihan aset:**
- Bangkrut ke pemain lain: semua uang + properti (termasuk yang digadai) langsung ke kreditor.
- Bangkrut ke Bank: uang hilang, semua properti kembali status `BANK`, bangunan hancur, lalu semua properti itu dilelang satu per satu.

### 3.15 Kondisi Game Selesai

| Kondisi | Pemicu | Pemenang |
|---------|--------|----------|
| **Bankruptcy** | Hanya 1 pemain aktif tersisa | Pemain yang tersisa. |
| **Max Turn** | Semua pemain mencapai batas turn (dari config) | Kekayaan terbanyak → jika seri: properti terbanyak → jika seri: kartu terbanyak → jika seri: semua yang seri menang. |
| Mode **Bankruptcy Only** | `MAX_TURN < 1` di config | Terus sampai 1 pemain tersisa. |

---

## 4. Daftar Command CLI

> Semua command di-parse oleh `CommandParser` → diteruskan ke `GameEngine::processCommand()` → hasilnya dikembalikan sebagai `CommandResult` → ditampilkan oleh `UiFormatter`.
> **UI tidak boleh memanggil logika game secara langsung.**

### 4.1 Command Input Pemain (bisa dipanggil kapan saja selama giliran)

| Command | Argumen | Keterangan |
|---------|---------|-----------|
| `LEMPAR_DADU` | — | Lempar dadu acak. **Hanya boleh 1× per giliran** (kecuali dapat double). |
| `ATUR_DADU X Y` | `X`, `Y` ∈ [1,6] | Set nilai dadu secara manual. Lanjutkan seperti `LEMPAR_DADU`. |
| `CETAK_PAPAN` | — | Render papan 11×11 ASCII berwarna ANSI ke terminal. |
| `CETAK_AKTA [KODE]` | Opsional kode properti | Tampilkan akta properti. Jika tidak ada argumen, tampilkan akta petak saat ini (harus berupa properti). |
| `CETAK_PROPERTI` | — | Tampilkan semua properti milik pemain aktif beserta statusnya dan total kekayaan properti. |
| `GADAI KODE` | Kode properti | Gadaikan properti milik sendiri. Jika ada bangunan di color group, program tawarkan jual bangunan dulu. |
| `TEBUS KODE` | Kode properti | Tebus properti yang sedang digadai; bayar harga beli penuh. |
| `BANGUN KODE` | Kode properti Street | Bangun 1 rumah/upgrade ke hotel di properti tersebut (syarat monopoli + pemerataan). |
| `GUNAKAN_KEMAMPUAN IDX [TARGET]` | `IDX` = nomor kartu (1-based); `TARGET` = kode petak / username (untuk TeleportCard, LassoCard, DemolitionCard) | Gunakan kartu kemampuan spesial. Hanya sebelum lempar dadu. |
| `SIMPAN [FILE]` | Nama file `.txt` (default: `file_save.txt`) | Simpan state ke file. **Hanya di awal giliran sebelum aksi apapun.** |
| `CETAK_LOG [N]` | Opsional N = jumlah entri terakhir | Tampilkan log transaksi. Tanpa argumen = semua. |
| `AKHIRI_GILIRAN` | — | Akhiri giliran saat ini, pindah ke pemain berikutnya. |
| `BANTUAN` | — | Tampilkan daftar command yang tersedia. |
| `KELUAR` | — | Tutup program. |

### 4.2 Command Otomatis (dipicu oleh engine, tidak diketik pemain)

| Trigger | Aksi Otomatis |
|---------|---------------|
| Mendarat di Street `BANK` | `BELI` — tampilkan akta, tanya y/n. Jika tidak → `LELANG`. |
| Mendarat di Railroad/Utility `BANK` | Kepemilikan langsung pindah ke pemain tanpa input. |
| Mendarat di properti `OWNED` milik orang lain | `BAYAR_SEWA` otomatis (dengan ShieldCard aktif → skip). |
| Mendarat di PPH/PBM | `BAYAR_PAJAK` — PPH tanya pilihan flat/%, PBM langsung potong. |
| Mendarat di FES | `FESTIVAL` — pilih properti untuk digandakan sewanya. |
| Mendarat di KSP/DNU | Ambil kartu, jalankan efek. |
| Mendarat di PPJ | Pindah ke PEN, status JAILED, giliran berakhir. |
| Mendarat tepat di GO | Terima gaji GO. |
| Melewati GO | Terima gaji GO (kecuali dari PPJ). |
| Tidak mampu bayar | `BANGKRUT` — panel likuidasi, lalu pengambilalihan aset. |
| Kondisi menang terpenuhi | `MENANG` — tampilkan pemenang dan ringkasan akhir. |
| Dapat kartu ke-4 | `DROP_KARTU` — pemain pilih kartu mana yang dibuang. |

### 4.3 Command yang Hanya Tersedia di Menu Awal

| Command | Keterangan |
|---------|-----------|
| `MUAT [FILE]` | Muat state dari file `.txt`. Hanya sebelum permainan dimulai. |

---

## 5. Constraints & Validasi

> Copilot harus memastikan setiap method yang mengimplementasikan logika di bawah ini **melempar `GameException`** (atau subclass-nya) dengan pesan yang informatif jika constraint dilanggar.

### 5.1 Constraint Pemain

| Constraint | Penanganan |
|------------|-----------|
| Jumlah pemain: 2–4 | Throw `GameException` jika di luar rentang. |
| Username tidak boleh kosong | Gunakan default "P1", "P2", dst. jika kosong. |
| Maks 3 kartu di tangan | Jika dapat kartu ke-4, simpan di `pendingSkillDraw`; UI minta pemain buang 1. |
| Skill card hanya 1× per giliran | Throw `GameException("Skill card hanya boleh dipakai sekali per giliran.")` |
| Skill card hanya sebelum lempar dadu | Engine cek flag `diceRolledThisTurn`; jika sudah lempar → throw. |

### 5.2 Constraint Dadu

| Constraint | Penanganan |
|------------|-----------|
| Nilai dadu: 1–6 | Throw `InvalidDiceValueException(value, faces)` |
| `ATUR_DADU` butuh tepat 2 argumen numerik | Throw `GameException("ATUR_DADU membutuhkan 2 argumen angka.")` |
| `LEMPAR_DADU` hanya 1× per giliran (kecuali double) | Engine cek flag; throw jika sudah lempar tanpa double. |

### 5.3 Constraint Properti

| Constraint | Penanganan |
|------------|-----------|
| `GADAI`: pemain harus pemilik | Throw `NotOwnerException(username, propCode)` |
| `GADAI`: properti harus `OWNED` | Throw `AlreadyMortgagedException` jika sudah digadai. |
| `GADAI`: tidak boleh ada bangunan di color group | Throw `HasBuildingsException(propCode)` jika ada; tawarkan jual bangunan dulu. |
| `TEBUS`: properti harus `MORTGAGED` | Throw `NotMortgagedException(propCode)` |
| `TEBUS`: pemain harus cukup uang | Throw `InsufficientFundsException` |
| `BANGUN`: hanya Street | Throw `BuildNotAllowedException(propCode)` untuk Railroad/Utility. |
| `BANGUN`: harus monopoli color group | Throw `GameException` dengan pesan jelas. |
| `BANGUN`: harus merata (selisih ≤ 1) | Hanya tampilkan petak yang memenuhi syarat pemerataan. |
| `BANGUN`: hotel sudah max | Throw `MaxBuildingLevelException(propCode)` |

### 5.4 Constraint Save/Load

| Constraint | Penanganan |
|------------|-----------|
| `SIMPAN` hanya di awal giliran | Throw `GameException` jika dadu sudah dilempar giliran ini. |
| `MUAT` hanya sebelum permainan dimulai | Throw `GameException` jika game sudah berjalan. |
| Ekstensi file harus `.txt` | Throw `SaveLoadException` jika bukan `.txt`. |
| File tidak ditemukan | Throw `ConfigFileNotFoundException(filename)` |
| Format file tidak valid | Throw `ConfigParseException` atau `SaveLoadException` dengan lokasi error. |
| Jumlah pemain di file: 2–4 | Validasi saat deserialize; throw jika di luar rentang. |
| `FMULT` harus 1/2/4/8 | Throw `SaveLoadException` saat parse. |
| Status pemain harus ACTIVE/JAILED/BANKRUPT | Throw `SaveLoadException` saat parse. |

### 5.5 Constraint Arsitektur

| Constraint |
|------------|
| **Tidak boleh ada `struct`** — gunakan `class` dengan atribut `private`. |
| **Tidak boleh ada `using namespace std`** di file `.hpp`. |
| **UI Layer tidak boleh memanggil logika game** — hanya boleh memanggil `engine.processCommand()` dan membaca `CommandResult`. |
| **Tidak boleh ada `dynamic_cast` berlebihan** — gunakan virtual dispatch. `dynamic_cast` hanya diizinkan untuk kasus yang benar-benar diperlukan (misal: `StreetProperty*` dari `Property*` saat akses fitur eksklusif street). |
| **Tidak ada global variable** — semua state ada di dalam object. |
| **Tidak ada magic number** — semua konstanta dari config atau named constant. |
| Kompilasi dengan `g++ -std=c++17 -Wall -Wextra` **tanpa warning**. |
| Jalankan di **Linux / WSL** tanpa modifikasi. |

---

## 6. Format File Save

```
<TURN_SAAT_INI> <MAX_TURN>
<JUMLAH_PEMAIN>
<USERNAME> <UANG> <POSISI_PETAK> <STATUS>
<JUMLAH_KARTU_TANGAN>
<JENIS_KARTU_1> [<NILAI_KARTU_1>] [<SISA_DURASI_1>]
<JENIS_KARTU_2> [<NILAI_KARTU_2>] [<SISA_DURASI_2>]
... (ulangi untuk setiap pemain)
<URUTAN_GILIRAN_1> <URUTAN_GILIRAN_2> ... <URUTAN_GILIRAN_N>
<GILIRAN_AKTIF_SAAT_INI>
<JUMLAH_PROPERTI>
<KODE> <JENIS> <PEMILIK> <STATUS> <FMULT> <FDUR> <N_BANGUNAN>
... (satu baris per properti)
<JUMLAH_KARTU_DECK>
<JENIS_KARTU>
... (satu baris per kartu)
<JUMLAH_ENTRI_LOG>
<TURN> <USERNAME> <JENIS_AKSI> <DETAIL>
... (satu baris per entri)
```

**Keterangan format:**
- `STATUS` pemain: `ACTIVE`, `JAILED`, atau `BANKRUPT`.
- `JENIS_KARTU`: `MoveCard`, `DiscountCard`, `ShieldCard`, `TeleportCard`, `LassoCard`, `DemolitionCard`.
- `NILAI_KARTU` dan `SISA_DURASI` opsional — hilangkan token jika tidak ada nilai.
- `JENIS` properti: `street`, `railroad`, `utility`.
- `STATUS` properti: `BANK`, `OWNED`, `MORTGAGED`.
- `FMULT`: 1, 2, 4, atau 8.
- `FDUR`: 0–3.
- `N_BANGUNAN`: `0`–`4` untuk rumah, `H` untuk hotel.

---

## 7. Koneksi UI → Core Layer

> Bagian ini menjelaskan bagaimana Copilot harus menghubungkan kode UI ke Core.

### 7.1 Alur Data Satu Perintah

```
Pemain mengetik string
        │
        ▼
CommandParser::parse(raw)
  → menghasilkan Command { type, args, raw }
        │
        ▼
GameEngine::processCommand(cmd)
  → dispatch ke handler sesuai CommandType
  → memanggil manager (PropertyManager, CardManager, dst.)
  → mencatat ke TransactionLogger
  → mengembalikan CommandResult { commandName, events, success }
        │
        ▼
UiFormatter::format(result)
  → mengubah events menjadi string berwarna ANSI
  → menampilkan ke std::cout
        │  (khusus CETAK_PAPAN)
        ▼
BoardRenderer::render(board, players, turn, maxTurn)
  → menampilkan papan 11×11 ke std::cout
```

### 7.2 Konvensi `CommandResult`

```cpp
// Setiap handler di processCommand() harus:
CommandResult result;
result.commandName = "NAMA_PERINTAH";   // wajib diisi

// Tambahkan event untuk setiap informasi yang perlu ditampilkan:
result.addEvent(GameEventType::SYSTEM,   UiTone::INFO,    "Judul", "Pesan...");
result.addEvent(GameEventType::MONEY,    UiTone::SUCCESS, "Judul", "Pesan...");
result.addEvent(GameEventType::PROPERTY, UiTone::WARNING, "Judul", "Pesan...");
result.addEvent(GameEventType::DICE,     UiTone::ERROR,   "Judul", "Pesan...");

result.success = true;  // false jika aksi gagal (bukan exception)
return result;
```

### 7.3 Aturan UI yang Wajib Diikuti Copilot

- `GameUI::run()` hanya boleh memanggil `parser.parse()`, `engine.processCommand()`, dan `renderer.render()`.
- Semua teks output harus melalui `UiFormatter::format()` — tidak boleh `std::cout` langsung di `GameUI` kecuali untuk prompt input.
- `BoardRenderer` hanya boleh membaca dari `Board` dan `vector<Player*>` — tidak boleh memiliki state sendiri.
- Warna ANSI harus menggunakan `AnsiTheme::apply(color, text)` — tidak boleh hardcode escape code di tempat lain.
- Jika terminal tidak mendukung warna (`NO_COLOR` env var di-set), `AnsiTheme` harus mengembalikan string kosong untuk kode warna.

---

## 8. Konfigurasi Default (jika file config tidak ditemukan)

| Parameter | Default |
|-----------|---------|
| `MAX_TURN` | 15 |
| `SALDO_AWAL` | 1500 |
| `GO_SALARY` | 200 |
| `JAIL_FINE` | 50 |
| `PPH_FLAT` | 150 |
| `PPH_PERSENTASE` | 10 |
| `PBM_FLAT` | 200 |

---

## 9. Hierarki Kelas Penting

```
Tile (abstract)
├── PropertyTile          → has-a Property (shared_ptr)
├── GoTile
├── JailTile
├── GoToJailTile          → uses JailTile::sendToJail()
├── TaxTile               → type: PPH atau PBM
├── FestivalTile
├── CardTile              → type: CHANCE atau COMMUNITY
└── FreeParkingTile

Property (abstract)
├── StreetProperty        → buildingLevel, colorGroup
├── RailroadProperty
└── UtilityProperty

Card (abstract)
├── ActionCard (abstract)
│   ├── GoToNearestRailroadCard
│   ├── MoveBackCard
│   ├── GoToJailCard
│   ├── BirthdayCard
│   ├── DoctorFeeCard
│   └── ElectionCard
└── SkillCard (abstract)
    ├── MoveCard
    ├── DiscountCard
    ├── ShieldCard
    ├── TeleportCard
    ├── LassoCard
    └── DemolitionCard
```

---

## 10. Instruksi Khusus untuk GitHub Copilot

Saat diminta menulis atau melengkapi kode untuk proyek ini, **Copilot harus**:

1. **Selalu merujuk ke dokumen ini** sebagai sumber aturan. Jika ada keraguan tentang logika permainan, ikuti Seksi 3. Jika ada keraguan tentang command, ikuti Seksi 4.

2. **Tidak pernah membuat `struct`** — gunakan `class` dengan atribut `private` dan getter/setter yang konsisten.

3. **Selalu melempar exception yang tepat** (Seksi 5) — jangan return `false` diam-diam tanpa informasi.

4. **Mengikuti pola `CommandResult`** (Seksi 7.2) untuk semua handler baru di `processCommand()`.

5. **Tidak menambahkan `std::cout` di Core Layer** — semua output harus melalui `CommandResult::addEvent()`.

6. **Menjaga SRP** — jika sebuah method mulai mengurus lebih dari satu tanggung jawab, pecah ke method helper atau kelas baru.

7. **Menggunakan forward declaration** di header file jika hanya butuh pointer/referensi, bukan full include.

8. **Compile target**: `g++ -std=c++17 -Wall -Wextra -I./include` di Linux. Tidak boleh ada warning.

9. **Path include** mengikuti konvensi proyek:
   - Core classes: `../../include/core/NamaKelas.hpp`
   - Model classes: `../../include/models/NamaKelas.hpp`
   - View classes: `../../include/views/NamaKelas.hpp`
   - Utils: `../../include/utils/NamaKelas.hpp`

10. Saat membuat test, gunakan pola `TestHarness` yang sudah ada di `GameEngineTest.cpp` — inject semua manager melalui setter, jangan buat engine tanpa dependency.
