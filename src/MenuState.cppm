module;
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Window/Event.hpp>

export module core:MenuState;
import std;
import :State;
import :GUI;

export namespace core {
    class MenuState final : public State {
    public:
        MenuState(StateManager* l_stateManager, SharedContext* l_context);

        ~MenuState() override = default;

        void onCreate() override;

        void onActivate() override;

        void onDeactivate() override;

        void onDestroy() override;

        void update(const float l_dt) override;

        void draw() override;

        void handleEvent(const sf::Event &l_event) override;

    private:
        void selectButton(std::size_t index);

        std::shared_ptr<GUI::Button> createMenuButton(
            const sf::Font &font,
            const std::string &text,
            sf::Vector2f position,
            std::function<void()> onClick,
            unsigned int characterSize = 8);

        std::optional<sf::Sprite> m_menuBackgroundSprite;
        GUI::Container m_guiContainer;
        std::vector<std::shared_ptr<GUI::Button> > m_buttons;
        std::size_t m_selectedIndex{0};
        bool m_hasLoadedTexture{false};
        bool m_hasLoadedFont{false};
    };
} // namespace core
