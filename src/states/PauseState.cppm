export module core:PauseState;

import :State;
export namespace core {
    class PauseState final : public State {
    public:
        PauseState(StateManager* l_stateManager, SharedContext* l_context);

        ~PauseState() = default;


        void onCreate() override;

        void onActivate() override;

        void onDeactivate() override;

        void onDestroy() override;


        void update(const float l_dt) override;

        void draw() override;
    };
} // namespace core
