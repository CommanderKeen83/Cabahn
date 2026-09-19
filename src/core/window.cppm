module;
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

export module core:window;

import std;

export namespace core {

class Window {
public:
  Window(unsigned int width, unsigned int height, std::string_view title)
      : m_window(sf::VideoMode({width, height}), std::string(title)) {
    m_window.setFramerateLimit(60);
  }

  [[nodiscard]] bool isOpen() const { return m_window.isOpen(); }

  void close() { m_window.close(); }

  void handleEvents() {
    while (const std::optional<sf::Event> event = m_window.pollEvent()) {
      if (event)
    }
  }

  void clear(sf::Color color = sf::Color::Black) { m_window.clear(color); }

  void display() { m_window.display(); }

  void draw(const sf::Drawable &drawable) { m_window.draw(drawable); }

  [[nodiscard]] sf::RenderWindow &getRaw() { return m_window; }

  [[nodiscard]] const sf::RenderWindow &getRaw() const { return m_window; }

  [[nodiscard]] sf::RenderWindow &getRenderWindow() { return m_window; }

  [[nodiscard]] const sf::RenderWindow &getRenderWindow() const {
    return m_window;
  }

  [[nodiscard]] const sf::View &getView() const { return m_window.getView(); }

  [[nodiscard]] sf::FloatRect getViewBounds() const {
    const auto &view = m_window.getView();
    return sf::FloatRect(view.getCenter() - view.getSize() / 2.f,
                         view.getSize());
  }

  [[nodiscard]] sf::FloatRect getViewRect() const { return getViewBounds(); }

  [[nodiscard]] sf::Vector2u getSize() const { return m_window.getSize(); }

private:
  sf::RenderWindow m_window;
};

} // namespace core
