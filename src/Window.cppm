module;

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
export module core:Window;

import :EventManager;
export namespace core {
    class Window {
    public:
        Window(const unsigned int width, const unsigned int height,
               const std::string title)
            : m_renderWindow(sf::VideoMode{{width, height}}, title), m_eventManager(), m_isOpen(true) {
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

        void clear(const sf::Color l_color = sf::Color::Black) {
            m_renderWindow.clear(l_color);
        }

        void display() { m_renderWindow.display(); }
        sf::RenderWindow* getRenderWindow() { return &m_renderWindow; }
        EventManager* getEventManager() { return &m_eventManager; }

        std::optional<sf::Event> pollEvent() { return m_renderWindow.pollEvent(); }

    private:
        sf::RenderWindow m_renderWindow;
        EventManager m_eventManager;
        bool m_isOpen;
    };
} // namespace core
