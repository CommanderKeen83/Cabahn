module;

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

export module core:Window;

import :EventManager;
import std;

export namespace core {
    class Window {
    public:
        // Borderless fullscreen window matching desktop resolution
        explicit Window(const std::string& title,
                        const unsigned int virtualWidth = 320,
                        const unsigned int virtualHeight = 180)
            : Window(sf::VideoMode::getDesktopMode().size.x,
                     sf::VideoMode::getDesktopMode().size.y,
                     title,
                     virtualWidth,
                     virtualHeight,
                     sf::Style::None) {
        }

        Window(const unsigned int width, const unsigned int height,
               const std::string& title,
               const unsigned int virtualWidth = 320,
               const unsigned int virtualHeight = 180,
               const std::uint32_t style = sf::Style::Default)
            : m_title(title),
              m_renderWindow(sf::VideoMode{{width, height}}, title, style, sf::State::Windowed),
              m_canvas(sf::Vector2u{virtualWidth, virtualHeight}),
              m_virtualSize(virtualWidth, virtualHeight),
              m_eventManager(),
              m_isOpen(true) {
            if (style == sf::Style::None) {
                m_renderWindow.setPosition({0, 0});
            }
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
            m_canvas.setSmooth(false);

            // Clear real window with black for letterbox/pillarbox borders
            m_renderWindow.clear(sf::Color::Black);

            sf::Sprite canvasSprite(m_canvas.getTexture());

            const auto winSize = m_renderWindow.getSize();
            const float scale = calculateScale(winSize);

            canvasSprite.setScale({scale, scale});

            const float posX = std::round((static_cast<float>(winSize.x) - static_cast<float>(m_virtualSize.x) * scale) / 2.0f);
            const float posY = std::round((static_cast<float>(winSize.y) - static_cast<float>(m_virtualSize.y) * scale) / 2.0f);
            canvasSprite.setPosition({posX, posY});

            m_renderWindow.draw(canvasSprite);
            m_renderWindow.display();
        }

        void setIntegerScaling(bool enable) { m_integerScaling = enable; }
        [[nodiscard]] bool isIntegerScaling() const { return m_integerScaling; }

        sf::RenderTarget& getCanvas() { return m_canvas; }
        sf::RenderWindow* getRenderWindow() { return &m_renderWindow; }
        EventManager* getEventManager() { return &m_eventManager; }
        std::optional<sf::Event> pollEvent() { return m_renderWindow.pollEvent(); }
        [[nodiscard]] sf::Vector2u getVirtualSize() const { return m_virtualSize; }

        [[nodiscard]] sf::Vector2f mapPixelToVirtual(const sf::Vector2i& pixelPos) const {
            const auto winSize = m_renderWindow.getSize();
            const float scale = calculateScale(winSize);
            if (scale <= 0.0f) {
                return {0.f, 0.f};
            }

            const float posX = std::round((static_cast<float>(winSize.x) - static_cast<float>(m_virtualSize.x) * scale) / 2.0f);
            const float posY = std::round((static_cast<float>(winSize.y) - static_cast<float>(m_virtualSize.y) * scale) / 2.0f);

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
        [[nodiscard]] float calculateScale(const sf::Vector2u& winSize) const {
            const float scaleX = static_cast<float>(winSize.x) / static_cast<float>(m_virtualSize.x);
            const float scaleY = static_cast<float>(winSize.y) / static_cast<float>(m_virtualSize.y);
            float scale = std::min(scaleX, scaleY);
            if (m_integerScaling) {
                scale = std::max(1.0f, std::floor(scale));
            }
            return scale;
        }

        std::string m_title;
        sf::RenderWindow m_renderWindow;
        sf::RenderTexture m_canvas;
        sf::Vector2u m_virtualSize;
        EventManager m_eventManager;
        bool m_isOpen;
        bool m_integerScaling{false};
    };
} // namespace core
