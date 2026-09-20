#include <SFML/Audio.hpp>
#include <cassert>

import core;
import std;

int main() {
    std::println("Starting AudioManager tests...");

    core::AudioManager audioManager;

    const std::filesystem::path menuPath = core::utils::getResourcePath("music/menu_screen.wav");
    const std::filesystem::path introPath = core::utils::getResourcePath("music/gameintroscreen2.wav");
    const std::filesystem::path continuePath = core::utils::getResourcePath("music/menu_screen_continue.wav");

    // Test 1: Track registration and validation
    std::println("Test 1: Track registration and validation...");
    assert(!audioManager.hasTrack("menu"));
    assert(!audioManager.hasTrack("intro"));

    // Invalid path must fail
    assert(!audioManager.loadTrack("invalid", "non_existent_audio_file.wav"));

    // Valid paths must succeed
    assert(audioManager.loadTrack("menu", menuPath));
    assert(audioManager.loadTrack("intro", introPath));
    assert(audioManager.hasTrack("menu"));
    assert(audioManager.hasTrack("intro"));
    assert(audioManager.getTrackCount() == 2);

    auto retrievedPath = audioManager.getTrackPath("menu");
    assert(retrievedPath.has_value());
    assert(*retrievedPath == menuPath);

    // Test 2: Playback and properties
    std::println("Test 2: Playback and properties...");
    assert(audioManager.isStopped());
    assert(audioManager.getCurrentTrackName().empty());

    assert(audioManager.play("menu", true));
    assert(audioManager.getCurrentTrackName() == "menu");
    assert(audioManager.isLooping());

    audioManager.setMasterVolume(80.0f);
    assert(audioManager.getMasterVolume() == 80.0f);

    audioManager.setMusicVolume(50.0f);
    assert(audioManager.getMusicVolume() == 50.0f);
    assert(audioManager.getVolume() == 50.0f);

    audioManager.setPitch(1.25f);
    assert(audioManager.getPitch() == 1.25f);

    // Test 3: Pause and Resume
    std::println("Test 3: Pause and Resume...");
    audioManager.pause();
    assert(audioManager.isPaused());

    audioManager.resume();
    assert(!audioManager.isPaused());

    // Test 4: Immediate Track Changing
    std::println("Test 4: Immediate Track Changing...");
    assert(audioManager.changeTrack("intro", false, 0.0f));
    assert(audioManager.getCurrentTrackName() == "intro");
    assert(!audioManager.isLooping());

    // Test 5: Crossfade Track Changing
    std::println("Test 5: Crossfade Track Changing...");
    assert(audioManager.changeTrack("menu", true, 0.4f));
    // Simulate frames
    audioManager.update(0.2f);
    audioManager.update(0.25f); // Completed fade
    assert(audioManager.getCurrentTrackName() == "menu");
    assert(audioManager.isLooping());

    // Test 6: Stop
    std::println("Test 6: Stop...");
    audioManager.stop();
    assert(audioManager.isStopped());
    assert(audioManager.getCurrentTrackName().empty());

    // Test 7: Muting
    std::println("Test 7: Muting...");
    assert(!audioManager.isMuted());
    audioManager.setMuted(true);
    assert(audioManager.isMuted());
    audioManager.setMuted(false);
    assert(!audioManager.isMuted());

    // Test 8: Sound effect playback
    std::println("Test 8: Sound effect playback...");
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(continuePath)) {
        audioManager.playSound(buffer, 80.0f);
        assert(audioManager.getActiveSoundCount() == 1);
        audioManager.stopAllSounds();
        assert(audioManager.getActiveSoundCount() == 0);
    }

    // Test 9: Unload and Clear
    std::println("Test 9: Unload and Clear...");
    audioManager.unloadTrack("intro");
    assert(!audioManager.hasTrack("intro"));
    assert(audioManager.hasTrack("menu"));
    assert(audioManager.getTrackCount() == 1);

    audioManager.clearTracks();
    assert(audioManager.getTrackCount() == 0);

    // Test 10: MenuState audio integration
    std::println("Test 10: MenuState audio integration...");
    core::ResourceManager resourceManager;
    core::AudioManager menuAudioManager;
    core::SharedContext context;
    context.m_resourceManager = &resourceManager;
    context.m_audioManager = &menuAudioManager;
    core::StateManager stateManager(&context);
    stateManager.switchTo(core::StateType::Menu);

    assert(menuAudioManager.hasTrack("menu_screen"));
    assert(menuAudioManager.getCurrentTrackName() == "menu_screen");
    assert(menuAudioManager.isPlaying());
    assert(menuAudioManager.isLooping());

    std::println("All AudioManager tests PASSED successfully!");
    return 0;
}
