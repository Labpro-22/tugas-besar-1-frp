# Laporan M1 Tugas Besar 1
# IF2010 Pemrograman Berorientasi Objek 2025/2026
# Nimonspoli

| | |
|---|---|
| **Kode Kelompok** | FRP |
| **Nama Kelompok** | Furap |

**Anggota Kelompok:**
1. 13524006 / Gabriella Botimada Lubis
2. 13524032 / Juan Oloando Simanungkalit
3. 13524064 / Stefani Angeline Oroh
4. 13524093 / Reinsen Silitonga
5. 13524098 / Reva Natania Sitohang

---

## 1. Diagram Kelas

Proses pembuatan diagram kelas menggunakan Graphviz. Detail desain kelas secara lengkap dapat dilihat pada lampiran kelas diagram.

### Ringkasan Relasi Antar Kelas (dari Diagram)

Diagram kelas dibagi ke dalam beberapa lapisan utama:

#### UI Layer
- **GameUI** memiliki (has-a) `CommandParser` dan `BoardRenderer`, serta menggunakan (uses) `GameEngine`.
- **CommandParser** memproduksi (produces) objek `Command`.
- **BoardRenderer** digunakan oleh `GameUI`.

#### Core Layer
- **GameEngine** memiliki (has-a): `Board`, `TurnManager`, `Bank`, `Dice`, `AuctionManager`, `BankruptcyManager`, `PropertyManager`, `CardManager`, `EffectManager`, `TransactionLogger`.
- **Board** memiliki banyak (has-many) `Tile` via `unique_ptr`.
- **PropertyTile** memiliki (has-a) `Property` via `shared_ptr`.
- **Player** mereferensikan banyak (owns*) `Property` dan memegang (holds*) `SkillCard`.
- **CardManager** memiliki (has) `Deck<T>`.

#### Hierarki Tile (Inheritance)
`Tile` ← `PropertyTile`, `GoTile`, `JailTile`, `GoToJailTile`, `TaxTile`, `FestivalTile`, `CardTile`, `FreeParkingTile`

#### Hierarki Property (Inheritance)
`Property` ← `StreetProperty`, `RailroadProperty`, `UtilityProperty`

#### Hierarki Card (Inheritance)
- `Card` ← `ActionCard` ← `GoToNearestRailroadCard`, `MoveBackCard`, `GoToJailCard`, `BirthdayCard`, `DoctorFeeCard`, `ElectionCard`
- `Card` ← `SkillCard` ← `MoveCard`, `DiscountCard`, `ShieldCard`, `TeleportCard`, `LassoCard`, `DemolitionCard`

#### Data Access Layer
- **SaveLoadManager** memiliki (has-a) `GameStateSerializer` dan mengakses (read/write) `GameEngine`.
- **ConfigLoader** mengkonfigurasi (configures) `GameEngine`.
- **TransactionLogger** menyimpan (stores) `LogEntry`.
- **Property** menggunakan (uses) `GameContext`.
- **GoToJailTile** menggunakan (uses) `JailTile::sendToJail()`.

---

## 2. Deskripsi Kelas

### 2.1. Kelas GameUI

GameUI adalah pintu masuk utama seluruh interaksi antara pemain manusia dan sistem permainan. Kelas ini bertanggung jawab untuk tampilan menu awal, pengambilan input perintah dari pemain pada setiap giliran, dan penyampaian pesan hasil aksi kembali ke layar. GameUI tidak boleh menyentuh logika permainan sama sekali, hanya meneruskan input ke `GameEngine` melalui `processCommand()` dan menampilkan hasilnya. Dengan demikian, seluruh logika di Core Layer bebas dari kode I/O. GameUI memiliki `CommandParser` untuk mengubah string mentah menjadi `Command` terstruktur, dan `BoardRenderer` untuk menampilkan papan.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `CommandParser` | `parser` | Parser yang mengubah input string mentah menjadi objek `Command` |
| - | `BoardRenderer` | `renderer` | Renderer yang menangani tampilan papan permainan di terminal |
| - | `GameEngine*` | `engine` | Pointer ke `GameEngine` sebagai pusat logika permainan |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `showMainMenu()` | Menampilkan menu utama: pilihan New Game atau Load Game |
| + | `void` | `showTurnMenu(const Player& p)` | Menampilkan menu giliran: daftar perintah yang tersedia untuk pemain aktif |
| + | `std::string` | `promptCommand()` | Meminta input perintah dari pemain dan mengembalikan string mentah |
| + | `void` | `showMessage(const std::string& msg)` | Menampilkan pesan hasil aksi atau notifikasi ke layar |
| + | `void` | `run()` | Loop utama UI: terus meminta input dan meneruskan ke `GameEngine` hingga game selesai |
| - | `void` | `displayWelcome()` | Menampilkan layar sambutan saat program pertama kali dijalankan |

#### Relasi dengan Kelas Lain

1. **has-a** `CommandParser` : untuk parsing input perintah pemain
2. **has-a** `BoardRenderer` : untuk render tampilan papan ke layar
3. **uses** `GameEngine` : meneruskan `Command` hasil parsing ke `processCommand()`

---

### 2.2. Kelas CommandParser

`CommandParser` mengurai (parse) string input mentah yang diketik pemain menjadi objek `Command` yang terstruktur dan mudah diproses `GameEngine`. Pemisahan parser ke kelas tersendiri memastikan `GameUI` tidak perlu mengurus logika string manipulation dan `GameEngine` tidak perlu mengurus format raw input. Method `parse()` mengidentifikasi `CommandType` dari kata pertama input (misalnya `'LEMPAR_DADU'` → `CommandType::ROLL_DICE`) dan mengekstrak sisa kata sebagai argumen ke `vector<string>`. Validasi format awal dilakukan di sini, sedangkan validasi semantik (misalnya kode properti valid atau tidak) tetap dilakukan di layer Core.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `Command` | `parse(const std::string& raw)` | Mengurai string input mentah menjadi objek `Command` berisi type dan args |
| - | `CommandType` | `resolveType(const std::string& token)` | Memetakan string perintah ke nilai enum `CommandType` yang sesuai |
| - | `std::vector<std::string>` | `tokenize(const std::string& raw)` | Memecah string input berdasarkan spasi menjadi token-token kata |

#### Relasi dengan Kelas Lain

1. **produces** `Command` : hasil parsing dikembalikan sebagai objek `Command`
2. **digunakan oleh** `GameUI` : dipanggil setiap kali pemain mengetik perintah

---

### 2.3. Kelas BoardRenderer

`BoardRenderer` adalah kelas khusus yang bertanggung jawab merender papan permainan ke layar terminal dalam format ASCII dengan warna ANSI. Ia menerima referensi ke `Board` (untuk mendapatkan semua tile dan posisinya) serta `vector<Player>` (untuk mengetahui posisi semua bidak dan kepemilikan properti). Kelas ini menangani seluruh detail visual: kode warna per kategori tile, simbol bangunan (`^/*/H`), indikator bidak pemain (angka dalam kurung), status penjara, legenda, dan indikator turn. Dengan memisahkan renderer ke kelas tersendiri, kelas `Board` bebas dari tanggung jawab tampilan.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `cellWidth` | Lebar satu sel papan dalam karakter (default: 10) |
| - | `int` | `sideLength` | Jumlah sel per sisi papan (default: 11 untuk papan 40 petak) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `render(const Board& b, const std::vector<Player>& ps, int turn, int maxTurn)` | Merender seluruh papan permainan beserta semua informasi status ke layar |
| - | `std::string` | `buildCell(const Tile& t, const std::vector<Player>& ps)` | Membangun string satu sel papan termasuk warna, kode, kepemilikan, dan bidak |
| - | `std::string` | `colorCode(const std::string& category)` | Mengembalikan kode warna ANSI sesuai kategori warna tile |
| - | `std::string` | `buildingSymbol(BuildingLevel lvl)` | Mengembalikan simbol bangunan: `^/^^/^^^/^^^^/H` sesuai level |
| - | `void` | `renderLegend(int turn, int maxTurn)` | Merender panel tengah berisi legenda, kode warna, dan info turn |
| - | `std::string` | `playerTokens(int tileIdx, const std::vector<Player>& ps)` | Menghasilkan string token pemain yang berada di tile tersebut |

#### Relasi dengan Kelas Lain

1. **uses** `Board` : membaca tile untuk membangun tampilan papan
2. **uses** `vector<Player>` : membaca posisi dan data pemain untuk tampilan bidak
3. **digunakan oleh** `GameUI` : dipanggil saat perintah `CETAK_PAPAN`

---

### 2.4. Kelas GameEngine

`GameEngine` adalah pusat orkestrasi permainan yang mengelola alur besar. Class tidak menangani detail bisnis secara langsung, melainkan mendelegasikan ke manager yang tepat. `GameEngine` menyimpan state global (board, players, turn), menggunakan `TurnManager` untuk mengatur urutan giliran, memanggil `handleLanding()` yang mendispatch ke `PropertyManager`/`CardManager`/`BankruptcyManager` bergantung tile yang diinjak, dan mencatat setiap aksi ke `TransactionLogger`. `GameEngine` menjadi titik temu antara UI Layer (menerima `Command`) dan semua manager di Core Layer.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `Board` | `board` | Papan permainan berisi semua tile |
| - | `std::vector<Player>` | `players` | Daftar seluruh pemain aktif dan bangkrut |
| - | `TurnManager` | `turnManager` | Mengelola urutan dan nomor giliran |
| - | `Bank` | `bank` | Entitas bank untuk transaksi ke/dari bank |
| - | `Dice` | `dice` | Dadu untuk pergerakan pemain |
| - | `AuctionManager` | `auctionManager` | Menjalankan mekanisme lelang properti |
| - | `BankruptcyManager` | `bankruptcyManager` | Menangani proses kebangkrutan |
| - | `PropertyManager` | `propertyManager` | Mengelola logika kepemilikan dan bangunan properti |
| - | `CardManager` | `cardManager` | Mengelola semua deck dan penggunaan kartu |
| - | `EffectManager` | `effectManager` | Mengelola efek sementara lintas giliran |
| - | `TransactionLogger` | `logger` | Mencatat semua kejadian signifikan permainan |
| - | `bool` | `gameOver` | Flag penanda permainan telah berakhir |
| - | `int` | `maxTurn` | Batas maksimum giliran dari konfigurasi (0 = mode bankruptcy only) |
| - | `int` | `goSalary` | Gaji yang didapat pemain saat melewati/berhenti di GO |
| - | `int` | `jailFine` | Denda keluar penjara dari konfigurasi |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `startNewGame(int nPlayers, std::vector<std::string> names)` | Menginisialisasi permainan baru: board, players, urutan acak, distribusi saldo awal |
| + | `void` | `loadGame(const std::string& filename)` | Memuat state permainan dari file via `SaveLoadManager` |
| + | `void` | `run()` | Loop utama permainan: terus menjalankan turn hingga `gameOver` |
| + | `void` | `processCommand(const Command& cmd)` | Mendispatch `Command` dari UI ke handler yang sesuai |
| + | `void` | `executeTurn()` | Menjalankan satu giliran: distribusi kartu, eksekusi aksi, cek kondisi menang |
| + | `void` | `moveCurrentPlayer(int steps)` | Memindahkan bidak pemain aktif, mendeteksi lewat GO, memanggil `handleLanding` |
| + | `void` | `handleLanding(Player& p, Tile& t)` | Mendispatch ke `tile.onLand()` yang akan memanggil logic yang sesuai |
| + | `void` | `checkWinCondition()` | Mengecek apakah kondisi akhir permainan sudah terpenuhi |
| + | `void` | `endGame()` | Menampilkan hasil akhir permainan dan menghentikan loop |
| + | `Player&` | `getCurrentPlayer()` | Mengembalikan referensi ke pemain yang sedang giliran |
| + | `Player*` | `getPlayerByName(const std::string& name)` | Mencari dan mengembalikan pointer ke pemain berdasarkan username |
| + | `std::vector<Player*>` | `getActivePlayers()` | Mengembalikan daftar pointer pemain yang masih aktif (tidak bangkrut) |
| - | `void` | `initBoard(const std::vector<PropertyDef>& defs)` | Membangun `Board` dari definisi properti hasil `ConfigLoader` |
| - | `void` | `handleJailTurn(Player& p)` | Menangani giliran pemain yang sedang di penjara |
| - | `void` | `awardPassGoSalary(Player& p)` | Memberikan gaji GO saat pemain melewati petak GO |

#### Relasi dengan Kelas Lain

1. **has-a** `Board` : menyimpan papan permainan
2. **has-a** `TurnManager` : mengatur urutan giliran
3. **has-a** `Bank`, `Dice` : entitas dasar permainan
4. **has-a** `PropertyManager`, `AuctionManager`, `BankruptcyManager`, `CardManager`, `EffectManager` : delegasi logika bisnis
5. **has-a** `TransactionLogger` : pencatatan semua aksi
6. **digunakan oleh** `GameUI` : menerima `Command` dari UI
7. **dikonfigurasi oleh** `ConfigLoader` : saat inisialisasi
8. **diakses oleh** `SaveLoadManager` : untuk simpan/muat state

---

### 2.5. Kelas TurnManager

`TurnManager` memisahkan logika pengaturan giliran dari `GameEngine` agar tanggung jawab lebih bersih dan terfokus. Ia menyimpan urutan giliran (`turnOrder`) yang ditentukan secara acak saat awal permainan, melacak index pemain aktif (`currentIndex`), dan menghitung nomor turn global (`currentTurnNumber`). Method `grantExtraTurn()` dipanggil saat pemain melempar double agar giliran tidak berpindah. `GameEngine` hanya perlu bertanya siapa yang bermain sekarang dan berikutnya tanpa mengelola detail urutan sendiri. `TurnManager` juga bertanggung jawab mengecualikan pemain yang sudah bangkrut dari urutan giliran.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::vector<int>` | `turnOrder` | Urutan index pemain yang bermain (hasil pengacakan awal) |
| - | `int` | `currentIndex` | Index di dalam `turnOrder` yang menunjuk pemain aktif saat ini |
| - | `int` | `currentTurnNumber` | Nomor turn saat ini (bertambah setelah semua pemain selesai satu putaran) |
| - | `bool` | `extraTurnGranted` | Flag bahwa giliran tambahan (karena double) telah diberikan |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `initializeOrder(int nPlayers)` | Mengacak urutan giliran untuk sejumlah `nPlayers` pemain |
| + | `int` | `getCurrentPlayerIndex()` | Mengembalikan index pemain (dalam vector players di `GameEngine`) yang sedang giliran |
| + | `void` | `nextPlayer()` | Berpindah ke pemain berikutnya dalam `turnOrder`, melewati pemain bangkrut |
| + | `void` | `grantExtraTurn()` | Memberikan giliran tambahan kepada pemain saat ini (tidak berpindah ke berikutnya) |
| + | `int` | `getTurnNumber()` | Mengembalikan nomor turn saat ini |
| + | `void` | `removePlayer(int playerIndex)` | Menghapus pemain yang bangkrut dari urutan giliran |
| + | `void` | `resetExtraTurn()` | Mereset flag extra turn setelah giliran tambahan dieksekusi |
| + | `bool` | `hasExtraTurn()` | Mengecek apakah pemain aktif memiliki giliran tambahan yang belum digunakan |

#### Relasi dengan Kelas Lain

1. **digunakan oleh** `GameEngine` : dipanggil untuk navigasi urutan giliran
2. Tidak memiliki dependency ke kelas lain secara langsung

---

### 2.6. Kelas Board

`Board` merepresentasikan papan permainan sebagai koleksi `Tile` yang tersusun membentuk jalur melingkar. Ia menyimpan tile dalam `vector<unique_ptr<Tile>>` untuk kepemilikan memori yang aman dan polimorfis, serta `map<string, int>` untuk akses cepat tile berdasarkan kode huruf. `Board` menyediakan antarmuka akses tile (by index atau by code) dan membantu kalkulasi movement wrap-around (`posisi % size`). `Board` tidak memiliki logika permainan, hanya struktur data papan yang bersih.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::vector<std::unique_ptr<Tile>>` | `tiles` | Koleksi semua tile papan dengan kepemilikan polimorfis |
| - | `std::map<std::string, int>` | `codeToIndex` | Pemetaan kode tile (misalnya "JKT") ke indeks dalam vector tiles |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `addTile(std::unique_ptr<Tile> tile)` | Menambahkan tile ke papan saat inisialisasi |
| + | `Tile&` | `getTileByIndex(int idx)` | Mengakses tile berdasarkan posisi index (dengan wrap-around `% size()`) |
| + | `Tile&` | `getTileByCode(const std::string& code)` | Mengakses tile berdasarkan kode huruf uniknya |
| + | `int` | `size() const` | Mengembalikan jumlah total tile di papan |
| + | `int` | `getIndexOf(const std::string& code) const` | Mengembalikan index numerik dari kode tile tertentu |
| + | `int` | `distanceTo(int from, int to) const` | Menghitung jarak searah jarum jam dari posisi from ke to |

#### Relasi dengan Kelas Lain

1. **has-many** `Tile` (`unique_ptr`) : kepemilikan dan akses polimorfis semua tile
2. **digunakan oleh** `GameEngine` : untuk navigasi dan resolusi landing
3. **digunakan oleh** `BoardRenderer` : untuk iterasi tile dalam rendering tampilan
4. **digunakan oleh** berbagai Manager : untuk pencarian tile spesifik

---

### 2.7. Kelas Player

`Player` merepresentasikan kondisi lengkap seorang pemain: uang tunai, posisi di papan, properti yang dimiliki, kartu di tangan, status permainan, dan flag efek sementara. Kelas ini mengimplementasikan operator overloading sesuai spesifikasi: `operator+=` dan `operator-=` untuk operasi uang (menambah/memotong saldo), `operator>` dan `operator<` untuk perbandingan kekayaan total antar pemain saat menentukan pemenang. `Player` bersifat semi-pasif: menyimpan data dan menyediakan operasi dasar, sedangkan logika kapan harus memotong uang atau kapan bangkrut dikelola oleh manager terkait.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::string` | `username` | Nama unik pemain sebagai identifikasi dalam permainan |
| - | `int` | `money` | Jumlah uang tunai pemain saat ini |
| - | `int` | `position` | Posisi bidak pemain (index tile di `Board`, 0-based) |
| - | `PlayerStatus` | `status` | Status pemain: `ACTIVE`, `JAILED`, atau `BANKRUPT` |
| - | `std::vector<Property*>` | `ownedProperties` | Daftar pointer ke properti yang dimiliki pemain |
| - | `std::vector<std::shared_ptr<SkillCard>>` | `handCards` | Kartu kemampuan di tangan pemain (maks 3) |
| - | `bool` | `hasUsedSkillThisTurn` | Flag apakah pemain sudah menggunakan kartu skill pada giliran ini |
| - | `bool` | `shieldActive` | Flag apakah `ShieldCard` aktif pada giliran ini |
| - | `int` | `jailTurns` | Jumlah giliran yang sudah dihabiskan di penjara (maks 3 sebelum wajib bayar) |
| - | `int` | `consecutiveDoubles` | Jumlah double berturut-turut dalam satu giliran (reset tiap giliran) |
| - | `int` | `discountPercent` | Persentase diskon aktif (dari `DiscountCard`, 0 = tidak aktif) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `move(int steps, int boardSize)` | Memindahkan posisi bidak maju sejumlah steps dengan wrap-around |
| + | `void` | `setPosition(int pos)` | Memindahkan bidak langsung ke posisi tertentu (teleport/jail) |
| + | `void` | `addMoney(int amount)` | Menambah uang tunai pemain |
| + | `void` | `deductMoney(int amount)` | Mengurangi uang tunai pemain (tidak negatif, validasi di caller) |
| + | `bool` | `canAfford(int amount) const` | Mengecek apakah saldo cukup untuk membayar sejumlah amount |
| + | `void` | `addProperty(Property* prop)` | Menambahkan properti ke daftar kepemilikan |
| + | `void` | `removeProperty(Property* prop)` | Menghapus properti dari daftar kepemilikan |
| + | `void` | `addCard(std::shared_ptr<SkillCard> card)` | Menambahkan kartu skill ke tangan (validasi jumlah di `CardManager`) |
| + | `void` | `removeCard(int idx)` | Membuang kartu skill pada index tertentu dari tangan |
| + | `bool` | `isBankrupt() const` | Mengembalikan true jika status == `BANKRUPT` |
| + | `bool` | `isJailed() const` | Mengembalikan true jika status == `JAILED` |
| + | `int` | `getTotalWealth() const` | Menghitung total kekayaan: uang + harga beli semua properti + nilai bangunan |
| + | `int` | `countProperties() const` | Mengembalikan jumlah properti yang dimiliki (untuk tie-breaking) |
| + | `int` | `countCards() const` | Mengembalikan jumlah kartu di tangan (untuk tie-breaking) |
| + | `Player&` | `operator+=(int amount)` | Menambah saldo (`money += amount`), mengembalikan `*this` |
| + | `Player&` | `operator-=(int amount)` | Mengurangi saldo (`money -= amount`), mengembalikan `*this` |
| + | `bool` | `operator>(const Player& other) const` | Membandingkan total kekayaan: true jika kekayaan ini > other |
| + | `bool` | `operator<(const Player& other) const` | Membandingkan total kekayaan: true jika kekayaan ini < other |

#### Relasi dengan Kelas Lain

1. **owns*** `Property` (pointer) : daftar properti milik pemain
2. **holds*** `SkillCard` (`shared_ptr`) : kartu kemampuan di tangan
3. **digunakan oleh** hampir semua Manager : sebagai subjek aksi
4. **digunakan oleh** `BoardRenderer` : untuk tampilan posisi dan informasi

---

### 2.8. Kelas Bank

`Bank` mengabstraksikan semua transaksi yang melibatkan entitas Bank sebagai pihak yang membayar atau menerima uang. Dengan kelas ini, seluruh transfer uang ke/dari bank tidak tersebar hardcoded di berbagai manager, cukup memanggil metode Bank yang tepat. Hal ini menjaga Single Responsibility: `Player` mengurus uang pemain, `Bank` mengurus aliran dari/ke bank. `Bank` tidak menyimpan saldo sendiri (bank diasumsikan memiliki dana tak terbatas sesuai aturan Monopoli), sehingga atributnya minimal.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `goSalary` | Jumlah gaji GO yang dibayarkan ke pemain (dari konfigurasi) |
| - | `int` | `jailFine` | Besar denda keluar penjara (dari konfigurasi) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `paySalary(Player& p, int amount)` | Memberikan gaji GO kepada pemain (`Player += amount`) |
| + | `void` | `collectTax(Player& p, int amount)` | Memungut pajak dari pemain ke bank (`Player -= amount`) |
| + | `void` | `collectJailFine(Player& p)` | Memungut denda keluar penjara dari pemain |
| + | `void` | `payMortgage(Player& p, int value)` | Memberikan uang nilai gadai ke pemain |
| + | `void` | `collectRedemption(Player& p, int price)` | Memungut harga tebus properti dari pemain |
| + | `void` | `collectSalePrice(Player& p, int price)` | Menerima pembayaran properti yang dijual pemain ke bank |
| + | `void` | `payPropertySale(Player& p, int value)` | Membayar nilai jual properti ke pemain saat menjual ke bank |

#### Relasi dengan Kelas Lain

1. **uses** `Player` : sebagai pihak yang menerima atau membayar
2. **digunakan oleh** `PropertyManager`, `BankruptcyManager`, `EffectManager`, dan `Tile` : untuk transaksi bank

---

### 2.9. Kelas Dice

`Dice` mengenkapsulasi logika kedua dadu permainan agar tidak tersebar di `GameEngine`. Ia menyimpan hasil lemparan terakhir (`die1`, `die2`) dan mendukung dua mode: `rollRandom()` untuk lemparan acak (perintah `LEMPAR_DADU`) dan `setManual(x, y)` untuk lemparan manual (perintah `ATUR_DADU`). `isDouble()` digunakan `GameEngine` untuk mendeteksi giliran tambahan atau kondisi tiga-double-berturut yang mengirim pemain ke penjara.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `die1` | Nilai dadu pertama dari lemparan terakhir |
| - | `int` | `die2` | Nilai dadu kedua dari lemparan terakhir |
| - | `int` | `faces` | Jumlah sisi dadu (default: 6) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `std::pair<int,int>` | `rollRandom()` | Mengacak nilai kedua dadu (1–6), menyimpan ke `die1`/`die2`, mengembalikan pair |
| + | `std::pair<int,int>` | `setManual(int x, int y)` | Mengatur nilai dadu secara manual, memvalidasi rentang 1–6 |
| + | `bool` | `isDouble() const` | Mengembalikan true jika `die1 == die2` |
| + | `int` | `getTotal() const` | Mengembalikan `die1 + die2` (total langkah atau faktor pengali utility) |
| + | `int` | `getDie1() const` | Getter nilai dadu pertama |
| + | `int` | `getDie2() const` | Getter nilai dadu kedua |

#### Relasi dengan Kelas Lain

1. **digunakan oleh** `GameEngine` : dipanggil saat `LEMPAR_DADU` dan `ATUR_DADU`
2. **digunakan oleh** `UtilityProperty::calculateRent` via `GameContext`, total dadu sebagai faktor pengali

---

### 2.10. Kelas Tile (Abstract Base Class)

`Tile` adalah kelas abstrak yang menjadi dasar semua petak pada papan permainan. Ia mendefinisikan interface umum yang wajib diimplementasikan setiap petak konkret: virtual method `onLand()` yang dipanggil `GameEngine` setiap kali pemain mendarat. Dengan polimorfisme, `GameEngine` cukup memanggil `tile.onLand(player, game)` tanpa perlu tahu tipe konkret tile; masing-masing subclass menjalankan perilaku yang sesuai. `Tile` menyimpan index posisi di papan, kode unik, dan nama lengkap yang digunakan oleh `BoardRenderer`.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| # | `int` | `index` | Posisi tile dalam papan (0-based index) |
| # | `std::string` | `code` | Kode huruf unik tile (misal "JKT", "BDG", "GO") |
| # | `std::string` | `name` | Nama lengkap tile untuk tampilan |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `virtual ~Tile()` | `~Tile()` | Virtual destructor untuk keamanan penghapusan polimorfis |
| + | `virtual void` | `onLand(Player& p, GameEngine& game) = 0` | Pure virtual: dipanggil saat pemain mendarat di tile ini |
| + | `virtual bool` | `isProperty() const` | Mengembalikan false secara default; dioverride oleh `PropertyTile` |
| + | `virtual std::string` | `getDisplayInfo() const` | Mengembalikan string info singkat tile untuk `BoardRenderer` |
| + | `int` | `getIndex() const` | Getter index posisi tile |
| + | `std::string` | `getCode() const` | Getter kode huruf tile |
| + | `std::string` | `getName() const` | Getter nama lengkap tile |

#### Relasi dengan Kelas Lain

1. **parent dari** semua tile konkret (`PropertyTile`, `GoTile`, `JailTile`, `GoToJailTile`, `TaxTile`, `FestivalTile`, `CardTile`, `FreeParkingTile`)
2. **dimiliki oleh** `Board` : via `vector<unique_ptr<Tile>>`
3. **digunakan oleh** `GameEngine` : dipanggil `onLand()` saat pemain mendarat

---

### 2.11. Kelas PropertyTile

`PropertyTile` adalah petak yang berasosiasi dengan satu objek `Property` (Street, Railroad, atau Utility). Saat `onLand()` dipanggil, `PropertyTile` mendelegasikan seluruh logika ke `PropertyManager` di `GameEngine`: jika properti berstatus `BANK`, menawarkan pembelian atau akuisisi otomatis; jika berstatus `OWNED` milik pemain lain, membayar sewa. `PropertyTile` hanya menjadi jembatan antara posisi di papan (`Tile`) dan data aset (`Property`). Satu `PropertyTile` memiliki tepat satu `Property`.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::shared_ptr<Property>` | `property` | Properti yang berasosiasi dengan tile ini (Street/Railroad/Utility) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | Mendispatch ke `PropertyManager` berdasarkan status properti (`BANK`/`OWNED`/`MORTGAGED`) |
| + | `bool` | `isProperty() const` | Override: mengembalikan true |
| + | `Property&` | `getProperty()` | Getter referensi ke objek `Property` yang dipegang |
| + | `const Property&` | `getProperty() const` | Getter const untuk akses read-only ke `Property` |

#### Relasi dengan Kelas Lain

1. **extends** `Tile` : mewarisi index, code, name, dan interface `onLand()`
2. **has-a** `Property` (`shared_ptr`) : kepemilikan data aset properti
3. **uses** `PropertyManager` (via `GameEngine`) : mendelegasikan logika beli/sewa

---

### 2.12. Kelas GoTile

`GoTile` merepresentasikan petak Mulai (GO), titik awal permainan dan sumber pendapatan tetap. Saat `onLand()` dipanggil (pemain berhenti tepat di GO), `GoTile` memanggil `Bank::paySalary()` untuk memberikan gaji. Deteksi pemain melewati GO (tidak berhenti) dilakukan di `GameEngine::moveCurrentPlayer()` secara terpisah, bukan di `onLand()`. `GoTile` menyimpan nilai gaji yang dibaca dari konfigurasi.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `salary` | Jumlah gaji yang diterima pemain saat berhenti di petak GO |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | Memanggil `Bank::paySalary(p, salary)` dan mencatat ke `TransactionLogger` |
| + | `std::string` | `getDisplayInfo() const` | Mengembalikan `"GO | Gaji: M{salary}"` |

#### Relasi dengan Kelas Lain

1. **extends** `Tile`
2. **uses** `Bank` (via `GameEngine`) : untuk membayarkan gaji GO

---

### 2.13. Kelas JailTile

`JailTile` merepresentasikan petak Penjara yang memiliki dua fungsi berbeda berdasarkan cara pemain tiba: (1) **Hanya Mampir (Visiting)**: jika pemain tiba dari lemparan dadu normal, tidak ada aksi apapun; (2) **Dalam Penjara**: jika pemain dikirim oleh `GoToJailTile` atau kartu/mekanisme lain, status pemain diubah menjadi `JAILED`. Logika handling giliran pemain di penjara (coba double, bayar denda, atau paksa keluar di giliran ke-4) ditangani oleh `GameEngine::handleJailTurn()`.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | Jika `p.isJailed()` false: tampilkan pesan "Hanya Mampir". Jika true: sudah ditangani oleh `handleJailTurn()` |
| + | `void` | `sendToJail(Player& p)` | Mengatur posisi dan status pemain menjadi di penjara (dipanggil dari `GoToJailTile` dan kartu) |
| + | `std::string` | `getDisplayInfo() const` | Mengembalikan info jumlah tahanan dan pengunjung saat ini |

#### Relasi dengan Kelas Lain

1. **extends** `Tile`
2. **dipanggil** `sendToJail()` oleh `GoToJailTile` dan `GoToJailCard`
3. Logika giliran penjara dikelola oleh `GameEngine`

---

### 2.14. Kelas GoToJailTile

`GoToJailTile` adalah petak jebakan; saat pemain mendarat di sini, bidak langsung dipindahkan ke `JailTile` dengan status `JAILED` tanpa melewati GO (tidak mendapat gaji). Giliran pemain berakhir seketika setelah landing di tile ini. `onLand()` memanggil `JailTile::sendToJail()` dan memberitahu `GameEngine` untuk mengakhiri giliran.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | Memanggil `sendToJail(p)`, mengakhiri giliran pemain, tidak memberikan gaji GO |
| + | `std::string` | `getDisplayInfo() const` | Mengembalikan `"Pergi ke Penjara!"` |

#### Relasi dengan Kelas Lain

1. **extends** `Tile`
2. **uses** `JailTile::sendToJail()` : untuk mengirim pemain ke penjara
3. **uses** `GameEngine` : untuk mengakhiri giliran secara paksa

---

### 2.15. Kelas TaxTile

`TaxTile` merepresentasikan kedua petak pajak: Pajak Penghasilan (PPH) dan Pajak Barang Mewah (PBM). Tipe pajak ditentukan oleh atribut `TaxType`. Untuk PPH, pemain diberi pilihan membayar flat atau persentase dari total kekayaan sebelum menghitung kekayaannya. Untuk PBM, langsung dipotong nilai flat. Jika pemain tidak mampu membayar, `TaxTile` memicu `BankruptcyManager::handleDebt()` dengan Bank sebagai kreditor.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `TaxType` | `type` | Jenis pajak: `PPH` (Pajak Penghasilan) atau `PBM` (Pajak Barang Mewah) |
| - | `int` | `flatAmount` | Jumlah pajak flat (PPH flat atau PBM) |
| - | `int` | `percentage` | Persentase untuk PPH (0 untuk PBM) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | Menampilkan pilihan dan memproses pembayaran pajak sesuai tipe |
| - | `void` | `handlePPH(Player& p, GameEngine& game)` | Menangani logika PPH: tampil pilihan, hitung, potong, atau trigger bankruptcy |
| - | `void` | `handlePBM(Player& p, GameEngine& game)` | Langsung memotong PBM flat; jika gagal trigger bankruptcy |
| + | `std::string` | `getDisplayInfo() const` | Mengembalikan info jenis dan besar pajak |

#### Relasi dengan Kelas Lain

1. **extends** `Tile`
2. **uses** `BankruptcyManager` (via `GameEngine`) : jika pemain tidak mampu membayar
3. **uses** `Bank` (via `GameEngine`) : untuk menerima pembayaran pajak

---

### 2.16. Kelas FestivalTile

`FestivalTile` memicu efek festival yang melipatgandakan sewa properti milik pemain. Saat `onLand()`, pemain diminta memilih salah satu properti miliknya. `FestivalTile` mendelegasikan seluruh logika penerapan dan penguatan efek ke `EffectManager::applyFestival()`. `EffectManager` yang mengelola `festivalMultiplier` (1/2/4/8) dan `festivalDuration` (0–3) pada objek `Property` yang dipilih.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | Menampilkan daftar properti pemain, meminta pilihan, mendelegasikan ke `EffectManager` |
| + | `std::string` | `getDisplayInfo() const` | Mengembalikan `"Festival : Pilih properti untuk digandakan sewanya"` |

#### Relasi dengan Kelas Lain

1. **extends** `Tile`
2. **uses** `EffectManager` (via `GameEngine`) : untuk menerapkan/memperkuat efek festival pada properti

---

### 2.17. Kelas CardTile

`CardTile` merepresentasikan petak Kesempatan atau Dana Umum. Atribut `drawType` menentukan dari deck mana kartu ditarik. Saat `onLand()`, `CardTile` memanggil `CardManager::drawChanceCard()` atau `drawCommunityCard()` sesuai tipe, yang kemudian mengeksekusi efek kartu secara langsung pada pemain.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `CardDrawType` | `drawType` | Tipe kartu: `CHANCE` (Kesempatan) atau `COMMUNITY` (Dana Umum) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | Memanggil `CardManager` sesuai `drawType` untuk mengambil dan mengeksekusi kartu |
| + | `std::string` | `getDisplayInfo() const` | Mengembalikan `"Kesempatan"` atau `"Dana Umum"` |

#### Relasi dengan Kelas Lain

1. **extends** `Tile`
2. **uses** `CardManager` (via `GameEngine`) : untuk menarik dan mengeksekusi kartu

---

### 2.18. Kelas FreeParkingTile

`FreeParkingTile` merepresentasikan petak Bebas Parkir, rest area tanpa aksi apapun. `onLand()` tidak melakukan apapun (no-op). Kelas ini tetap dibuat sebagai kelas tersendiri (bukan case dalam switch) untuk menghormati prinsip Open/Closed dan polimorfisme: setiap tile adalah objek dengan perilaku masing-masing.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onLand(Player& p, GameEngine& game)` | No-op: tidak ada aksi saat mendarat di Bebas Parkir |
| + | `std::string` | `getDisplayInfo() const` | Mengembalikan `"Bebas Parkir, Tidak ada aksi"` |

#### Relasi dengan Kelas Lain

1. **extends** `Tile`

---

### 2.19. Kelas Property (Abstract Base Class)

`Property` merepresentasikan aset properti yang dapat dimiliki pemain, sengaja dipisahkan dari `Tile` karena data kepemilikan (owner, status, bangunan, efek festival) harus tetap ada dan konsisten meski pemain berpindah posisi. `Property` mendefinisikan interface abstrak `calculateRent()` yang diimplementasikan berbeda oleh setiap subclass. Atribut `festivalMultiplier` dan `festivalDuration` dikelola oleh `EffectManager`. Enum `OwnershipStatus` (`BANK`/`OWNED`/`MORTGAGED`) menentukan apakah sewa berlaku.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| # | `std::string` | `code` | Kode huruf unik properti (sama dengan kode tile terkait) |
| # | `std::string` | `name` | Nama lengkap properti |
| # | `PropertyType` | `type` | Jenis: `STREET`, `RAILROAD`, atau `UTILITY` |
| # | `Player*` | `owner` | Pointer ke pemilik (`nullptr` jika berstatus `BANK`) |
| # | `OwnershipStatus` | `status` | Status: `BANK`, `OWNED`, atau `MORTGAGED` |
| # | `int` | `purchasePrice` | Harga beli properti dari konfigurasi |
| # | `int` | `mortgageValue` | Nilai gadai properti dari konfigurasi |
| # | `int` | `festivalMultiplier` | Pengganda sewa festival: 1 (tidak aktif), 2, 4, atau 8 |
| # | `int` | `festivalDuration` | Sisa durasi efek festival: 0 = tidak aktif, 1–3 = aktif |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `virtual ~Property()` | `~Property()` | Virtual destructor untuk penghapusan polimorfis yang aman |
| + | `virtual int` | `calculateRent(const GameContext& ctx) const = 0` | Pure virtual: menghitung sewa berdasarkan kondisi saat ini (polimorfis) |
| + | `virtual bool` | `canBuild() const` | Mengembalikan false secara default; dioverride oleh `StreetProperty` |
| + | `virtual int` | `getSellValue() const` | Nilai jual ke bank: `purchasePrice` + setengah harga bangunan (jika ada) |
| + | `bool` | `isOwned() const` | True jika status == `OWNED` |
| + | `bool` | `isMortgaged() const` | True jika status == `MORTGAGED` |
| + | `bool` | `isBank() const` | True jika status == `BANK` (belum dimiliki siapapun) |
| + | `void` | `setOwner(Player* p)` | Mengatur pemilik baru dan status `OWNED` (`nullptr` untuk `BANK`) |
| + | `void` | `setStatus(OwnershipStatus s)` | Mengatur status langsung (untuk mortgage/redeem/bankruptcy) |
| + | `Player*` | `getOwner() const` | Getter pointer ke pemilik |
| + | `std::string` | `getCode() const` | Getter kode properti |
| + | `int` | `getPurchasePrice() const` | Getter harga beli |
| + | `int` | `getMortgageValue() const` | Getter nilai gadai |
| + | `int` | `getFestivalMultiplier() const` | Getter pengganda festival saat ini |
| + | `void` | `setFestivalMultiplier(int m)` | Setter pengganda festival (1/2/4/8) |
| + | `int` | `getFestivalDuration() const` | Getter sisa durasi festival |
| + | `void` | `setFestivalDuration(int d)` | Setter sisa durasi festival (0–3) |

#### Relasi dengan Kelas Lain

1. **parent dari** `StreetProperty`, `RailroadProperty`, `UtilityProperty`
2. **dimiliki oleh** `PropertyTile` (`shared_ptr`)
3. **direferensikan oleh** `Player` (`vector<Property*>`)
4. **diakses oleh** `PropertyManager`, `BankruptcyManager`, `EffectManager` : untuk logika bisnis

---

### 2.20. Kelas StreetProperty

`StreetProperty` adalah jenis properti paling umum dalam permainan, dengan sistem bangunan dan monopoli color group. Selain atribut dari `Property`, ia menyimpan `colorGroup`, harga bangunan (`houseCost`, `hotelCost`), tabel sewa per level (`rentLevels[0..5]`: tanah kosong hingga hotel), dan level bangunan saat ini (`BuildingLevel`). `calculateRent()` mengecek status monopoli (jika owner monopoli dan belum ada bangunan, sewa × 2) lalu mengambil nilai dari `rentLevels` sesuai `buildingLevel`, kemudian mengalikan dengan `festivalMultiplier`. Pembangunan rumah/hotel hanya boleh saat pemain monopoli color group dengan pemerataan antar petak.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::string` | `colorGroup` | Kelompok warna properti (misal "BIRU_TUA", "HIJAU") |
| - | `int` | `houseCost` | Harga membangun satu rumah dari konfigurasi |
| - | `int` | `hotelCost` | Harga upgrade dari 4 rumah ke hotel dari konfigurasi |
| - | `std::vector<int>` | `rentLevels` | Tabel sewa: [0]=tanah, [1]=1rumah, ..., [4]=4rumah, [5]=hotel |
| - | `BuildingLevel` | `buildingLevel` | Level bangunan saat ini: `NONE(0)`, `HOUSE_1–4`, `HOTEL` |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `int` | `calculateRent(const GameContext& ctx) const` | Menghitung sewa: cek monopoli × 2 jika no-build, atau `rentLevels[lvl] × festivalMultiplier` |
| + | `bool` | `canBuild() const` | True jika owner memonopoli color group dan properti tidak digadaikan |
| + | `int` | `getSellValue() const` | `purchasePrice + (jumlahBangunan × houseCost/2)` atau `(hotelCost/2)` jika hotel |
| + | `void` | `buildHouse()` | Menaikkan `buildingLevel` satu level (`NONE→HOUSE_1→...→HOUSE_4`); validasi di `PropertyManager` |
| + | `void` | `buildHotel()` | Mengupgrade dari `HOUSE_4` ke `HOTEL`; validasi semua petak group sudah `HOUSE_4` |
| + | `void` | `demolishBuildings()` | Mereset `buildingLevel` ke `NONE` dan mengembalikan nilai bangunan ke bank (`DemolitionCard`/Bankruptcy) |
| + | `int` | `getBuildingSellValue() const` | Nilai jual bangunan ke bank (setengah harga beli bangunan) |
| + | `std::string` | `getColorGroup() const` | Getter nama color group |
| + | `BuildingLevel` | `getBuildingLevel() const` | Getter level bangunan saat ini |
| + | `int` | `getHouseCost() const` | Getter harga rumah |
| + | `int` | `getHotelCost() const` | Getter harga hotel |

#### Relasi dengan Kelas Lain

1. **extends** `Property` : mewarisi semua atribut dan method dasar properti
2. **dikelola oleh** `PropertyManager` : untuk beli, sewa, bangun, gadai, tebus

---

### 2.21. Kelas RailroadProperty

`RailroadProperty` merepresentasikan empat stasiun kereta dalam permainan. Berbeda dengan Street, Railroad diperoleh secara otomatis tanpa proses beli (pemain pertama yang menginjak langsung menjadi pemilik). `calculateRent()` menghitung sewa berdasarkan jumlah Railroad yang dimiliki pemilik saat ini, nilainya dibaca dari konfigurasi `railroad.txt` via `GameContext`. Semakin banyak Railroad yang dimiliki, sewa semakin tinggi.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::map<int,int>` | `rentByCount` | Pemetaan jumlah railroad yang dimiliki → biaya sewa (dari config) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `int` | `calculateRent(const GameContext& ctx) const` | Hitung jumlah railroad milik owner lewat ctx, lookup di `rentByCount` |
| + | `bool` | `canBuild() const` | Selalu false, Railroad tidak dapat dibangun |
| + | `int` | `getSellValue() const` | `purchasePrice` (tidak ada bangunan, nilai jual = harga beli) |

#### Relasi dengan Kelas Lain

1. **extends** `Property`
2. **uses** `GameContext` : untuk mengakses daftar properti pemilik guna menghitung jumlah railroad

---

### 2.22. Kelas UtilityProperty

`UtilityProperty` merepresentasikan PLN dan PAM. Seperti Railroad, Utility diperoleh otomatis tanpa beli. `calculateRent()` mengalikan total angka dadu pemain yang mendarat (dari `GameContext::diceTotal`) dengan faktor pengali sesuai jumlah Utility yang dimiliki pemilik (dari konfigurasi `utility.txt`). Satu utility: faktor 4; dua utility: faktor 10.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::map<int,int>` | `multiplierByCount` | Pemetaan jumlah utility yang dimiliki → faktor pengali (dari config) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `int` | `calculateRent(const GameContext& ctx) const` | `ctx.diceTotal × multiplierByCount[jumlah utility milik owner]` |
| + | `bool` | `canBuild() const` | Selalu false, Utility tidak dapat dibangun |
| + | `int` | `getSellValue() const` | `purchasePrice` (tidak ada bangunan) |

#### Relasi dengan Kelas Lain

1. **extends** `Property`
2. **uses** `GameContext` : untuk `diceTotal` dan daftar properti pemilik

---

### 2.23. Kelas Card

Kelas abstrak yang mendefinisikan interface universal semua kartu. Setiap kartu memiliki deskripsi teks dan method `apply()` yang dipanggil saat kartu dieksekusi. `apply()` menerima referensi `Player` dan `GameEngine` agar kartu dapat memicu efek seperti memindahkan pemain, berinteraksi dengan bank, atau mempengaruhi pemain lain.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| # | `std::string` | `description` | Teks deskripsi efek kartu yang ditampilkan saat kartu ditarik |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `virtual ~Card()` | `~Card()` | Virtual destructor untuk penghapusan polimorfis |
| + | `virtual void` | `apply(Player& p, GameEngine& game) = 0` | Pure virtual: menerapkan efek kartu pada pemain dan game state |
| + | `std::string` | `getDescription() const` | Getter deskripsi kartu |

#### Relasi dengan Kelas Lain

1. **parent dari** `ActionCard` dan `SkillCard`
2. **dikelola dalam** `Deck<T>` oleh `CardManager`

---

### 2.24. Kelas ActionCard

`ActionCard` adalah kelas menengah untuk kartu Kesempatan dan Dana Umum. Kelas ini tidak menambah atribut baru namun berfungsi sebagai tipe yang dapat digunakan sebagai parameter template `Deck<ActionCard>` sehingga `chanceDeck` dan `communityDeck` terpisah secara tipe dari `skillDeck`. Semua kartu konkret Kesempatan dan Dana Umum extends `ActionCard`.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `virtual void` | `apply(Player& p, GameEngine& game) = 0` | Masih pure virtual, setiap kartu konkret wajib mengimplementasikan |

#### Relasi dengan Kelas Lain

1. **extends** `Card`
2. **parent dari** semua kartu Kesempatan dan Dana Umum konkret
3. **digunakan sebagai** parameter template `Deck<ActionCard>`

---

### 2.25. Kelas GoToNearestRailroadCard

Kartu Kesempatan yang memindahkan pemain ke stasiun kereta terdekat dari posisi saat ini (searah jarum jam). `apply()` mencari tile Railroad terdekat menggunakan `Board::distanceTo()`, memindahkan pemain, memberikan gaji GO jika melewatinya, lalu memicu `onLand()` tile Railroad yang dituju (akuisisi otomatis atau bayar sewa).

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Cari Railroad terdekat, pindahkan pemain, proses landing di Railroad |

#### Relasi dengan Kelas Lain

1. **extends** `ActionCard`
2. **uses** `Board` (via `GameEngine`) untuk mencari railroad terdekat

---

### 2.26. Kelas MoveBackCard

Kartu Kesempatan yang memundurkan posisi pemain sebanyak 3 petak dari posisi saat ini. Pemain tidak mendapatkan gaji GO jika mundur melewati petak GO. Setelah pindah, `onLand()` tile tujuan dipicu.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `steps` | Jumlah petak yang harus dimundurkan (default: 3) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Mundurkan posisi pemain `steps` petak, proses landing di tile baru |

#### Relasi dengan Kelas Lain

1. **extends** `ActionCard`

---

### 2.27. Kelas GoToJailCard

Kartu Kesempatan yang mengirim pemain langsung ke penjara. `apply()` memanggil `JailTile::sendToJail` dan mengakhiri giliran pemain tanpa memberikan gaji GO meski melewati petak GO.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Kirim pemain ke penjara, akhiri giliran |

#### Relasi dengan Kelas Lain

1. **extends** `ActionCard`
2. **uses** `JailTile::sendToJail()` (via `GameEngine`)

---

### 2.28. Kelas BirthdayCard

Kartu Dana Umum yang meminta setiap pemain lain membayar M100 kepada pemain aktif. `apply()` mengiterasi semua pemain aktif lain, memotong M100 dari masing-masing (atau memicu bankruptcy jika tidak mampu), dan menambahkan total ke pemain aktif.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `amountPerPlayer` | Jumlah uang yang diterima dari setiap pemain lain (default: 100) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Iterasi pemain aktif lain, transfer `amountPerPlayer` ke pemain aktif |

#### Relasi dengan Kelas Lain

1. **extends** `ActionCard`
2. **uses** semua Player aktif lain (via `GameEngine`)

---

### 2.29. Kelas DoctorFeeCard

Kartu Dana Umum yang membebankan biaya dokter sebesar M700 kepada pemain ke Bank. Jika pemain tidak mampu membayar, memicu `BankruptcyManager::handleDebt()` dengan Bank sebagai kreditor. `ShieldCard` aktif akan membatalkan efek kartu ini.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `fee` | Besar biaya dokter (default: 700) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Potong fee dari pemain ke bank; jika tidak mampu, trigger `BankruptcyManager`; cek shield terlebih dahulu |

#### Relasi dengan Kelas Lain

1. **extends** `ActionCard`
2. **uses** `BankruptcyManager` (via `GameEngine`)

---

### 2.30. Kelas ElectionCard

Kartu Dana Umum yang mengharuskan pemain membayar M200 kepada setiap pemain lain. `apply()` mengiterasi semua pemain aktif, memindahkan M200 dari pemain aktif ke setiap pemain lain. Jika tidak mampu membayar salah satu, memicu bankruptcy.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `int` | `amountPerPlayer` | Jumlah yang harus dibayar ke setiap pemain lain (default: 200) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Transfer `amountPerPlayer` dari pemain aktif ke setiap pemain lain |

#### Relasi dengan Kelas Lain

1. **extends** `ActionCard`
2. **uses** semua Player aktif lain (via `GameEngine`)

---

### 2.31. Kelas SkillCard

`SkillCard` adalah kelas untuk kartu kemampuan spesial yang dipegang pemain di tangan (maksimal 3 kartu). Berbeda dengan `ActionCard`, `SkillCard` hanya bisa digunakan sebelum melempar dadu dan bersifat sekali pakai. Atribut `value` menyimpan parameter kartu (jumlah langkah untuk `MoveCard`, persentase diskon untuk `DiscountCard`), dan `duration` menyimpan sisa masa berlaku. Tipe `SkillCard` digunakan sebagai parameter `Deck<SkillCard>` di `CardManager`.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| # | `int` | `value` | Parameter nilai kartu: langkah (Move), persentase (Discount), atau 0 (kartu tanpa nilai) |
| # | `int` | `duration` | Sisa durasi berlakunya kartu (hanya relevan untuk `DiscountCard`) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `virtual void` | `apply(Player& p, GameEngine& game) = 0` | Pure virtual: efek spesifik setiap kartu kemampuan |
| + | `virtual bool` | `canUseBeforeRoll() const` | Mengembalikan true, semua `SkillCard` hanya bisa dipakai sebelum dadu |
| + | `int` | `getValue() const` | Getter nilai parameter kartu |
| + | `int` | `getDuration() const` | Getter sisa durasi |
| + | `void` | `setDuration(int d)` | Setter sisa durasi (digunakan `EffectManager`) |
| + | `virtual std::string` | `getTypeName() const = 0` | Mengembalikan nama tipe kartu (untuk save/load dan display) |

#### Relasi dengan Kelas Lain

1. **extends** `Card`
2. **parent dari** `MoveCard`, `DiscountCard`, `ShieldCard`, `TeleportCard`, `LassoCard`, `DemolitionCard`
3. **digunakan sebagai** parameter template `Deck<SkillCard>`
4. **disimpan dalam** `Player::handCards`

---

### 2.32. Kelas MoveCard

Kartu kemampuan yang memindahkan pemain maju sejumlah petak. Nilai langkah (`value`) ditentukan secara acak saat kartu pertama kali didistribusikan ke pemain. `apply()` memanggil `GameEngine::moveCurrentPlayer(value)` yang juga menangani deteksi lewat GO dan landing di tile baru.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Pindahkan pemain maju `value` petak, proses landing normal |
| + | `std::string` | `getTypeName() const` | Mengembalikan `"MoveCard"` |

#### Relasi dengan Kelas Lain

1. **extends** `SkillCard`

---

### 2.33. Kelas DiscountCard

Kartu kemampuan yang memberikan diskon persentase acak selama 1 giliran. Persentase (`value`) ditentukan saat kartu didistribusikan. `apply()` mengaktifkan `discountPercent` pada `Player` dan `duration`. `EffectManager::onTurnEnd()` mengurangi duration dan mereset diskon saat habis. Diskon diterapkan pada transaksi pembelian properti dalam giliran aktif.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Set `p.discountPercent = value` dan `duration = 1` |
| + | `std::string` | `getTypeName() const` | Mengembalikan `"DiscountCard"` |

#### Relasi dengan Kelas Lain

1. **extends** `SkillCard`
2. Efeknya dikelola oleh `EffectManager`

---

### 2.34. Kelas ShieldCard

Kartu kemampuan yang melindungi pemain dari semua tagihan dan sanksi merugikan selama 1 giliran. `apply()` mengaktifkan `p.shieldActive`. Setiap kali ada potongan uang (sewa, pajak, biaya kartu), sistem mengecek `shieldActive` terlebih dahulu; jika true, tagihan dibatalkan. `EffectManager::onTurnEnd()` mereset `shieldActive` ke false.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Set `p.shieldActive = true` untuk giliran ini |
| + | `std::string` | `getTypeName() const` | Mengembalikan `"ShieldCard"` |

#### Relasi dengan Kelas Lain

1. **extends** `SkillCard`
2. Efeknya diperiksa di `PropertyManager`, `TaxTile`, `CardTile` sebelum memotong uang

---

### 2.35. Kelas TeleportCard

Kartu kemampuan yang memberikan pemain kebebasan berpindah ke petak manapun. `apply()` meminta pemain memasukkan kode tile tujuan, lalu memanggil `GameEngine::handleLanding()` di tile tersebut tanpa memberikan gaji GO. Validasi kode tile dilakukan sebelum pemindahan.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Minta input kode tile, validasi, teleport pemain, proses landing di tujuan |
| + | `std::string` | `getTypeName() const` | Mengembalikan `"TeleportCard"` |

#### Relasi dengan Kelas Lain

1. **extends** `SkillCard`
2. **uses** `GameEngine` untuk validasi tile dan teleport

---

### 2.36. Kelas LassoCard

Kartu kemampuan yang menarik satu pemain lawan yang posisinya di depan pemain aktif ke posisi pemain aktif saat ini. `apply()` menampilkan daftar pemain yang posisinya di depan, meminta pilihan, lalu memindahkan posisi lawan ke posisi pemain aktif. Pemain yang ditarik tidak memproses landing di posisi baru.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Tampilkan kandidat lawan, minta pilihan, pindahkan posisi lawan ke posisi pemain aktif |
| + | `std::string` | `getTypeName() const` | Mengembalikan `"LassoCard"` |

#### Relasi dengan Kelas Lain

1. **extends** `SkillCard`
2. **uses** `Player` lain dan `Board`

---

### 2.37. Kelas DemolitionCard

Kartu kemampuan yang menghancurkan bangunan pada satu properti milik pemain lawan. `apply()` menampilkan daftar properti lawan yang memiliki bangunan, meminta pilihan, lalu memanggil `StreetProperty::demolishBuildings()` untuk menghancurkan semua bangunan di properti tersebut. Nilai bangunan dikembalikan ke bank.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `apply(Player& p, GameEngine& game)` | Tampilkan properti lawan berisi bangunan, pilih, demolish bangunan, kembalikan nilai ke bank |
| + | `std::string` | `getTypeName() const` | Mengembalikan `"DemolitionCard"` |

#### Relasi dengan Kelas Lain

1. **extends** `SkillCard`
2. **uses** `StreetProperty::demolishBuildings()`

---

### 2.38. Kelas Deck\<T\>

`Deck<T>` adalah generic class template yang mengelola tumpukan kartu untuk semua jenis kartu (`T = ActionCard` atau `SkillCard`). Ia memisahkan `drawPile` (kartu siap diambil) dan `discardPile` (kartu sudah dipakai). Saat `drawPile` habis, `refillFromDiscardIfNeeded()` memindahkan isi `discardPile` ke `drawPile` dan mengocoknya ulang. Penggunaan template menghindari duplikasi kode untuk setiap jenis deck.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::vector<std::shared_ptr<T>>` | `drawPile` | Tumpukan kartu yang siap untuk diambil |
| - | `std::vector<std::shared_ptr<T>>` | `discardPile` | Tumpukan kartu yang sudah digunakan dan menunggu dikocok ulang |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `shuffle()` | Mengocok `drawPile` secara acak menggunakan `std::shuffle` |
| + | `std::shared_ptr<T>` | `draw()` | Mengambil kartu teratas dari `drawPile`; panggil refill jika kosong |
| + | `void` | `discard(std::shared_ptr<T> card)` | Memindahkan kartu ke `discardPile` setelah digunakan |
| + | `void` | `refillFromDiscardIfNeeded()` | Jika `drawPile` kosong: pindahkan `discardPile` ke `drawPile` dan kocok ulang |
| + | `int` | `size() const` | Mengembalikan jumlah kartu di `drawPile` |
| + | `void` | `addCard(std::shared_ptr<T> card)` | Menambahkan kartu ke `drawPile` (digunakan saat inisialisasi) |
| + | `std::vector<std::shared_ptr<T>>` | `getAllCards() const` | Mengembalikan semua kartu (`drawPile` + `discardPile`) untuk save/load |

#### Relasi dengan Kelas Lain

1. Parameter `T`: `ActionCard` atau `SkillCard`
2. **dimiliki oleh** `CardManager`: tiga instance `chanceDeck`, `communityDeck`, `skillDeck`

---

### 2.39. Kelas PropertyManager

`PropertyManager` mengeksekusi semua logika terkait kepemilikan properti agar tidak menumpuk di `GameEngine`. Ia menangani seluruh siklus kepemilikan, yaitu penawaran beli Street, akuisisi otomatis Railroad/Utility, pembayaran sewa (dengan polimorfisme `calculateRent()`), pembangunan rumah/hotel dengan validasi pemerataan, serta gadai dan tebus. Semua transaksi uang diteruskan ke `Bank`, dan setiap aksi dicatat ke `TransactionLogger`.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `Bank&` | `bank` | Referensi ke `Bank` untuk semua transaksi uang properti |
| - | `AuctionManager&` | `auctionManager` | Referensi ke `AuctionManager` untuk memicu lelang saat perlu |
| - | `BankruptcyManager&` | `bankruptcyManager` | Referensi ke `BankruptcyManager` untuk memicu proses bankruptcy |
| - | `TransactionLogger&` | `logger` | Referensi ke logger untuk mencatat semua transaksi properti |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `tryBuyStreet(Player& p, StreetProperty& prop)` | Tampilkan akta, minta konfirmasi; jika beli: transfer uang dan kepemilikan; jika tidak: trigger lelang |
| + | `void` | `acquireAutoOwnedProperty(Player& p, Property& prop)` | Akuisisi Railroad/Utility otomatis ke pemain tanpa biaya |
| + | `void` | `payRent(Player& payer, Player& owner, Property& prop, int diceTotal)` | Hitung sewa via `calculateRent(ctx)`, transfer dari payer ke owner; jika gagal: trigger bankruptcy |
| + | `void` | `mortgageProperty(Player& p, Property& prop)` | Validasi tidak ada bangunan, jual bangunan jika ada, gadaikan properti, transfer nilai gadai |
| + | `void` | `redeemProperty(Player& p, Property& prop)` | Validasi saldo cukup, bayar harga beli ke bank, ubah status ke `OWNED` |
| + | `void` | `buildOnStreet(Player& p, StreetProperty& prop)` | Validasi monopoli dan pemerataan, potong biaya bangunan, naikkan `buildingLevel` |
| + | `void` | `sellBuildingsInGroup(Player& p, const std::string& colorGroup)` | Menjual semua bangunan dalam satu color group ke bank sebelum gadai |
| + | `bool` | `ownsFullColorGroup(const Player& p, const std::string& color) const` | Mengecek apakah pemain memiliki semua Street dalam color group tertentu |
| + | `std::vector<StreetProperty*>` | `getBuildableStreets(const Player& p, const std::string& color)` | Mengembalikan daftar Street dalam color group yang memenuhi syarat pemerataan untuk dibangun |
| + | `int` | `countRailroadsOwned(const Player& p) const` | Menghitung jumlah Railroad milik pemain (untuk `GameContext`) |
| + | `int` | `countUtilitiesOwned(const Player& p) const` | Menghitung jumlah Utility milik pemain (untuk `GameContext`) |

#### Relasi dengan Kelas Lain

1. **uses** `Bank` untuk semua transaksi uang properti
2. **uses** `AuctionManager` untuk memicu lelang saat pemain menolak beli atau tidak mampu
3. **uses** `BankruptcyManager` untuk memicu bankruptcy jika sewa tidak mampu dibayar
4. **uses** `TransactionLogger` untuk mencatat beli, sewa, bangun, gadai, tebus
5. **digunakan oleh** `GameEngine`: dipanggil dari `handleLanding()` dan `processCommand()`

---

### 2.40. Kelas AuctionManager

`AuctionManager` menjalankan mekanisme lelang properti yang terdiri dari putaran bidding antar pemain aktif. `runAuction()` mengatur urutan bid/pass mulai dari pemain setelah trigger player, memvalidasi bahwa bid tidak melebihi saldo pemain, mendeteksi kondisi akhir lelang (pass berturut-turut sejumlah n-1 pemain), dan memindahkan kepemilikan properti ke pemenang. Minimal satu pemain harus bid sebelum lelang berakhir.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `TransactionLogger&` | `logger` | Referensi ke logger untuk mencatat setiap bid dan hasil lelang |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `runAuction(Property& prop, std::vector<Player*>& activePlayers, Player* triggerPlayer)` | Menjalankan satu sesi lelang lengkap dari awal hingga pemenang ditetapkan |
| - | `int` | `getStartIndex(const std::vector<Player*>& players, Player* trigger)` | Menentukan index awal bidding (pemain setelah trigger player) |
| - | `bool` | `isAuctionOver(int consecutivePasses, int nPlayers)` | Cek kondisi akhir: pass berturut >= n-1 dan minimal ada satu bid |
| - | `void` | `transferProperty(Property& prop, Player* winner, int finalBid)` | Memindahkan kepemilikan ke pemenang dan memotong saldo sesuai bid akhir |

#### Relasi dengan Kelas Lain

1. **uses** `Player` untuk operasi bid dan transfer kepemilikan
2. **uses** `Property` untuk memindahkan status dan kepemilikan
3. **uses** `TransactionLogger` untuk mencatat setiap bid dan hasil akhir lelang
4. **digunakan oleh** `PropertyManager` dan `BankruptcyManager`

---

### 2.41. Kelas BankruptcyManager

`BankruptcyManager` menangani seluruh proses kebangkrutan. `handleDebt()` pertama menghitung estimasi dana maksimum dari likuidasi total aset. Jika dapat menutup kewajiban, pemain diarahkan ke panel likuidasi interaktif. Jika tidak, `declareBankrupt()` dieksekusi. Jika kreditor pemain lain, semua aset diserahkan ke kreditor; jika Bank, uang hilang, properti kembali ke status `BANK` dan dilelang satu per satu.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `Bank&` | `bank` | Referensi ke `Bank` untuk transaksi saat likuidasi ke bank |
| - | `AuctionManager&` | `auctionManager` | Referensi untuk menjalankan lelang properti saat bangkrut ke bank |
| - | `TransactionLogger&` | `logger` | Referensi ke logger untuk mencatat semua aksi kebangkrutan |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `bool` | `canCoverDebt(const Player& p, int debt) const` | True jika uang tunai + estimasi likuidasi >= debt |
| + | `int` | `estimateLiquidationValue(const Player& p) const` | Menghitung maksimum dana yang bisa diperoleh dari jual/gadai semua aset |
| + | `void` | `handleDebt(Player& p, int amount, Creditor creditor, GameEngine& game)` | Entry point: cek likuidasi possible, arahkan ke panel likuidasi atau langsung declare bankrupt |
| + | `void` | `liquidateAssets(Player& p, int targetAmount, GameEngine& game)` | Menampilkan panel likuidasi interaktif: pemain memilih jual/gadai hingga kewajiban terpenuhi |
| + | `void` | `declareBankrupt(Player& p, Creditor creditor, GameEngine& game)` | Menyatakan pemain bangkrut, transfer aset ke kreditor atau bank, lelang properti jika perlu |
| - | `void` | `transferAssetsToBankruptCreditor(Player& bankrupt, Player& creditor)` | Mentransfer semua properti dan uang sisa ke pemain kreditor |
| - | `void` | `returnAssetsToBank(Player& bankrupt, GameEngine& game)` | Mengembalikan properti ke status `BANK` dan menjalankan lelang satu per satu |

#### Relasi dengan Kelas Lain

1. **uses** `Bank` untuk transaksi saat bangkrut ke bank
2. **uses** `AuctionManager` untuk lelang properti pasca-bankruptcy ke bank
3. **uses** `Player` sebagai subjek dan kreditor
4. **uses** `TransactionLogger` untuk mencatat seluruh proses kebangkrutan
5. **digunakan oleh** `GameEngine`: dipanggil dari berbagai titik saat pemain gagal bayar

---

### 2.42. Kelas CardManager

`CardManager` memegang semua deck kartu (`chanceDeck`, `communityDeck`, `skillDeck`) dan menyediakan operasi pengambilan serta penerapan kartu. `drawChanceCard()` dan `drawCommunityCard()` menarik kartu dari deck yang sesuai dan langsung mengeksekusi `apply()`. `drawSkillCard()` dipanggil di awal setiap giliran untuk semua pemain; jika pemain sudah memiliki 3 kartu dan mendapat kartu ke-4, `handleExcessSkillCards()` memicu prompt drop secara otomatis. Penggunaan `Deck<ActionCard>` dan `Deck<SkillCard>` mendemonstrasikan generic class template dalam satu kelas.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `Deck<ActionCard>` | `chanceDeck` | Tumpukan kartu Kesempatan (3 jenis kartu) |
| - | `Deck<ActionCard>` | `communityDeck` | Tumpukan kartu Dana Umum (3 jenis kartu) |
| - | `Deck<SkillCard>` | `skillDeck` | Tumpukan kartu Kemampuan Spesial (15 kartu total) |
| - | `TransactionLogger&` | `logger` | Referensi ke logger untuk mencatat pengambilan dan penggunaan kartu |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `drawChanceCard(Player& p, GameEngine& game)` | Tarik dari `chanceDeck`, tampilkan deskripsi, panggil `apply()`, buang ke discard |
| + | `void` | `drawCommunityCard(Player& p, GameEngine& game)` | Tarik dari `communityDeck`, tampilkan deskripsi, panggil `apply()`, buang ke discard |
| + | `void` | `drawSkillCard(Player& p)` | Tarik dari `skillDeck`, tambahkan ke tangan pemain; jika >3, panggil `handleExcess` |
| + | `void` | `handleExcessSkillCards(Player& p)` | Tampilkan daftar 4 kartu di tangan, minta pemain membuang 1, eksekusi drop |
| + | `void` | `useSkillCard(Player& p, int idx, GameEngine& game)` | Gunakan kartu pada index `idx`: panggil `apply()`, hapus dari tangan, buang ke discard |
| + | `void` | `initializeDecks()` | Membuat dan mengisi semua deck dengan kartu yang sesuai saat inisialisasi game |
| + | `std::vector<std::string>` | `getSkillDeckStateForSave() const` | Mengembalikan state deck untuk kebutuhan save/load |

#### Relasi dengan Kelas Lain

1. **has-a** `Deck<ActionCard>` (`chanceDeck`, `communityDeck`): generic template
2. **has-a** `Deck<SkillCard>` (`skillDeck`): generic template
3. **uses** `Player`: sebagai penerima dan pengguna kartu
4. **uses** `TransactionLogger`: mencatat semua aktivitas kartu
5. **digunakan oleh** `GameEngine` dan `CardTile`

---

### 2.43. Kelas EffectManager

`EffectManager` mengelola seluruh efek sementara yang berlaku lintas giliran agar logika durasi tidak tersebar di berbagai tempat. Bertanggung jawab atas penguatan/pengurangan `festivalMultiplier` dan `festivalDuration` pada `Property`, reset flag per giliran (`hasUsedSkillThisTurn`, `shieldActive`, `consecutiveDoubles`), dan penerapan/penghapusan `discountPercent` dari `DiscountCard`. `onTurnStart()` dipanggil di awal giliran setiap pemain, `onTurnEnd()` dipanggil di akhir.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `onTurnStart(Player& p, GameEngine& game)` | Reset flag per giliran pemain aktif: `hasUsedSkill`, `consecutiveDoubles` |
| + | `void` | `onTurnEnd(Player& p, GameEngine& game)` | Dekremen durasi efek aktif (festival, discount), reset efek yang habis |
| + | `void` | `decrementTemporaryEffects(Player& p)` | Mengurangi `festivalDuration` semua properti milik pemain; reset multiplier jika 0 |
| + | `void` | `applyFestival(Player& p, Property& prop)` | Menerapkan atau memperkuat efek festival pada properti yang dipilih pemain |
| + | `void` | `resetShield(Player& p)` | Mereset `shieldActive` ke false di akhir giliran |
| + | `void` | `applyDiscount(Player& p)` | Mengaktifkan `discountPercent` dari `DiscountCard` dan mengatur durasinya |
| + | `void` | `decrementDiscount(Player& p)` | Mengurangi durasi discount; reset ke 0 jika habis |

#### Relasi dengan Kelas Lain

1. **uses** `Player` : untuk membaca/mengubah flag dan daftar properti
2. **uses** `Property` : untuk mengelola festival multiplier dan durasi
3. **digunakan oleh** `GameEngine` : dipanggil `onTurnStart` dan `onTurnEnd` setiap giliran
4. **digunakan oleh** `FestivalTile` : untuk `applyFestival()`

---

### 2.44. Kelas TransactionLogger

`TransactionLogger` mencatat setiap kejadian signifikan permainan secara otomatis ke dalam `vector<LogEntry>`. Logger bekerja di semua `GameEngine`, Manager, dan Tile yang relevan memanggil `log()` setiap kali terjadi transaksi uang, pergerakan, penggunaan kartu, lelang, festival, bankruptcy, dan save/load. `printAll()` digunakan untuk `CETAK_LOG` tanpa argumen, `printLastN(n)` untuk `CETAK_LOG N`. Saat game di-save, `getEntries()` digunakan oleh `GameStateSerializer`.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| - | `std::vector<LogEntry>` | `entries` | Daftar semua entri log selama sesi permainan berlangsung |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `log(int turn, const std::string& user, const std::string& action, const std::string& detail)` | Menambahkan `LogEntry` baru ke `entries` |
| + | `void` | `printAll() const` | Menampilkan seluruh `entries` ke layar dengan format standar |
| + | `void` | `printLastN(int n) const` | Menampilkan n entri terakhir dari `entries` |
| + | `const std::vector<LogEntry>&` | `getEntries() const` | Getter referensi ke seluruh `entries` untuk keperluan save/load |
| + | `void` | `loadEntries(const std::vector<LogEntry>& saved)` | Memuat `entries` dari file save saat load game |
| + | `void` | `clear()` | Menghapus semua `entries` (saat memulai game baru) |

#### Relasi dengan Kelas Lain

1. **stores** `LogEntry` : satu entri per kejadian
2. **dipanggil oleh** semua Manager dan `GameEngine` : untuk pencatatan aksi
3. **diakses oleh** `GameStateSerializer` untuk menyertakan log dalam file save

---

### 2.45. Kelas ConfigLoader

`ConfigLoader` bertugas membaca dan memparsing semua file konfigurasi (`.txt`) saat program pertama kali dijalankan. Setiap method load bertanggung jawab atas satu file konfigurasi dan mengembalikan struct data yang kemudian digunakan `GameEngine` untuk membangun `Board` dan menginisialisasi semua objek. `ConfigLoader` tidak menyentuh logika game, hanya mengubah teks menjadi struktur C++ terstruktur.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `std::vector<PropertyDef>` | `loadProperties(const std::string& path)` | Membaca `property.txt`: ID, kode, nama, jenis, warna, harga, nilai gadai, harga bangunan, tabel sewa |
| + | `std::map<int,int>` | `loadRailroadConfig(const std::string& path)` | Membaca `railroad.txt`: pemetaan jumlah railroad → biaya sewa |
| + | `std::map<int,int>` | `loadUtilityConfig(const std::string& path)` | Membaca `utility.txt`: pemetaan jumlah utility → faktor pengali |
| + | `TaxConfig` | `loadTaxConfig(const std::string& path)` | Membaca `tax.txt`: `PPH_FLAT`, `PPH_PERSENTASE`, `PBM_FLAT` |
| + | `SpecialConfig` | `loadSpecialConfig(const std::string& path)` | Membaca `special.txt`: `GO_SALARY`, `JAIL_FINE` |
| + | `MiscConfig` | `loadMiscConfig(const std::string& path)` | Membaca `misc.txt`: `MAX_TURN`, `SALDO_AWAL` |
| - | `std::vector<std::string>` | `splitLine(const std::string& line)` | Utility: memecah baris teks menjadi token berdasarkan whitespace |
| - | `void` | `validateConfig(const std::vector<PropertyDef>& defs)` | Memvalidasi konsistensi data konfigurasi sebelum digunakan |

#### Relasi dengan Kelas Lain

1. **digunakan oleh** `GameEngine` saat `startNewGame()` untuk menginisialisasi semua komponen
2. Tidak bergantung pada kelas lain, murni membaca file dan mengembalikan struct data

---

### 2.46. Kelas SaveLoadManager

`SaveLoadManager` mengatur operasi simpan dan muat state permainan ke/dari file `.txt`. `save()` meminta `GameStateSerializer` untuk mengubah seluruh state menjadi format teks, lalu menulisnya ke file, tetapi hanya boleh dipanggil di awal giliran sebelum aksi apapun. `load()` membaca file dan menyerahkan kontennya ke `GameStateSerializer::deserialize()` untuk merestorasi state `GameEngine`. `SaveLoadManager` juga menangani kasus edge, seperti file sudah ada, file tidak ditemukan, dan file rusak.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `void` | `save(const GameEngine& game, const std::string& filename)` | Serialisasi state via `GameStateSerializer` dan tulis ke file; cek overwrite jika ada |
| + | `void` | `load(GameEngine& game, const std::string& filename)` | Baca file, deserialisi via `GameStateSerializer`, restorasi state `GameEngine` |
| - | `bool` | `fileExists(const std::string& filename) const` | Mengecek apakah file dengan nama tersebut sudah ada |
| - | `bool` | `promptOverwrite(const std::string& filename) const` | Menampilkan prompt konfirmasi timpa file yang sudah ada |
| - | `std::string` | `readFile(const std::string& filename) const` | Membaca seluruh isi file ke string; melempar exception jika gagal |
| - | `void` | `writeFile(const std::string& filename, const std::string& data) const` | Menulis string data ke file; melempar exception jika gagal |

#### Relasi dengan Kelas Lain

1. **has-a** `GameStateSerializer` untuk mendelegasikan format teks ke serializer
2. **menggunakan** `GameEngine` untuk membaca state saat save, merestorasi state saat load
3. **digunakan oleh** `GameEngine`, dipanggil dari `processCommand()` untuk `SIMPAN` dan `MUAT`

---

### 2.47. Kelas GameStateSerializer

`GameStateSerializer` memisahkan logika format teks dari `SaveLoadManager`. `serialize()` mengiterasi semua komponen state `GameEngine` (turn, pemain, properti, deck, log) dan menghasilkan string dengan format persis sesuai spesifikasi Save/Load. `deserialize()` melakukan kebalikannya: memparsing string tersebut baris per baris dan merestorasi semua objek ke state tersimpan. Pemisahan ini memudahkan perubahan format save di masa depan tanpa mengubah `SaveLoadManager`.

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `std::string` | `serialize(const GameEngine& game)` | Mengubah seluruh state `GameEngine` menjadi string teks terstruktur sesuai format spesifikasi |
| + | `void` | `deserialize(const std::string& data, GameEngine& game)` | Memparsing string data dan merestorasi state ke `GameEngine` |
| - | `std::string` | `serializePlayers(const std::vector<Player>& players)` | Menghasilkan blok teks state semua pemain |
| - | `std::string` | `serializeProperties(const Board& board)` | Menghasilkan blok teks state semua properti |
| - | `std::string` | `serializeDeck(const CardManager& cm)` | Menghasilkan blok teks state deck kartu kemampuan |
| - | `std::string` | `serializeLog(const TransactionLogger& logger)` | Menghasilkan blok teks semua entri log |
| - | `void` | `parsePlayers(std::istringstream& ss, GameEngine& game)` | Memparsing blok pemain dari stream dan merestorasi Player objects |
| - | `void` | `parseProperties(std::istringstream& ss, GameEngine& game)` | Memparsing blok properti dan mengupdate status semua `Property` di `Board` |
| - | `void` | `parseLog(std::istringstream& ss, GameEngine& game)` | Memparsing blok log dan merestorasi entries ke `TransactionLogger` |

#### Relasi dengan Kelas Lain

1. **digunakan oleh** `SaveLoadManager`
2. **menggunakan** `GameEngine`, `Board`, `Player`, `CardManager`, `TransactionLogger` sebagai sumber data serialize dan target deserialize

---

### 2.48. Kelas Command

`Command` merepresentasikan satu perintah dari pemain setelah diparse oleh `CommandParser`. Ia menyimpan `CommandType` (enum untuk setiap jenis perintah) dan `vector<string>` argumen tambahan. `GameEngine` menerima `Command` dan mendispatch ke handler yang sesuai berdasarkan `type`.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `CommandType` | `type` | Jenis perintah (enum: `PRINT_BOARD`, `ROLL_DICE`, `BUILD`, `SAVE`, dst.) |
| + | `std::vector<std::string>` | `args` | Argumen tambahan (misal: kode properti, nama file, nilai dadu) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | | `Command(CommandType t, std::vector<std::string> a)` | Konstruktor utama dengan tipe dan argumen |
| + | `bool` | `hasArgs() const` | True jika vector `args` tidak kosong |

#### Relasi dengan Kelas Lain

1. **diproduksi oleh** `CommandParser`
2. **digunakan oleh** `GameEngine` untuk dispatch ke handler

---

### 2.49. Kelas LogEntry

`LogEntry` menyimpan satu entri log permainan: nomor turn, username pemain yang melakukan aksi, jenis aksi (sebagai string), dan detail aksi lengkap termasuk nilai uang jika ada. Digunakan secara internal oleh `TransactionLogger` dan oleh `GameStateSerializer` saat menyimpan log ke file.

#### Atribut

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `int` | `turn` | Nomor turn saat entri ini dicatat |
| + | `std::string` | `username` | Username pemain yang melakukan aksi |
| + | `std::string` | `action` | Jenis aksi (misal: `"DADU"`, `"BELI"`, `"SEWA"`, `"KARTU"`) |
| + | `std::string` | `detail` | Detail aksi lengkap (misal: `"Lempar: 3+4=7, mendarat di BDG"`) |

#### Method

| Visibilitas | Tipe | Nama | Keterangan |
|---|---|---|---|
| + | `std::string` | `toString() const` | Menghasilkan string format tampilan: `"[Turn N] User | ACTION | detail"` |

#### Relasi dengan Kelas Lain

1. **disimpan dalam** `TransactionLogger`
2. **digunakan oleh** `GameStateSerializer` untuk serialisasi log

---

## 3. Justifikasi

### 3.1. Alasan Pemilihan Desain

Desain kelas dibangun atas tiga prinsip utama yaitu **Layered Architecture**, **Single Responsibility Principle**, dan **Composition over Inheritance**. Prinsip ini dipilih sebagai fondasi karena spesifikasi permainan memiliki banyak mekanisme yang kompleks dan saling berinteraksi, sehingga pemisahan yang jelas antar tanggung jawab menjadi kunci utama agar kode tetap dapat dipahami, dikembangkan, dan diuji secara independen.

**Layered Architecture (UI → Core → Data Access)** dipilih karena memisahkan input/output dari logika permainan. Tanpa pemisahan ini, setiap perubahan tampilan (misalnya menambahkan pewarnaan baru di papan) akan berisiko merusak logika game. Sebaliknya, dengan lapisan yang bersih, lapisan UI dapat diganti dengan GUI sepenuhnya tanpa menyentuh Core Layer sama sekali.

Pemilihan inheritance pada hierarki `Tile`, `Property`, dan `Card` didasari oleh kebutuhan polimorfisme. `GameEngine` tidak perlu tahu tipe konkret tile saat memproses landing, `PropertyManager` tidak perlu tahu apakah properti adalah Street/Railroad/Utility saat menghitung sewa, dan `CardManager` tidak perlu tahu efek spesifik setiap kartu.

### 3.2. Kelebihan Desain Kelas

**Modularitas tinggi:** Setiap kelas memiliki satu tanggung jawab yang jelas. Penambahan jenis tile baru (misalnya `PajakBonusTile`) cukup membuat subclass `Tile` baru dengan `onLand()` yang sesuai, tanpa mengubah `GameEngine`, `Board`, atau tile lain. Hal yang sama berlaku untuk kartu baru atau jenis properti baru.

**Polimorfisme yang bermakna:** Ketiga hierarki utama (`Tile`, `Property`, `Card`) menggunakan virtual function secara logis sesuai use case, bukan sekadar formalitas. `onLand()` di `Tile`, `calculateRent()` di `Property`, dan `apply()` di `Card` masing-masing adalah titik ekstensi yang nyata.

**Reusability via Generic Class:** `Deck<T>` menghindari duplikasi kode untuk `chanceDeck`, `communityDeck`, dan `skillDeck`. Tanpa template, harus ada tiga kelas deck yang hampir identik dengan hanya perbedaan tipe elemen.

**Testability:** Karena setiap Manager berdiri sendiri dan menerima dependensi lewat referensi (bukan membuat sendiri), setiap Manager dapat diuji secara unit dengan mock/stub untuk dependensinya.

**Pemisahan Data dari Posisi (Property vs Tile):** Memisahkan `PropertyTile` (posisi di papan) dari `Property` (data aset) adalah keputusan desain yang penting. Data kepemilikan, bangunan, dan efek festival pada `Property` harus tetap ada dan konsisten terlepas dari posisi pemain; pemisahan ini memungkinkan hal tersebut.

**Operator Overloading yang semantis:** Penggunaan `operator+=` dan `operator-=` pada `Player` untuk operasi uang membuat kode seperti `player += salary` dan `player -= rent` lebih ekspresif dan lebih mudah dibaca dibandingkan `player.addMoney(salary)` yang panjang, sambil tetap menjaga enkapsulasi.

### 3.3. Kekurangan Desain Kelas

**Ketergantungan silang antar Manager:** `PropertyManager` perlu referensi ke `AuctionManager` dan `BankruptcyManager`; `BankruptcyManager` perlu referensi ke `AuctionManager`. Ketergantungan ini wajar secara domain bisnis (proses beli properti memang bisa berujung lelang yang bisa berujung bankruptcy), namun meningkatkan coupling antar Manager. Solusi ideal adalah menggunakan Observer Pattern atau Event System, namun hal tersebut meningkatkan kompleksitas yang mungkin tidak sebanding untuk skala proyek ini.

**GameEngine masih menjadi pusat hubung:** Ia tetap menjadi titik temu yang mengetahui semua Manager sekaligus. Untuk skala proyek yang lebih besar, bisa dipertimbangkan pola Mediator yang lebih formal, namun untuk proyek ini dinilai sudah cukup bersih.

**`Property::calculateRent()` membutuhkan `GameContext` yang cukup besar:** `GameContext` menyertakan referensi ke `Board` dan seluruh `vector<Player>` agar subclass dapat menghitung kepemilikan. Ini berarti setiap panggilan `calculateRent()` membawa overhead berupa referensi ke struktur besar. Alternatifnya adalah meneruskan nilai spesifik (jumlah railroad, total dadu) sebagai parameter terpisah, namun pendekatan itu akan memecah polimorfisme karena setiap subclass butuh parameter yang berbeda.

**Kompleksitas `BankruptcyManager`:** Mekanisme likuidasi interaktif (panel jual/gadai) menjadikan `BankruptcyManager` sebagai kelas yang paling kompleks dalam desain. Alternatif yang lebih bersih adalah memisahkan `LiquidationPanel` sebagai kelas UI tersendiri, namun hal ini menambah jumlah kelas yang perlu dikelola.

### 3.4. Kendala Dalam Pemilihan Desain

**Dilema UI Otomatis vs Interaktif:** Beberapa aksi dalam spesifikasi bersifat otomatis namun tetap memerlukan interaksi pengguna. Cara yang diambil adalah Core Logic menentukan apa yang harus terjadi (logika bisnis), sementara UI Layer menampilkan prompt dan mengambil input, lalu hasilnya diteruskan kembali ke Core. Hal ini memerlukan sedikit coupling terkontrol antara Manager dan `GameUI`.

**Penempatan Logika Penjara:** Mekanisme penjara melibatkan tiga kelas (`JailTile`, `GoToJailTile`, `GoToJailCard`) dan juga logika giliran. Logika giliran tetap di `GameEngine::handleJailTurn()` karena terkait erat dengan `TurnManager`, sementara `JailTile` hanya menyimpan state dan menyediakan `sendToJail()`.

**`Shared_ptr` vs Raw Pointer untuk Property:** `Property` dipegang oleh `PropertyTile` (`shared_ptr`) sekaligus direferensikan oleh `Player` (raw pointer). Ini menyebabkan mixed ownership. Keputusan diambil karena `PropertyTile` adalah pemilik `Property`, sementara `Player` hanya perlu referensi; raw pointer lebih ringan dan tepat untuk referensi non-owning. Konsekuensinya, tim harus berhati-hati agar tidak ada dangling pointer saat pemain bangkrut.

---

## 4. Lampiran

- Furap_Class_Diagram.pdf
