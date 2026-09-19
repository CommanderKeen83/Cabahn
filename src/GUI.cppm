module;

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

export module core:GUI;

import std;

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

    class Button : public Element {
    public:
        enum class State {
            Normal,
            Hovered,
            Pressed,
            Disabled
        };

        struct Style {
            sf::Color fillColor{40, 40, 40, 200};
            sf::Color outlineColor{120, 120, 120, 255};
            sf::Color textColor{220, 220, 220, 255};
        };

        Button(const sf::Font &font, const std::string &text = "", const sf::Vector2f &size = {80.f, 20.f},
               unsigned int characterSize = 11)
            : m_size(size), m_string(text), m_characterSize(characterSize) {
            m_shape.setSize(size);
            m_shape.setOutlineThickness(1.0f);

            m_normalStyle = {
                .fillColor = sf::Color(35, 35, 45, 210),
                .outlineColor = sf::Color(80, 80, 110, 220),
                .textColor = sf::Color(210, 210, 220, 255)
            };
            m_hoverStyle = {
                .fillColor = sf::Color(70, 70, 100, 240),
                .outlineColor = sf::Color(140, 140, 200, 255),
                .textColor = sf::Color(255, 255, 255, 255)
            };
            m_pressedStyle = {
                .fillColor = sf::Color(20, 20, 30, 240),
                .outlineColor = sf::Color(180, 180, 220, 255),
                .textColor = sf::Color(180, 200, 255, 255)
            };
            m_disabledStyle = {
                .fillColor = sf::Color(25, 25, 25, 120),
                .outlineColor = sf::Color(50, 50, 50, 100),
                .textColor = sf::Color(100, 100, 100, 150)
            };

            setFont(font);
            applyCurrentStyle();
        }

        void setText(const std::string &text) {
            m_string = text;
            if (m_text) {
                m_text->setString(m_string);
                centerText();
            }
        }

        [[nodiscard]] const std::string &getText() const { return m_string; }

        void setFont(const sf::Font &font) {
            m_text.emplace(font, m_string, m_characterSize);
            centerText();
            applyCurrentStyle();
        }

        void setCharacterSize(unsigned int size) {
            m_characterSize = size;
            if (m_text) {
                m_text->setCharacterSize(size);
                centerText();
            }
        }

        void setTextSize(unsigned int size) {
            setCharacterSize(size);
        }

        [[nodiscard]] unsigned int getCharacterSize() const { return m_characterSize; }
        [[nodiscard]] unsigned int getTextSize() const { return m_characterSize; }

        void setSize(const sf::Vector2f &size) {
            m_size = size;
            m_shape.setSize(size);
            centerText();
        }

        void setSizeToText(float paddingX = 4.0f, float paddingY = 2.0f) {
            if (m_text) {
                const auto bounds = m_text->getLocalBounds();
                setSize({bounds.size.x + paddingX * 2.0f, bounds.size.y + paddingY * 2.0f});
            }
        }

        [[nodiscard]] sf::Vector2f getSize() const { return m_size; }

        void setOnClick(std::function<void()> callback) {
            m_onClick = std::move(callback);
        }

        void setStyle(State state, const Style &style) {
            switch (state) {
                case State::Normal: m_normalStyle = style;
                    break;
                case State::Hovered: m_hoverStyle = style;
                    break;
                case State::Pressed: m_pressedStyle = style;
                    break;
                case State::Disabled: m_disabledStyle = style;
                    break;
            }
            applyCurrentStyle();
        }

        void setStyle(State state, sf::Color fill, sf::Color outline, sf::Color text) {
            setStyle(state, Style{fill, outline, text});
        }

        void setFillColor(sf::Color color) {
            if (m_backgroundEnabled) {
                m_normalStyle.fillColor = color;
            } else {
                m_normalStyle.textColor = color;
            }
            applyCurrentStyle();
        }

        void setFillColor(State state, sf::Color color) {
            switch (state) {
                case State::Normal: m_normalStyle.fillColor = color;
                    break;
                case State::Hovered: m_hoverStyle.fillColor = color;
                    break;
                case State::Pressed: m_pressedStyle.fillColor = color;
                    break;
                case State::Disabled: m_disabledStyle.fillColor = color;
                    break;
            }
            applyCurrentStyle();
        }

        void setTextColor(sf::Color color) {
            m_normalStyle.textColor = color;
            applyCurrentStyle();
        }

        void setTextColor(State state, sf::Color color) {
            switch (state) {
                case State::Normal: m_normalStyle.textColor = color;
                    break;
                case State::Hovered: m_hoverStyle.textColor = color;
                    break;
                case State::Pressed: m_pressedStyle.textColor = color;
                    break;
                case State::Disabled: m_disabledStyle.textColor = color;
                    break;
            }
            applyCurrentStyle();
        }

        void setTextHoverColor(sf::Color color) {
            setTextColor(State::Hovered, color);
        }

        void setTextPressedColor(sf::Color color) {
            setTextColor(State::Pressed, color);
        }

        [[nodiscard]] sf::Color getTextColor() const {
            return m_normalStyle.textColor;
        }

        [[nodiscard]] sf::Color getTextColor(State state) const {
            switch (state) {
                case State::Normal: return m_normalStyle.textColor;
                case State::Hovered: return m_hoverStyle.textColor;
                case State::Pressed: return m_pressedStyle.textColor;
                case State::Disabled: return m_disabledStyle.textColor;
            }
            return m_normalStyle.textColor;
        }

        void setSelectedColor(sf::Color color) {
            m_hoverStyle.textColor = color;
            if (m_backgroundEnabled) {
                m_hoverStyle.fillColor = color;
            }
            applyCurrentStyle();
        }

        [[nodiscard]] sf::Color getSelectedColor() const {
            return m_hoverStyle.textColor;
        }

        void setOnHover(std::function<void()> callback) {
            m_onHover = std::move(callback);
        }

        void resetHover() override {
            if (m_state == State::Hovered) {
                m_state = State::Normal;
                applyCurrentStyle();
            }
        }

        void click() {
            if (m_enabled && m_onClick) {
                m_onClick();
            }
        }

        void setSelected(bool selected) {
            m_selected = selected;
            applyCurrentStyle();
        }

        [[nodiscard]] bool isSelected() const {
            return m_selected;
        }

        void setBackgroundEnabled(bool enabled) {
            m_backgroundEnabled = enabled;
            centerText();
        }

        [[nodiscard]] bool isBackgroundEnabled() const {
            return m_backgroundEnabled;
        }

        [[nodiscard]] State getState() const { return m_state; }

        [[nodiscard]] sf::FloatRect getLocalBounds() const override {
            if (!m_backgroundEnabled && m_text) {
                const auto textBounds = m_text->getLocalBounds();
                const float width = std::max(m_size.x, textBounds.size.x);
                const float height = std::max(m_size.y, textBounds.size.y);
                return sf::FloatRect({0.f, 0.f}, {width, height});
            }
            if (m_size.x > 0.0f && m_size.y > 0.0f) {
                return sf::FloatRect({0.f, 0.f}, m_size);
            }
            if (m_text) {
                return m_text->getLocalBounds();
            }
            return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
        }

        void handleEvent(const sf::Event &event, const sf::Vector2f &parentMousePos) override {
            if (!m_visible || !m_enabled) {
                if (m_state != State::Disabled) {
                    m_state = State::Disabled;
                    applyCurrentStyle();
                }
                return;
            }

            if (m_state == State::Disabled) {
                m_state = State::Normal;
                applyCurrentStyle();
            }

            const sf::Vector2f buttonLocalPos = getInverseTransform().transformPoint(parentMousePos);
            const bool isInside = getLocalBounds().contains(buttonLocalPos);

            if (event.is<sf::Event::MouseMoved>()) {
                if (isInside) {
                    const bool wasHovered = (m_state == State::Hovered);
                    if (m_state != State::Pressed) {
                        m_state = State::Hovered;
                    }
                    if (!wasHovered && m_onHover) {
                        m_onHover();
                    }
                } else {
                    m_state = State::Normal;
                }
                applyCurrentStyle();
            } else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left && isInside) {
                    m_state = State::Pressed;
                    applyCurrentStyle();
                }
            } else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    if (m_state == State::Pressed && isInside) {
                        m_state = State::Hovered;
                        applyCurrentStyle();
                        if (m_onClick) {
                            m_onClick();
                        }
                    } else {
                        m_state = isInside ? State::Hovered : State::Normal;
                        applyCurrentStyle();
                    }
                }
            }
        }

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
            if (!m_visible) {
                return;
            }
            states.transform *= getTransform();
            if (m_backgroundEnabled) {
                target.draw(m_shape, states);
            }
            if (m_text) {
                target.draw(*m_text, states);
            }
        }

    private:
        void centerText() {
            if (!m_text) {
                return;
            }
            const sf::FloatRect textBounds = m_text->getLocalBounds();
            if (!m_backgroundEnabled) {
                m_text->setOrigin({textBounds.position.x, textBounds.position.y});
                m_text->setPosition({0.0f, 0.0f});
            } else {
                m_text->setOrigin({
                    textBounds.position.x + textBounds.size.x / 2.0f,
                    textBounds.position.y + textBounds.size.y / 2.0f
                });
                m_text->setPosition({m_size.x / 2.0f, m_size.y / 2.0f});
            }
        }

        void applyCurrentStyle() {
            const Style* activeStyle = &m_normalStyle;
            if (!m_enabled) {
                activeStyle = &m_disabledStyle;
            } else if (m_state == State::Pressed) {
                activeStyle = &m_pressedStyle;
            } else if (m_state == State::Hovered || m_selected) {
                activeStyle = &m_hoverStyle;
            } else {
                activeStyle = &m_normalStyle;
            }

            m_shape.setFillColor(activeStyle->fillColor);
            m_shape.setOutlineColor(activeStyle->outlineColor);
            if (m_text) {
                m_text->setFillColor(activeStyle->textColor);
            }
        }

        sf::RectangleShape m_shape;
        std::optional<sf::Text> m_text;
        sf::Vector2f m_size{80.f, 20.f};
        std::string m_string;
        unsigned int m_characterSize{11};
        State m_state{State::Normal};
        bool m_backgroundEnabled{true};
        bool m_selected{false};
        std::function<void()> m_onClick;
        std::function<void()> m_onHover;

        Style m_normalStyle;
        Style m_hoverStyle;
        Style m_pressedStyle;
        Style m_disabledStyle;
    };

    class Container : public Element {
    public:
        Container() = default;

        template<typename T, typename... Args>
        std::shared_ptr<T> createWidget(Args &&... args) {
            auto widget = std::make_shared<T>(std::forward<Args>(args)...);
            m_elements.push_back(widget);
            return widget;
        }

        void addWidget(std::shared_ptr<Element> widget) {
            if (widget) {
                m_elements.push_back(std::move(widget));
            }
        }

        void removeWidget(const std::shared_ptr<Element> &widget) {
            std::erase(m_elements, widget);
        }

        void clear() {
            m_elements.clear();
        }

        [[nodiscard]] std::size_t getWidgetCount() const {
            return m_elements.size();
        }

        void setBackground(const sf::Vector2f &size,
                           sf::Color fillColor,
                           sf::Color outlineColor = sf::Color::Transparent,
                           float outlineThickness = 0.0f) {
            m_backgroundShape.setSize(size);
            m_backgroundShape.setFillColor(fillColor);
            m_backgroundShape.setOutlineColor(outlineColor);
            m_backgroundShape.setOutlineThickness(outlineThickness);
            m_hasBackground = true;
        }

        void setBackgroundEnabled(bool enabled) {
            m_hasBackground = enabled;
        }

        void layoutVertical(float startX, float startY, float spacing) {
            float currentY = startY;
            for (auto &element: m_elements) {
                element->setPosition({startX, currentY});
                currentY += element->getLocalBounds().size.y + spacing;
            }
        }

        void update(float dt) override {
            if (!m_visible) {
                return;
            }
            for (auto &element: m_elements) {
                if (element->isVisible()) {
                    element->update(dt);
                }
            }
        }

        void handleEvent(const sf::Event &event, const sf::Vector2f &parentMousePos) override {
            if (!m_visible || !m_enabled) {
                return;
            }

            const sf::Vector2f localMousePos = getInverseTransform().transformPoint(parentMousePos);

            if (event.is<sf::Event::MouseMoved>()) {
                // Find ONE topmost element the mouse is actually over.
                Element* hovered = nullptr;
                for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
                    auto &element = *it;
                    if (!element->isVisible() || !element->isEnabled()) {
                        continue;
                    }
                    const sf::Vector2f local = element->getInverseTransform().transformPoint(localMousePos);
                    if (element->getLocalBounds().contains(local)) {
                        hovered = element.get();
                        break;
                    }
                }

                for (auto &element: m_elements) {
                    if (!element->isVisible() || !element->isEnabled()) {
                        continue;
                    }
                    if (element.get() == hovered) {
                        element->handleEvent(event, localMousePos);
                    } else {
                        element->resetHover();
                    }
                }
                return;
            }

            // All other events (clicks, etc.) pass through normally.
            for (auto &element: m_elements) {
                if (element->isVisible() && element->isEnabled()) {
                    element->handleEvent(event, localMousePos);
                }
            }
        }

        [[nodiscard]] sf::FloatRect getLocalBounds() const override {
            if (m_hasBackground) {
                return m_backgroundShape.getLocalBounds();
            }

            if (m_elements.empty()) {
                return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
            }

            float minX = std::numeric_limits<float>::max();
            float minY = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float maxY = std::numeric_limits<float>::lowest();

            for (const auto &element: m_elements) {
                const auto bounds = element->getGlobalBounds();
                minX = std::min(minX, bounds.position.x);
                minY = std::min(minY, bounds.position.y);
                maxX = std::max(maxX, bounds.position.x + bounds.size.x);
                maxY = std::max(maxY, bounds.position.y + bounds.size.y);
            }

            return sf::FloatRect({minX, minY}, {maxX - minX, maxY - minY});
        }

        void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
            if (!m_visible) {
                return;
            }
            // Child rendering happens locally to the container:
            states.transform *= getTransform();

            if (m_hasBackground) {
                target.draw(m_backgroundShape, states);
            }

            for (const auto &element: m_elements) {
                if (element->isVisible()) {
                    target.draw(*element, states);
                }
            }
        }

    private:
        std::vector<std::shared_ptr<Element> > m_elements;
        sf::RectangleShape m_backgroundShape;
        bool m_hasBackground{false};
    };
} // namespace core::gui

export namespace GUI {
    using Element = core::gui::Element;
    using Label = core::gui::Label;
    using Button = core::gui::Button;
    using Container = core::gui::Container;
}
