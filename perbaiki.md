format save and load masih salah. Gunakan format berikut.



Untuk format save and  load file save:
<TURN_SAAT_INI> <MAX_TURN>
<JUMLAH_PEMAIN>
<STATE_PEMAIN_1>
<STATE_PEMAIN_2>
…
<STATE_PEMAIN_N>
<URUTAN_GILIRAN_1> <URUTAN_GILIRAN_2> … <URUTAN_GILIRAN_N>
<GILIRAN_AKTIF_SAAT_INI>
<STATE_PROPERTI>
<STATE_DECK>
<STATE_LOG>

untuk format save and load state pemain:
<USERNAME> <UANG> <POSISI_PETAK> <STATUS>
<JUMLAH_KARTU_TANGAN>
<JENIS_KARTU_1> <NILAI_KARTU_1> <SISA_DURASI_1>
<JENIS_KARTU_2> <NILAI_KARTU_2> <SISA_DURASI_2>

Keterangan:
<USERNAME> adalah username pemain
<UANG> adalah jumlah uang pemain
<POSISI_PETAK> adalah kode petak tempat pemain berada
<STATUS> berisi status dari pemain  yang bernilai ACTIVE, BANKRUPT, atau JAILED (jika terjebak di penjara)
<JENIS_KARTU> adalah jenis kartu kemampuan (contoh: MoveCard, ShieldCard)
<NILAI_KARTU> adalah parameter nilai kartu tersebut (contoh: jumlah langkah untuk MoveCard, persentase diskon untuk DiscountCard).Untuk card yang tidak memiliki nilai tertentu, kosongkan nilai ini
<SISA_DURASI> adalah sisa durasi berlakunya kartu tersebut, hanya untuk kartu jenis DiscountCard. Untuk kartu jenis lain, kosongkan nilai ini

untuk format save and load state properti:
<JUMLAH_PROPERTI>
<KODE_PETAK> <JENIS> <PEMILIK> <STATUS> <FMULT> <FDUR> <N_BANGUNAN>
…

Keterangan
<KODE_PETAK> adalah kode petak dari properti
<JENIS> adalah jenis properti yang bernilai street, railroad, atau utility
<PEMILIK> berisi username pemilik atau BANK jika belum dimiliki
<STATUS> adalah status dari properti yang bernilai BANK, OWNED, atau MORTGAGED
<FMULT> adalah harga bangunan akibat festival yang bernilai 1, 2, 4, atau 8 (1 = tidak aktif)
<FDUR> adalah durasi dari festival yang bernilai 0–3 (0 = tidak aktif)
<N_BANGUNAN> bernilai 0–4 untuk rumah, atau H untuk hotel. Untuk properti selain street, isi dengan 0.

untuk format save and load state deck:
<JUMLAH_KARTU_DECK_KEMAMPUAN>
<JENIS_KARTU_1>
…

untuk format save and load state log:
<JUMLAH_ENTRI_LOG>
<TURN> <USERNAME> <JENIS_AKSI> <DETAIL>
…

berikut adalah contoh dari isi file yang berisi state yang disimpan:
15 30
4
Uname1 1500 JKT ACTIVE
2
MoveCard 5
ShieldCard
Uname2 800 BDG ACTIVE
0
Uname3 2200 PKR ACTIVE
1
DiscountCard 30 1
Uname4 0 PJR BANKRUPT
0
Uname2 Uname4 Uname1 Uname3
Uname2
22
JKT  street   Uname1 OWNED     2 2 1
BDG  street   Uname2 MORTGAGED 0 1 0
KAI  railroad Uname1 OWNED     1 0 0
SBY  street   BANK   BANK      0 1 0
PLN  utility  Uname3 OWNED     1 0 0
...
12
MoveCard
DiscountCard
TeleportCard

...
10
1 Uname1 BELI Beli Jakarta (JKT) seharga M400
1 Uname2 DADU Lempar: 4+5=9 mendarat di Surabaya (SBY)
...


Transaction Logger mencatat setiap kejadian signifikan dalam permainan secara otomatis ke dalam log terstruktur. Logger bekerja di latar belakang dan tidak memerlukan perintah khusus untuk aktif. 
Setiap entri log memuat informasi berikut: nomor turn, username pemain yang relevan, jenis aksi, dan detail aksi (termasuk nilai uang jika ada). Jenis aksi yang dicatat meliputi:
●	Lemparan dadu dan pergerakan bidak
●	Pembelian properti (termasuk perolehan Railroad/Utility otomatis)
●	Pembayaran sewa dan pajak
●	Pembangunan dan penjualan bangunan
●	Gadai dan tebus properti
●	Penggunaan kartu kemampuan spesial
●	Pengambilan kartu Kesempatan dan Dana Umum
●	Setiap bid dan hasil akhir lelang
●	Aktivasi dan penguatan efek festival
●	Kebangkrutan dan pengambilalihan aset
●	Save dan load permainan
Logger menyimpan full log selama sesi berlangsung di memori. Saat game di-save, full log ikut tersimpan ke dalam file.
