Kartu kesempatan merupakan kartu dengan kemampuan khusus dimana ketika seorang player berhenti di petak "Kartu Kesempatan" maka akan ada suatu aktivitas yang dijalankan terhadap player tersebut. Saat ini ada 3 kartu kesempatan:
1. Pergi ke penjara 
2. Mundur 3 petak
3. Pergi ke stasiun terdekat

Saya ingin menambahkan bonus kreatifitias dengan menambahkan kartu bertipe ini. Tambahkan 2 kartu bertipe "Kartu Kesempatan" dengan ketentuan sebagai berikut:
1. Bebas dari penjara. Jika seorang player mendapatkan kartu ini, maka kartu ini dapat membebaskan player tersebut dari penjara jika player itu jatuh ke penjara.
2. Pergi ke festival. Jika seorang player mendapatkan kartu ini, maka player tersebut akan maju ke festival paling dekat.

Logika tambahan:
Untuk kartu festival logikanya sama kayak pergi ke penjara. Jadi ketika seorang player mendapatkan kartu ini maka player tersebut langsung berpindah ke festival terdekat. Tetapi, ketika seorang mendapatkan kartu bebas dari penjara maka kartu tersebut disimpan (diletakkan di index ke 4).

Jadi, pada inventory punya index 1 sampai 4. Index ke 4 dikhususkan untuk kartu kesempatan(bebas dari penjara)


Tambahkan juga 2 kartu untuk kartu dana umum
1. Kartu dana umum "Arisan", yaitu player yang mendapatkan kartu ini akan mendapatkan uang M300 dari bank. Uang player ini bertambah M300 dari bank.
2. Kartu dana umum "Begal", yaitu player yang mendapatkan kartu ini harus membayar M200 ke bank. Uang player ini berkurang M200 ke bank.

Untuk kartu dana umum ini sama logika nya dengan kartu umum sebelumnya. Hanya menambah variansi dari kartu ini.

implementasi bonus ini pada kode saya dan tetap perhatikan aturan spesifikasi.