module;

#include <SFML/Graphics/Color.hpp>

module core;

import :StateManager;
import :ResourceManager;
import :SharedContext;
import :Utilities;
import :Window;
namespace core {
MenuState::MenuState(StateManager *l_statemanager, SharedContext *l_context)
    : State(l_statemanager, l_context) {}

void MenuState::onCreate() {
  const std::string resourceID = "main_menu_texture";
  const std::string resourcePath =
      utils::getResourcePath("graphics/main_menu.png");
  if (m_context->m_resourceManager->loadTexture(resourceID, resourcePath)) {
    m_menuBackgroundSprite.emplace(
        m_context->m_resourceManager->getTexture(resourceID));
  }
}

void MenuState::onActivate() {}

void MenuState::onDeactivate() {}

void MenuState::onDestroy() {
  m_menuBackgroundSprite.reset();
  m_context->m_resourceManager->releaseTexture("main_menu_texture");
}

void MenuState::update(const float l_dt) {}

void MenuState::draw() {
  m_context->m_window->clear(sf::Color::Blue);
  if (m_menuBackgroundSprite) {
    m_context->m_window->draw(*m_menuBackgroundSprite);
  }
}

} // namespace core