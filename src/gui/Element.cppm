module;

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

export module core:GUI_Element;

export namespace core::gui {
    class Element : public sf::Drawable, public sf::Transformable {
    public:
        Element() = default;

        ~Element() override = default;

        virtual void update(float /*dt*/) {
        }

        virtual void handleEvent(const sf::Event & /*event*/, const sf::Vector2f & /*parentMousePos*/) {
        }

        virtual void resetHover() {
        }

        [[nodiscard]] virtual sf::FloatRect getLocalBounds() const = 0;

        [[nodiscard]] virtual sf::FloatRect getGlobalBounds() const {
            return getTransform().transformRect(getLocalBounds());
        }

        void setVisible(bool visible) { m_visible = visible; }
        [[nodiscard]] bool isVisible() const { return m_visible; }

        void setEnabled(bool enabled) { m_enabled = enabled; }
        [[nodiscard]] bool isEnabled() const { return m_enabled; }

    protected:
        bool m_visible{true};
        bool m_enabled{true};
    };
} // namespace core::gui

export namespace GUI {
    using Element = core::gui::Element;
}
