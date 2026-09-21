export module core:OptionState;

import :State;
export namespace core {
    class OptionState final : public State {
    public:
        OptionState(StateManager* l_stateManager, SharedContext* l_context);

        ~OptionState() = default;


        void onCreate() override;

        void onActivate() override;

        void onDeactivate() override;

        void onDestroy() override;


        void update(const float l_dt) override;

        void draw() override;
    };

    using OptionsState = OptionState;
} // namespace core
