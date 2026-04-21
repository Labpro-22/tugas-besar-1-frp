# Spesifikasi Tugas Besar 1: Nimonspoli

**IF2010 Pemrograman Berorientasi Objek**
**Version history:**
Revisi 1: 06 April 2026- Perubahan deadline M1

## Daftar Isi

Daftar Isi........................................................................................................................................................................2
Deskripsi Persoalan...................................................................................................................................................4
Alur Program............................................................................................................................................................... 5
Konfigurasi................................................................................................................................................................... 5
Mekanisme...................................................................................................................................................................7
Papan Permainan..........................................................................................................................................................7
PETAK PROPERTI................................................................................................................................................ 8
PETAK AKSI............................................................................................................................................................ 9
Petak Spesial................................................................................................................................................................11
Dadu dan Giliran Bermain......................................................................................................................................12
Kepemilikan Properti..............................................................................................................12
Street(e.g., BDG, DEN, MTR, dst.).................................................................................13
Railroad(e.g., TUG, GUB)...............................................................................................13
Utility(PLN dan PAM)...................................................................................................... 13
Ketentuan Sewa.................................................................................................................... 13
Gadai..................................................................................................................................... 14
Peningkatan Properti...............................................................................................................................................15
Lelang Tanah.................................................................................................................................................................15
Mekanisme Lelang............................................................................................................................................. 15
Pa j ak................................................................................................................................................................................16
Festival...........................................................................................................................................................................16
Kartu Kesem p atan dan Dana Umum................................................................................................................. 17
Kartu Kesem p atan.............................................................................................................................................17
Kartu Dana Umum.............................................................................................................................................17
Kartu Kemam p uan S p esial.....................................................................................................................................18
Aturan Kartu Kemam p uan S p esial..............................................................................................................18
Jenis Kartu Kemam p uan S p esial..................................................................................................................18
Ban g krut........................................................................................................................................................................18
Mekanisme Likuidasi Aset............................................................................................... 19
Pen g ambilalihan Asset.............................................................................................................................................19
Kondisi Game Selesai............................................................................................................................................... 19
MAX TURN...........................................................................................................................................................20
BANKRUPTCY.................................................................................................................................................... 20
Save/Load......................................................................................................................................................................20
Transaction Lo gg er....................................................................................................................................................22
O p erasi dan Perintah..............................................................................................................................................23

1. CETAK_PAPAN................................................................................................................. 23
2. LEMPAR_DAD U ................................................................................................................ 23
3. ATUR_DADU X Y.............................................................................................................. 24
4. CETAK_AKTA..........................................................................................................................................................24
5. CETAK_PROPERTI............................................................................................................................................... 25
6. BELI(OTOMATIS)................................................................................................................................................. 25
7. BAYAR_SEWA(OTOMATIS)............................................................................................................................. 26
8. BAYAR_PAJAK(OTOMATIS)............................................................................................................................ 27
9. GADA I........................................................................................................................................................................28
10. TEBUS..................................................................................................................................................................... 29
11. BANGUN............................................................................................................................................................... 29
12. LELANG(OTOMATIS).....................................................................................................31
13. FESTIVAL(OTOMATIS).................................................................................................. 31
14. BANGKRUT DAN AMBIL ALIH ASET(OTOMATIS)....................................................... 32
15. SIMPAN........................................................................................................................... 34
16. MUAT...............................................................................................................................35
17. CETAK_LOG....................................................................................................................35
18. MENANG(OTOMATIS)................................................................................................... 36
19. KESEMPATAN DAN DANA UMUM(OTOMATIS)........................................................... 37
20. GUNAKAN KEMAMPUAN...............................................................................................37
21. DROP KARTU KEMAMPUAN(OTOMATIS)................................................................... 38
    Bonus..........................................................................................................................................40
    Antarmuka Pengguna Grafis/Graphical User Interface(GUI)................................................................. 40
    COM................................................................................................................................................................................40
    Papan Dinamis...................................................................................................................... 40
    Kreativitas.................................................................................................................................................................... 41
    S p esifikasi Sistem.................................................................................................................................................... 42
    Ketentuan Umum.......................................................................................................................................................42
    Ketentuan Teknis....................................................................................................................................................... 42
    Panduan Pen g er j aan...............................................................................................................................................43
    Milestone Pen g um p ulan....................................................................................................................................... 45
    Milestone 1...................................................................................................................................................................45
    Milestone 2...................................................................................................................................................................45
    Asistensi......................................................................................................................................................................45
    Q nA.............................................................................................................................................................................. 46

## Deskripsi Persoalan

Di suatu pagi yang cerah, Gro terbangun dari tidurnya. Dari ruang santai terdengar gelak tawa yang cukup ramai. Saat menghampiri sumber suara tersebut, Gro mendapati Kebin dan Stewart sedang asyik memainkan gim konsol terbaru. Namun, suasana segera berubah ketika Gro menyadari bahwa g im tersebut ternyata dibeli men gg unakan akun miliknya tanpa se p en g etahuann y a.

Melihat kedua Nimons y an g lan g sun g ketakutan dan ham p ir menan g is, amarah Gro p erlahan mereda. Ia j ustru menda p at sebuah ide. Dari p ada Kebin dan Stewart terus membeli g im baru y an g men g uras isi dom p etn y a, lebih baik ia men y ediakan sendiri sebuah p ermainan y an g tidak kalah seru untuk mereka mainkan.

Gro p un terin g at p ada sebuah p ermainan p a p an y an g p ernah ia temui saat berkun j un g ke sebuah board g ame ca f é bersama istrin y a. Permainan itu bernama Nimons p oli, sebuah p ermainan strate g i y an g membuat p ara p emainn y a da p at membeli p ro p erti, memba y ar sewa, men g ikuti lelan g , hin gg a men g elola keka y aan sambil berkelilin g p a p an p ermainan.

Sa y an g n y a, meski p un Gro adalah mantan kriminal hebat y an g kini beralih men j adi a g en mata-mata, ia sama sekali tidak memiliki kemam p uan untuk men g emban g kan g im semacam itu. Oleh karena itu, Gro membutuhkan bantuan Anda untuk memban g un g im Nimons p oli dalam bahasa C++ den g an memanfaatkan konse p -konse p OOP. Bantulah Gro membuat p ermainan y an g seru a g ar Kebin, Stewart, dan p ara Nimons lainn y a da p at bermain tan p a kembali diam-diam men g habiskan uan g Gro.

## Alur Program

1. Saat program dijalankan, program akan membaca berkas konfigurasi yang memiliki format seperti pada Konfigurasi
2. Program akan menampilkan menu dan memberikan 2 opsi untuk menginisiasi state program
   a. Load Game
   i. Melakukan load state game berdasarkan Save/Load
   ii. Urutan giliran pemain diambil dari state tersimpan di Save/Load
   b. New Game
   i. Melakukan input jumlah pemain(Min: 2 Pemain, Maks: 4)
   ii. Menginputkan username masing-masing pemain
   iii. Urutan giliran pemain akan diputuskan pada awal permainan melalui pengacakan random
3. Satu siklus giliran pemain disebut turn.(Pemain 1 → Pemain 2 →... → Pemain N)
4. Permainan berjalan sesuai dengan Mekanisme permainan
5. Dalam permainan, pemain dapat memilih sebuah perintah dari daftar perintah berikut Operasi dan Perintah. Lakukan validasi masukan untuk setiap perintah.
6. Berakhirnya permainan mengikuti aturan Kondisi Game Selesai

## Konfigurasi

1. Konfi g urasi Pro p erty
   Setia p p ro p erti terdiri dari ID y an g meru p akan indeks, kode huruf, nama, blok warna, har g a beli tanah dasar, har g a ban g un rumah/hotel dan har g a sewa bertin g kat mulai dari tanah koson g (Level 0) hin gg a hotel(Level 5). Kode huruf di j amin unik.
   | p ro p ert y .txt
   | ---|
   | ID KODE NAMA JEN IS WARNA HARGA_LAHAN N ILA I_GADA I UPG_RUMAH UPG_HT RENT_L0...RENT_L5 |
   | 2 GRT GARUT STREET COKLAT 60 40 20 50 2... 250
   4 TSK TASIKMALAYA STREET COKLAT 60 40 50 50 4... 320
   6 GBR STASIUN_GAMBIR RA ILROAD DEFAULT 0 20
   7 BGR BOGOR STREET BIRU_MUDA 100 80 20 50 6... 400
   9 DPK DEP O K S TREET BIR U \_M U DA 100 80 20 50 6... 400
   10 BK S BEKA S I S TREET BIR U \_M U DA 120 90 20 50 8... 450
   12 M G L MA G ELAN G S TREET MERAH_M U DA 140 100 100 100 10... 750
   13 PLN PLN U TILITY AB U \_AB U 0 25
   14 SO L SO L O S TREET MERAH_M U DA 140 100 100 100 10 50... 750
   15 Y OG Y OG YAKARTA S TREET MERAH_M U DA 160 120 100 100 12... 900
   16 S TB S TA S I U N_BAND U N G RA ILR O AD DEFA U LT 0 40
   17 MAL MALAN G S TREET O RAN G E 180 135 100 100 14... 950
   19 S M G S EMARAN G S TREET O RAN G E 180 140 100 100 14... 950 |
   | 20 SBY SURABAYA STREET ORANGE 200 150 100 100 16... 1000
   22 MKS MAKASSAR STREET MERAH 220 175 150 150 18... 1050
   24 BLP BALIKPAPAN STREET MERAH 220 175 150 150 18... 1050
   25 MND MANADO STREET MERAH 240 190 150 150 20... 1100
   26 TUG STASIUN_TUGU RA ILROAD DEFAULT 0 50
   27 PLB PALEMBANG STREET KUN ING 260 200 150 150 22... 1150
   28 PKB PEKANBARU STREET KUN ING 260 210 150 150 22... 1150
   29 PAM PAM UTILITY ABU_ABU 0 60
   30 MED MEDAN STREET KUN ING 280 225 150 150 24... 1200
   32 BDG BANDUNG STREET HIJAU 300 250 200 200 26... 1275
   33 DEN DENPASAR STREET HIJAU 300 260 200 200 26... 1275
   35 MTR MATARAM STREET HIJAU 320 280 200 200 28... 1400
   36 GUB STASIUN_GUBENG RA ILROAD DEFAULT 0 120
   38 JKT JAKARTA STREET BIRU_TUA 350 300 200 200 35... 1500
   40 IKN IBU_KOTA_NUSANTARA STREET BIRU_TUA 400 350 200 200 50... 2000 |

2. Konfigurasi Fasilitas Publik
   a. Railroad
   Setiap railroad memiliki biaya sewa yang bergantung pada banyaknya railroad yang dimiliki oleh pemilik petak. Semakin banyak railroad yang dimiliki oleh pemain, semakin besar biaya sewa yang harus dibayarkan oleh pemain lain yang mendarat di petak railroad miliknya.
   | railroad.txt
   | ---|
   | JUMLAH_RA ILROAD BIAYA_SEWA |
   | 1 25
   2 50
   3 100
   4 200 |

   b. Utilit y
   Setia p utilit y memiliki bia y a sewa dihitun g dari total an g ka dadu y an g dilem p ar p emain y an g mendarat, dikalikan faktor p en g ali sesuai j umlah utilit y y an g dimiliki p emilik p etak. Semakin ban y ak utilit y y an g dimiliki oleh p emain, semakin besar faktor p en g alin y a.
   | ut ili t y . t x t
   | ---|
   | JU MLAH\_ U TILITY FAKT O R_PEN G ALI |
   | 1 4
   2 10 |

3. Konfi g urasi Pa j ak
   Konfigurasi pajak terdiri dari besar pembayaran flat Pajak Penghasilan(PPH), persentase pembayaran Pajak Penghasilan terhadap total kekayaan pemain, serta besar Pajak Barang Mewah(PBM).
   | tax.txt
   | ---|
   | PPH_FLAT PPH_PERSENTASE PBM_FLAT |
   | 150 10 200 |

4. Konfigurasi Petak Spesial
   Konfigurasi petak spesial terdiri dari besaran gaji yang diterima pemain saat berhenti tepat di petak GO atau melewatinya, serta besaran denda yang harus dibayarkan pemain untuk keluar dari penjara.
   | special.txt
   | ---|
   | GO_SALARY JA IL_FINE |
   | 200 50 |

5. Konfigurasi Lain Lain
   Konfigurasi lain-lain terdiri dari konfigurasi jumlah maksimal giliran bermain dan saldo tiap pemain di awal permainan dimulai.
   | misc.txt
   | ---|
   | MAX_TURN SALDO_AWAL |
   | 15 1000 |

## Mekanisme

Permainan Nimons p oli dilaksanakan dalam sebuah p a p an p ermainan y an g berisi dari 40 p etak membentuk p erse g i. Dalam p a p an p ermainan Nimons p oli terda p at 2 ti p e p etak, y aitu p etak p ro p erti dan p etak aksi.

### Papan Permainan

Gambar 1. Contoh Papan Permainan Nimonspoli

#### PETAK PROPERTI

Terda p at 28 Pro p erti dalam p ermainan, den g an masin g -masin g p ro p erti memiliki kartu sertifikat y an g merinci Har g a Sewa, Bia y a Pemban g unan, serta Nilai Gadai(untuk detailn y a bisa dilihat di Ke p emilikan Pro p erti). Pro p erti tersebut diba g i seba g ai berikut:
● Petak Lahan adalah p ro p erti p alin g ban y ak den g an total j umlah 22 lahan dalam p a p an p ermainan. Setia p p etak lahan termasuk dalam sebuah set warna y an g berisi total 2/3 lahan. Mulai dari p etak MULAI dan ber g erak c lockwise, set warna adalah cokelat, biru muda, p ink, oran g e, merah, kunin g , hi j au, dan biru tua. Set cokelat dan biru tua han y a terdiri dari 2 lahan dan sisan y a 3 lahan.
● Petak Stasiun
Petak stasiun adalah petak yang terdapat di tengah pada setiap sisinya sehingga total jumlahnya ada 4.
● Petak Utilitas
Petak utilitas terdiri dari 2 petak, PLN dan PAM.

#### PETAK AKSI

Terdapat 12 petak aksi pada papan yang terdiri dari:
● Empat(4) Petak Kartu yang terbagi menjadi Kartu Kesempatan dan Dana Umum
● Dua(2) Petak Festival
● Dua(2) Petak Pajak
● Empat(4) Petak Spesial yang terdapat di ujung papan permainan.

Berikut merupakan tabel urutan petak dan gambar papan permainan Nimonspoil:
| Indeks | Kode | Nama Lengkap Petak | Tipe | Kategori/Warna |
| --- | --- | --- | --- | --- |
| 1 | GO | Petak Mulai | Petak Spesial | Default |
| 2 | GRT | Garut | Petak Lahan | Coklat |
| 3 | DNU | Dana Umum | Petak Kartu | Default |
| 4 | TSK | Tasikmalaya | Petak Lahan | Coklat |
| 5 | PPH | Pa j ak Pen g hasilan | Petak Pa j ak | Default |
| 6 | GBR | Stasiun Gambir | Petak Stasiun | Default |
| 7 | BGR | Bo g or | Petak Lahan | Biru Muda |
| 8 | FES | Festival | Petak Festival | Defau lt |
| 9 | DPK | De p ok | Petak Lahan | Biru Muda |
| 10 | BKS | Bekasi | Petak Lahan | Biru Muda |
| 11 | PEN | Pen j ara | Petak S p esial | De f ault |
| 12 | MGL | Ma g elan g | Petak Lahan | Merah Muda(Pink) |
| 13 | PLN | PLN | Petak Utilitas | Abu-abu |
| 14 | SOL | Solo | Petak Lahan | Merah Muda(Pink) |
| 15 | YOG | Yo g y akarta | Petak Lahan | Merah Muda(Pink) |
| 16 | STB | Stasiun Bandun g | Petak Stasiun | De f ault |
| 17 | MAL | Malang | Petak Lahan | Orange |
| 18 | DNU | Dana Umum | Petak Kartu | Default |
| 19 | SMG | Semarang | Petak Lahan | Orange |
| 20 | SBY | Surabaya | Petak Lahan | Orange |
| 21 | BBP | Bebas Parkir | Petak Spesial | Default |
| 22 | MKS | Makassar | Petak Lahan | Merah |
| 23 | KSP | Kesempatan | Petak Kartu | Default |
| 24 | BLP | Balikpapan | Petak Lahan | Merah |
| 25 | MND | Manado | Petak Lahan | Merah |
| 26 | TUG | Stasiun Tugu | Petak Stasiun | Default |
| 27 | PLB | Palembang | Petak Lahan | Kuning |
| 28 | PKB | Pekanbaru | Petak Lahan | Kuning |
| 29 | PAM | PAM | Petak Utilitas | Abu-abu |
| 30 | MED | Medan | Petak Lahan | Kuning |
| 31 | PPJ | Petak Per g i ke Pen j ara | Petak S p esial | Default |
| 32 | BDG | Bandun g | Petak Lahan | Hi j au |
| 33 | DEN | Dena p asar | Petak Lahan | Hi j au |
| 34 | FES | Festival | Petak Festival | Default |
| 35 | MTR | Mataram | Petak Lahan | Hi j au |
| 36 | GUB | Stasiun Guben g | Petak Stasiun | De f ault |
| 37 | KSP | Kesem p atan | Petak Kartu | De f ault |
| 38 | JKT | Jakarta | Petak Lahan | Biru Tua |
| 39 | PBM | Pa j ak Baran g Mewah | Petak Pa j ak | De f ault |
| 40 | IKN | Ibu Kota Nusantara | Petak Lahan | Biru Tua |

De f ault = Tentukan warna d e f ault p a p an p ermainan kalian!
De f ault = Tentukan warna d e f ault p a p an p ermainan kalian!
De f ault = Tentukan warna d e f ault p a p an p ermainan kalian!
De f ault = Tentukan warna d e f ault p a p an p ermainan kalian!
De f ault = Tentukan warna d e f ault p a p an p ermainan kalian!
Warna bisa kalian sesuaikan kembali a g ar visualisasi lebih enak dilihat, teta p i semua p etak teta p harus dalam kategori yang sama

```
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
|[DF] BBP  |[MR] MKS  |[DF] KSP  |[MR] BLP  |[MR] MND  |[DF] TUG  |[KN] PLB  |[KN] PKB  |[AB] PAM  |[KN] MED  |[DF] PPJ  |
| (1)      |P1^       |          |P2^^      |P3^^^     |P1        |P2^       |P2^       |P4        |P2^^      |         |
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
|[OR] SBY  |          |[HJ] BDG  |          |          |          |          |          |          |          |         |
|          |          |P3*       |          |P4^       |          |          |          |          |          |         |
+----------+          +----------+==========+==========+==========+==========+==========+==========+==========+---------+
|[OR] SMG  |          ================================== NIMONSPOLI ================================== |[HJ] DEN  |
|P3^^      |          ===================================================================================== |P4^       |
+----------+          ===================================================================================== +----------+
|[DF] DNU  |          |           | TURN 15/ 50 |           |           |           |           |           |         |
|          |          +----------+-------------+----------+----------+----------+----------+----------+----------+---------+
|[OR] MAL  |          |[DF] FES  |           |           |           |           |           |           |           |P4^^    |
|P3^       |          ----------------------------------|P4^^      |--------------------------------------------|---------+
+----------+          LEGENDA KEPEMILIKAN& STATUS       +----------+
|[DF] STB  | P1-P4: Properti milik Pemain 1-4           |[DF] GUB  |
|P1        | ^ : Rumah Level 1                          |P2        |
+----------+ ^^ : Rumah Level 2                         +----------+
|[PK] YOG  | ^^^ : Rumah Level 3                        |[DF] KSP  |
|          | *: Hotel(Maksimal)                         |P4^^^     |
+----------+ (1)-(4): Bidak(IN=Tahanan, V=Mampir)       +----------+
|[PK] SOL  | ----------------------------------         |[BT] JKT  |
|P4^^      |                                            |P1^ (4)   |
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
|[DF] PEN  |[BM] BKS  |[BM] DPK  |[DF] FES  |[BM] BGR  |[DF] GBR  |[DF] PPH  |[CK] TSK  |[DF] DNU  |[CK] GRT  |[DF] GO   |
| IN:2 V:3 |P2^       |P2^^      |          |P2^^^     |P3        |P1^^      |P1^       |          |          |         |
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
```

Tampilan di atas hanyalah contoh visualisasi, untuk ketentuan visualisasi papan permainan dapat dilihat pada bag ian Bab Operasi dan Perintah.

### Petak Spesial

Pada p a p an p ermainan Nimons p oli terda p at 4 p etak s p esial den g an mekanismen y a masin g -masin g y an g terdiri dari:

1. Petak Mulai(Go)
   Petak Go adalah titik awal p ermainan untuk semua p ermainan. Mekanisme utaman y a adalah seba g ai sumber p enda p atan teta p . Setia p kali bidak p emain berhenti te p at di p etak ini atau melewati p etak ini dalam g ilirann y a, p emain berhak menerima uan g dari Bank sebesar g a j i y an g ditulis di file konfi g urasi.
2. Petak Pen j ara
   Petak ini terba g i men j adi dua ba g ian den g an mekanisme y an g berbeda ter g antun g ba g aimana p emain bisa sam p ai di p etak ini:
   ● Han y a Mam p ir: Jika p emain berhenti di p etak ini dari hasil lem p aran dadu normal, p emain han y a dian gg a p seba g ai p en g un j un g . Tidak ada p enalti, p emain aman, dan p ada g iliran berikutn y a p emain bisa ber g erak se p erti biasa.
   ● Di Dalam Pen j ara: Pemain berstatus tahanan j ika p emain ke p en j ara karena mendarat di Petak “Per g i ke Pen j ara“, menda p atkan kartu y an g men y uruh masuk p en j ara, atau melem p ar dadu d ouble ti g a kali berturut-turut. Untuk keluar p ada g iliran berikutn y a, p emain memiliki ti g a p ilihan:
   i. Membayar denda ke Bank sebesar denda yang ditulis di file konfigurasi sebelum melempar dadu.
   ii. Menggunakan kartu “Bebas dari Penjara”(j ika punya).
   iii. Mencoba melempar dadu dan harus mendapatkan angka double. Jika gagal, pemain tidak bergerak untuk giliran tersebut. Batas giliran percobaan dengan melempar dadu adalah 3 giliran. Pada giliran ke-4, pemain wajib keluar penjara dengan cara(i), yaitu membayar denda ke Bank.
3. Petak Bebas Parkir
   Petak ini berfungsi sebagai rest area. Jika pemain berhenti di sini, tidak ada aksi apa pun yang terjadi.
4. Petak Pergi ke Penjara
   Petak ini adalah petak jebakan yang terdapat pada papan permainan. Jika pemain mendarat di petak ini, pemain harus memindahkan bidak pemain ke Petak Penjara dan statusnya langsung menjadi tahanan. Giliran pemain berakhir saat itu juga dan pemain tidak mendapatkan gaji dari melewati Petak Mulai.

### Dadu dan Giliran Bermain

Permainan Nimonspoli menggunakan dua dadu bersisi 6(2d6) untuk menentukan pergerakan pemain. Dadu dapat dilempar secara random atau ditentukan manual angka tiap dadunya. Berikut merupakan aturan penggunaan dadu baik ketika di-random maupun ditentukan manual:
● Pemain mengocok kedua dadu dan memajukan bidaknya searah jarum jam sesuai dengan total an g ka yang muncul pada kedua dadu tersebut.
● Jika seoran g p emain melem p ar dan menda p atkan an g ka y an g sama p ada kedua dadu (misaln y a 6 dan 6), p emain tersebut men y elesaikan aksin y a di p etak tem p at ia p er g i, lalu berhak men g ocok dadu la g i untuk g iliran tambahan.
● Jika p emain menda p atkan an g ka double seban y ak ti g a kali berturut-turut dalam satu g iliran y an g sama, p emain tersebut dian gg a p “melan gg ar batas kece p atan”. Bidak tidak di g erakkan berdasarkan an g ka dadu keti g a, teta p i p emain lan g sun g memindahkan bidakn y a ke Petak Pen j ara dan g ilirann y a berakhir.

### Kepemilikan Properti

(Untuk kemudahan, terms y an g di g unakan p ada ba g ian ini akan men g acu p ada p ermainan mono p oli asli a g ar kalian da p at men g acu den g an lebih mudah dari p ermainan aslin y a)
Pro p erti adalah p etak tanah y an g da p at dimiliki oleh p emain. Terda p at ti g a j enis p ro p erti dalam p ermainan ini, y aitu Street, Railroad, dan Utilit y . Setia p p ro p erti memiliki Akta Ke p emilikan y an g men y im p an informasi berikut dan didefinisikan di file konfi g urasi: nama p ro p erti dan j enisn y a (Street/Railroad/Utilit y), color g rou p (khusus Street), harg a beli, tabel har g a sewa p er level ban g unan, nilai g adai, serta har g a ban g unan p er rumah dan hotel(khusus Street).
Setiap properti memiliki salah satu dari tiga status berikut. Status BANK berarti properti belum dimiliki siapapun. Status OWNED berarti properti dimiliki oleh salah satu pemain dan sewa berlaku normal. Status MORTGAGED berarti properti dimiliki pemain tetapi sedang digadaikan sehingga sewa tidak berlaku.

#### Street(e.g., BDG, DEN, MTR, dst.)

Street adalah jenis properti yang paling umum dalam permainan. Setiap Street termasuk dalam sebuah color group, yaitu kelompok properti yang berbagi warna yang sama. Pemain yang berhasil memiliki seluruh Street dalam satu color group disebut memonopoli color group tersebut, dan mendapatkan keuntungan berupa sewa yang lebih tinggi serta kemampuan untuk membangun rumah dan hotel di atasnya.
Ketika pemain mendarat di petak Street berstatus BANK, program secara otomatis menampilkan informasi Akta Kepemilikan dan menawarkan pembelian kepada pemain. Jika pemain memilih membeli dan uangnya mencukupi, uang pemain dikurangi sebesar harga beli dan status properti berubah menjadi OWNED. Jika pemain memilih tidak membeli atau uang tidak mencukupi, properti langsung masuk sistem lelang sesuai mekanisme yang berlaku.

#### Railroad(e.g., TUG, GUB)

Railroad dimiliki oleh pemain pertama yang tiba di petak tersebut secara otomatis tanpa perlu membayar. Tidak ada proses beli maupun lelang . Meskipun cara perolehannya berbeda denga n Street, Railroad tetap dapat di g adaikan, ditebus, dan di j ual ke pe main lain. Semakin banya k Railroad y an g dimiliki oleh p emain, otomatis bia y a sewa akan semakin tin gg i p ula(ditentukan melalui config).

#### Utility(PLN dan PAM)

Utilit y memiliki mekanisme ke p emilikan y an g sama den g an Railroad. Pemain p ertama y an g tiba di p etak Utilit y lan g sun g menda p atkan ke p emilikann y a secara otomatis tan p a p erlu memba y ar, dan tidak ada p roses beli mau p un lelan g . Stok utilitas bersifat sudah terban g un se j ak awal. Artin y a p emain han y a p erlu men g in j akn y a untuk memilikin y a.
Perhitun g an bia y a sewa Utilit y ditentukan oleh total an g ka dadu y an g dida p atkan p emain ketika mendarat, dikalikan den g an faktor p en g ali y an g ber g antun g p ada j umlah p etak Utilit y y an g dimiliki oleh p emilikn y a. Semakin ban y ak Utilit y y an g dimiliki maka faktor p en g ali akan semakin besar (faktor p en g ali dibaca melalui c onfi g). Meski p un cara p erolehann y a berbeda den g an Street, Utilit y teta p da p at di g adaikan, ditebus, dan di j ual ke p emain lain

#### Ketentuan Sewa

Ketika pemain mendarat di properti berstatus OWNED milik pemain lain, pembayaran sewa dilakukan secara otomatis dan hasilnya ditampilkan ke layar beserta log transaksi. Jika properti berstatus MORTGAGED, tidak ada sewa yang dikenakan.
Besaran sewa bergantung pada jenis properti:

1. Street: Besaran sewa bergantung pada jumlah bangunan yang ada dan status monopoli color group, dengan nilai yang didefinisikan di file konfigurasi per properti. Apabila pemain memonopoli seluruh Street dalam satu color group dan belum ada bangunan yang didirikan, sewa yang dikenakan adalah dua kali sewa dasar. Setelah bangunan mulai didirikan, sewa mengikuti tabel yang tertera pada Akta Kepemilikan, mulai dari 1 rumah hingga hotel. Jika efek Festival aktif pada properti tersebut, nilai sewa yang dikenakan adalah nilai sewa terkini setelah penggandaan Festival diterapkan.
2. Railroad: Besaran sewa ditentukan oleh jumlah Railroad yang dimiliki oleh pemilik properti tersebut, sesuai tabel yang didefinisikan di file konfigurasi.
3. Utility: Besaran sewa dihitung dari total angka dadu yang dilempar pemain yang mendarat, dikalikan faktor pengali sesuai jumlah Utility yang dimiliki pemiliknya, sebagaimana didefinisikan di file konfigurasi.
   Apabila pemain tidak mampu membayar sewa, pemain memasuki kondisi kebangkrutan. Alur kebangkrutan ditangani oleh bagian Kebangkrutan dan Pengambilalihan Aset.

#### Gadai

Pemain da p at men gg adaikan p ro p erti y an g dimilikin y a ke p ada Bank ka p an sa j a selama g iliran mereka. S y arat men gg adaikan adalah p ro p erti berstatus OWNED dan tidak memiliki ban g unan y an g berdiri di atasn y a. Jika masih ada ban g unan p ada color g rou p y an g sama, semua ban g unan p ada seluruh color g rou p tersebut harus di j ual ke Bank terlebih dahulu den g an har g a seten g ah dari har g a beli ban g unan sebelum p ro p erti da p at di g adaikan. Pen j ualan ban g unan dan p en gg adaian p ro p erti da p at dilakukan dalam satu ran g kaian aksi GADAI y an g sama tan p a p erlu meman gg il p erintah ulan g .
Ketika p ro p erti berhasil di g adaikan, p emain menerima uan g sebesar nilai g adai p ro p erti tersebut sesuai y an g tertera p ada Akta Ke p emilikan dan status p ro p erti berubah men j adi MORTGAGED.
Untuk menebus p ro p erti y an g sedan g di g adaikan, p emain memba y ar Bank sebesar har g a beli p enuh p ro p erti tersebut dan status p ro p erti kembali men j adi OWNED. Pro p erti berstatus MORTGAGED tidak da p at men g hasilkan p enda p atan sewa selama belum ditebus.
Pro p erti berstatus MORTGAGED tidak da p at lan g sun g di j ual ke Bank dalam p roses likuidasi. Pemain harus menebus p ro p erti tersebut terlebih dahulu(memba y ar har g a beli p enuh) sebelum da p at men j ualn y a kembali ke Bank den g an har g a beli p enuh. Oleh karena itu, dalam kondisi ban g krut, men gg adaikan p ro p erti umumn y a lebih men g untun g kan dari p ada menebus lalu men j ual.

#### Peningkatan Properti

Pemain dapat meningkatkan properti jenis Street yang dimilikinya dengan membangun rumah dan hotel selama giliran mereka. Peningkatan properti hanya dapat dilakukan apabila pemain telah memonopoli seluruh Street dalam satu color group. Setiap peningkatan akan menaikkan nilai sewa yang harus dibayar oleh pemain lain yang mendarat di properti tersebut.
Stok bangunan tidak terbatas. Pemain dapat membangun rumah dan hotel tanpa dibatasi oleh ketersediaan stok fisik.
Terdapat dua jenis bangunan yang dapat didirikan: rumah dan hotel. Rumah dapat dibangun hingga maksimal 4 buah per petak. Setelah seluruh petak dalam satu color group memiliki 4 rumah masing-masing, pemain dapat meng-upgrade ke hotel. Hanya boleh ada 1 hotel per petak dan hotel merupakan tingkat bangunan tertinggi yang dapat dicapai. Petak yang sudah berstatus hotel tidak dapat dibangun lebih lanjut.
Pembangunan rumah harus dilakukan secara merata di seluruh petak dalam satu color group. Artinya, selisih jumlah rumah antar petak dalam satu color group tidak boleh melebihi 1 pada setiap saat. Pemain tidak dapat menambah rumah ke-2 di suatu petak sebelum seluruh petak lain dalam color group yang sama memiliki setidaknya 1 rumah. Petak yang tidak memenuhi syarat pemerataan ini tidak akan ditampilkan sebagai pilihan pada perintah BANGUN.

#### Lelang Tanah

Ketika seoran g p emain berhenti p ada p etak p ro p erti street, ia da p at memilih untuk TIDAK membeli p ro p erti tersebut(atau tidak mam p u membeli). Ketika hal ini ter j adi, p etak p ro p erti tersebut akan secara otomatis dilelan g . Ketika seoran g p emain ban g krut ke Bank dan p ermainan masih terus berlan j ut, SEMUA p etak p ro p erti y an g dimiliki p emain y an g ban g krut tersebut akan otomatis dilelan g satu p er satu. Urutan p ro p erti y an g dilelan g dibebaskan.

#### Mekanisme Lelang

Berikut meru p akan lan g kah-lan g kah ter j adin y a lelan g :

1. Lelan g dimulai dari p emain y an g memiliki g iliran setelah p emain y an g memicu lelan g , lalu berlan j ut sesuai urutan g iliran p ermainan.
2. Pada g ilirann y a, setia p p emain han y a da p at memilih satu dari dua aksi:
   a. Pass: Tidak men g a j ukan p enawaran dan melewatkan g iliran.
   b. Bid: Men g a j ukan p enawaran har g a untuk membeli p ro p erti.
3. Penawaran p ertama( bid awal) minimal adalah M0. Setia p b id berikutn y a harus lebih tin gg i dari bid sebelumn y a.
4. Lelan g berlan j ut hin gg a ter j adi p ass berturut-turut seban y ak(j umlah p emain- 1). Jika lelan g ter j adi karena kondisi ban g krut, j umlah p emain tidak men g hitun g p emain y an g ban g krut tersebut. Jika kondisi tersebut terca p ai, maka:
   a. Pemain y an g terakhir melakukan b id men j adi p emenan g lelan g .
   b. Pemenang membayar sesuai nilai bid terakhir.
   c. Properti diberikan kepada pemenang.
5. Minimal terdapat satu pemain yang melakukan bid. Jika terjadi kondisi(4) tanpa ada pemain yang melakukan bid, maka pemain terakhir yang tidak melakukan pass harus melakukan bid. Lelang berlanjut seperti biasa.
6. Pemain tidak dapat melakukan bid melebihi uang yang dimiliki.
7. Setelah lelang selesai, permainan kembali ke urutan giliran normal(bukan berdasarkan urutan lelang).
8. Giliran lelang bukan bagian dari giliran permainan biasa. Giliran lelang tidak mempengaruhi durasi efek apa pun atau perhitungan giliran untuk batas maksimum jumlah giliran.

#### Pa j ak

Terdapat dua petak pajak dalam permainan yang akan dikenakan kepada pemain apabila token mereka mendarat di atasnya. Kedua petak ini tidak dapat dimiliki oleh siapapun dan pajaknya langsung dibayarkan ke Bank.
Petak pertama adalah Pajak Penghasilan(PPH). Ketika pemain mendarat di petak ini, pemain dihadapkan pada dua pilihan: membayar sejumlah flat, atau membayar persentase dari total kekayaan mereka. Pemain harus memutuskan pilihan mana yang akan diambil sebelum menghitung total kekayaannya. Total kekayaan dihitung dari seluruh uang tunai yang dimiliki, ditambah harga beli seluruh properti yang dimiliki baik yang sedang digadaikan maupun tidak, ditambah har g a beli seluruh ban g unan ya n g telah didirikan. Jika pe main memilih ops i flat namun uan g n y a tidak mencuku p i, p emain lan g sun g memasuki kondisi keban g krutan tan p a ditawarkan o p si kedua. Persentase p emba y aran serta j umlah flat y an g p erlu diba y arkan didefinisikan di file konfi g urasi.
Petak kedua adalah Pa j ak Baran g Mewah(PBM). Ketika p emain mendarat di p etak ini, p emain lan g sun g dikenakan p a j ak sebesar nilai y an g didefinisikan di file konfi g urasi tan p a p ilihan lain.
A p abila p emain tidak mam p u memba y ar p a j ak(baik PPH mau p un PBM), p emain memasuki kondisi keban g krutan den g an Bank seba g ai kreditor. Alur keban g krutan ditan g ani oleh ba g ian Keban g krutan dan Pen g ambilalihan Aset.

#### Festival

Petak festival meru p akan p etak aksi y an g da p at meli p at g andakan har g a sewa dari p ro p erti y an g dimiliki. Ketika seoran g p emain berhenti di p etak festival, p emain tersebut da p at memilih salah satu dari p ro p erti y an g ia miliki. Har g a sewa dari p ro p erti y an g di p ilih akan naik dua kali li p at selama ti g a g iliran. Jika p emain berhenti kembali di p etak festival dan memilih p ro p erti y an g sama, durasi dikembalikan men j adi ti g a g iliran(j ika durasi sudah berkuran g ) dan har g a sewa naik dua kali lipat lagi. Hal ini dapat berulang hingga tiga kali(maksimum harga sewa naik delapan kali lipat), selebihnya hanya mengembalikan durasi.
Berikut contoh skenario yang mungkin terjadi:

1. Pemain A berhenti pada petak festival lalu memilih properti Z(harga sewa awal 5.000). Harga sewa properti Z menjadi 10.000 selama tiga giliran pemain A.
2. Giliran pemain A lagi, durasi festival pada properti Z berkurang satu. Pemain A berhenti di petak festival lagi dan memilih properti Y(harga sewa dari 2.000 menjadi 4.000) selama tiga giliran. Perhitungan durasi terpisah untuk setiap properti(Z tersisa dua giliran dan Y tiga giliran).
3. Giliran pemain A lagi, durasi Z= 1, Y= 2. Pemain A berhenti di petak festival lagi dan memilih properti Z. Harga sewa properti Z menjadi 20.000 dan durasi kembali menjadi tiga giliran.
4. Skenario(3) terulang kembali, durasi Z= 2, Y= 1. Harga sewa properti Z menjadi 40.000 dan durasi kembali menjadi tiga giliran.
5. Skenario(3) terulang kembali, durasi Z= 2, Y= 0. Harga sewa properti Y kembali seperti semula. Harga sewa properti Z tetap 40.000(max kenaikan harga sewa sebanyak tiga kali). Durasi kembali menjadi tiga giliran.

#### Kartu Kesem p atan dan Dana Umum

Dalam Permainan Nimonspoli, terdapat tumpukan Kartu Kesempatan dan Dana Umum. Papan p ermainan j u g a memiliki beberapa petak khusus yang mewakili kedua j enis kartu tersebut. Apabila seoran g p emain mendarat di salah satu p etak ini, ia wa j ib men g ambil kartu teratas dari tum p ukan y an g sesuai dan se g era men j alankan instruksi y an g tertera di dalamn y a. Setelah di g unakan kartu akan kembali direshuffle dalam tum p ukan.

#### Kartu Kesem p atan

Berikut adalah aturan Kartu Kesem p atan y an g ada dalam Permainan Nimons p oli:
1."Per g i ke stasiun terdekat."
2."Mundur 3 p etak."
3."Masuk Pen j ara."

#### Kartu Dana Umum

Berikut adalah aturan Kartu Dana Umum y an g ada dalam Permainan Nimons p oli:
1."Ini adalah hari ulan g tahun Anda. Da p atkan M100 dari setia p p emain." 2. "Bia y a dokter. Ba y ar M700." 3. "Anda mau n y ale g . Ba y ar M200 ke p ada setia p p emain."

#### Kartu Kemam p uan S p esial

Selain Kartu Kesempatan dan Dana Umum terdapat pula tumpukan Kartu Kemampuan Spesial. Kartu Kemampuan Spesial merupakan kartu tangan yang memberikan keuntungan taktis bagi pemain.

#### Aturan Kartu Kemam p uan S p esial

Berikut adalah aturan Kartu Kemampuan Spesial:

1. Penggunaan kartu dibatasi maksimal 1 kali dalam 1 giliran.
2. Pada awal giliran, semua pemain akan mendapatkan masing-masing 1 kartu acak.
3. Setiap pemain hanya boleh memiliki maksimal 3 kartu di tangannya.
4. Jika seorang pemain mendapatkan kartu keempat, ia diwajibkan untuk membuang 1 kartu yang dimilikinya.
5. Kartu ini bersifat sekali pakai. Setelah dipakai, kartu akan dibuang dan diletakkan di tempat pembuangan.
6. Apabila kartu di dalam deck habis, seluruh kartu di tempat pembuangan akan dikocok ulang untuk membuat deck baru.
7. Kartu ini hanya bisa digunakan sebelum melempar dadu.

#### Jenis Kartu Kemam p uan S p esial

Terdapat beberapa jenis Kartu Kemampuan Spesial dalam Permainan Nimonspoli:

1. MoveCard(4 lembar): Memungkinkan pemain untuk bergerak maju sekian petak. Nominal lan g kah per g erakan ditentukan secara acak pada saat pemain pertama kali mendapatkan kartu ini.
2. DiscountCard(3 lembar): Memberikan diskon den g an p ersentase acak ke p ada p emain saat kartu dida p atkan. Masa berlaku DiscountCard adalah 1 g iliran.
3. ShieldCard(2 lembar): Melindun g i p emain dari ta g ihan sewa mau p un sanksi a p a p un y an g meru g ikan selama 1 g iliran.
4. Tele p ortCard(2 lembar): Memberikan p emain kebebasan untuk ber p indah ke p etak mana p un di atas p a p an p ermainan.
5. LassoCard(2 lembar): Menarik satu p emain lawan y an g berada di de p an p osisi p emain saat ini ke p etak tem p at p emain tersebut berada.
6. DemolitionCard(2 lembar): Men g hancurkan satu p ro p erti milik p emain lawan.

#### Ban g krut

Kondisi ban g krut(bankru p tc y) ter j adi ketika seoran g p emain tidak mam p u memenuhi kewa j iban p emba y aran, baik beru p a sewa, p a j ak, mau p un efek dari kartu tertentu. Ketika p emain tidak memiliki cuku p uan g tunai( cash) untuk memba y ar kewa j ibann y a, sistem akan men g hitun g j umlah maksimum dana y an g masih da p at di p eroleh melalui p roses likuidasi aset. Jika kewa j iban masih da p at di p enuhi melalui likuidasi, p emain wa j ib melakukan likuidasi tersebut. Namun, j ika setelah seluruh kemungkinan likuidasi dilakukan pemain tetap tidak mampu membayar, maka pemain dinyatakan bangkrut.

#### Mekanisme Likuidasi Aset

Pemain dapat melakukan likuidasi aset melalui beberapa cara berikut:

1. Menjual Properti ke Bank
   ○ Properti dapat dijual kembali ke bank dengan harga sesuai nilai beli yang tertera pada akta kepemilikan.
   ○ Jika properti berupa street dan memiliki bangunan(rumah atau hotel), maka nilai jual properti akan ditambah dengan setengah dari total harga bangunan yang ada di atasnya.
2. Menggadaikan(Mortgage) Properti
   ○ Properti dapat digadaikan ke bank sesuai dengan ketentuan pegadaian.
   ○ Pemain akan menerima sejumlah uang berdasarkan nilai gadai properti tersebut.
   ○ Properti yang sedang digadaikan tidak dapat menghasilkan pendapatan(sewa).
   Pemain bebas memilih kombinasi likuidasi mana yang ingin dilakukan, selama total dana yang dapat diperoleh masih memungkinkan pelunasan.

#### Pen g ambilalihan Asset

Pengambilalihan Aset terjadi ketika pemain dinyatakan bangkrut dan dibedakan berdasarkan siapa yang menjadi kreditor:
● Bang krut ke pemain lain: Seluruh aset(uang sisa dan properti beserta ban g unan di atasn y a) diserahkan lan g sun g ke p emain kreditor. Pro p erti y an g sedan g di g adaikan diserahkan dalam kondisi ter g adai sehin gg a kreditor menan gg un g status g adai tersebut.
● Ban g krut ke Bank(akibat p a j ak atau kartu): Seluruh uan g sisa diserahkan ke Bank dan hilan g dari p eredaran. Seluruh p ro p erti dikembalikan ke status BANK dan dilelan g satu p er satu. Ban g unan dihancurkan dan stok dikembalikan ke Bank.
Setelah ban g krut, p emain keluar dari p ermainan. Giliran lelan g aset tidak dihitun g seba g ai g iliran p ermainan biasa dan tidak mem p en g aruhi durasi efek a p a p un.
Kondisi kelan j utan p ermainan setelah keban g krutan:
● Jika masih ada ≥ 2 p emain aktif → p ermainan berlan j ut normal.
● Jika han y a tersisa 1 p emain → p emain tersebut menan g .

#### Kondisi Game Selesai

Permainan Nimons p oli berakhir ketika menca p ai salah satu dari dua kondisi: Bankru p tc y atau Max Turn .

#### MAX TURN

Permainan Nimonspoli berakhir ketika semua pemain telah mencapai batas maksimum giliran. Jumlah batas maksimum giliran didefinisikan pada file config. Pemenang adalah pemain dengan uang terbanyak. Jika seri, pemain yang memiliki jumlah petak properti terbanyak yang menjadi pemenang. Jika seri, pemain yang memiliki jumlah kartu terbanyak yang menjadi pemenang. Jika seri, semua pemain yang masih seri menjadi pemenang. Jika kondisi bankruptcy terjadi sebelum mencapai batas maksimum giliran, ikuti aturan bankruptcy.

#### BANKRUPTCY

Jika batas maksimum giliran bukan merupakan angka yang valid(<1) pada file config, Nimonspoli berakhir secara bankruptcy. Permainan Nimonspoli terus berlanjut tanpa batas turn hingga hanya tersisa satu pemain yang tidak bangkrut. Pemain tersebut dinyatakan sebagai pemenang.

#### Save/Load

Sistem Save/Load memungkinkan seluruh state permainan disimpan ke file eksternal dengan ekstensi.txt dan dimuat kembali di sesi berikutnya. Penyimpanan data hanya dapat dilakukan di awal giliran pemain, saat belum ada aksi apapun yang dijalankan oleh pemain tersebut(termasuk melemparkan dadu), sedangkan muat data hanya dapat dilakukan saat program berjalan pertama kali(sebelum permainan dimulai).
Format File Save menggunakan struktur teks terstruktur sebagai berikut:
<TURN*SAAT* IN I><MAX*TURN>
<JUMLAH_PEMA IN>
<STATE_PEMA IN_1>
<STATE_PEMA IN_2>
...
<STATE_PEMA IN_N>
<URUTAN_GILIRAN_1><URUTAN_GILIRAN_2>...<URUTAN_GILIRAN_N>
<GILIRAN_AKTIF_SAAT* IN I>
<STATE_PROPERTI>
<STATE_DECK>
<STATE_LOG>

Format State Pemain
<USERNAME><UANG><POSISI_PETAK><STATUS>
< JU MLAH_KART U \_TAN G AN>
< J EN I S \_KART U \_1><NILA I_KART U _1>< S I S A_D U RA S I_1>
< J EN I S \_KART U \_2><NILA I_KART U _2>< S I S A_D U RA S I_2>
Keteran g an:
●<USERNAME> adalah username p emain
●<UANG> adalah j umlah uan g p emain
●<POSISI_PETAK> adalah kode p etak tem p at p emain berada
●<STATUS> berisi status dari pemain yang bernilai ACTIVE, BANKRUPT, atau JAILED(j ika terjebak di penjara)
●<JENIS_KARTU> adalah jenis kartu kemampuan(contoh: MoveCard, ShieldCard)
●<NILAI_KARTU> adalah parameter nilai kartu tersebut(contoh: jumlah langkah untuk MoveCard, persentase diskon untuk DiscountCard).Untuk card yang tidak memiliki nilai tertentu, kosongkan nilai ini
●<SISA_DURASI> adalah sisa durasi berlakunya kartu tersebut, hanya untuk kartu jenis DiscountCard. Untuk kartu jenis lain, kosongkan nilai ini

Format State Properti
<JUMLAH_PROPERTI>
<KODE_PETAK><JEN IS><PEM ILIK><STATUS><FMULT><FDUR><N_BANGUNAN>
...
Keterangan
●<KODE_PETAK> adalah kode petak dari properti
●<JENIS> adalah jenis properti yang bernilai street, railroad, atau utility
●<PEMILIK> berisi username pemilik atau BANK j ika belum dimiliki
●<STATUS> adalah status dari properti yang bernilai BANK, OWNED, atau MORTGAGED
●<FMULT> adalah harga bangunan akibat festival yang bernilai 1, 2, 4, atau 8(1= tidak aktif)
●<FDUR> adalah durasi dari festival yang bernilai 0–3(0= tidak aktif)
●<N_BANGUNAN> bernilai 0–4 untuk rumah, atau H untuk hotel. Untuk properti selain street, isi den g an 0.

Format State Deck
<JUMLAH_KARTU_DECK_KEMAMPUAN>
<JEN IS_KARTU_1>
...

Format State Lo g
<JUMLAH_ENTRI_LOG>
<TURN><USERNAME><JEN IS_AKSI><DETA IL>
...

Berikut adalah contoh isi file y an g berisi state y an g disim p an.

```
15 30
4
U n a m e 1 1500 J KT A C TIVE
2
M o v eCa r d 5
S hi e l dCa r d
U n a m e 2 800 BD G A C TIVE
0
Uname3 2200 PKR ACTIVE
1
DiscountCard 30 1
Uname4 0 PJR BANKRUPT
0
Uname2 Uname4 Uname1 Uname3
Uname2
22
JKT  street  Uname1 OWNED 2 2 1
BDG  street  Uname2 MORTGAGED 0 1 0
KA I  railroad Uname1 OWNED 1 0 0
SBY  street  BANK  BANK 0 1 0
PLN  utility  Uname3 OWNED 1 0 0
...
12
MoveCard
DiscountCard
TeleportCard
...
10 1 Uname1 BELI Beli Jakarta(JKT) seharga M400
1 Uname2 DADU Lempar: 4+5=9 mendarat di Surabaya(SBY)
...
```

#### Transaction Lo gg er

Transaction Lo gg er mencatat setia p ke j adian si g nifikan dalam p ermainan secara otomatis ke dalam lo g terstruktur. Lo gg er beker j a di latar belakan g dan tidak memerlukan p erintah khusus untuk aktif.
Setia p entri lo g memuat informasi berikut: nomor turn, username p emain y an g relevan, j enis aksi, dan detail aksi(termasuk nilai uan g j ika ada). Jenis aksi y an g dicatat meli p uti:
● Lem p aran dadu dan p er g erakan bidak
● Pembelian p ro p erti(termasuk p erolehan Railroad/Utilit y otomatis)
● Pemba y aran sewa dan p a j ak
● Pemban g unan dan p en j ualan ban g unan
● Gadai dan tebus p ro p erti
● Pen gg unaan kartu kemam p uan s p esial
● Pen g ambilalihan kartu Kesem p atan dan Dana Umum
● Setia p bid dan hasil akhir lelan g
● Aktivasi dan p en g uatan efek festival
● Keban g krutan dan p en g ambilalihan aset
● Save dan load p ermainan
Logger menyimpan full log selama sesi berlangsung di memori. Saat game di-save, full log ikut tersimpan ke dalam file.

#### O p erasi dan Perintah

1. CETAK_PAPAN
   Perintah untuk menampilkan papan permainan. Ketentuan visualisasi papan permainan adalah sebagai berikut:
   ● Gunakan Colored Print untuk petak lahan sesuai dengan kategori/warnanya.
   ● Tampilkan posisi bidak semua pemain dengan jelas di semua kemungkinan skenario.
   ● Tampilkan semua kepemilikan dan status lahan dengan jelas.
   ● Tampilkan legenda papan permainan serta indikator giliran.

```
> CETAK_PAPAN
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
|[DF] BBP  |[MR] MKS  |[DF] KSP  |[MR] BLP  |[MR] MND  |[DF] TUG  |[KN] PLB  |[KN] PKB  |[AB] PAM  |[KN] MED  |[DF] PPJ  |
| (1)      |P1^       |          |P2^^      |P3^^^     |P1        |P2^       |P2^       |P4        |P2^^      |         |
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
|[OR] SBY  |          |[HJ] BDG  |          |          |          |          |          |          |          |         |
|          |          |P3*       |          |P4^       |          |          |          |          |          |         |
+----------+          +----------+==========+==========+==========+==========+==========+==========+==========+---------+
|[OR] SMG  |          ================================== NIMONSPOLI ================================== |[HJ] DEN  |
|P3^^      |          ===================================================================================== |P4^       |
+----------+          ===================================================================================== +----------+
|[DF] DNU  |          |           | TURN 15/ 50 |           |           |           |           |           |         |
|          |          +----------+-------------+----------+----------+----------+----------+----------+----------+---------+
|[OR] MAL  |          |[DF] FES  |           |           |           |           |           |           |           |P4^^    |
|P3^       |          ----------------------------------|P4^^      |--------------------------------------------|---------+
+----------+          LEGENDA KEPEMILIKAN& STATUS       +----------+
|[DF] STB  | P1-P4: Properti milik Pemain 1-4           |[DF] GUB  |
|P1        | ^ : Rumah Level 1                          |P2        |
+----------+ ^^ : Rumah Level 2                         +----------+
|[PK] YOG  | ^^^ : Rumah Level 3                        |[DF] KSP  |
|          | *: Hotel(Maksimal)                         |P4^^^     |
+----------+ (1)-(4): Bidak(IN=Tahanan, V=Mampir)       +----------+
|[PK] SOL  | ----------------------------------         |[BT] JKT  |
|P4^^      |                                            |P1^ (4)   |
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
|[DF] PEN  |[BM] BKS  |[BM] DPK  |[DF] FES  |[BM] BGR  |[DF] GBR  |[DF] PPH  |[CK] TSK  |[DF] DNU  |[CK] GRT  |[DF] GO   |
| IN:2 V:3 |P2^       |P2^^      |          |P2^^^     |P3        |P1^^      |P1^       |          |          |         |
+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+
```

2. LEMPAR_DAD U
   Perintah untuk melem p ar dadu secara random.

```
> LEMPAR_DAD U
M e n g oco k dadu ...
Hasil: 6+ 1= 7
Memajukan Bidak Pemain1 sebanyak 7 petakk...
Bidak mendarat di: Bandung.
// Jika bidak mendarat di Petak Lahan yang belum dimiliki siapa-siapa, lanjut ke skenario BELI
// Jika bidak mendarat di Petak Lahan yang sudah dimiliki seseorang, lanjut ke skenario BAYAR_SEWA
// Jika bidak mendarat di Petak Pajak, lanjut ke skenario BAYAR_PAJAK
```

3. ATUR_DADU X Y
   Perintah untuk mengatur hasil lemparan dadu secara manual.

```
> ATUR_DADU 2 5
Dadu diatur secara manual.
Hasil: 2+ 5= 7
Memajukan Bidak Pemain1 sebanyak 7 petakk...
Bidak mendarat di: Bandung.
```

4. CETAK_AKTA
   Perintah untuk menam p ilkan informasi len g ka p Akta Ke p emilikan dari sebuah p ro p erti. Da p at di p an gg il ka p an sa j a selama g iliran p emain.

```
> CETAK_AKTA
Masukkan kode p etak: JKT
+================================+
| AKTA KEPEM ILIKAN              |
| [BIRU T U A] J AKARTA(J KT)    |
+================================+
| H a r g a B e li : M400        |
| Nil a i Gada i : M200          |
+--------------------------------+
| Se w a (u nim p r o v ed) : M50 |
| Se w a (1 ru m a h) : M200     |
| Se w a (2 ru m a h) : M600     |
| Se w a (3 ru m a h) : M1.400   |
| Sewa(4 rumah) : M1.700         |
| Sewa(hotel) : M2.000           |
+--------------------------------+
| Harga Rumah : M200             |
| Harga Hotel : M200             |
+================================+
| Status: OWNED(Pemain A)        |
+================================+
// Jika kode tidak ditemukan:
Petak"XYZ" tidak ditemukan atau bukan properti.
```

5. CETAK_PROPERTI
   Perintah untuk menampilkan seluruh properti yang dimiliki oleh pemain yang sedang bermain beserta statusnya.

```
> CETAK_PROPERTI
=== Properti Milik: Pemain A===
[BIRU TUA] - Jakarta(JKT) 1 rumah  M400  OWNED
- Ibu Kota Nusantara(IKN)  Hotel  M400  OWNED
[HIJAU]
- Bandun g (BDG) M300  MORTGAGED[M]
- Den p asar(DEN) M300  OWNED
- Mataram(MTR) M320  OWNED
[STASIUN]
- Stasiun Gambir(GBR)  M200  OWNED
- Stasiun Tu g u(TUG) M200  OWNED
Total keka y aan p ro p erti: M3.720
// Jika tidak p un y a p ro p erti:
Kamu belum memiliki p ro p erti a p a p un.
```

6. BELI(OTOMATIS)
   Perintah ini di p icu secara otomatis ketika p emain mendarat di p etak p ro p erti berstatus BANK. Untuk Railroad dan Utilit y , ke p emilikan lan g sun g ber pindah tan p a p rom p t p embelian.

```
// Kasus Street: Kamu mendarat di Jakarta(JKT)!
+================================+
| [BIRU TUA] JAKARTA(JKT)        |
| Harga Beli : M400              |
| Sewa dasar : M50               |
| ...                            |
+================================+
Uang kamu saat ini: M1.500 Apakah kamu ingin membeli properti ini seharga M400?(y/n): y Jakarta kini menjadi milikmu!
Uang tersisa: M1.100
---
// Jika tidak mau atau uang tidak cukup:
Properti ini akan masuk ke sistem lelang...
---
// Kasus Railroad: Kamu mendarat di Stasiun Gambir(GBR)!
Belum ada yang menginjaknya duluan, stasiun ini kini menjadi milikmu!
---
// Kasus Utility: Kamu mendarat di PLN!
Belum ada ya n g meng in ja kn ya duluan, PLN kini menjad i milikmu!
```

7. BAYAR_SEWA(OTOMATIS)
   Perintah ini di p icu secara otomatis ketika p emain mendarat di p ro p erti berstatus OWNED milik p emain lain.

```
Kamu mendarat di Jakarta(JKT), milik Pemain B!
Kondisi : 2 rumah
Sewa : M600
Ua n g k a m u : M1.200-> M600
Ua n g P e m a in B: M800 -> M1.400
---
// J ik a p r o p e r t i d i g ada ik a n:
K a m u m e n da r at d i Ja k a r ta (J KT), milik Pe m a in B.
Pr o p e r t i ini seda n g d i g ada ik a n[M]. Tida k ada se w a y a n g d ik e n a k a n.
---
// Jika tidak mampu bayar:
Kamu tidak mampu membayar sewa penuh!(M600)
Uang kamu saat ini: M200 // Alur dilanjutkan ke Kebangkrutan
```

8. BAYAR_PAJAK(OTOMATIS)
   Perintah ini dipicu secara otomatis ketika pemain mendarat di petak PPH atau PBM.

```
// Kasus PPH: Kamu mendarat di Pajak Penghasilan(PPH)!
Pilih opsi pembayaran pajak:
1. Bayar flat M150
2. Bayar 10% dari total kekayaan
(Pilih sebelum menghitung kekayaan!)
Pilihan(1/2): 2
Total kekayaan kamu:
- Uang tunai : M1.500
- Harga beli properti: M1.400(termasuk yang digadaikan)
- Harga beli bangu nan: M300
Total : M3.200
Pa j ak 10% : M320
Ua n g kamu: M1.500-> M1.180
---
// Jika p ilih flat ta p i uan g tidak cuku p :
Kamu mendarat di Pa j ak Pen g hasilan(PPH)!
Pilih o p si p emba y aran p a j ak:
1. Ba y ar flat M150
2. Ba y ar 10% dari total keka y aan
(Pilih sebelum meng hitun g keka y aan!)
Pilihan(1/2): 1
Kamu tidak mam p u memba y ar p a j ak flat M150!
Ua n g k a m u saat ini: M80
// Al u r d il a n j ut k a n k e K eba n g kr uta n
---
// K asus PBM:
K a m u m e n da r at d i P a j a k B a r a n g M e w a h(PBM)!
P a j a k sebesa r M150 l a n g su n g d i p oto n g .
Ua n g k a m u : M1.500-> M1.350
---
// Jika tidak mampu bayar PBM: Kamu mendarat di Pajak Barang Mewah(PBM)!
Pajak sebesar M150 langsung dipotong.
Kamu tidak mampu membayar pajak!
Uang kamu saat ini: M80 // Alur dilanjutkan ke Kebangkrutan
```

9. GADA I
   Perintah untuk menggadaikan properti milik pemain ke Bank. Dapat dilakukan kapan saja selama giliran pemain.

```
> GADA I
=== Properti yang Dapat Digadaikan===
1. Jakarta(JKT) [BIRU TUA] Nilai Gadai: M200
2. Stasiun Gambir(GBR)[STASIUN]  Nilai Gadai: M100
Pilih nomor properti(0 untuk batal): 1
Jakarta berhasil digadaikan.
Kamu menerima M200 dari Bank.
Uang kamu sekarang: M700 Catatan: Sewa tidak dapat dipungut dari properti yang digada ikan.
---
// Jika masih ada ban g unan di color g rou p :
Jakarta tidak da p at di g adaikan!
Masih terda p at ban g unan di color g rou p [BIRU TUA].
Ban g unan harus di j ual terlebih dahulu.
Daftar ban g unan di color g rou p [BIRU TUA]:
1. Jakarta(JKT) - 2 rumah -> Nilai j ual ban g unan: M200
2. Ibu Kota Nusantara(IKN)- 1 rumah-> Nilai j ual ban g unan: M100
Jual semua ban g unan color g rou p [BIRU TUA]?(y /n): y
B a n g u n a n Ja k a r ta te r j ua l. K a m u m e n e rim a M200.
B a n g u n a n I bu K ota N usa n ta r a te r j ua l. K a m u m e n e rim a M100.
Ua n g k a m u se k a r a n g : M800
L a n j ut m e n gg ada ik a n Ja k a r ta ?( y /n): y
Ja k a r ta be rh as il d i g ada ik a n.
K a m u m e n e rim a M200 da ri B a nk.
Ua n g k a m u se k a r a n g : M1.000
Catata n: Se w a t i da k da p at d i p u n g ut da ri p r o p e r t i y a n g digadaikan.
---
// Jika tidak ada yang bisa digadaikan: Tidak ada properti yang dapat digadaikan saat ini.
```

10. TEBUS
    Perintah untuk menebus properti yang sedang digadaikan. Dapat dilakukan kapan saja selama giliran pemain.

```
> TEBUS
=== Properti yang Sedang Digadaikan===
1. Jakarta(JKT) [BIRU TUA] [M]  Harga Tebus: M400
2. Bandung(BDG) [HIJAU] [M]  Harga Tebus: M300
Uang kamu saat ini: M800 Pilih nomor properti(0 untuk batal): 1
Jakarta berhasil ditebus!
Kamu membayar M400 ke Bank.
Uang kamu sekarang: M400
---
// Jika uan g tidak cuku p :
Ua n g kamu tidak cuku p untuk menebus Jakarta.
Har g a tebus: M400 \| Ua n g kamu: M150
---
// Jika tidak ada y an g di g adaikan:
Tidak ada p ro p erti y an g sedan g di g adaikan.
```

11. BANGUN
    Perintah untuk memban g un rumah atau men g u p g rade ke hotel. Da p at dilakukan ka p an sa j a selama g iliran p emain.

```
> BAN GU N
=== Co l o r G r ou p y a n g M e m e n u hi S y a r at ===
1.[BIRU T U A]
- Jakarta(JKT) : 1 rumah(Harga rumah: M200)
- Ibu Kota Nusantara(IKN) : 1 rumah(Harga rumah: M200)
2.[HIJAU]
- Bandung(BDG) : 0 rumah(Harga rumah: M150)
- Denpasar(DEN) : 0 rumah(Harga rumah: M150)
- Mataram(MTR) : 0 rumah(Harga rumah: M150)
Uang kamu saat ini: M1.500 Pilih nomor color group(0 untuk batal): 1
Color group[BIRU TUA]: // Hanya petak yang memenuhi syarat pemerataan yang ditampilkan
- Jakarta(JKT) : 1 rumah <- dapat dibangun
- Ibu Kota Nusantara(IKN): 1 rumah <- dapat dibangun
Pilih petak(0 untuk batal): 1 // Jakarta
Kamu membangun 1 rumah di Jakarta. Biaya: M200 Uang tersisa: M1.300
- Jakarta(JKT) : 2 rumah
- Ibu Kota Nusantara(IKN): 1 rumah
---// Upgrade ke hotel(saat semua petak sudah punya 4 rumah):
Color group[BIRU TUA]:
- Jakarta(JKT) : 4 rumah <- siap upgrade ke hotel
- Ibu Kota Nusantara(IKN): 4 rumah<- siap upg rade ke hotel
Seluruh color g rou p [BIRU TUA] sudah memiliki 4 rumah. Siap di-u pg rade ke hotel!
Pilih p etak(0 untuk batal): 1 // Jakarta
U pg rade ke hotel? Bia y a: M200(y /n): y
Jakarta di-u pg rade ke Hotel!
Ua n g tersisa: M1.100
---
// Jika p etak sudah berstatus hotel:
Color g rou p [BIRU TUA]:
- Jakarta(JKT) : Hotel <- sudah maksimal, tidak da p at diban g un
- Ibu Kota Nusantara(IKN): 4 rumah<- siap u pg rade ke hotel
Pilih p eta k(0 u n tu k bata l): 2 // Ibu K ota N usa n ta r a
U pg r ade k e h ote l? Bi a y a : M200( y /n): y
I bu K ota N usa n ta r a d i-u pg r ade k e H ote l!
Ua n g te r s i sa : M900
---
// J ik a s y a r at m o n o p o li be l u m te r p e n u hi:
Ti da k ada co l o r g r ou p y a n g m e m e n u hi s y a r at u n tu k d i ba n g u n.
K a m u h a r us m e miliki se l u r u h p eta k da l a m satu co l o r g r ou p
```

12. LELANG(OTOMATIS)
    Perintah ini dipicu otomatis ketika salah satu dari kondisi berikut terpenuhi:
    ● Pemain menolak membeli properti
    ● Pemain tidak mampu membeli
    ● Pemain bangkrut ke Bank(semua properti dilelang)

```
Properti Jakarta(JKT) akan dilelang!
Urutan lelang dimulai dari pemain setelah Pemain A.
Giliran: Pemain B
Aksi(PASS/ BID<jumlah>)
> BID 100
Penawaran tertinggi: M100(Pemain B)
Giliran: Pemain C
Aksi(PASS/ BID<jumlah>)
> BID 150
Penawaran tertingg i: M150(Pemain C)
Giliran: Pemain D Aksi(PASS/ BID<j umlah>)
> PASS
Giliran: Pemain A Aksi(PASS/ BID<j umlah>): > PASS
Giliran: Pemain B Aksi(PASS/ BID<j umlah>): > PASS
Lelan g selesai!
P e m e n a n g : P e m a in C
H a r g a a khir: M150
Pr o p e r t i Ja k a r ta (J KT) kini d imiliki P e m a in C .
```

13. FESTIVAL(OTOMATIS)
    Perintah ini dipicu saat pemain mendarat di petak festival.

```
Kamu mendarat di petak Festival!
Daftar properti milikmu:
- JKT(Jakarta) - BDG(Bandung) - DEN(Denpasar)
Masukkan kode properti: JKT
Efek festival aktif!
Sewa awal: M5000
Sewa sekarang: M10000 Durasi: 3 giliran
// SKENARIO MEM ILIH FESTIVAL YANG SAMA
Masukkan kode properti: JKT
Efek diperkuat!
Sewa sebelumnya: M10000 Sewa sekarang: M20000 Durasi di-reset menjad i: 3 g iliran
// SKENARIO MEM ILIH FESTIVAL YANG SAMA NAMUN MULTIPLIER SEWA MAKSIMAL
Masukkan kode p ro p erti: JKT
Efek sudah maksimum(harg a sewa sudah di g andakan ti g a kali)
Durasi di-reset men j adi: 3 g iliran
// SKENARIO ERROR(TAMPILAN DIBEBASKAN, DI BAWAH HANYA CONTOH)
Masukkan kode p ro p erti: XYZ
-> Kode p ro p erti tidak valid!
M asu kk a n k ode p r o p e r t i: MND
-> Pr o p e r t i bu k a n milikm u !
// Inf o rm as i m e n g e n a i e f e k f est iv a l d i ta m p ilk a n j u g a m e l a l u i out p ut C ETAK_AKTA, C ETAK_PR O PERTI, da n BAYAR_ S EWA. Cu k u p ta m p ilk a n k ete r a n g a n e f e k f est iv a l a k t if, s i sa du r as i, da n h a r g a se w a te r ba r u
```

14. BANGKRUT DAN AMBIL ALIH ASET(OTOMATIS)
    Perintah ini dipicu otomatis ketika pemain tidak memiliki cukup uang tunai untuk membayar kewajiban. Sistem terlebih dahulu menghitung potensi dana dari likuidasi seluruh aset. Jika likuidasi dapat menutup kewajiban, pemain diarahkan ke panel likuidasi dan wajib melikuidasi hingga kewajiban terpenuhi. Jika tidak, pemain langsung dinyatakan bangkrut.
    Skenario 1: Likuidasi dapat menutup kewajiban

```
Skenario 2: Likuidasi tidak da p at menutu p kewa j iban p emba y aran ke p emain lain
K a m u t i da k da p at m e m ba y a r se w a M1200 k e p ada U n a m e 2!
Kamu tidak dapat membayar sewa M600 kepada Uname2!
Uang kamu : M200
Total kewajiban: M600 Kekurangan : M400
Estimasi dana maksimum dari likuidasi:
Jual Jakarta(JKT) [MERAH]  → M400 Gadai Kupang (KPG) [UNGU]  → M100 Total potensi → M500
Dana likuidasi dapat menutup kewajiban.
Kamu wajib melikuidasi aset untuk membayar.
=== Panel Likuidasi===
Ua n g kamu saat ini: M200 \| Kewa j iban: M600
[Jual ke Bank]
1. Jakarta(JKT) [MERAH]  Harg a Jual: M400
2. Bandun g (BDG) [MERAH]  Harg a Jual: M375(termasuk 1 rumah: M75)
[Gadaikan]
3. Ku p an g (KPG) [UNGU]  Nilai Gadai: M100
Pilih aksi(0 j ika sudah cuku p) : 1
Jakarta ter j ual ke Bank. Kamu menerima M400.
Ua n g k a m u se k a r a n g : M600
K e w a j i ba n M600 te r p e n u hi. M e m ba y a r k e U n a m e 2...
Ua n g k a m u : M600 → M0
Ua n g U n a m e 2: M300 → M900
Uang kamu : M150
Total kewajiban: M1200
Estimasi dana maksimum dari likuidasi:
Jual semua properti+ bangunan → M750 Total aset+ uang tunai : M900
Tidak cukup untuk menutup kewajiban M1200.
Uname1 dinyatakan BANGKRUT!
Kreditor: Uname2
Pengalihan aset ke Uname2: - Uang tunai sisa : M150
- Bandung(BDG) [MERAH]  OWNED(1 rumah) - Kupang (KPG) [UNGU]  MORTGAGED[M]
Uname2 menerima semua aset Uname1.
Uname1 telah keluar dari permainan.
Permainan berlanjut dengan 3 pemain tersisa.
```

Skenario 3: Likuidasi tidak dapat menutup kewajiban pembayaran ke bank

```
Kamu tidak dapat membayar Pajak Barang Mewah M800!
Uang kamu : M100
Total kewaj iban: M800
Estimasi dana maksimum dari likuidasi:
Jual semua p ro p erti+ ban g unan → M550
Total aset+ uan g tunai : M650
Tidak cuku p untuk menutu p kewa j iban M800.
Uname3 din y atakan BANGKRUT!
Kreditor: Bank
Uan g sisa M100 diserahkan ke Bank.
Seluruh p ro p erti dikembalikan ke status BANK.
Ban g unan dihancurkan — stok dikembalikan ke Bank.
Pro p erti akan dilelan g satu p er satu:
→ L e l a n g : Ja k a r ta (J KT)...
→ L e l a n g : K uta i (KU T)...
[Le l a n g be r j a l a n sesua i m e k a ni s m e LELAN G]
U n a m e 3 te l a h k e l ua r da ri p e rm a in a n.
P e rm a in a n be rl a n j ut de n g a n 2 p e m a in te r s isa .
```

15. SIMPAN
    Perintah untuk menyimpan seluruh state permainan ke file. Hanya dapat dipanggil di awal giliran pemain(belum melakukan aksi apapun).

```
> SIMPAN game_sesi1.txt
Menyimpan permainan...
Permainan berhasil disimpan ke: game_sesi1.txt
---
// Jika nama file sudah ada:
File"game_sesi1.nmp" sudah ada.
Timpa file lama?(y/n): y File berhasil ditimpa.
---
// Jika gagal menyimpan: Gagal menyimpan file! Pastikan direktori dapat ditulis.
```

16. MUAT
    Perintah untuk memuat state permainan dari file. Hanya dapat dilakukan sebelum p ermainan dimulai.

```
> MUAT g ame_sesi1.txt
Memuat p ermainan...
Permainan berhasil dimuat. Melan j utkan g iliran Uname2...
---
// Jika file tidak ditemukan:
File" g ame_sesi1.txt" tidak ditemukan.
---
// J ik a fil e r usa k atau f o rm at t i da k d ik e n a li:
Ga g a l m e m uat fil e ! Fil e r usa k atau f o rm at t i da k d ik e n a li.
```

17. CETAK_LOG
    Perintah untuk menampilkan entri log. Dapat diikuti argumen opsional untuk menentukan jumlah entri terbaru yang ditampilkan. Jika tidak memakai argumen, cetak semua log.
    Cetak semua log
    Den g an ar g umen j umlah baris

```
> CETAK_LOG 5
=== Lo g Transaksi(5 Terakhir)===
[Turn 14] Uname3 \| SEWA \| Ba y ar M10000 ke Uname2(JKT, festival aktif x2)
[Turn 14] Uname4 \| KARTU \| Pakai Tele p ortCard → p indah ke Bali(BAL)
[Turn 14] Uname4 \| BELI \| Beli Bali(BAL) seharg a M300 [Turn 15] Uname1 \| DADU \| Lem p ar: 5+2=7 → mendarat di Stas i u n(KA I)
[Tu rn 15] U n a m e 1 \| RA ILR O AD \| KA I kini milik U n a m e 1(oto m at i s)
```

18. MENANG(OTOMATIS)
    Perintah ini di p icu saat kondisi kemenan g an ter p enuhi

```
> CETAK_LOG
=== Log Transaksi Penuh===
[Turn 13] Uname1 \| DADU \| Lempar: 3+4=7 → mendarat di Bandung(BDG)
[Turn 13] Uname1 \| SEWA \| Bayar M300 ke Uname3(BDG, 1 rumah)
[Turn 13] Uname2 \| DADU \| Lempar: 6+6=12(double) → mendarat di Parkir [Turn 13] Uname2 \| DOUBLE \| Giliran tambahan ke-1 [Turn 13] Uname2 \| DADU \| Lempar: 2+3=5 → mendarat di Festival [Turn 13] Uname2 \| FESTIVAL \| Jakarta(JKT): sewa M5000 → M10000, durasi 3 giliran [Turn 14] Uname3 \| DADU \| Lempar: 1+2=3 → mendarat di Jakarta(JKT)
[Turn 14] Uname3 \| SEWA \| Bayar M10000 ke Uname2(JKT, festival aktif x2)
[Turn 14] Uname4 \| KARTU \| Pakai Telepo rtCard → p indah ke Bali(BAL)
[Turn 14] Uname4 \| BELI \| Beli Bali(BAL) seharg a M300
// Tampilan menang dibebaskan sesuai kreativitas kalian.
// Berikut hanya penjelasan informasi apa saja yang harus // ditampilkan
// SKENARIO MAX TURN
// Tampilkan tanda permainan selesai // Tampilkan semua informasi penentu kemenangan // Tampilkan pemenangnya(bisa lebih dari satu jika seri terus) // Tampilkan summary permainan(opsional)
Permainan selesai!(Batas giliran tercapai)
Rekap pemain:
Pemain A
Uang : M5000
Properti : 6 Kartu : 2
Pemain B
Uang : M5000
Properti : 5 Kartu : 4
Pemain C
Uang : M4500
Pemenan g : Pemain A
// SKENARIO BANKRUPTCY
// Tam p ilkan tanda p ermainan selesai
// Tam p ilkan p emenan g n y a(hany a satu)
// Tam p ilkan summar y p ermainan(op sional)
Permainan selesai!(Semua p emain kecuali satu ban g krut)
Pemain tersisa:
- Pemain C
Pemenan g : Pemain C
```

19. KESEMPATAN DAN DANA UMUM(OTOMATIS)
    Perintah ini di p icu saat p emain mendarat di p etak kesem p atan atau dana umum.

```
K a m u m e n da r at d i P eta k K ese m p ata n!
M e n g a m b il k a r tu ...
Kartu:"Mundur 3 petak." Bidak dipindahkan ke IKN.
Kamu mendarat di Petak Dana Umum!
Mengambil kartu...
Kartu:"Biaya dokter. Bayar M700." Kamu membayar M700 ke Bank. Sisa Uang= M1.500.
// Jika tidak mampu bayar:
Kamu tidak mampu membayar biaya dokter!(M700)
Uang kamu saat ini: M200 // Alur dilanjutkan ke Kebangkrutan
```

20. GUNAKAN KEMAMPUAN
    Perintah yang digunakan oleh pemain untuk mengaktifkan Kartu Kemampuan Spesial yang ada di tangannya.

```
> GUNAKAN_KEMAMPUAN
Daftar Kartu Kemampuan Spesial Anda:
1. MoveCard- Maju 4 Petak
2. ShieldCard- Kebal tagihan atau sanksi selama 1 turn
0. Batal
Pilih kartu y an g in g in di g unakan(0-2): 2
ShieldCard diaktifkan! Anda kebal terhada p ta g ihan atau sanksi selama g iliran ini.
> LEMPAR_DADU
Men g ocok dadu...
Hasil: 1+ 2= 3
Mema j ukan Bidak Pemain1 seban y ak 3 p etakk...
Bidak mendarat di: Dana Umum.
Men g ambil kartu...
Kartu:"Bia y a dokter. Ba y ar M700."
[S HIELD A C TIVE]: Efe k S hi e l dCa r d m e lin du n g i An da !
T a g ih a n M700 d i bata lk a n. Ua n g An da teta p : M150.
// S k e n a ri o Suda h M e n gg u n a k a n K e m a m p ua n p ada G ilir a n Ini
> GU NAKAN_KEMAMP U AN
K a m u suda h m e n gg u n a k a n k a r tu k e m a m p ua n p ada g ilir a n ini!
P e n gg u n aa n k a r tu d i batas i m a k s im a l 1 k a li da l a m 1 g ilir a n.
```

21. DROP KARTU KEMAMPUAN(OTOMATIS)
    Perintah ini dipicu secara otomatis pada awal giliran jika seorang pemain sudah memiliki 3 kartu kemampuan di tangan dan menerima kartu ke-4.

```
// Skenario Digunakan Setelah Lempar Dadu > GUNAKAN_KEMAMPUAN
Kartu kemampuan hanya bisa digunakan SEBELUM melempar dadu.
Kamu mendapatkan 1 kartu acak baru!
Kartu yang didapat: TeleportCard.
PERINGATAN: Kamu sudah memiliki 3 kartu di tangan(Maksimal 3)! Kamu diwajibkan membuang 1 kartu.
Daftar Kartu Kemampuan Anda:
1. MoveCard- Maju 4 Petak
2. ShieldCard- Kebal tagihan/sanksi
3. LassoCard- Menarik lawan
4. TeleportCard- Pindah ke petak manapun
Pilih nomor kartu yang ingin dibuang(1-4): 3
LassoCard telah dibuang. Sekarang kamu memiliki 3 kartu di tangan.
```

## Bonus

### Antarmuka Pengguna Grafis/Graphical User Interface(GUI)

Tampilan antarmuka berbasis teks(CLI) membuat para nimons tidak leluasa melihat visualisasi peta di Nimonspoli. Untuk membantu para nimons menguasai kota kota di Nimonspoli, buatlah sebuah tampilan antarmuka pengguna grafis sederhana.
Tentunya APG harus dibuat menggunakan bahasa C++ agar tetap cocok dengan sistem yang telah ada di laboratorium para Nimons. Pastikan implementasi APG hanya untuk tampilan saja, tidak diperbolehkan menggunakan game engine, tidak boleh mengatur game state dan sebagainya.
Tidak ada spesifikasi khusus seperti apakah tampilan antarmukanya. Kerjakan sesuai dengan kreativitas kalian.
Keseluruhan fungsionalitas dalam aplikasi harus dapat berjalan dan diterapkan dengan baik dalam APG tanpa terkecuali. Saran Library: Raylib, wxWidget, imgui, sfml

### COM

Para nimons terkadang merasa kesepian saat tidak ada teman untuk diajak bermain Nimonspoli. Untuk menemani dan membantu pada nimons menguji strategi terbaik mereka, buatlah sebuah fitur pemain komputer/Computer Player(COM) yang dapat bertindak sebagai lawan main secara otomatis. Pastikan COM mampu melakukan aksi-aksi dasar permainan secara mandiri(seperti melem p ar dadu, memutuskan p embelian p ro p erti, hin gg a memba y ar sewa) tanp a adan y a cam p ur tan g an dari p emain manusia. Tidak ada s p esifikasi khusus sebera p a cerdas COM ini. Keseluruhan fun g sionalitas p ermainan harus teta p ber j alan den g an lancar saat COM ikut serta, dan setia p aksi y an g diambil oleh COM harus diinformasikan den g an j elas ke p ada p emain, baik melalui CLI mau p un APG(j ika kalian j u g a men g er j akan bonus tersebut).

### Papan Dinamis

Salah satu inti dari OOP adalah kemam p uan men g abstraksikan sesuatu. Hal ini terkait den g an sebera p a baik kalian men gg eneralisasi sebuah konse p sehin gg a sistem tidak terikat p ada satu bentuk y an g kaku. Bonus ini hadir untuk men g u j i se j auh mana desain kalian benar-benar fleksibel, bukan sekadar hard-coded untuk satu konfi g urasi p a p an.
Pada mode Pa p an Dinamis, susunan p etak p ada p a p an p ermainan tidak la g i men g ikuti urutan teta p y an g telah ditentukan di s p esifikasi. Seba g ai g antin y a, susunan p etak dibaca dari sebuah file konfi g urasi tambahan y an g da p at dikustomisasi. Jumlah p etak tidak harus 40, dan urutan serta j enis p etak da p at diatur bebas, selama memenuhi s y arat berikut:
● Teta p terda p at te p at 1 Petak Mulai(GO) dan 1 Petak Pen j ara.
● Jumlah petak minimal adalah 20 dan maksimal 60.
● Setiap properti yang direferensikan di papan harus terdefinisi di property.txt.
● Seluruh mekanisme permainan(sewa, lelang, festival, pajak, dll.) harus tetap berjalan dengan benar di atas konfigurasi papan apapun.
Jika desain kelas kalian sudah cukup abstrak dan terstruktur, implementasi ini seharusnya tidak memerlukan perombakan besar. Cukup ubah bagian yang membaca dan membangun papan. Jika tidak, kalian akan tahu di mana letak masalahnya.

### Kreativitas

Bantulah para nimons dalam memonopoli dunia Nimonspoli dengan cara membuatkan fitur tambahan bagi mereka. Maksimal fitur kreativitas yang diimplementasikan adalah 1. Fitur yang diimplementasikan sebaiknya tidak mengubah/mengganggu fitur yang sudah dituliskan pada spesifikasi wajib. Tentunya nilai dari fitur kreativitas ini bergantung terhadap kompleksitas fitur yang diimplementasikan(fitur sederhana seperti bantuan/keluar tidak dihitung yah).
Saran ide:
-Account Based& Leaderboard
-Kartu tambahan untuk Skill Cards

## Spesifikasi Sistem

### Ketentuan Umum

Buatlah aplikasi berbasis Antarmuka Baris Perintah(ABP)/ Command Line Interface(CLI) untuk permainan ini dalam bahasa C++ dan dapat di-compile dengan memanfaatkan makefile.

### Ketentuan Teknis

Berikut adalah hal-hal yang minimal wajib diimplementasikan di aplikasi yang Anda buat. Diperbolehkan menambah dengan memerhatikan konsep-konsep OOP serta desain dari aplikasi kalian!
Perlu dicatat, yang ditulis disini hanyalah minimal dan sangat besar kemungkinan kelompok kalian akan menambah abstraksi/konsep lainnya untuk membuat aplikasi yang kalian buat.

1. Inheritance dan Polymorphism Contoh: Tile(Petak Papan), SkillCard, serta ActionCard.
2. Exception dan Exception Handling
   a. Validasi uang tidak cukup saat membeli properti, membayar sewa, atau melakukan bid lelang.
   b. Validasi slot penuh pada kepemilikan Kartu Skill di tanga n maksimal 3.
   c. Validasi lainn y a j ika di p erlukan.
3. Function Overloadin g dan O p erator Overloadin g
   a. Pen gg unaan+ atau+= da p at di g unakan untuk menambah atau memoton g uan g tunai p emain(misal: saat menerima g a j i atau memba y ar denda).
   b. Pen gg unaan< atau> untuk membandin g kan total keka y aan ob j ek antar p emain p ada p enentuan klasemen p emenan g di akhir batas turn.
4. Abstract Class dan Virtual Function
   Contoh: Sebuah kelas abstrak Tile y an g memiliki virtual function(misaln y a onLanded()) y an g akan di-inherit oleh masin g -masin g kelas s p esifik se p erti Pro p ert y Tile, Utilit y Tile, dan S p ecialTile(Festival/Pen j ara).
5. Generic Function atau Generic Class
   Contoh: Generic class CardDeck<T> untuk men g -handle berba g ai j enis tum p ukan kartu y an g berbeda p ada p ermainan(sep erti tum p ukan Kartu Skill, kartu Chance, dan kartu Communit y Chest).
6. STL( Standard Tem p late Librar y)
   Bebas dipakai pada kelas apapun yang mendukung pembuatan gim. Wajib bersifat krusial ke struktur data dan mekanisme permainan, bukan hanya untuk print atau diletakkan pada sebuah kelas namun tidak terpakai. Wajib dipakai: Vector dan Map. Boleh menambah STL lainnya jika merasa diperlukan.

## Panduan Pengerjaan

Tugas Besar ini dikerjakan secara sendiri/berkelompok(senyaman kalian mengerjakan) yang berisi 5 anggota. Silakan mendata kelompok di . Serta buatlah IF2010 OOP- Kelompok Tugas Besar 1 tim pada Github Classroom dengan nama tim merupakan kode kelompok. Tim hanya dibuat oleh perwakilan salah satu anggota saja, sisanya bisa langsung join ke tim yang sudah dibuat
Semua link kebutuhan tugas besar ada di drive ini: Public
Adapun ketentuan lain seperti berikut:

1. Wajib bisa dijalankan di sistem operasi LINUX dengan C++11, C++14 atau C++17.
   a. Akan ada pengurangan nilai j ika terjadi anomali seperti keluaran yang tidak muncul karena sejauh pengerjaan, kalian mengerjakan di Windows.
   b. Berhak tidak dinilai jika program tidak jalan total atau gagal mensimulasikan mayoritas kondisi untuk test case yang dibuat asisten.
   c. Windows Subsystem Linux atau WSL diperbolehkan untuk substitusi sistem operasi LINUX.
   d. Dihimbau untuk setia p kelom p ok men g er j akan di WSL atau Linux dari p ada diakhir saat demo tidak j alan sama sekali/ada kasus unik.
2. Han y a di p erbolehkan men gg unakan p ustaka umum dan STL. Tuliskan j ustifikasi p ustaka y an g di p akai p ada la p oran anda.
3. Tidak boleh p la g iat dari internet mau p un kelom p ok lain.
4. Usahakan p en gg unaan statik, kelas dan method lebih ban y ak(~90%) dibandin g kan den g an p en gg unaan g lobal variable, fun g si dan p rosedur. Pen g ecualian p en gg unaan struct, tidak di p erkenankan sama sekali, silakan g unakan kelas dan ob j ek.
5. Dekom p osisi y an g baik dan im p lementasi y an g tidak terlalu kom p leks(sebuah method tidak terlalu p an j an g ). Pecah-p ecah dan buat method baru a g ar tidak terlalu kom p leks.
6. Dilaran g men gg unakan librar y y an g memodifikasi ti p e dari instance se p erti variant, an y , d y namic_cast, dll. Pen gg unaan d y namic cast di p erbolehkan teta p i tidak boleh overuse
   Selain p enilaian dari tes fun g sional, p ro g ram akan dinilai dari se g i desain arsitektur. Untuk menda p atkan nilai y an g maksimal, p astikan p ro g ram Anda mematuhi p rinsi p p rinsi p desain berikut:
7. Im p lementasi OOP y an g te p at!
   Jan g an sekadar men gg unakan 6 konse p OOP dasar han y a untuk men gg u g urkan kewa j iban. Gunakan konse p tersebut secara lo g is dan sesuai den g an u se case y an g dihada p i!
8. Prinsip DRY(Don’t Repeat Yourself)
   Tidak boleh memiliki kode duplikat, pindahkan ke fungsi/kelas.
9. Struktur Kelas dan Layered Architecture
   Bangun struktur kelas dan hierarki yang rapi dan mudah dipahami alurnya. Program wajib mengimplementasikan Layered Architecture untuk memisahkan tanggung jawab. Setidaknya terdiri dari 3 lapis:
   a. User Interaction(UI) Layer: Murni bertugas menangani input/output dari dan ke pengguna(layar terminal/GUI).
   b. Game Logic(Core) Layer: Inti aturan permainan, kalkulasi, manajemen state, dll. Intinya core logic nya di sini
   c. Data Access Layer: Murni bertugas menangani pembacaan dan penulisan file (seperti Konfigurasi atau fitur Save/Load)
10. Implementasikan layered architecture: jelaskan singkat bahwa ada 3 layer(user interaction, game logic, data access),
11. Prinsip SL dari SOLID
    a. S(Single Responsibility): Sebuah kelas hanya memiliki satu tanggung jawab.
    b. L(Liskov Substitution): Child class harus bisa merepresentasikan dan menggantikan parent class tanpa merusak fungsionalitas program
12. Terdapat “PENALTI KERAS” jika:
    a. Apabila paradigma program masih bersifat prosedural atau fungsional yang hanya dibun g kus dalam bentuk kelas a g ar seolah-olah terlihat seperti OOP. Semua kelas tidak memiliki behavior, han y a memiliki method run(), dan main program han y a bertu g as meman gg il run() tersebut satu p er satu dari atas ke bawah.
    b. God Class, Memban g un keseluruhan p ro g ram han y a den g an satu atau bebera p a kelas besar(God Class) dan main p ro g ram. Pastikan setia p kelas dan ob j ek y an g dibuat ikut ber p artisi p asi dan memiliki behavior dalam mekanisme sistem
    c. Membuat kelas y an g tan gg un g j awabn y a tidak relevan!
    Contoh: Membuat kelas den g an nama an gg ota kelom p ok dan kelas mere p resentasikan p eker j aan an gg ota kelom p ok tersebut
    d. Ob f uscation(Penamaan y an g buruk) terhada p nama kelas, variable, method. Penamaan harus deskri p tif dan memiliki arti. Contoh buruk(Kelas Foo atau Kelas Bar y an g berisi method a(), b(), c() dan memiliki variable int cnt, stack<T> st, vector<strin g > vs)
    e. Kom p osisi Ob j ek y an g buruk. Misaln y a, melakukan seluruh p roses eksekusi lo g ika di dalam konstruktor kelas. Contoh lainn y a mun g kin semua kelas p ada p ro g ram han y a p un y a method run kemudian main p ro g ram meman gg il run tersebut 1 p er 1

## Milestone Pengumpulan

Tugas besar ini akan dibagi menjadi 2 Milestone. Berikut adalah penjelasan setiap Milestone.

### Milestone 1

Pada milestone 1, kalian wajib melakukan beberapa hal berikut:

1. Menentukan anggota kelompok dan menulisnya pada sheets yang telah ditetapkan
2. Membuat tim pada github classroom
3. Mengumpulkan desain kelas sesuai dengan format berikut
   . Desain kelas ini akan menjadi tolak ukur IF2010_TB1_Laporan M1_XXX.docx bag ian bagaimana kalian mendesain suatu solusi persoalan berdasarkan konsep konsep OOP yang telah dipelajari. Desain ini juga akan membantu kalian sebagai bahan acuan untuk implementasi di M2.
   Pengumpulan milestone 1 dilakukan oleh perwakilan kelompok pada link berikut sebelum 12 April 2026 pukul 23.59 WIB

### Milestone 2

Pada milestone 2, kalian wajib melakukan beberapa hal berikut:

1. Melakukan implementasi tu g as besar secara keseluruhan
2. Men g um p ulkan form asistensi
3. Melakukan dokumentasi terhada p im p lementasi kalian p ada dokumen La p oran Tu g as Besar. Berikut adalah format La p oran Tu g as Besar IF2010_TB1_La p oran M2_XXX.docx La p oran dan kode p ro g ram dikum p ulkan bersamaan den g an rilis di Github Classroom sesuai deadline(24 A p ril 2026 p ukul 23.59).

## Asistensi

1. Asistensi sinkron wa j ib dilakukan minimal satu kali den g an ketentuan harus dilaksanakan sebelum deadline p en g um p ulan milestone 2, maksimal men g hubun g i asisten 24 j am sebelum asistensi.
2. Asistensi di g unakan untuk bertan y a men g enai s p esifikasi mau p un p en g er j aan y an g sekiran y a belum terba y an g dan belum j elas.
3. Alokasi asisten untuk tia p kelom p ok akan diumumkan setelah deadline p endaftaran kelom p ok.
4. Diwa j ibkan untuk men g isi form asistensi y an g ditandatan g ani p ula oleh asisten IF2010_TB1_Asistensi_XXX.docx

## QnA

Pertanyaan terkait dengan spesifikasi yang belum jelas atau hal-hal yang berkaitan dengan masalah teknis pada tugas besar dapat ditanyakan pada sheets berikut QnA- Tugas Besar 1

**Pertanyaan (Mahasiswa)** | **Jawaban (Asisten)**
Halo kak, salam kenal aku upin! | Halo! aku ipin!
untuk spek GUI, yang dimaksud dengan 'GUI tidak dapat mengubah game state' itu apakah berarti GUI hanya berupa output dan input masih kita lakukan di CLI? | Maksudnya adalah layer GUI hanya untuk tampilan. Jadi untuk semua perubahan game state itu tetap diwajibkan di logic C++ nya, bukan di layer GUI. Dikhawatirkan ada yang menggunakan game engine atau logic yang tercampur di layer GUI. Layer GUI diperbolehkan untuk input/output, tidak harus input dari CLI.
Ini dari spesifikasi beneran boleh sendirian (tidak berkelompok) atau salah tulis ya? | Tidak jadi ges, khawatirnya malah ada kelompok yang jumlahnya nanggung. Total anggota yang diperbolehkan itu 5, jika tidak mampu membentuk kelompok lagi karena sisa mahasiswa <5, gabung ke kelompok yang sudah ada. Kelompok tersebut menjadi beranggotakan 6 orang
"<< LELANG >> Ingin mengonfirmasi dari langkah ke-5 di mekanisme lelang. Berbunyi : Minimal terdapat satu pemain yang melakukan bid. Jika terjadi kondisi (4) tanpa ada pemain yang melakukan bid, maka pemain terakhir yang tidak melakukan pass harus melakukan bid. Lelang berlanjut seperti biasa. Ini maksudnya gimana ya bang Contoh kasusnya seperti apa ya bang? " | "Misal urutan permainan A, B, C, D Pemain A memicu lelang (mungkin karena tidak membeli properti) Lelang dimulai dari pemain setelah A. Giliran lelang: B, C, D, A B: pass C: pass D: pass Terjadi pass sebanyak jumlah pemain - 1, namun belum ada bid sama sekali di lelang ini. Oleh karena itu, pemain A (pemain selanjutnya) tidak memiliki opsi pass dan hanya bisa bid."
"<< LELANG >> Ketika seorang pemain nge-pass di putaran 1 lelang, tetapi ternyata lelang masih berlanjut karena pemain lain, apakah pemain yang nge-pass di putaran 1 akan dapat ikutan ngebid lagi" | Boleh, pass bukan berarti pemain keluar dari lelang. Pass berarti pemain skip bid saja di gilirannya. Kalo giliran pemain itu lagi (karena yang lainnya bid) boleh pass atau bid seperti biasa
"<< PAJAK >> Berbunyi : jika pemain memilih opsi flat namun uangnya tidak mencukupi, pemain langsung memasuki kondisi kebangkrutan tanpa ditawarkan opsi kedua. Jika hiduplah seorang jurangan yang banyak lahan-lahan tapi duitnya dikit sehingga ketika dia milih opsi persentase pajaknya gede dan dia ga bisa bayar, apakah dia dipindahkan ke kondisi flat dulu baru dimasukkan kondisi bangkrut atau langsung kondisi bangkrut?" | langsung bangkrut
"<<GOD CLASS>> Berbunyi: God Class, Membangun keseluruhan program hanya dengan satu atau beberapa kelas besar (God Class) dan main program. Pastikan setiap kelas dan objek yang dibuat ikut berpartisipasi dan memiliki behavior dalam mekanisme sistem Jika ada kelas paling atas lah, ibarat GameManager yang misalnya menyimpan vector of Petaks, vector of Players dan informasi global misalnya turnCount, dll. Apakah kelas itu by definition termasuk God Class? " | Kuncinya ada di tanggung jawab. Kalau tanggung jawabnya cuma sebagai orkestrator yang cuma nyuruh nyuruh objek lain sesuai tanggung jawabnya mah tetep aman ya. Kecuali yang dilakuin itu cmn nanya nanya info state kelas lain terus ujung ujungnya kerjaannya ttp dilakuin sm Game manager itu, itu god class. Jadi ttp pastiin kerjaan tiap objek yang seharusnya punya tanggung jawab itu, ttp dilakuin sama objek masing masing. Bukan sama satu kelas tertentu doang
"Di antara tool-tools berikut, mana yang diperbolehkan (jika tidak mengapa): 1. CMake (bukan make) untuk build sehingga bisa include library seperti SFML lebih rapi 2. LLVM tools seperti Clangd (dan clang-format serta clang-tidy) untuk preformat kode agar pakai standar dan konvensi yang sama 3. Github workflows/action untuk autoformat kode/commit sebelum di push" | gaskeun, boleh yah
Kak kalau kami bikin diagram kelas di milestone 1 tanpa pikirin dulu spesifikasi bonus boleh ga?? tapi nanti di implementasiinya bisa jadi ada bonus gituh | boleeh
Atur dadu manual apakah bisa setiap saat? | bisa setiap saat asal di awal giliran. dia interchangeable sama perintah LEMPAR_DADU
Bolehkah menambahkan fitur comment pada fitur Loading/Saving untuk memperjelas atribut-atribut yang disimpan pada file tersebut supaya lebih nyaman untuk dibaca? | Jika comment yang dimaksud adalah komentar pada file yang disave silakan disesuaikan saja selama data yang di save/load masih sama seperti di spek. Jika bukan, boleh dijelaskan fitur comment seperti apa?
Apakah class exception perlu digambarkan juga dalam class diagram? | Iya untuk custom class exception, untuk exception bawaan tidak usah
Apakah kartu "Bebas dari Penjara" termasuk kartu kemampuan? Dapatnya dari kartu Kesempatan dan Dana Umum kan ya? | Benar, Kartu "Bebas dari Penjara" termasuk ke dalam Kartu Kesempatan, tetapi tidak wajib untuk kalian implementasikan
Apakah Singleton Pattern boleh digunakan? | boleh buanget
"Apakah likuidasi/jual properti kepada bank atau player dapat dilakukan kapan saja? Atau hanya saat bangkrut? Lalu, apa commandnya? Soalnya belum ada walau sudah disebut berkali-kali terkait menjual properti ke bank dan player. Beberapa kemunculan menjual: ""Railroad tetap dapat digadaikan, ditebus, dan dijual ke pemain lain"" ""Properti berstatus MORTGAGED tidak dapat langsung dijual ke Bank dalam proses likuidasi""" | Likuidasi properti hanya dilakukan saat bangkrut ya. Untuk command-nya sendiri tidak ada karena proses ini terjadi secara otomatis saat pengguna tidak memiliki cukup cash ketika memiliki kewajiban pembayaran tertentu.
Apakah constructor dan destructor sebaiknya di-include dalam Class Diagram? | Boilerplate code seperti constructor, destructor, setter, getter dll yang tidak berkaitan dengan relasi antar kelas tidak perlu diinclude. Kecuali memang menunjukan keterkaitan hubungan antar kelas. Kira kira aja kalau emang ngaruh ke desain atau core logic include, klo ngga, tidak perlu
Apabila harga tebus = harga beli, apakah untuk property utility dan railroad memiliki harga tebus khusus? agar pemain tidak bisa mengeksploitasi gadai-tebus properti tersebut apabila harga tebus=0. Dan apakah pemain hanya dapat menebus property gadaian dari miliknya sendiri? | Harga tebus untuk utility dan railroad adalah 2x harga gadai yaa. Serta ya, pemain hanya bisa menebus property miliknya sendiri.
"<< Kemampuan Spesial >> Di Kemampuan Spesial itu ada ""Demolition Card"" berbunyi ""DemolitionCard (2 lembar): Menghancurkan satu properti milik pemain lawan."" Hal ini maksdunya gimana ya? Apakah ini pengubahan status OWNED => BANK atau hanya menghancurkan rumah/hotel Nah tapi untuk keduanya, kasusnya kalo misalnya ni Colorrun untuk warna X ada 3 bangunan a, b, c di mana a itu terdiri dari 3 rumah, b 3 rumah, c 2 rumah. Misalnya si player yang memakai kartu ini memilih untuk menghapus c, apa yang akan terjadi ke c, apakah ini bakal mengakibtkan exception karena menyalahi aturan selisih maksimal 1 atau bagaimana ya? Atau apakah pemilihan akan dilakukan secara random ? Selain itu, kalo tanah kosong belum ada bangunan kasusnya gimana ya? Thanks bang hehe " | Demolition Card itu bakal ngehacurin semua bangunan di atas properti lawan, tapi status kepemilikannya tetap OWNED alias nggak berubah jadi BANK. Kalau targetnya tanah kosong, berarti kartu ini nggak bakal ada efeknya karena murni cuma buat menghapus rumah atau hotel. Dalam kasus kamu, kalau kamu ngancurin properti C, bangunan di atasnya bakal hilang total, sedangkan properti A dan B tetap punya 3 rumah. Tapi ingat, si pemilik nggak bakal bisa nambah rumah lagi karena sekarang ada selisih bangunan yang lebih dari 1 antara C dengan A atau B, jadi dia harus ngebangun ulang C dulu biar rata sesuai aturan pemerataan.
"<<MUAT>> ""Program akan menampilkan menu dan memberikan 2 opsi untuk menginisiasi state program: Load Game dan New Game"" Namun, ada command untuk melakukan MUAT. Apakah load game harus melalui command MUAT? Atau boleh dengan melakukan input saat memilih load game atau new game saat program dijalankan?" | Command MUAT jadinya akan dihilangkan dan akan di-update di spesifikasi. Jadi, saat menjalankan program, pengguna akan memilih untuk memulai permainan dari awal atau menggunakan state yang sebelumnya sudah disimpan.
Bolehkah memakai class yang hanya berisi data layaknya struct? | boleh kasih contohnya?
Apabila ketika ingin mengimplementasikan program untuk milestone 2, ternyata ada desain yang keliru dari yang dibuat di milestone 1 apakah boleh melakukan penyesuaian dalam program yang dibuat atau desain program harus benar-benar mengikuti desain dari milestone 1? | Boleh melakukan penyesuaian dengan penjelasan pada laporan M2
Apakah parameter method pada class diagram harus ditulis? | Dianjurkan untuk ditulis. Cara kalian medekomposisi permasalahan menjadi desain kelas dan kelengkapan kalian dalam membuat UML + penjelasan, termasuk ke dalam komponen penilaian desain oo
Apakah akan diberikan seluruh file konfigurasi lengkap yang dibutuhkan? | Akan diberikan untuk keperluan demo
Apakah boleh pakai DTO untuk keperluan konfigurasi? | boleh
bolehkah pakai constexpr? |
Apakah ada sample config lengkap beserta mungkin game statenya juga? |
Bagaimana caara perolehan kartu kemampuan? Apakah ketika meninjak petak tertentu atau setiap menginjak start? Pada tabel petak yang terdapat pada spek, tidak terdapat 'Petak Kemampuan', hanya terdapat 'Petak Kesempatan' dan 'Petak Dana Umum'. | Kartu kemampuan otomatis didapat setiap awal giliran
Apakah boleh menggunakan std::unique_ptr untuk menyimpan vector berisi pointer? | Diperbolehkan ya
"Dalam konteks format save player, di spek menyatakan format berikut: <USERNAME> <UANG> <POSISI_PETAK> <STATUS> dengan status hanya bisa ACTIVE, BANKRUPT, atau JAILED. Masalahnya, di saat yang sama mekanisme penjara di spec menyatakan ""Batas giliran percobaan dengan melempar dadu adalah 3 giliran. Pada giliran ke-4, pemain wajib keluar penjara dengan cara membayar denda."" Artinya, pemain harus tau sudah giliran berapa dia di penjara. Maka dari itu, apakah saya boleh membuat agar format status di encode menjadi: - JAILED_1 (giliran pertama di penjara); - JAILED_2 (giliran kedua); - JAILED_3 (giliran ketiga)?" | Diperbolehkan ya
Apakah jika mengerjakan bonus GUI, program harus bisa dijalankan dengan terminal saja juga? | Tidak harus
