module;
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

export module core:application;

import std;
import :context;
import :events;
import :window;
import :states;

export namespace core {

class Application {
public:
  Application(unsigned int width = 800, unsigned int height = 600,
              std::string_view title = "SFML Application")
      : m_window(width, height, title), m_context{m_window, m_events, m_states},
        m_states(m_context) {}

  virtual ~Application() = default;

  Context &getContext() { return m_context; }
  Window &getWindow() { return m_window; }
  EventManager &getEventManager() { return m_events; }
  StateManager &getStateManager() { return m_states; }

  void run() {
    sf::Clock clock;
    // main loop
    while (m_window.isOpen()) {
      float dt = clock.restart().asSeconds();
      handleEvents();
      update(dt);
      render();
    }
  }

private:
  void handleEvent() { m_events.handleEvents(); }

  void update(const float &l_dt) { m }
  void render() {}

private:
  Window m_window;
  EventManager m_events;
  StateManager m_states;
  Context m_context;
};

} // namespace core
