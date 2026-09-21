#include <SFML/Window/Event.hpp>
#include <cassert>

import core;
import std;

int main() {
    std::println("Starting GameIntroState tests...");

    core::ResourceManager resourceManager;
    core::AudioManager audioManager;
    core::SharedContext context;
    context.m_resourceManager = &resourceManager;
    context.m_audioManager = &audioManager;

    core::StateManager stateManager(&context);

    // Test 1: Initial state is Menu
    std::println("Test 1: Initial Menu state...");
    stateManager.switchTo(core::StateType::Menu);
    assert(stateManager.getTopStateType() == core::StateType::Menu);

    // Test 2: Transition from Menu to GameIntro
    std::println("Test 2: Transition to GameIntroState...");
    stateManager.switchTo(core::StateType::GameIntro);
    assert(stateManager.getTopStateType() == core::StateType::GameIntro);
    assert(stateManager.hasState(core::StateType::GameIntro));

    // Update simulation
    stateManager.update(0.05f);

    // Test 3: Pressing any key skips GameIntro and starts GameState
    std::println("Test 3: Skipping intro via KeyPressed...");
    sf::Event::KeyPressed keyEvent{};
    keyEvent.code = sf::Keyboard::Key::Space;
    sf::Event event(keyEvent);
    stateManager.handleEvent(event);

    stateManager.processRequests();

    assert(stateManager.getTopStateType() == core::StateType::Game);
    assert(!stateManager.hasState(core::StateType::GameIntro));

    std::println("All GameIntroState tests passed successfully!");
    return 0;
}
