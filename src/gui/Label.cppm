module;

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

export module core:GUI_Label;

import :GUI_Element;
import std;

export namespace core::gui {
    class Label : public Element {
    public:
        enum class Alignment {
            Left,
            Center,
            Right
        };

        Label() = default;

        Label(const sf::Font &font, const std::string &text = "", unsigned int characterSize = 12)
            : m_string(text), m_characterSize(characterSize) {
            setFont(font);
        }

        void setFont(const sf::Font &font) {
            m_font = &font;
            m_text.emplace(font, m_string, m_characterSize);
            m_text->setFillColor(m_fillColor);
            m_text->setOutlineColor(m_outlineColor);
            m_text->setOutlineThickness(m_outlineThickness);
            updateAlignmentOrigin();
        }

        void setString(const std::string &text) {
            m_string = text;
            if (m_text) {
                m_text->setString(m_string);
                updateAlignmentOrigin();
            }
        }

        [[nodiscard]] const std::string &getString() const { return m_string; }

        void setCharacterSize(unsigned int size) {
            m_characterSize = size;
            if (m_text) {
                m_text->setCharacterSize(size);
                updateAlignmentOrigin();
            }
        }

        [[nodiscard]] unsigned int getCharacterSize() const { return m_characterSize; }

        void setFillColor(sf::Color color) {
            m_fillColor = color;
            if (m_text) {
                m_text->setFillColor(color);
            }
        }

        [[nodiscard]] sf::Color getFillColor() const { return m_fillColor; }

        void setOutlineColor(sf::Color color) {
            m_outlineColor = color;
            if (m_text) {
                m_text->setOutlineColor(color);
            }
        }

        void setOutlineThickness(float thickness) {
            m_outlineThickness = thickness;
            if (m_text) {
                m_text->setOutlineThickness(thickness);
                updateAlignmentOrigin();
            }
        }

        void setAlignment(Alignment alignment) {
            m_alignment = alignment;
            updateAlignmentOrigin();
        }

        [[nodiscard]] Alignment getAlignment() const { return m_alignment; }

        [[nodiscard]] sf::FloatRect getLocalBounds() const override {
            if (m_text) {
                return m_text->getLocalBounds();
            }
            return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
        }

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
            if (!m_visible || !m_text) {
                return;
            }
            states.transform *= getTransform();
            target.draw(*m_text, states);
        }

    private:
        void updateAlignmentOrigin() {
            if (!m_text) {
                return;
            }
            const auto bounds = m_text->getLocalBounds();
            switch (m_alignment) {
                case Alignment::Left:
                    m_text->setOrigin({bounds.position.x, bounds.position.y});
                    break;
                case Alignment::Center:
                    m_text->setOrigin({bounds.position.x + bounds.size.x / 2.0f, bounds.position.y});
                    break;
                case Alignment::Right:
                    m_text->setOrigin({bounds.position.x + bounds.size.x, bounds.position.y});
                    break;
            }
        }

        std::optional<sf::Text> m_text;
        const sf::Font* m_font{nullptr};
        std::string m_string;
        unsigned int m_characterSize{12};
        sf::Color m_fillColor{sf::Color::White};
        sf::Color m_outlineColor{sf::Color::Transparent};
        float m_outlineThickness{0.0f};
        Alignment m_alignment{Alignment::Left};
    };
} // namespace core::gui

export namespace GUI {
    using Label = core::gui::Label;
}
