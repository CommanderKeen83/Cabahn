module;

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

export module core:GUI_Container;

import :GUI_Element;
import std;

export namespace core::gui {
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
            float currentY = std::round(startY);
            for (auto &element: m_elements) {
                element->setPosition({std::round(startX), std::round(currentY)});
                currentY += std::round(element->getLocalBounds().size.y) + spacing;
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
    using Container = core::gui::Container;
}
