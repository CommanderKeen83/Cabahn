module;
#include <SFML/Window/Event.hpp>

export module core:EventManager;

export namespace core {

class EventManager {
public:
  EventManager() {}

  void handleEvent(const sf::Event &l_event) {}

private:
};

} // namespace core