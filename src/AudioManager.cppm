module;

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundSource.hpp>

export module core:AudioManager;

import std;

export namespace core {

    class AudioManager {
    public:
        using Status = sf::SoundSource::Status;

        AudioManager() = default;
        ~AudioManager() {
            stop();
            stopAllSounds();
        }

        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;
        AudioManager(AudioManager&&) noexcept = default;
        AudioManager& operator=(AudioManager&&) noexcept = default;

        // =========================================================================
        // Track Management
        // =========================================================================

        /**
         * @brief Validates that the audio track file exists and can be opened, then registers it.
         * @param name Unique identifier for the track.
         * @param path Filesystem path to the audio file.
         * @return True if valid and registered successfully.
         */
        bool loadTrack(const std::string& name, const std::filesystem::path& path) {
            std::error_code ec;
            if (!std::filesystem::exists(path, ec) || !std::filesystem::is_regular_file(path, ec)) {
                return false;
            }

            // Verify with a temporary stream probe that the audio format is readable
            sf::Music testStream;
            if (!testStream.openFromFile(path)) {
                return false;
            }

            m_tracks[name] = path;
            return true;
        }

        /**
         * @brief Registers an audio track path without upfront format testing.
         */
        void registerTrack(const std::string& name, const std::filesystem::path& path) {
            m_tracks[name] = path;
        }

        [[nodiscard]] bool hasTrack(const std::string& name) const {
            return m_tracks.contains(name);
        }

        [[nodiscard]] std::optional<std::filesystem::path> getTrackPath(const std::string& name) const {
            auto it = m_tracks.find(name);
            if (it != m_tracks.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        void unloadTrack(const std::string& name) {
            if (m_currentTrackName == name) {
                stop();
            }
            m_tracks.erase(name);
        }

        void clearTracks() {
            stop();
            m_tracks.clear();
        }

        [[nodiscard]] std::size_t getTrackCount() const {
            return m_tracks.size();
        }

        // =========================================================================
        // Playback Controls
        // =========================================================================

        /**
         * @brief Plays a registered track by name.
         * @param name Registered track identifier.
         * @param loop Whether the track should loop.
         * @return True if playback started.
         */
        bool play(const std::string& name, bool loop = true) {
            auto it = m_tracks.find(name);
            if (it == m_tracks.end()) {
                return false;
            }
            return playPath(it->second, name, loop);
        }

        /**
         * @brief Plays an audio file directly from path.
         */
        bool playFile(const std::filesystem::path& path, bool loop = true) {
            return playPath(path, path.filename().string(), loop);
        }

        /**
         * @brief Resumes or restarts current track.
         */
        void play() {
            if (m_currentMusic) {
                m_currentMusic->play();
            }
        }

        void pause() {
            if (m_currentMusic && m_currentMusic->getStatus() == Status::Playing) {
                m_currentMusic->pause();
            }
        }

        void resume() {
            if (m_currentMusic && m_currentMusic->getStatus() == Status::Paused) {
                m_currentMusic->play();
            }
        }

        void stop() {
            cancelTransition();
            if (m_currentMusic) {
                m_currentMusic->stop();
                m_currentMusic.reset();
            }
            m_currentTrackName.clear();
        }

        /**
         * @brief Changes the currently playing track to a new one, with optional smooth crossfade.
         * @param name Identifier of the new track to play.
         * @param loop Whether the new track should loop.
         * @param fadeDuration Transition crossfade duration in seconds. If <= 0, changes immediately.
         * @return True if transition or change was successfully initiated.
         */
        bool changeTrack(const std::string& name, bool loop = true, float fadeDuration = 0.0f) {
            auto it = m_tracks.find(name);
            if (it == m_tracks.end()) {
                return false;
            }

            // If the same track is already playing, simply ensure loop mode matches
            if (m_currentTrackName == name && isPlaying()) {
                setLooping(loop);
                return true;
            }

            // Immediate switch if requested or if nothing is currently playing
            if (fadeDuration <= 0.0f || !isPlaying()) {
                cancelTransition();
                return play(name, loop);
            }

            // Crossfade transition
            auto nextMusic = std::make_unique<sf::Music>();
            if (!nextMusic->openFromFile(it->second)) {
                return false;
            }

            nextMusic->setLooping(loop);
            nextMusic->setPitch(m_pitch);
            nextMusic->setVolume(0.0f);
            nextMusic->play();

            m_transition = Transition{
                .nextMusic = std::move(nextMusic),
                .nextTrackName = name,
                .duration = fadeDuration,
                .elapsed = 0.0f,
                .active = true
            };

            return true;
        }

        // =========================================================================
        // Volume and Audio Properties
        // =========================================================================

        void setMasterVolume(float volume) {
            m_masterVolume = std::clamp(volume, 0.0f, 100.0f);
            applyMusicVolume();
        }

        [[nodiscard]] float getMasterVolume() const {
            return m_masterVolume;
        }

        void setMusicVolume(float volume) {
            m_musicVolume = std::clamp(volume, 0.0f, 100.0f);
            applyMusicVolume();
        }

        [[nodiscard]] float getMusicVolume() const {
            return m_musicVolume;
        }

        void setVolume(float volume) {
            setMusicVolume(volume);
        }

        [[nodiscard]] float getVolume() const {
            return getMusicVolume();
        }

        void setSoundVolume(float volume) {
            m_soundVolume = std::clamp(volume, 0.0f, 100.0f);
        }

        [[nodiscard]] float getSoundVolume() const {
            return m_soundVolume;
        }

        void setMuted(bool muted) {
            m_muted = muted;
            applyMusicVolume();
        }

        [[nodiscard]] bool isMuted() const {
            return m_muted;
        }

        void setLooping(bool loop) {
            m_looping = loop;
            if (m_currentMusic) {
                m_currentMusic->setLooping(loop);
            }
        }

        [[nodiscard]] bool isLooping() const {
            return m_currentMusic ? m_currentMusic->isLooping() : m_looping;
        }

        void setPitch(float pitch) {
            m_pitch = std::max(0.1f, pitch);
            if (m_currentMusic) {
                m_currentMusic->setPitch(m_pitch);
            }
        }

        [[nodiscard]] float getPitch() const {
            return m_pitch;
        }

        // =========================================================================
        // Status and Information
        // =========================================================================

        [[nodiscard]] Status getStatus() const {
            return m_currentMusic ? m_currentMusic->getStatus() : Status::Stopped;
        }

        [[nodiscard]] bool isPlaying() const {
            return getStatus() == Status::Playing;
        }

        [[nodiscard]] bool isPaused() const {
            return getStatus() == Status::Paused;
        }

        [[nodiscard]] bool isStopped() const {
            return getStatus() == Status::Stopped;
        }

        [[nodiscard]] const std::string& getCurrentTrackName() const {
            return m_currentTrackName;
        }

        // =========================================================================
        // Sound Effect (SFX) Playback
        // =========================================================================

        void playSound(const sf::SoundBuffer& buffer, float volume = 100.0f, float pitch = 1.0f) {
            pruneStoppedSounds();

            constexpr std::size_t MaxConcurrentSounds = 32;
            if (m_activeSounds.size() >= MaxConcurrentSounds) {
                // Drop the oldest playing sound to make room
                m_activeSounds.erase(m_activeSounds.begin());
            }

            auto sound = std::make_unique<sf::Sound>(buffer);
            const float effectiveVol = m_muted ? 0.0f : (volume * (m_soundVolume / 100.0f) * (m_masterVolume / 100.0f));
            sound->setVolume(effectiveVol);
            sound->setPitch(pitch);
            sound->play();

            m_activeSounds.push_back(std::move(sound));
        }

        void stopAllSounds() {
            for (auto& sound : m_activeSounds) {
                sound->stop();
            }
            m_activeSounds.clear();
        }

        [[nodiscard]] std::size_t getActiveSoundCount() const {
            return m_activeSounds.size();
        }

        // =========================================================================
        // Update Hook
        // =========================================================================

        /**
         * @brief Updates active volume fades and cleans finished sound effects.
         * @param dt Elapsed frame time in seconds.
         */
        void update(float dt) {
            pruneStoppedSounds();

            if (!m_transition.active) {
                return;
            }

            m_transition.elapsed += dt;
            const float progress = std::clamp(m_transition.elapsed / m_transition.duration, 0.0f, 1.0f);
            const float baseVolume = calculateEffectiveMusicVolume();

            // Fade out current track
            if (m_currentMusic) {
                m_currentMusic->setVolume((1.0f - progress) * baseVolume);
            }

            // Fade in next track
            if (m_transition.nextMusic) {
                m_transition.nextMusic->setVolume(progress * baseVolume);
            }

            // Completed transition
            if (progress >= 1.0f) {
                if (m_currentMusic) {
                    m_currentMusic->stop();
                }
                m_currentMusic = std::move(m_transition.nextMusic);
                m_currentTrackName = std::move(m_transition.nextTrackName);
                if (m_currentMusic) {
                    m_currentMusic->setVolume(baseVolume);
                }
                m_transition.active = false;
            }
        }

    private:
        bool playPath(const std::filesystem::path& path, const std::string& name, bool loop) {
            cancelTransition();

            if (m_currentTrackName == name && isPlaying()) {
                setLooping(loop);
                return true;
            }

            auto music = std::make_unique<sf::Music>();
            if (!music->openFromFile(path)) {
                return false;
            }

            m_looping = loop;
            music->setLooping(loop);
            music->setPitch(m_pitch);
            music->setVolume(calculateEffectiveMusicVolume());
            music->play();

            m_currentMusic = std::move(music);
            m_currentTrackName = name;
            return true;
        }

        [[nodiscard]] float calculateEffectiveMusicVolume() const {
            if (m_muted) {
                return 0.0f;
            }
            return (m_musicVolume * m_masterVolume) / 100.0f;
        }

        void applyMusicVolume() {
            const float effective = calculateEffectiveMusicVolume();
            if (m_currentMusic && !m_transition.active) {
                m_currentMusic->setVolume(effective);
            }
        }

        void cancelTransition() {
            if (m_transition.active) {
                if (m_transition.nextMusic) {
                    m_transition.nextMusic->stop();
                    m_transition.nextMusic.reset();
                }
                m_transition.active = false;
            }
        }

        void pruneStoppedSounds() {
            std::erase_if(m_activeSounds, [](const auto& sound) {
                return sound->getStatus() == Status::Stopped;
            });
        }

    private:
        struct Transition {
            std::unique_ptr<sf::Music> nextMusic{nullptr};
            std::string nextTrackName;
            float duration{0.0f};
            float elapsed{0.0f};
            bool active{false};
        };

        std::unordered_map<std::string, std::filesystem::path> m_tracks;
        std::unique_ptr<sf::Music> m_currentMusic{nullptr};
        std::string m_currentTrackName;

        Transition m_transition;
        std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;

        float m_masterVolume{100.0f};
        float m_musicVolume{100.0f};
        float m_soundVolume{100.0f};
        float m_pitch{1.0f};
        bool m_looping{true};
        bool m_muted{false};
    };

} // namespace core
