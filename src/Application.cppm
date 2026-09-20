module;
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Color.hpp>
export module core:Application;

import :Window;
import :StateManager;
import :SharedContext;
import :ResourceManager;
import :AudioManager;

export namespace core {
    class Application {
    public:
        Application()
            : m_window(800, 600, "Enemata v0.1"),
              m_resourceManager(),
              m_audioManager(),
              m_context(),
              m_stateManager(&m_context) {
            m_context.m_window = &m_window;
            m_context.m_eventmanager = m_window.getEventManager();
            m_context.m_resourceManager = &m_resourceManager;
            m_context.m_audioManager = &m_audioManager;
            m_stateManager.switchTo(StateType::Menu);
        }

        void run() {
            sf::Clock clock;
            // main loop
            while (m_window.isOpen()) {
                const float dt = clock.restart().asSeconds();
                while (const auto event = m_window.pollEvent()) {
                    handleEvents(*event);
                }
                update(dt);
                render();
                lateUpdate();
            }
        }

    private:
        void handleEvents(const sf::Event &l_event) {
            m_window.handleEvents(l_event);
            m_stateManager.handleEvent(l_event);
        }

        void update(const float l_dt) {
            m_stateManager.update(l_dt);
            m_audioManager.update(l_dt);
        }
        void lateUpdate() {
            m_stateManager.processRequests();
        }

        void render() {
            m_window.clear(sf::Color::Black);
            m_stateManager.draw();
            m_window.display();
        }

    private:
        Window m_window;
        ResourceManager m_resourceManager;
        AudioManager m_audioManager;
        SharedContext m_context;
        StateManager m_stateManager;
    };
} // namespace core
