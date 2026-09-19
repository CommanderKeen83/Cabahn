module;

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

export module core:Window;

import :EventManager;
import std;

export namespace core {
    class Window {
    public:
        Window(const unsigned int width, const unsigned int height,
               const std::string& title,
               const unsigned int virtualWidth = 320,
               const unsigned int virtualHeight = 180)
            : m_renderWindow(sf::VideoMode{{width, height}}, title),
              m_canvas(sf::Vector2u{virtualWidth, virtualHeight}),
              m_virtualSize(virtualWidth, virtualHeight),
              m_eventManager(),
              m_isOpen(true) {
            // Nearest-neighbor scaling for retro pixel art
            m_canvas.setSmooth(false);
        }

        ~Window() {
            m_renderWindow.close();
        }

        void handleEvents(const sf::Event &l_event) {
            if (l_event.is<sf::Event::Closed>()) {
                m_isOpen = false;
            }
            m_eventManager.handleEvent(l_event);
        }

        [[nodiscard]] bool isOpen() const { return m_isOpen; }

        // Clears the internal 320x180 retro canvas
        void clear(const sf::Color l_color = sf::Color::Black) {
            m_canvas.clear(l_color);
        }

        // Draw directly to the retro canvas
        void draw(const sf::Drawable& l_drawable) {
            m_canvas.draw(l_drawable);
        }

        // Finalize canvas and render scaled to real window with letterboxing
        void display() {
            m_canvas.display();

            // Clear real window with black for letterbox/pillarbox borders
            m_renderWindow.clear(sf::Color::Black);

            sf::Sprite canvasSprite(m_canvas.getTexture());

            const auto winSize = m_renderWindow.getSize();
            const float scaleX = static_cast<float>(winSize.x) / static_cast<float>(m_virtualSize.x);
            const float scaleY = static_cast<float>(winSize.y) / static_cast<float>(m_virtualSize.y);
            const float scale = std::min(scaleX, scaleY);

            canvasSprite.setScale({scale, scale});

            const float posX = (static_cast<float>(winSize.x) - static_cast<float>(m_virtualSize.x) * scale) / 2.0f;
            const float posY = (static_cast<float>(winSize.y) - static_cast<float>(m_virtualSize.y) * scale) / 2.0f;
            canvasSprite.setPosition({posX, posY});

            m_renderWindow.draw(canvasSprite);
            m_renderWindow.display();
        }

        sf::RenderTarget& getCanvas() { return m_canvas; }
        sf::RenderWindow* getRenderWindow() { return &m_renderWindow; }
        EventManager* getEventManager() { return &m_eventManager; }
        std::optional<sf::Event> pollEvent() { return m_renderWindow.pollEvent(); }
        [[nodiscard]] sf::Vector2u getVirtualSize() const { return m_virtualSize; }

        [[nodiscard]] sf::Vector2f mapPixelToVirtual(const sf::Vector2i& pixelPos) const {
            const auto winSize = m_renderWindow.getSize();
            const float scaleX = static_cast<float>(winSize.x) / static_cast<float>(m_virtualSize.x);
            const float scaleY = static_cast<float>(winSize.y) / static_cast<float>(m_virtualSize.y);
            const float scale = std::min(scaleX, scaleY);
            if (scale <= 0.0f) {
                return {0.f, 0.f};
            }

            const float posX = (static_cast<float>(winSize.x) - static_cast<float>(m_virtualSize.x) * scale) / 2.0f;
            const float posY = (static_cast<float>(winSize.y) - static_cast<float>(m_virtualSize.y) * scale) / 2.0f;

            return { (static_cast<float>(pixelPos.x) - posX) / scale,
                     (static_cast<float>(pixelPos.y) - posY) / scale };
        }

        [[nodiscard]] sf::Vector2f getMouseVirtualPosition() const {
            return mapPixelToVirtual(sf::Mouse::getPosition(m_renderWindow));
        }
        void close() {
            m_isOpen = false;
        }
    private:
        sf::RenderWindow m_renderWindow;
        sf::RenderTexture m_canvas;
        sf::Vector2u m_virtualSize;
        EventManager m_eventManager;
        bool m_isOpen;
    };
} // namespace core
