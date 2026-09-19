export module core:GameState;

import :State;
export namespace core {
    class GameState final : public State {
    public:
        GameState(StateManager* l_stateManager, SharedContext* l_context);

        ~GameState() = default;


        void onCreate() override;

        void onActivate() override;

        void onDeactivate() override;

        void onDestroy() override;


        void update(const float l_dt) override;

        void draw() override;
    };
} // namespace core
