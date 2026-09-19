module;

#include <SFML/Graphics/Color.hpp>

module core;

import :StateManager;

namespace core {
    MenuState::MenuState(StateManager* l_statemanager, SharedContext* l_context) : State(l_statemanager, l_context) {

    }

    void MenuState::onCreate() {}

    void MenuState::onActivate() {}

    void MenuState::onDeactivate(){}

    void MenuState::onDestroy() {}


    void MenuState::update(const float l_dt){}

    void MenuState::draw() {
        m_context->m_window->clear(sf::Color::Blue);
    }

}