module;
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

export module core:GameIntroState;

import std;
import :State;

export namespace core {
    class GameIntroState final : public State {
    public:
        GameIntroState(StateManager* l_stateManager, SharedContext* l_context);
        ~GameIntroState() override = default;

        void onCreate() override;
        void onActivate() override;
        void onDeactivate() override;
        void onDestroy() override;

        void update(const float l_dt) override;
        void draw() override;
        void handleEvent(const sf::Event& l_event) override;

    private:
        void skipIntro();

        std::optional<sf::Sprite> m_backgroundSprite;
        std::optional<sf::Sprite> m_stormcloudSprite;
        std::optional<sf::Text> m_storyText;
        std::optional<sf::Text> m_skipPromptText;

        std::string m_formattedStory;
        float m_passedTime{0.0f};
        float m_revealSpeed{12.0f};
        sf::Vector2f m_stormcloudPos{-660.0f, -210.0f};
        float m_darkness{1.0f};
        float m_fadeSpeed{0.02f};

        bool m_hasLoadedBackground{false};
        bool m_hasLoadedStormcloud{false};
        bool m_hasLoadedFont{false};
        bool m_hasFinished{false};
    };
} // namespace core
