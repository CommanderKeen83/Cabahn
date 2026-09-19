module core;

import :StateManager;


namespace core {
    GameState::GameState(StateManager* l_statemanager, SharedContext* l_context) : State(l_statemanager, l_context){}

    void GameState::onCreate() {}

    void GameState::onActivate() {}

    void GameState::onDeactivate(){}

    void GameState::onDestroy() {}


    void GameState::update(const float l_dt){}

    void GameState::draw(){}

}
