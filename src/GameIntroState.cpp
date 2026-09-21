module;

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

module core;

import :StateManager;
import :ResourceManager;
import :SharedContext;
import :Utilities;
import :Window;
import :AudioManager;

namespace core {

namespace {
constexpr std::string_view BackgroundTextureId = "game_intro_background";
constexpr std::string_view StormCloudTextureId = "game_intro_stormcloud";
constexpr std::string_view FontId = "game_intro_font";
constexpr std::string_view IntroMusicTrackId = "game_intro_music";

constexpr std::string_view BackgroundImagePath = "graphics/gameintrobackground.png";
constexpr std::string_view StormCloudImagePath = "graphics/stormcloud4.png";
constexpr std::string_view PrimaryFontPath = "fonts/PublicPixel.ttf";
constexpr std::string_view FallbackFontPath = "fonts/main.ttf";
constexpr std::string_view IntroMusicPath = "music/gameintroscreen2.wav";

constexpr unsigned int StoryCharacterSize = 7;
constexpr unsigned int SkipPromptCharacterSize = 7;

constexpr sf::Vector2f InitialStormcloudPos{-660.0f, -210.0f};
constexpr float StormcloudTargetX = -72.0f;
constexpr float StormcloudSpeedX = 10.0f;
constexpr float StormcloudScale = 0.4f;

const std::string RawIntroText =
    "Long ago, a contract was made and the Realm of Enemata\n"
    "was sealed away.\n"
    "For centuries, the Kingdom of Rabahn prospered, peace held.\n"
    "The old stories became songs. The songs became whispers.\n"
    "But something changed in the Kingdom of Rabahn.\n"
    "The harvests grew thin. The nights became restless.\n"
    "The contract expired. On a calm evening, a storm brewed up -\n"
    "and the Seal cracked.";

std::string formatStoryText(const std::string& input, const std::size_t maxLineLength = 36) {
    std::string result;
    std::istringstream stream(input);
    std::string line;
    bool firstLine = true;

    while (std::getline(stream, line)) {
        while (!line.empty() && (line.back() == ' ' || line.back() == '\r')) {
            line.pop_back();
        }
        if (line.empty()) {
            if (!firstLine) result += '\n';
            firstLine = false;
            continue;
        }

        std::istringstream words(line);
        std::string word;
        std::string currentLine;

        while (words >> word) {
            if (currentLine.empty()) {
                currentLine = word;
            } else if (currentLine.size() + 1 + word.size() <= maxLineLength) {
                currentLine += " " + word;
            } else {
                if (!firstLine) result += '\n';
                result += currentLine;
                firstLine = false;
                currentLine = word;
            }
        }
        if (!currentLine.empty()) {
            if (!firstLine) result += '\n';
            result += currentLine;
            firstLine = false;
        }
    }
    return result;
}
} // namespace

GameIntroState::GameIntroState(StateManager* l_statemanager, SharedContext* l_context)
    : State(l_statemanager, l_context),
      m_formattedStory(formatStoryText(RawIntroText)),
      m_passedTime(0.0f),
      m_revealSpeed(12.0f),
      m_stormcloudPos(InitialStormcloudPos),
      m_darkness(1.0f),
      m_fadeSpeed(0.02f) {}

void GameIntroState::onCreate() {
    // 1. Background image
    const auto bgPath = utils::getResourcePath(BackgroundImagePath);
    if (m_context->m_resourceManager->loadTexture(std::string(BackgroundTextureId), bgPath)) {
        m_backgroundSprite.emplace(m_context->m_resourceManager->getTexture(std::string(BackgroundTextureId)));
        m_hasLoadedBackground = true;
    } else {
        std::println(stderr, "GameIntroState [WARNING]: Failed to load background '{}'.", bgPath.string());
    }

    // 2. Storm cloud texture
    const auto cloudPath = utils::getResourcePath(StormCloudImagePath);
    if (m_context->m_resourceManager->loadTexture(std::string(StormCloudTextureId), cloudPath)) {
        m_stormcloudSprite.emplace(m_context->m_resourceManager->getTexture(std::string(StormCloudTextureId)));
        m_stormcloudSprite->setScale({StormcloudScale, StormcloudScale});
        m_stormcloudSprite->setPosition(m_stormcloudPos);
        m_hasLoadedStormcloud = true;
    } else {
        std::println(stderr, "GameIntroState [WARNING]: Failed to load storm cloud '{}'.", cloudPath.string());
    }

    // 3. Font
    const auto primaryFontPath = utils::getResourcePath(PrimaryFontPath);
    if (!m_context->m_resourceManager->loadFont(std::string(FontId), primaryFontPath)) {
        const auto fallbackFontPath = utils::getResourcePath(FallbackFontPath);
        if (!m_context->m_resourceManager->loadFont(std::string(FontId), fallbackFontPath)) {
            std::println(stderr, "GameIntroState [ERROR]: Failed to load font '{}'.", fallbackFontPath.string());
            return;
        }
    }
    m_hasLoadedFont = true;
    const auto& font = m_context->m_resourceManager->getFont(std::string(FontId));

    // 4. Story text setup
    m_storyText.emplace(font, "", StoryCharacterSize);
    m_storyText->setPosition({18.0f, 14.0f});
    m_storyText->setFillColor(sf::Color::White);
    m_storyText->setOutlineColor(sf::Color(0, 0, 0, 220));
    m_storyText->setOutlineThickness(1.0f);
    m_storyText->setLineSpacing(1.2f);

    // 5. Skip prompt setup
    m_skipPromptText.emplace(font, "press space or enter to continue", SkipPromptCharacterSize);
    m_skipPromptText->setPosition({20.0f, 155.0f});
    m_skipPromptText->setFillColor(sf::Color::Transparent);
    m_skipPromptText->setOutlineColor(sf::Color(0, 0, 0, 220));
    m_skipPromptText->setOutlineThickness(1.0f);

    // 6. Audio track
    if (m_context && m_context->m_audioManager) {
        const auto musicPath = utils::getResourcePath(IntroMusicPath);
        if (!m_context->m_audioManager->hasTrack(std::string(IntroMusicTrackId))) {
            m_context->m_audioManager->loadTrack(std::string(IntroMusicTrackId), musicPath);
        }
    }
}

void GameIntroState::onActivate() {
    m_passedTime = 0.0f;
    m_stormcloudPos = InitialStormcloudPos;
    m_darkness = 1.0f;
    m_hasFinished = false;

    if (m_backgroundSprite) {
        m_backgroundSprite->setColor(sf::Color::White);
    }
    if (m_stormcloudSprite) {
        m_stormcloudSprite->setPosition(m_stormcloudPos);
    }
    if (m_storyText) {
        m_storyText->setString("");
    }
    if (m_skipPromptText) {
        m_skipPromptText->setFillColor(sf::Color::Transparent);
    }

    if (m_context && m_context->m_audioManager) {
        const auto musicPath = utils::getResourcePath(IntroMusicPath);
        if (!m_context->m_audioManager->hasTrack(std::string(IntroMusicTrackId))) {
            m_context->m_audioManager->loadTrack(std::string(IntroMusicTrackId), musicPath);
        }
        m_context->m_audioManager->play(std::string(IntroMusicTrackId), true);
    }
}

void GameIntroState::onDeactivate() {
    if (m_context && m_context->m_audioManager) {
        m_context->m_audioManager->stop();
    }
}

void GameIntroState::onDestroy() {
    m_backgroundSprite.reset();
    m_stormcloudSprite.reset();
    m_storyText.reset();
    m_skipPromptText.reset();

    if (m_hasLoadedBackground) {
        m_context->m_resourceManager->releaseTexture(std::string(BackgroundTextureId));
        m_hasLoadedBackground = false;
    }
    if (m_hasLoadedStormcloud) {
        m_context->m_resourceManager->releaseTexture(std::string(StormCloudTextureId));
        m_hasLoadedStormcloud = false;
    }
    if (m_hasLoadedFont) {
        m_context->m_resourceManager->releaseFont(std::string(FontId));
        m_hasLoadedFont = false;
    }
}

void GameIntroState::skipIntro() {
    if (m_hasFinished) {
        return;
    }
    m_hasFinished = true;
    m_stateManager->switchTo(StateType::Game);
    m_stateManager->remove(StateType::GameIntro);
}

void GameIntroState::handleEvent(const sf::Event& l_event) {
    if (const auto* key = l_event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter ||
            key->code == sf::Keyboard::Key::Space ||
            key->code == sf::Keyboard::Key::Escape) {
            skipIntro();
            return;
        }
        skipIntro();
    } else if (l_event.is<sf::Event::MouseButtonPressed>()) {
        skipIntro();
    }
}

void GameIntroState::update(const float l_dt) {
    m_passedTime += l_dt;

    // Darkness fade after 20 seconds
    if (m_passedTime > 20.0f) {
        m_darkness = std::max(0.353f, m_darkness - (m_fadeSpeed * l_dt));
        if (m_backgroundSprite) {
            const auto intensity = static_cast<std::uint8_t>(m_darkness * 255.0f);
            m_backgroundSprite->setColor(sf::Color(intensity, intensity, intensity));
        }
    }

    // Storm cloud movement
    if (m_stormcloudPos.x < StormcloudTargetX) {
        m_stormcloudPos.x += l_dt * StormcloudSpeedX;
        if (m_stormcloudSprite) {
            m_stormcloudSprite->setPosition(m_stormcloudPos);
        }
    }

    // Typewriter reveal based on passed_time * reveal_speed
    const auto charsToShow = static_cast<std::size_t>(m_passedTime * m_revealSpeed);
    if (m_storyText) {
        if (charsToShow < m_formattedStory.size()) {
            m_storyText->setString(m_formattedStory.substr(0, charsToShow));
        } else {
            m_storyText->setString(m_formattedStory);
        }
    }

    // Skip prompt appears after 42 seconds with sinusoidal pulsation
    if (m_passedTime > 42.0f && m_skipPromptText) {
        const float sinVal = std::sin(m_passedTime * 4.0f);
        const auto flickerAlpha = static_cast<std::uint8_t>(((sinVal + 1.0f) / 2.0f) * 255.0f);
        m_skipPromptText->setFillColor(sf::Color(255, 210, 80, flickerAlpha));
    }
}

void GameIntroState::draw() {
    m_context->m_window->clear(sf::Color::Black);

    if (m_backgroundSprite) {
        m_context->m_window->draw(*m_backgroundSprite);
    }
    if (m_stormcloudSprite) {
        m_context->m_window->draw(*m_stormcloudSprite);
    }
    if (m_storyText) {
        m_context->m_window->draw(*m_storyText);
    }
    if (m_skipPromptText && m_passedTime > 42.0f) {
        m_context->m_window->draw(*m_skipPromptText);
    }
}

} // namespace core
