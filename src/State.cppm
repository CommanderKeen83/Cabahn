//
// Created by sf on 9/17/26.
//

export module core:State;

export namespace core {
    class StateManager;
    class SharedContext;

    class State {
    public:
        explicit State(StateManager* l_stateManager, SharedContext* l_context)
            : m_stateManager(l_stateManager), m_context(l_context) {
        }

        virtual ~State() = default;

        virtual void onCreate() = 0;

        virtual void onActivate() = 0;

        virtual void onDeactivate() = 0;

        virtual void onDestroy() = 0;

        virtual void update(const float l_dt) = 0;

        virtual void draw() = 0;

    protected:
        StateManager* m_stateManager;
        SharedContext* m_context;
    };
}
