module;
#include <SFML/Window/Event.hpp>

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

        virtual void handleEvent(const sf::Event& /*l_event*/) {}

    protected:
        StateManager* m_stateManager;
        SharedContext* m_context;
    };
}
