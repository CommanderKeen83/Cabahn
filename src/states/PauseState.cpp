module core;

import :StateManager;


namespace core {
    PauseState::PauseState(StateManager* l_statemanager, SharedContext* l_context) : State(l_statemanager, l_context){}

    void PauseState::onCreate() {}

    void PauseState::onActivate() {}

    void PauseState::onDeactivate(){}

    void PauseState::onDestroy() {}


    void PauseState::update(const float l_dt){}

    void PauseState::draw(){}

}
