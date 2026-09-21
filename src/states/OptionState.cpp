module core;

import :StateManager;


namespace core {
    OptionState::OptionState(StateManager* l_statemanager, SharedContext* l_context) : State(l_statemanager, l_context){}

    void OptionState::onCreate() {}

    void OptionState::onActivate() {}

    void OptionState::onDeactivate(){}

    void OptionState::onDestroy() {}


    void OptionState::update(const float l_dt){}

    void OptionState::draw(){}

}
