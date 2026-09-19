#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdio>

import core;
import std;

void createDummyWav(const std::filesystem::path& path) {
    // 44-byte standard RIFF WAV header for 8000 Hz, 16-bit mono PCM with 16 samples
    std::ofstream file(path, std::ios::binary);
    const char header[] = {
        'R', 'I', 'F', 'F',
        0x2c, 0x00, 0x00, 0x00, // Chunk size: 44
        'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ',
        0x10, 0x00, 0x00, 0x00, // Subchunk1Size (16 for PCM)
        0x01, 0x00,             // AudioFormat (1 = PCM)
        0x01, 0x00,             // NumChannels (1 = mono)
        0x40, 0x1f, 0x00, 0x00, // SampleRate (8000)
        static_cast<char>(0x80), 0x3e, 0x00, 0x00, // ByteRate (8000 * 1 * 2 = 16000)
        0x02, 0x00,             // BlockAlign (1 * 2 = 2)
        0x10, 0x00,             // BitsPerSample (16)
        'd', 'a', 't', 'a',
        0x08, 0x00, 0x00, 0x00, // Subchunk2Size (8 bytes = 4 samples)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    file.write(header, sizeof(header));
}

int main() {
    std::println("Starting ResourceManager tests...");

    // Setup temporary files
    const std::filesystem::path testImagePath = "test_image.png";
    const std::filesystem::path testSoundPath = "test_sound.wav";

    {
        sf::Image dummyImg({16, 16}, sf::Color::Green);
        if (!dummyImg.saveToFile(testImagePath)) {
            std::println(stderr, "Failed to create dummy image");
            return 1;
        }
    }
    createDummyWav(testSoundPath);

    core::ResourceManager manager;

    // Test 1: Load image and verify reference
    std::println("Test 1: Loading Texture...");
    if (!manager.loadTexture("player", testImagePath)) {
        std::println(stderr, "Failed to load texture");
        return 1;
    }
    if (!manager.has<sf::Texture>("player")) {
        std::println(stderr, "Texture 'player' should exist");
        return 1;
    }
    if (manager.getRefCount<sf::Texture>("player") != 1) {
        std::println(stderr, "Texture refCount should be 1");
        return 1;
    }

    // Verify get returns reference
    sf::Texture& tex = manager.getTexture("player");
    if (tex.getSize().x != 16 || tex.getSize().y != 16) {
        std::println(stderr, "Texture size mismatch");
        return 1;
    }

    // Test 2: Increment refCount on re-load
    std::println("Test 2: RefCount increment on reload...");
    manager.loadTexture("player", testImagePath);
    if (manager.getRefCount<sf::Texture>("player") != 2) {
        std::println(stderr, "Texture refCount should be 2 after second load");
        return 1;
    }

    // Test 3: Release decrements refCount
    std::println("Test 3: Release decrements refCount...");
    if (!manager.releaseTexture("player")) {
        std::println(stderr, "releaseTexture failed");
        return 1;
    }
    if (manager.getRefCount<sf::Texture>("player") != 1) {
        std::println(stderr, "Texture refCount should be 1 after release");
        return 1;
    }
    if (!manager.has<sf::Texture>("player")) {
        std::println(stderr, "Texture should still exist with refCount 1");
        return 1;
    }

    // Test 4: Final release frees the resource
    std::println("Test 4: Final release frees resource...");
    if (!manager.releaseTexture("player")) {
        std::println(stderr, "second releaseTexture failed");
        return 1;
    }
    if (manager.has<sf::Texture>("player")) {
        std::println(stderr, "Texture should no longer exist after final release");
        return 1;
    }
    if (manager.getRefCount<sf::Texture>("player") != 0) {
        std::println(stderr, "Texture refCount should be 0");
        return 1;
    }

    // Verify get throws when not found
    bool threw = false;
    try {
        [[maybe_unused]] auto& missing = manager.getTexture("player");
    } catch (const std::exception& e) {
        threw = true;
    }
    if (!threw) {
        std::println(stderr, "Expected getTexture to throw for unloaded resource");
        return 1;
    }

    // Test 5: Audio loading and release
    std::println("Test 5: Loading Audio/SoundBuffer...");
    if (!manager.loadAudio("beep", testSoundPath)) {
        std::println(stderr, "Failed to load audio");
        return 1;
    }
    if (!manager.has<sf::SoundBuffer>("beep")) {
        std::println(stderr, "Sound 'beep' should exist");
        return 1;
    }
    sf::SoundBuffer& sound = manager.getAudio("beep");
    if (sound.getChannelCount() != 1) {
        std::println(stderr, "Sound channel count mismatch");
        return 1;
    }
    if (!manager.releaseAudio("beep")) {
        std::println(stderr, "Failed to release audio");
        return 1;
    }
    if (manager.has<sf::SoundBuffer>("beep")) {
        std::println(stderr, "Sound 'beep' should be released");
        return 1;
    }

    // Test 6: Auto-deduction by extension
    std::println("Test 6: Auto-deduction load and release...");
    if (!manager.load("auto_img", testImagePath)) {
        std::println(stderr, "Auto load image failed");
        return 1;
    }
    if (!manager.load("auto_snd", testSoundPath)) {
        std::println(stderr, "Auto load sound failed");
        return 1;
    }
    if (!manager.has<sf::Texture>("auto_img") || !manager.has<sf::SoundBuffer>("auto_snd")) {
        std::println(stderr, "Auto loaded resources missing");
        return 1;
    }
    if (!manager.release("auto_img") || !manager.release("auto_snd")) {
        std::println(stderr, "Auto release failed");
        return 1;
    }

    // Test 7: Image loading and release
    std::println("Test 7: Image loading and release...");
    if (!manager.loadImage("raw_img", testImagePath)) {
        std::println(stderr, "Failed to load sf::Image");
        return 1;
    }
    sf::Image& imgRef = manager.getImage("raw_img");
    if (imgRef.getSize().x != 16) {
        std::println(stderr, "Image width mismatch");
        return 1;
    }
    if (!manager.releaseImage("raw_img")) {
        std::println(stderr, "Failed to release sf::Image");
        return 1;
    }

    // Clean up temporary files
    std::filesystem::remove(testImagePath);
    std::filesystem::remove(testSoundPath);

    std::println("All ResourceManager tests PASSED successfully!");
    return 0;
}
