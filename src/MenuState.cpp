module;

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

module core;

import :StateManager;
import :ResourceManager;
import :SharedContext;
import :Utilities;
import :Window;
import :GUI;

namespace core {

namespace {
// Resource identifiers
constexpr std::string_view TextureId = "main_menu_texture";
constexpr std::string_view FontId = "main_font";

// Asset paths (relative to resources/)
constexpr std::string_view BackgroundImagePath = "graphics/main_menu.png";
constexpr std::string_view PrimaryFontPath = "fonts/PublicPixel.ttf";
constexpr std::string_view FallbackFontPath = "fonts/main.ttf";

// Layout configuration (within the 320x180 virtual retro canvas)
constexpr sf::Vector2f MenuContainerPosition{230.0f, 105.0f};
constexpr sf::Vector2f ButtonBoxSize{74.0f, 18.0f}; // Dimensions of clickable button box
constexpr unsigned int ButtonTextSize = 8;          // Font character size (8px is native for PublicPixel!)
constexpr float ButtonStartX = 8.0f;
constexpr float ButtonStartY = 25.0f;
constexpr float ButtonSpacingY = 12.0f;

// Color palette: The selected button is Yellow, unselected buttons are Grey
constexpr sf::Color ButtonNormalColor{180, 180, 180};
constexpr sf::Color ButtonSelectedColor{sf::Color::Yellow};
} // namespace

MenuState::MenuState(StateManager *l_statemanager, SharedContext *l_context)
    : State(l_statemanager, l_context) {}

void MenuState::selectButton(std::size_t index) {
  if (index >= m_buttons.size()) {
    return;
  }
  m_selectedIndex = index;
  for (std::size_t i = 0; i < m_buttons.size(); ++i) {
    m_buttons[i]->setSelected(i == index);
  }
}

std::shared_ptr<GUI::Button>
MenuState::createMenuButton(const sf::Font &font, const std::string &text,
                            sf::Vector2f position,
                            std::function<void()> onClick,
                            const unsigned int characterSize) {
  const std::size_t index = m_buttons.size();
  auto btn = m_guiContainer.createWidget<GUI::Button>(font, text, ButtonBoxSize, characterSize);
  btn->setPosition(position);
  btn->setBackgroundEnabled(false);
  btn->setTextColor(ButtonNormalColor);
  btn->setSelectedColor(ButtonSelectedColor);
  btn->setCharacterSize(characterSize);
  btn->setOnClick(std::move(onClick));
  btn->setOnHover([this, index]() { selectButton(index); });
  m_buttons.push_back(btn);
  return btn;
}

void MenuState::onCreate() {
  const auto bgPath = utils::getResourcePath(BackgroundImagePath);
  if (m_context->m_resourceManager->loadTexture(std::string(TextureId),
                                                bgPath)) {
    m_menuBackgroundSprite.emplace(
        m_context->m_resourceManager->getTexture(std::string(TextureId)));
    m_hasLoadedTexture = true;
  } else {
    std::println(stderr,
                 "MenuState [WARNING]: Failed to load background '{}'. Solid "
                 "background will be used.",
                 bgPath.string());
  }

  const auto primaryFontPath = utils::getResourcePath(PrimaryFontPath);
  if (!m_context->m_resourceManager->loadFont(std::string(FontId),
                                              primaryFontPath)) {
    std::println(stderr,
                 "MenuState [WARNING]: Failed to load primary font '{}'. "
                 "Attempting fallback...",
                 primaryFontPath.string());
    const auto fallbackFontPath = utils::getResourcePath(FallbackFontPath);
    if (!m_context->m_resourceManager->loadFont(std::string(FontId),
                                                fallbackFontPath)) {
      std::println(stderr,
                   "MenuState [ERROR]: Failed to load fallback font '{}'. Menu "
                   "text cannot be rendered.",
                   fallbackFontPath.string());
      return;
    }
  }
  m_hasLoadedFont = true;
  const auto &font = m_context->m_resourceManager->getFont(std::string(FontId));

  m_guiContainer.clear();
  m_buttons.clear();
  m_guiContainer.setPosition(MenuContainerPosition);

  // Create menu buttons (all start with Grey text, and become Yellow when
  // selected)
  createMenuButton(font, "PLAY", {ButtonStartX, ButtonStartY},
                   [this]() { m_stateManager->switchTo(StateType::Game); },
                   ButtonTextSize);

  createMenuButton(font, "OPTIONS",
                   {ButtonStartX, ButtonStartY + ButtonSpacingY},
                   [this]() { m_stateManager->switchTo(StateType::Options); },
                   ButtonTextSize);

  createMenuButton(
      font, "QUIT", {ButtonStartX, ButtonStartY + 2.0f * ButtonSpacingY},
      [this]() { m_context->m_window->close(); },
      ButtonTextSize);

  // Initially select the first item (PLAY is yellow, other options are grey)
  selectButton(0);
}

void MenuState::onActivate() {}

void MenuState::onDeactivate() {}

void MenuState::onDestroy() {
  m_guiContainer.clear();
  m_buttons.clear();
  m_menuBackgroundSprite.reset();

  if (m_hasLoadedTexture) {
    m_context->m_resourceManager->releaseTexture(std::string(TextureId));
    m_hasLoadedTexture = false;
  }
  if (m_hasLoadedFont) {
    m_context->m_resourceManager->releaseFont(std::string(FontId));
    m_hasLoadedFont = false;
  }
}

void MenuState::handleEvent(const sf::Event &l_event) {
  // Keyboard navigation support: UP/DOWN or W/S moves selection, ENTER/SPACE
  // triggers it
  if (const auto *key = l_event.getIf<sf::Event::KeyPressed>()) {
    if (key->code == sf::Keyboard::Key::Up ||
        key->code == sf::Keyboard::Key::W) {
      if (!m_buttons.empty()) {
        const std::size_t prev =
            (m_selectedIndex == 0) ? m_buttons.size() - 1 : m_selectedIndex - 1;
        selectButton(prev);
      }
      return;
    }
    if (key->code == sf::Keyboard::Key::Down ||
        key->code == sf::Keyboard::Key::S) {
      if (!m_buttons.empty()) {
        const std::size_t next = (m_selectedIndex + 1) % m_buttons.size();
        selectButton(next);
      }
      return;
    }
    if (key->code == sf::Keyboard::Key::Enter ||
        key->code == sf::Keyboard::Key::Space) {
      if (m_selectedIndex < m_buttons.size()) {
        m_buttons[m_selectedIndex]->click();
      }
      return;
    }
  }

  const sf::Vector2f mouseVirtualPos =
      m_context->m_window->getMouseVirtualPosition();
  m_guiContainer.handleEvent(l_event, mouseVirtualPos);
}

void MenuState::update(const float l_dt) { m_guiContainer.update(l_dt); }

void MenuState::draw() {
  m_context->m_window->clear(sf::Color::Black);
  if (m_menuBackgroundSprite) {
    m_context->m_window->draw(*m_menuBackgroundSprite);
  }
  m_context->m_window->draw(m_guiContainer);
}

} // namespace core