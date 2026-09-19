export module core:MenuState;

import :State;
export namespace core {
    class MenuState final : public State {
    public:
        MenuState(StateManager* l_stateManager, SharedContext* l_context);

        ~MenuState() = default;


        void onCreate() override;

        void onActivate() override;

        void onDeactivate() override;

        void onDestroy() override;


        void update(const float l_dt) override;

        void draw() override;
    };
} // namespace core
