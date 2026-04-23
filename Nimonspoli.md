# Nimonspoli

Nimonspoli adalah sebuah proyek permainan papan digital yang terinspirasi dari permainan Monopoly, dikembangkan menggunakan bahasa C++. Proyek ini memiliki struktur modular yang membagi logika permainan, model data, utilitas, dan tampilan ke dalam beberapa folder dan file terpisah untuk memudahkan pengembangan dan pemeliharaan.

## Deskripsi Singkat

Permainan ini mensimulasikan permainan papan Monopoly, di mana pemain dapat membeli properti, membayar pajak, mengambil kartu kesempatan, dan lain-lain. Proyek ini terdiri dari beberapa komponen utama seperti pengelolaan kartu, papan permainan, pemain, tile spesial, serta sistem penyimpanan dan pemuatan data permainan.

## Struktur Folder dan File

Berikut adalah struktur folder dan file pada repository ini:

```
makefile
README.md
Nimonspoli.md
config/
    misc.txt
    property.txt
    railroad.txt
    special.txt
    tax.txt
    utility.txt
data/
    Alloutput.txt
    Board.txt
include/
    core/
        CardManager.hpp
        Command.hpp
        CommandResult.hpp
        EffectManager.hpp
        GameEngine.hpp
        TurnManager.hpp
    models/
        ActionCard.hpp
        Board.hpp
        Card.hpp
        CardTile.hpp
        ChanceCards.hpp
        CommunityCards.hpp
        Deck.hpp
        Dice.hpp
        FestivalTile.hpp
        FreeParkingTile.hpp
        GameContext.hpp
        GoTile.hpp
        GoToJailTile.hpp
        JailTile.hpp
        LogEntry.hpp
        Player.hpp
        Property.hpp
        PropertyTile.hpp
        RailroadProperty.hpp
        SkillCard.hpp
        SkillCards.hpp
        StreetProperty.hpp
        TaxTile.hpp
        Tile.hpp
        UtilityProperty.hpp
    utils/
        ConfigLoader.hpp
        GameException.hpp
        Gamestateserializer.hpp
        Saveloadmanager.hpp
    views/
        AnsiTheme.hpp
        BoardRenderer.hpp
        CommandParser.hpp
        GameUI.hpp
        UiFormatter.hpp
src/
    main.cpp
    core/
        CardManager.cpp
        EffectManager.cpp
        GameEngine.cpp
        test_card.cpp
        test_engine.cpp
        TurnManager.cpp
    models/
        ActionCard.cpp
        Board.cpp
        Card.cpp
        CardTile.cpp
        ChanceCards.cpp
        CommunityCards.cpp
        Dice.cpp
        FestivalTile.cpp
        FreeParkingTile.cpp
        GameContext.cpp
        GoTile.cpp
        GoToJailTile.cpp
        JailTile.cpp
        LogEntry.cpp
        Player.cpp
        Property.cpp
        PropertyTile.cpp
        RailroadProperty.cpp
        SkillCard.cpp
        SkillCards.cpp
        StreetProperty.cpp
        TaxTile.cpp
        Tile.cpp
        UtilityProperty.cpp
    utils/
        ConfigLoader.cpp
        GameException.cpp
        Gamestateserializer.cpp
        Saveloadmanager.cpp
    views/
        AnsiTheme.cpp
        BoardRenderer.cpp
        CommandParser.cpp
        GameUI.cpp
        UiFormatter.cpp
```

## Penjelasan Folder Utama

- **config/**: Berisi file konfigurasi untuk properti, tile spesial, pajak, utilitas, dan lain-lain.
- **data/**: Berisi data keluaran dan papan permainan.
- **include/**: Berisi file header (deklarasi) untuk core logic, model, utilitas, dan tampilan.
- **src/**: Berisi file implementasi (.cpp) dari seluruh fitur dan modul permainan.
  - **core/**: Implementasi logika utama permainan seperti engine, turn manager, dan card manager.
  - **models/**: Implementasi model data seperti pemain, kartu, tile, dan properti.
  - **utils/**: Implementasi utilitas seperti loader konfigurasi, serialisasi game state, dan exception.
  - **views/**: Implementasi tampilan dan antarmuka pengguna berbasis teks.
- **makefile**: Skrip build otomatis untuk mengompilasi seluruh proyek.
- **README.md**: Penjelasan singkat dan instruksi penggunaan.
- **Nimonspoli.md**: Dokumentasi lengkap mengenai proyek ini.

## Cara Build dan Menjalankan

1. Pastikan sudah terinstall compiler C++ (misal: g++).
2. Jalankan perintah berikut di terminal:
   ```bash
   make
   ./main
   ```

## Kontributor

- Silakan tambahkan nama kontributor di sini.

## Lisensi

- Silakan tambahkan informasi lisensi di sini jika diperlukan.
