#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <cassert>

import core;
import std;

int main() {
    std::println("Starting GUI UX-Elements tests...");

    core::ResourceManager manager;
    const std::filesystem::path fontPath = core::utils::getResourcePath("fonts/main.ttf");
    if (!manager.loadFont("main", fontPath)) {
        std::println(stderr, "Failed to load font from {}", fontPath.string());
        return 1;
    }
    const auto& font = manager.getFont("main");

    // Test 1: Container creation and widget management
    std::println("Test 1: Container creation and widget management...");
    GUI::Container container;
    container.setPosition({100.0f, 50.0f});
    container.setBackground({120.0f, 100.0f}, sf::Color::Black, sf::Color::White, 1.0f);

    assert(container.getWidgetCount() == 0);

    // Test 2: Label creation and properties
    std::println("Test 2: Label creation and properties...");
    auto label = container.createWidget<GUI::Label>(font, "Hello World", 14);
    assert(container.getWidgetCount() == 1);
    assert(label->getString() == "Hello World");
    assert(label->getCharacterSize() == 14);

    label->setString("Main Menu");
    assert(label->getString() == "Main Menu");
    label->setAlignment(GUI::Label::Alignment::Center);
    assert(label->getAlignment() == GUI::Label::Alignment::Center);

    // Test 3: Button creation and interactive states
    std::println("Test 3: Button creation and interactive states...");
    auto button = container.createWidget<GUI::Button>(font, "PLAY", sf::Vector2f{80.0f, 20.0f});
    assert(container.getWidgetCount() == 2);
    button->setPosition({10.0f, 30.0f});
    assert(button->getState() == GUI::Button::State::Normal);
    assert(button->isBackgroundEnabled());
    button->setBackgroundEnabled(false);
    assert(!button->isBackgroundEnabled());

    bool clicked = false;
    button->setOnClick([&clicked]() {
        clicked = true;
    });

    // Test 4: Mouse event handling local to container
    std::println("Test 4: Mouse event handling local to container...");
    // Container is at (100, 50). Button is at local (10, 30) with size (80, 20).
    // Button in parent coordinates is [110, 190] x [80, 100].

    // Inside button: parent pos = (120, 90) -> local to container = (20, 40) -> inside button [10..90, 30..50]
    sf::Vector2f insideParentPos{120.0f, 90.0f};
    sf::Vector2f outsideParentPos{50.0f, 50.0f};

    // 4a. Hover event
    sf::Event moveInsideEvent{sf::Event::MouseMoved{sf::Vector2i{120, 90}}};
    container.handleEvent(moveInsideEvent, insideParentPos);
    assert(button->getState() == GUI::Button::State::Hovered);

    // 4b. Move outside
    sf::Event moveOutsideEvent{sf::Event::MouseMoved{sf::Vector2i{50, 50}}};
    container.handleEvent(moveOutsideEvent, outsideParentPos);
    assert(button->getState() == GUI::Button::State::Normal);

    // 4c. Press inside
    sf::Event pressEvent{sf::Event::MouseButtonPressed{sf::Mouse::Button::Left, sf::Vector2i{120, 90}}};
    container.handleEvent(pressEvent, insideParentPos);
    assert(button->getState() == GUI::Button::State::Pressed);

    // 4d. Release inside -> triggers click
    sf::Event releaseEvent{sf::Event::MouseButtonReleased{sf::Mouse::Button::Left, sf::Vector2i{120, 90}}};
    container.handleEvent(releaseEvent, insideParentPos);
    assert(clicked == true);
    assert(button->getState() == GUI::Button::State::Hovered);

    // Test 5: Rendering locally to container
    std::println("Test 5: Container rendering to render target...");
    sf::RenderTexture renderTexture({320, 180});
    renderTexture.clear(sf::Color::Black);
    renderTexture.draw(container);
    renderTexture.display();

    // Test 6: Container clear
    std::println("Test 6: Container widget clear...");
    container.clear();
    assert(container.getWidgetCount() == 0);

    std::println("All GUI tests PASSED successfully!");
    return 0;
}
