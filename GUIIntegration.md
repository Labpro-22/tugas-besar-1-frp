# Nimonspoli - Panduan Integrasi C++ SFML GUI & GameEngine

## Konteks Proyek untuk AI Assistant (Copilot)
Proyek ini adalah "Nimonspoli", game papan turn-based (mirip Monopoli) berbasis C++. 
1. **Core Logic** sudah selesai dan berada di folder `core/` dan `models/`. Logika ini murni mereturn status (tidak ada blocking input `std::cin` di dalam loop utama engine).
2. **GUI Layer** menggunakan **SFML** dan diletakkan secara terisolasi di folder `viewsGUI/`. 
3. **Aturan Integrasi Mutlak:** Kelas-kelas di `viewsGUI/` TIDAK BOLEH memanipulasi data model secara langsung. GUI hanya membaca status (Read-Only) dari Engine, merender visual, dan mengirimkan trigger/perintah ke `GameEngine` melalui callback (Observer/Command pattern).

## Arsitektur State Machine GUI
Karena SFML menggunakan Game Loop `while(window.isOpen())`, kita menggunakan State Machine untuk menangani input non-blocking:
- `IDLE`: Pemain bisa klik tombol di ActionPanel (Lempar Dadu, Bangun, dll).
- `ANIMATING`: Bidak sedang bergerak, input UI dimatikan sementara.
- `WAITING_CONFIRMATION`: Muncul `PopupBox` (Modal dialog). Input diluar kotak dialog diblokir sampai pemain memilih "Ya/Tidak".

---

## 1. File: `viewsGUI/Theme.hpp`
```cpp
#ifndef THEME_HPP
#define THEME_HPP

#include <SFML/Graphics.hpp>

namespace viewsGUI {
    struct Theme {
        // Palet Warna Nimonspoli
        static inline const sf::Color Merah     = sf::Color(173, 74, 74);   // #ad4a4a
        static inline const sf::Color Pink      = sf::Color(218, 143, 166); // #da8fa6
        static inline const sf::Color Oranye    = sf::Color(217, 119, 80);  // #d97750
        static inline const sf::Color BiruMuda  = sf::Color(149, 158, 160); // #959ea0
        static inline const sf::Color TileAksi  = sf::Color(243, 224, 207); // #f3e0cf
        static inline const sf::Color Coklat    = sf::Color(123, 59, 13);   // #7b3b0d
        static inline const sf::Color Kuning    = sf::Color(229, 187, 105); // #e5bb69
        static inline const sf::Color BiruTua   = sf::Color(47, 79, 111);   // #2F4F6F
        static inline const sf::Color Hijau     = sf::Color(95, 143, 107);  // #5F8F6B
        static inline const sf::Color Utilitas  = sf::Color(183, 183, 183); // #B7B7B7

        // Warna UI Tambahan
        static inline const sf::Color Background = sf::Color(230, 240, 235);
        static inline const sf::Color Border     = sf::Color::Black;
        static inline const sf::Color TextDark   = sf::Color(30, 30, 30);
    };
}
#endif
```

---

## 2. File: `viewsGUI/Button.hpp`
```cpp
#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

namespace viewsGUI {
    class Button {
    public:
        Button(sf::Vector2f position, sf::Vector2f size, const std::string& text, const sf::Font& font);

        void render(sf::RenderWindow& window);
        void update(sf::Vector2f mousePos); 
        bool handleMouseClick(sf::Vector2f mousePos); 

        void setOnClick(std::function<void()> callback);
        void setEnabled(bool enabled);
        bool isEnabled() const { return m_isEnabled; }

    private:
        sf::RectangleShape m_shape;
        sf::Text m_text;
        bool m_isHovered;
        bool m_isEnabled;
        std::function<void()> m_onClick;

        sf::Color m_normalColor;
        sf::Color m_hoverColor;
        sf::Color m_disabledColor;
    };
}
#endif
```

---

## 3. File: `viewsGUI/Button.cpp`
```cpp
#include "Button.hpp"
#include "Theme.hpp"

namespace viewsGUI {
    Button::Button(sf::Vector2f position, sf::Vector2f size, const std::string& textStr, const sf::Font& font) 
        : m_isHovered(false), m_isEnabled(true), m_onClick(nullptr) 
    {
        m_shape.setPosition(position);
        m_shape.setSize(size);
        m_shape.setOutlineThickness(2.0f);
        m_shape.setOutlineColor(Theme::BiruTua); 

        m_normalColor = sf::Color(250, 250, 250); 
        m_hoverColor = sf::Color(220, 230, 240);  
        m_disabledColor = sf::Color(200, 200, 200); 
        m_shape.setFillColor(m_normalColor);

        m_text.setFont(font);
        m_text.setString(textStr);
        m_text.setCharacterSize(16);
        m_text.setFillColor(Theme::TextDark);

        sf::FloatRect textRect = m_text.getLocalBounds();
        m_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
    }

    void Button::setOnClick(std::function<void()> callback) {
        m_onClick = callback;
    }

    void Button::setEnabled(bool enabled) {
        m_isEnabled = enabled;
        if (!m_isEnabled) {
            m_shape.setFillColor(m_disabledColor);
            m_text.setFillColor(sf::Color(130, 130, 130)); 
        } else {
            m_shape.setFillColor(m_normalColor);
            m_text.setFillColor(Theme::TextDark);
        }
    }

    void Button::update(sf::Vector2f mousePos) {
        if (!m_isEnabled) return;
        m_isHovered = m_shape.getGlobalBounds().contains(mousePos);
        m_shape.setFillColor(m_isHovered ? m_hoverColor : m_normalColor);
    }

    bool Button::handleMouseClick(sf::Vector2f mousePos) {
        if (m_isEnabled && m_isHovered && m_onClick) {
            m_onClick(); 
            return true;
        }
        return false;
    }

    void Button::render(sf::RenderWindow& window) {
        window.draw(m_shape);
        window.draw(m_text);
    }
}
```

---

## 4. File: `viewsGUI/ActionPanel.hpp`
```cpp
#ifndef ACTION_PANEL_HPP
#define ACTION_PANEL_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Button.hpp"

namespace viewsGUI {
    class ActionPanel {
    public:
        ActionPanel(sf::Vector2f position, sf::Vector2f size, const sf::Font& font);

        void update(sf::Vector2f mousePos);
        void handleMouseClick(sf::Vector2f mousePos);
        void render(sf::RenderWindow& window);

        void updatePlayerInfo(const std::string& playerName, int money);
        
        std::shared_ptr<Button> getRollDiceBtn() { return m_btnRollDice; }
        std::shared_ptr<Button> getBuildBtn() { return m_btnBuild; }
        std::shared_ptr<Button> getMortgageBtn() { return m_btnMortgage; }
        std::shared_ptr<Button> getEndTurnBtn() { return m_btnEndTurn; }

    private:
        sf::Vector2f m_position;
        sf::Vector2f m_size;
        sf::Font m_font;
        sf::RectangleShape m_background;

        sf::RectangleShape m_statusBox;
        sf::Text m_titleText;
        sf::Text m_playerInfoText;

        std::vector<std::shared_ptr<Button>> m_buttons;
        std::shared_ptr<Button> m_btnRollDice;
        std::shared_ptr<Button> m_btnBuild;
        std::shared_ptr<Button> m_btnMortgage;
        std::shared_ptr<Button> m_btnUseCard;
        std::shared_ptr<Button> m_btnEndTurn;

        void initLayout();
    };
}
#endif
```

---

## 5. File: `viewsGUI/ActionPanel.cpp`
```cpp
#include "ActionPanel.hpp"
#include "Theme.hpp"

namespace viewsGUI {

    ActionPanel::ActionPanel(sf::Vector2f position, sf::Vector2f size, const sf::Font& font)
        : m_position(position), m_size(size), m_font(font)
    {
        m_background.setPosition(m_position);
        m_background.setSize(m_size);
        m_background.setFillColor(sf::Color(40, 45, 50)); 
        initLayout();
    }

    void ActionPanel::initLayout() {
        m_statusBox.setPosition(m_position.x + 20, m_position.y + 20);
        m_statusBox.setSize({m_size.x - 40, 100});
        m_statusBox.setFillColor(sf::Color(60, 65, 70));
        m_statusBox.setOutlineThickness(2.0f);
        m_statusBox.setOutlineColor(Theme::Kuning); 

        m_titleText.setFont(m_font);
        m_titleText.setString("GILIRAN SAAT INI");
        m_titleText.setCharacterSize(14);
        m_titleText.setFillColor(sf::Color(200, 200, 200));
        m_titleText.setPosition(m_position.x + 30, m_position.y + 30);

        m_playerInfoText.setFont(m_font);
        m_playerInfoText.setString("Pemain A | Uang: M1.500");
        m_playerInfoText.setCharacterSize(24);
        m_playerInfoText.setFillColor(sf::Color::White);
        m_playerInfoText.setPosition(m_position.x + 30, m_position.y + 60);

        float startY = m_position.y + 150;
        float padding = 20.0f;
        float btnWidth = (m_size.x - (3 * padding)) / 2; 
        float btnHeight = 60.0f;

        m_btnRollDice = std::make_shared<Button>(sf::Vector2f(m_position.x + padding, startY), sf::Vector2f(btnWidth, btnHeight), "LEMPAR DADU", m_font);
        m_btnBuild = std::make_shared<Button>(sf::Vector2f(m_position.x + padding * 2 + btnWidth, startY), sf::Vector2f(btnWidth, btnHeight), "BANGUN RUMAH", m_font);
        m_btnMortgage = std::make_shared<Button>(sf::Vector2f(m_position.x + padding, startY + btnHeight + padding), sf::Vector2f(btnWidth, btnHeight), "GADAI ASET", m_font);
        m_btnUseCard = std::make_shared<Button>(sf::Vector2f(m_position.x + padding * 2 + btnWidth, startY + btnHeight + padding), sf::Vector2f(btnWidth, btnHeight), "PAKAI KARTU", m_font);

        m_btnEndTurn = std::make_shared<Button>(
            sf::Vector2f(m_position.x + padding, startY + (btnHeight + padding) * 2), 
            sf::Vector2f(m_size.x - (2 * padding), btnHeight), 
            "AKHIRI GILIRAN", m_font);

        m_buttons.push_back(m_btnRollDice);
        m_buttons.push_back(m_btnBuild);
        m_buttons.push_back(m_btnMortgage);
        m_buttons.push_back(m_btnUseCard);
        m_buttons.push_back(m_btnEndTurn);
    }

    void ActionPanel::updatePlayerInfo(const std::string& playerName, int money) {
        m_playerInfoText.setString(playerName + " | Uang: M" + std::to_string(money));
    }

    void ActionPanel::update(sf::Vector2f mousePos) {
        for (auto& btn : m_buttons) btn->update(mousePos);
    }

    void ActionPanel::handleMouseClick(sf::Vector2f mousePos) {
        for (auto& btn : m_buttons) {
            if (btn->handleMouseClick(mousePos)) break; 
        }
    }

    void ActionPanel::render(sf::RenderWindow& window) {
        window.draw(m_background);
        window.draw(m_statusBox);
        window.draw(m_titleText);
        window.draw(m_playerInfoText);
        for (auto& btn : m_buttons) btn->render(window);
    }
}
```

---

## 6. File: `viewsGUI/BoardRenderer.hpp`
```cpp
#ifndef BOARD_RENDERER_HPP
#define BOARD_RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <string>

namespace viewsGUI {
    class BoardRenderer {
    public:
        BoardRenderer(float boardSize, const sf::Font& font);
        
        void render(sf::RenderWindow& window); 

        sf::Vector2f getTilePosition(int index);
        sf::Vector2f getTileSize(int index);

    private:
        float m_boardSize;
        float m_cornerSize;
        float m_tileSize;
        sf::Font m_font;

        void drawTile(sf::RenderWindow& window, int index, const std::string& name, sf::Color headerColor);
        void centerText(sf::Text& text, const sf::FloatRect& bounds);
    };
}
#endif
```

---

## 7. File: `viewsGUI/BoardRenderer.cpp`
```cpp
#include "BoardRenderer.hpp"
#include "Theme.hpp"

namespace viewsGUI {

    BoardRenderer::BoardRenderer(float boardSize, const sf::Font& font) 
        : m_boardSize(boardSize), m_font(font) 
    {
        m_cornerSize = 90.0f;
        m_tileSize = 60.0f; 
    }

    sf::Vector2f BoardRenderer::getTilePosition(int index) {
        if (index == 0) return { m_boardSize - m_cornerSize, m_boardSize - m_cornerSize }; 
        if (index > 0 && index < 10) return { m_boardSize - m_cornerSize - (index * m_tileSize), m_boardSize - m_cornerSize }; 
        if (index == 10) return { 0.f, m_boardSize - m_cornerSize }; 
        if (index > 10 && index < 20) return { 0.f, m_boardSize - m_cornerSize - ((index - 10) * m_tileSize) }; 
        if (index == 20) return { 0.f, 0.f }; 
        if (index > 20 && index < 30) return { m_cornerSize + ((index - 21) * m_tileSize), 0.f }; 
        if (index == 30) return { m_boardSize - m_cornerSize, 0.f }; 
        if (index > 30 && index < 40) return { m_boardSize - m_cornerSize, m_cornerSize + ((index - 31) * m_tileSize) }; 
        return { 0.f, 0.f }; 
    }

    sf::Vector2f BoardRenderer::getTileSize(int index) {
        if (index % 10 == 0) return { m_cornerSize, m_cornerSize }; 
        if ((index > 0 && index < 10) || (index > 20 && index < 30)) return { m_tileSize, m_cornerSize }; 
        return { m_cornerSize, m_tileSize }; 
    }

    void BoardRenderer::drawTile(sf::RenderWindow& window, int index, const std::string& name, sf::Color headerColor) {
        sf::Vector2f pos = getTilePosition(index);
        sf::Vector2f size = getTileSize(index);

        sf::RectangleShape tileShape(size);
        tileShape.setPosition(pos);
        tileShape.setFillColor(Theme::Background);
        tileShape.setOutlineThickness(-1.5f); 
        tileShape.setOutlineColor(Theme::Border);
        window.draw(tileShape);

        if (headerColor != sf::Color::Transparent && index % 10 != 0) {
            sf::RectangleShape headerShape;
            headerShape.setFillColor(headerColor);
            headerShape.setOutlineThickness(-1.5f);
            headerShape.setOutlineColor(Theme::Border);

            float headerThickness = 20.0f; 

            if (index > 0 && index < 10) { 
                headerShape.setSize({ m_tileSize, headerThickness });
                headerShape.setPosition(pos.x, pos.y);
            } else if (index > 10 && index < 20) { 
                headerShape.setSize({ headerThickness, m_tileSize });
                headerShape.setPosition(pos.x + m_cornerSize - headerThickness, pos.y);
            } else if (index > 20 && index < 30) { 
                headerShape.setSize({ m_tileSize, headerThickness });
                headerShape.setPosition(pos.x, pos.y + m_cornerSize - headerThickness);
            } else if (index > 30 && index < 40) { 
                headerShape.setSize({ headerThickness, m_tileSize });
                headerShape.setPosition(pos.x, pos.y);
            }
            window.draw(headerShape);
        }

        sf::Text text(name, m_font, 10); 
        text.setFillColor(Theme::TextDark);
        centerText(text, tileShape.getGlobalBounds());
        window.draw(text);
    }

    void BoardRenderer::centerText(sf::Text& text, const sf::FloatRect& bounds) {
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        text.setPosition(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    }

    void BoardRenderer::render(sf::RenderWindow& window) {
        sf::RectangleShape centerBoard({ m_boardSize, m_boardSize });
        centerBoard.setFillColor(sf::Color(245, 250, 245)); 
        window.draw(centerBoard);

        for (int i = 0; i < 40; ++i) {
            std::string name = "Petak " + std::to_string(i);
            sf::Color color = sf::Color::Transparent;

            if (i == 1 || i == 3) color = Theme::Coklat;
            else if (i == 6 || i == 8 || i == 9) color = Theme::BiruMuda;
            else if (i == 11 || i == 13 || i == 14) color = Theme::Pink;
            else if (i == 16 || i == 18 || i == 19) color = Theme::Oranye;
            else if (i == 21 || i == 23 || i == 24) color = Theme::Merah;
            else if (i == 26 || i == 27 || i == 29) color = Theme::Kuning;
            else if (i == 31 || i == 32 || i == 34) color = Theme::Hijau;
            else if (i == 37 || i == 39) color = Theme::BiruTua;
            
            if (i == 2 || i == 7 || i == 17 || i == 22 || i == 33 || i == 36) {
                color = sf::Color::Transparent; 
                name = "Aksi/Dana";
            }
            
            if (i == 0) name = "GO";
            if (i == 10) name = "JAIL";
            if (i == 20) name = "PARK";
            if (i == 30) name = "GO TO\nJAIL";

            drawTile(window, i, name, color);
        }
    }
}
```

---

## 8. File: `viewsGUI/PieceRenderer.hpp`
```cpp
#ifndef PIECE_RENDERER_HPP
#define PIECE_RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <queue>
#include <vector>

namespace viewsGUI {
    class PieceRenderer {
    public:
        PieceRenderer(int playerId, sf::Color playerColor, sf::Vector2f startPos);
        void moveAlongPath(const std::vector<sf::Vector2f>& pathQueue);
        void update(sf::Time dt);
        void render(sf::RenderWindow& window);
        bool isMoving() const { return m_isMoving; }

    private:
        sf::CircleShape m_shape;
        sf::Vector2f m_currentPos;
        std::queue<sf::Vector2f> m_pathQueue;
        bool m_isMoving;
        float m_speed;   
        sf::Vector2f m_offset; 
    };
}
#endif
```

---

## 9. File: `viewsGUI/PieceRenderer.cpp`
```cpp
#include "PieceRenderer.hpp"
#include <cmath>

namespace viewsGUI {

    PieceRenderer::PieceRenderer(int playerId, sf::Color playerColor, sf::Vector2f startPos)
        : m_isMoving(false), m_speed(400.0f) 
    {
        m_shape.setRadius(12.0f);
        m_shape.setOrigin(12.0f, 12.0f); 
        m_shape.setFillColor(playerColor);
        m_shape.setOutlineThickness(2.0f);
        m_shape.setOutlineColor(sf::Color::White);

        float offsetX = (playerId % 2 == 0) ? -10.0f : 10.0f;
        float offsetY = (playerId > 2) ? 10.0f : -10.0f;
        m_offset = sf::Vector2f(offsetX, offsetY);

        m_currentPos = startPos + m_offset;
        m_shape.setPosition(m_currentPos);
    }

    void PieceRenderer::moveAlongPath(const std::vector<sf::Vector2f>& pathQueue) {
        for (const auto& pos : pathQueue) {
            m_pathQueue.push(pos + m_offset); 
        }
        m_isMoving = true;
    }

    void PieceRenderer::update(sf::Time dt) {
        if (m_pathQueue.empty()) {
            m_isMoving = false;
            return;
        }

        sf::Vector2f targetPos = m_pathQueue.front();
        sf::Vector2f direction = targetPos - m_currentPos;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance < 5.0f) {
            m_currentPos = targetPos; 
            m_pathQueue.pop();        
        } else {
            direction /= distance; 
            m_currentPos += direction * m_speed * dt.asSeconds();
        }
        m_shape.setPosition(m_currentPos);
    }

    void PieceRenderer::render(sf::RenderWindow& window) {
        window.draw(m_shape);
    }
}
```

---

## 10. File: `viewsGUI/PopupBox.hpp`
```cpp
#ifndef POPUP_BOX_HPP
#define POPUP_BOX_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <functional>
#include "Button.hpp"

namespace viewsGUI {
    class PopupBox {
    public:
        PopupBox(sf::Vector2f windowSize, const sf::Font& font);
        void show(const std::string& title, const std::string& message,
                  const std::string& btn1Text, std::function<void()> onBtn1Click,
                  const std::string& btn2Text = "", std::function<void()> onBtn2Click = nullptr);

        void hide();
        bool isVisible() const { return m_isVisible; }

        void update(sf::Vector2f mousePos);
        bool handleMouseClick(sf::Vector2f mousePos);
        void render(sf::RenderWindow& window);

    private:
        bool m_isVisible;
        sf::Font m_font;
        sf::RectangleShape m_overlay; 
        sf::RectangleShape m_box;     
        sf::Text m_titleText;
        sf::Text m_messageText;

        std::shared_ptr<Button> m_btn1; 
        std::shared_ptr<Button> m_btn2; 

        void centerTextX(sf::Text& text, float yPos);
    };
}
#endif
```

---

## 11. File: `viewsGUI/PopupBox.cpp`
```cpp
#include "PopupBox.hpp"
#include "Theme.hpp"

namespace viewsGUI {

    PopupBox::PopupBox(sf::Vector2f windowSize, const sf::Font& font)
        : m_isVisible(false), m_font(font), m_btn1(nullptr), m_btn2(nullptr)
    {
        m_overlay.setSize(windowSize);
        m_overlay.setFillColor(sf::Color(0, 0, 0, 150)); 

        sf::Vector2f boxSize(450.0f, 250.0f);
        m_box.setSize(boxSize);
        m_box.setOrigin(boxSize.x / 2.0f, boxSize.y / 2.0f);
        m_box.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
        
        m_box.setFillColor(Theme::Background);
        m_box.setOutlineThickness(4.0f);
        m_box.setOutlineColor(Theme::BiruTua);

        m_titleText.setFont(m_font);
        m_titleText.setCharacterSize(22);
        m_titleText.setFillColor(Theme::TextDark);

        m_messageText.setFont(m_font);
        m_messageText.setCharacterSize(16);
        m_messageText.setFillColor(Theme::TextDark);
    }

    void PopupBox::centerTextX(sf::Text& text, float yPos) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top);
        text.setPosition(m_box.getPosition().x, yPos);
    }

    void PopupBox::show(const std::string& title, const std::string& message,
                        const std::string& btn1Text, std::function<void()> onBtn1Click,
                        const std::string& btn2Text, std::function<void()> onBtn2Click)
    {
        m_isVisible = true;

        m_titleText.setString(title);
        m_messageText.setString(message);

        float boxTopY = m_box.getPosition().y - (m_box.getSize().y / 2.0f);
        centerTextX(m_titleText, boxTopY + 30.0f);
        centerTextX(m_messageText, boxTopY + 80.0f);

        float btnY = boxTopY + 160.0f;
        float btnWidth = 120.0f;
        float btnHeight = 45.0f;

        auto wrapCallback = [this, callback = onBtn1Click]() {
            this->hide();
            if (callback) callback();
        };

        if (btn2Text.empty()) {
            float btnX = m_box.getPosition().x - (btnWidth / 2.0f);
            m_btn1 = std::make_shared<Button>(sf::Vector2f(btnX, btnY), sf::Vector2f(btnWidth, btnHeight), btn1Text, m_font);
            m_btn1->setOnClick(wrapCallback);
            m_btn2 = nullptr;
        } else {
            float spacing = 40.0f;
            float btn1X = m_box.getPosition().x - btnWidth - (spacing / 2.0f);
            float btn2X = m_box.getPosition().x + (spacing / 2.0f);

            auto wrapCallback2 = [this, callback = onBtn2Click]() {
                this->hide();
                if (callback) callback();
            };

            m_btn1 = std::make_shared<Button>(sf::Vector2f(btn1X, btnY), sf::Vector2f(btnWidth, btnHeight), btn1Text, m_font);
            m_btn1->setOnClick(wrapCallback);

            m_btn2 = std::make_shared<Button>(sf::Vector2f(btn2X, btnY), sf::Vector2f(btnWidth, btnHeight), btn2Text, m_font);
            m_btn2->setOnClick(wrapCallback2);
        }
    }

    void PopupBox::hide() {
        m_isVisible = false;
    }

    void PopupBox::update(sf::Vector2f mousePos) {
        if (!m_isVisible) return;
        if (m_btn1) m_btn1->update(mousePos);
        if (m_btn2) m_btn2->update(mousePos);
    }

    bool PopupBox::handleMouseClick(sf::Vector2f mousePos) {
        if (!m_isVisible) return false;
        bool clicked = false;
        if (m_btn1 && m_btn1->handleMouseClick(mousePos)) clicked = true;
        if (m_btn2 && m_btn2->handleMouseClick(mousePos)) clicked = true;
        return true; 
    }

    void PopupBox::render(sf::RenderWindow& window) {
        if (!m_isVisible) return;
        window.draw(m_overlay);
        window.draw(m_box);
        window.draw(m_titleText);
        window.draw(m_messageText);
        if (m_btn1) m_btn1->render(window);
        if (m_btn2) m_btn2->render(window);
    }
}
```

---

## 12. File: `viewsGUI/SfmlGuiManager.hpp`
```cpp
#ifndef SFML_GUI_MANAGER_HPP
#define SFML_GUI_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "BoardRenderer.hpp"
#include "ActionPanel.hpp"
#include "PopupBox.hpp"
#include "PieceRenderer.hpp"
// TODO COPILOT: #include "../core/GameEngine.hpp"

namespace viewsGUI {
    enum class GuiState { IDLE, ANIMATING, WAITING_CONFIRMATION };

    class SfmlGuiManager {
    public:
        // TODO COPILOT: Uncomment dan integrasikan parameter GameEngine
        SfmlGuiManager(/* core::GameEngine& engine */);
        void run();

    private:
        sf::RenderWindow m_window;
        // core::GameEngine& m_engine;
        
        sf::Font m_mainFont;
        GuiState m_currentState;

        std::unique_ptr<BoardRenderer> m_boardView;
        std::unique_ptr<ActionPanel> m_actionPanel;
        std::unique_ptr<PopupBox> m_popupBox;
        std::vector<std::unique_ptr<PieceRenderer>> m_players;

        void processEvents();
        void update(sf::Time dt);
        void render();
        void bindEngineCallbacks(); // Tempat wiring tombol ke GameEngine
    };
}
#endif
```

---

## 13. File: `viewsGUI/SfmlGuiManager.cpp`
```cpp
#include "SfmlGuiManager.hpp"
// #include "../core/GameEngine.hpp"

namespace viewsGUI {

    SfmlGuiManager::SfmlGuiManager(/* core::GameEngine& engine */) 
        // : m_engine(engine), m_currentState(GuiState::IDLE)
    {
        m_window.create(sf::VideoMode(1280, 720), "Nimonspoli");
        m_mainFont.loadFromFile("assets/fonts/Montserrat-Bold.ttf");

        m_boardView = std::make_unique<BoardRenderer>(720.0f, m_mainFont);
        m_actionPanel = std::make_unique<ActionPanel>(sf::Vector2f(720.f, 0.f), sf::Vector2f(560.f, 720.f), m_mainFont);
        m_popupBox = std::make_unique<PopupBox>(sf::Vector2f(1280.f, 720.f), m_mainFont);

        // TODO COPILOT: Inisialisasi m_players berdasarkan jumlah pemain dari m_engine
        
        bindEngineCallbacks();
    }

    void SfmlGuiManager::bindEngineCallbacks() {
        // TODO COPILOT: Implementasikan logika pemanggilan engine di sini
        /* Contoh:
        m_actionPanel->getRollDiceBtn()->setOnClick([this]() {
            if (m_currentState != GuiState::IDLE) return;
            
            auto result = m_engine.executeCommand("LEMPAR_DADU");
            
            // Trigger Animasi
            m_currentState = GuiState::ANIMATING;
            m_players[m_engine.getCurrentPlayerIndex()]->moveAlongPath(result.path);
        });
        */
    }

    void SfmlGuiManager::run() {
        sf::Clock clock;
        while (m_window.isOpen()) {
            sf::Time dt = clock.restart();
            processEvents();
            update(dt);
            render();
        }
    }

    void SfmlGuiManager::processEvents() {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) m_window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
                
                // HIRARKI INPUT BLOCKING
                if (m_popupBox->isVisible()) {
                    m_popupBox->handleMouseClick(mousePos);
                } else if (m_currentState == GuiState::IDLE) {
                    m_actionPanel->handleMouseClick(mousePos);
                }
            }
        }
    }

    void SfmlGuiManager::update(sf::Time dt) {
        sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

        if (m_popupBox->isVisible()) {
            m_popupBox->update(mousePos);
        } else {
            m_actionPanel->update(mousePos);
        }

        bool anyMoving = false;
        for (auto& player : m_players) {
            player->update(dt);
            if (player->isMoving()) anyMoving = true;
        }

        // TODO COPILOT: Transisi State setelah animasi selesai
        /*
        if (m_currentState == GuiState::ANIMATING && !anyMoving) {
            if (m_engine.isWaitingForConfirmation()) {
                 m_currentState = GuiState::WAITING_CONFIRMATION;
                 m_popupBox->show("BELI PROPERTI", ...);
            } else {
                 m_currentState = GuiState::IDLE;
            }
        }
        */
    }

    void SfmlGuiManager::render() {
        m_window.clear();
        m_boardView->render(m_window);
        m_actionPanel->render(m_window);
        
        for (auto& player : m_players) player->render(m_window);
        
        m_popupBox->render(m_window); 
        m_window.display();
    }
}
```

## Tugas untuk Copilot:
1. Pahami pola `State Machine` di `SfmlGuiManager.cpp`. Anda harus mencegah tombol ditekan berulang kali ketika `State != IDLE`.
2. Ubah `GameEngine` CLI agar mengembalikan nilai `CommandResult` yang komprehensif, bukan menggunakan `std::cin`.
3. Pada fungsi `bindEngineCallbacks()`, integrasikan fungsi-fungsi tombol dengan `GameEngine`.
4. Pastikan data yang dirender di `ActionPanel` (Uang, Nama Pemain) selalu sinkron dengan state dari `GameEngine` di setiap frame atau setiap kali status berubah.
```

***

Semoga panduan *all-in-one* ini melancarkan proses integrasi Anda! Jika Anda menemui kendala teknis saat Copilot merombak `GameEngine` untuk transisi state, jangan ragu untuk kembali dan bertanya. Selamat menyelesaikan Nimonspoli!