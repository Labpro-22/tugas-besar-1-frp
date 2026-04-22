# Catatan Kesesuaian Dengan Kode Saat Ini

Dokumen ini sudah disesuaikan dengan implementasi source yang ada sekarang, bukan lagi mengikuti asumsi versi lama.

## 1. Integrasi `ConfigLoader` ke `GameEngine`

Status: **sebagian besar sudah sesuai**

Yang sudah terjadi di kode:

- `GameEngine::initBoard()` sudah memakai `ConfigLoader` untuk membaca:
  - `config/property.txt`
  - `config/railroad.txt`
  - `config/utility.txt`
  - `config/tax.txt`
  - `config/special.txt`
  - `config/misc.txt`
- Nilai `goSalary`, `jailFine`, `maxTurn`, dan `initialBalance` sudah diambil dari hasil loader, bukan lagi hardcoded lokal.
- Properti juga sudah dibangun dari hasil parsing `PropertyDef`.

Referensi kode:

- `src/core/GameEngine.cpp` bagian `initBoard()`
- `src/utils/ConfigLoader.cpp`
- `include/utils/ConfigLoader.hpp`

Catatan penting:

- Yang **belum** dynamic adalah urutan petak papan. Saat ini posisi tile masih memakai konstanta `kBoardOrder` di `GameEngine.cpp`.
- Jadi, poin lama yang menyebut `GameEngine` masih explicit setup config **sudah tidak akurat lagi** untuk `property`, `tax`, `special`, dan `misc`.
- Namun kalau yang dimaksud adalah **layout papan full dynamic**, itu memang **belum selesai**.

## 2. Pesan dari Dana Umum belum keluar di CLI

Status: **masih belum sesuai**

Yang terjadi sekarang:

- Saat pemain mendarat di petak Dana Umum, `CardTile::onLand()` langsung memanggil `CardManager::drawCommunityCard(...)`.
- `CardManager` lalu langsung `draw()` kartu dan memanggil `card->apply(...)`.
- Efek kartu memang dijalankan, tetapi teks kartu tidak dipush ke event UI.

Dampaknya:

- CLI tidak menampilkan pesan eksplisit seperti isi kartu Dana Umum yang sedang diambil.
- `TransactionLogger` sebenarnya sudah punya method `logDrawCard(...)`, tetapi saat ini belum dipanggil dari flow pengambilan kartu.

Referensi kode:

- `src/models/CardTile.cpp`
- `src/core/CardManager.cpp`
- `src/models/CommunityCards.cpp`
- `src/core/TransactionLogger.cpp`
- `src/views/UiFormatter.cpp`

Kesimpulan:

- Efek kartu Dana Umum sudah berjalan.
- Pesan/teks kartunya di CLI masih belum muncul, jadi poin error ini masih valid.

## 3. Logika bangkrut saat efek kartu Dana Umum/Kesempatan

Status: **belum sepenuhnya sesuai spesifikasi**

Yang sudah ada:

- `BankruptcyManager` sudah memiliki alur likuidasi:
  - hitung kemampuan likuidasi
  - panel pilihan jual properti ke bank
  - panel pilihan gadai
  - pelunasan ke kreditor atau bank
  - transfer aset saat bangkrut
  - pengembalian aset ke bank
  - pelelangan aset bank setelah pemain bangkrut ke bank

Referensi utama:

- `src/core/BankruptcyManager.cpp`

Masalah yang masih ada:

- Kartu Dana Umum di `src/models/CommunityCards.cpp` masih melempar `InsufficientFundsException` langsung saat uang pemain tidak cukup.
- Alur ini belum dialihkan ke `BankruptcyManager::handleDebt(...)`.
- Akibatnya, kasus "dapat kartu lalu wajib bayar, tapi uang tidak cukup" belum mengikuti mekanisme likuidasi yang sudah dibuat.

Dampak praktis:

- UI hanya menerima exception/error biasa.
- Turn state bisa terasa macet atau tidak natural karena resolusi hutang tidak masuk ke flow bankruptcy interaktif.

Catatan tambahan:

- Perhitungan `computeMaxLiquidation()` saat ini menghitung kas + nilai jual properti yang belum digadai.
- Panel likuidasi sudah menyediakan opsi jual dan gadai, tetapi ringkasan estimasi masih belum sepenuhnya merepresentasikan semua variasi kombinasi aksi seperti yang dijelaskan di spesifikasi.

Kesimpulan:

- Dokumen lama benar bahwa kasus bangkrut dari efek kartu masih bermasalah.
- Tetapi sekarang sudah ada fondasi `BankruptcyManager`; masalah utamanya tinggal integrasi flow kartu ke manager tersebut.

## 4. Tambahan bonus: Dynamic Board + config loader untuk `aksi.txt`

Status: **belum diimplementasikan**

Kondisi kode sekarang:

- Petak spesial seperti `GO`, `DNU`, `PPH`, `FES`, `PEN`, `BBP`, `KSP`, `PPJ`, dan `PBM` memang sudah didukung.
- Tetapi posisi semuanya masih ditentukan oleh konstanta `kBoardOrder`.
- `ConfigLoader` saat ini belum punya parser untuk file seperti `aksi.txt`.

Artinya:

- Konsep tile di bonus dynamic board sudah cocok dengan model objek yang ada.
- Namun sumber datanya belum datang dari file konfigurasi board.

Yang perlu ada jika bonus ini ingin jadi sesuai kode:

1. Tambah loader baru di `ConfigLoader` untuk membaca `aksi.txt`.
2. Hilangkan ketergantungan `GameEngine::initBoard()` terhadap `kBoardOrder`.
3. Bentuk tile berdasarkan data `JENIS_PETAK` dari file config.
4. Pastikan validasi urutan spesial seperti `PEN` dan `PPJ` tetap aman.

Catatan tambahan untuk renderer:

- `BoardRenderer` saat ini masih mengasumsikan papan Monopoly standar `40` petak dalam layout `11x11`.
- Jadi kalau bonus dynamic board hanya memindahkan isi tile tetapi tetap 40 petak, renderer masih bisa dipakai.
- Kalau nanti ukuran/layout papan ikut berubah, `BoardRenderer` juga harus ikut diubah.

## Ringkasan Status

### Sudah sesuai dengan kode sekarang

- Integrasi `ConfigLoader` untuk properti, tax, special, railroad, utility, dan misc.
- Inisialisasi nilai global game dari file config.
- Fondasi `BankruptcyManager` beserta panel likuidasi.

### Masih belum sesuai / masih perlu dikerjakan

- Pesan kartu Dana Umum belum tampil eksplisit di CLI.
- Hutang akibat kartu Dana Umum/Kesempatan belum masuk ke alur `BankruptcyManager`.
- Dynamic board dari `aksi.txt` belum ada.
- Layout papan masih hardcoded melalui `kBoardOrder`.

## Versi Kesimpulan Singkat

Kalau disesuaikan dengan source sekarang, maka isi dokumen lama perlu diperbaiki menjadi:

1. Poin `ConfigLoader` **bukan lagi masalah utama**, karena sudah dipakai di `GameEngine`.
2. Poin pesan Dana Umum di CLI **masih valid**.
3. Poin logika bangkrut akibat kartu **masih valid**, tetapi sekarang problemnya adalah integrasi ke `BankruptcyManager`, bukan ketiadaan sistem bankruptcy.
4. Bonus dynamic board dengan `aksi.txt` **belum ada implementasinya** dan masih butuh perluasan `ConfigLoader` plus perubahan `GameEngine` dan kemungkinan `BoardRenderer`.
