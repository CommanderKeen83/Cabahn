module;
#include <SFML/Window/Event.hpp>

export module core:StateManager;
import :State;
import :MenuState;
import :GameState;
import :PauseState;
import :OptionState;
import std;

export namespace core {
    enum class StateType {
        Global = -1,
        Menu,
        Game,
        Pause,
        Options,
    };

    using StateFactory = std::unordered_map<StateType, std::function<std::unique_ptr<State>()>>;
    using StateContainer = std::vector<std::pair<StateType, std::unique_ptr<State>>>;
    using TypeContainer = std::vector<StateType>;

    class StateManager {
    public:
        StateManager(SharedContext* l_context)
            : m_context(l_context), m_stateFactory(), m_states(), m_toRemove() {
            registerState<MenuState>(StateType::Menu);
            registerState<GameState>(StateType::Game);
            registerState<PauseState>(StateType::Pause);
            registerState<OptionState>(StateType::Options);
        }

        ~StateManager() {
            clear();
        }

        template<typename T>
        void registerState(StateType l_type) {
            m_stateFactory[l_type] = [this]() -> std::unique_ptr<State> {
                return std::make_unique<T>(this, m_context);
            };
        }

        void handleEvent(const sf::Event& l_event) {
            if (!m_states.empty()) {
                m_states.back().second->handleEvent(l_event);
            }
        }

        void update(const float l_dt) {
            if (m_states.empty()) {
                return;
            }

            // Update all states in the stack from bottom (oldest) to top (newest)
            const auto count = m_states.size();
            for (std::size_t i = 0; i < count && i < m_states.size(); ++i) {
                m_states[i].second->update(l_dt);
            }

            processRequests();
        }

        void draw() {
            // Draw all states in the stack from bottom (oldest) to top (newest)
            for (const auto& [type, state] : m_states) {
                state->draw();
            }
        }

        void switchTo(StateType l_type) {
            for (auto it = m_states.begin(); it != m_states.end(); ++it) {
                if (it->first == l_type) {
                    if (it + 1 == m_states.end()) {
                        return; // already the active top state
                    }
                    m_states.back().second->onDeactivate();
                    auto statePair = std::move(*it);
                    m_states.erase(it);
                    statePair.second->onActivate();
                    m_states.push_back(std::move(statePair));
                    return;
                }
            }

            // State does not exist in stack yet: create and push on top
            if (!m_states.empty()) {
                m_states.back().second->onDeactivate();
            }
            createState(l_type);
            if (!m_states.empty()) {
                m_states.back().second->onActivate();
            }
        }

        void pop() {
            if (m_states.empty()) {
                return;
            }

            auto top = std::move(m_states.back());
            m_states.pop_back();
            top.second->onDeactivate();
            top.second->onDestroy();

            if (!m_states.empty()) {
                m_states.back().second->onActivate();
            }
        }

        void remove(StateType l_type) {
            m_toRemove.push_back(l_type);
        }

        void processRequests() {
            while (!m_toRemove.empty()) {
                StateType type = m_toRemove.back();
                m_toRemove.pop_back();
                removeState(type);
            }
        }

        void clear() {
            while (!m_states.empty()) {
                auto top = std::move(m_states.back());
                m_states.pop_back();
                top.second->onDeactivate();
                top.second->onDestroy();
            }
        }

        [[nodiscard]] bool hasState(StateType l_type) const {
            for (const auto& [type, state] : m_states) {
                if (type == l_type) {
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] StateType getTopStateType() const {
            if (m_states.empty()) {
                return StateType::Global;
            }
            return m_states.back().first;
        }

        [[nodiscard]] State* getTopState() const {
            if (m_states.empty()) {
                return nullptr;
            }
            return m_states.back().second.get();
        }

        [[nodiscard]] State* getState(StateType l_type) const {
            for (const auto& [type, state] : m_states) {
                if (type == l_type) {
                    return state.get();
                }
            }
            return nullptr;
        }

        [[nodiscard]] SharedContext* getContext() const {
            return m_context;
        }

        [[nodiscard]] std::size_t stateCount() const {
            return m_states.size();
        }

    private:
        void createState(StateType l_type) {
            auto it = m_stateFactory.find(l_type);
            if (it == m_stateFactory.end()) {
                return;
            }

            auto newState = it->second();
            m_states.emplace_back(l_type, std::move(newState));
            m_states.back().second->onCreate();
        }

        void removeState(StateType l_type) {
            for (auto it = m_states.begin(); it != m_states.end(); ++it) {
                if (it->first == l_type) {
                    const bool wasTop = (it + 1 == m_states.end());
                    it->second->onDeactivate();
                    it->second->onDestroy();
                    m_states.erase(it);
                    if (wasTop && !m_states.empty()) {
                        m_states.back().second->onActivate();
                    }
                    return;
                }
            }
        }

        StateFactory m_stateFactory;
        StateContainer m_states;
        TypeContainer m_toRemove;
        SharedContext* m_context;
    };
}
