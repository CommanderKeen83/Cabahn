module;

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

export module core:ResourceManager;

import std;

export namespace core {

    template<typename Resource>
    class ResourceCache {
    public:
        struct ResourceEntry {
            std::unique_ptr<Resource> resource;
            std::size_t refCount{0};
            std::filesystem::path path;
        };

        ResourceCache() = default;
        ~ResourceCache() = default;

        ResourceCache(const ResourceCache&) = delete;
        ResourceCache& operator=(const ResourceCache&) = delete;
        ResourceCache(ResourceCache&&) noexcept = default;
        ResourceCache& operator=(ResourceCache&&) noexcept = default;

        bool load(const std::string& l_id, const std::filesystem::path& l_path) {
            auto it = m_resources.find(l_id);
            if (it != m_resources.end()) {
                ++it->second.refCount;
                return true;
            }

            auto res = std::make_unique<Resource>();
            bool success = false;
            if constexpr (requires { res->loadFromFile(l_path); }) {
                success = res->loadFromFile(l_path);
            } else if constexpr (requires { res->openFromFile(l_path); }) {
                success = res->openFromFile(l_path);
            }

            if (!success) {
                return false;
            }

            m_resources[l_id] = ResourceEntry{
                .resource = std::move(res),
                .refCount = 1,
                .path = l_path
            };
            return true;
        }

        [[nodiscard]] Resource& get(const std::string& l_id) {
            auto it = m_resources.find(l_id);
            if (it == m_resources.end()) {
                throw std::runtime_error("ResourceManager: Resource '" + l_id + "' not found.");
            }
            return *it->second.resource;
        }

        [[nodiscard]] const Resource& get(const std::string& l_id) const {
            auto it = m_resources.find(l_id);
            if (it == m_resources.end()) {
                throw std::runtime_error("ResourceManager: Resource '" + l_id + "' not found.");
            }
            return *it->second.resource;
        }

        bool release(const std::string& l_id) {
            auto it = m_resources.find(l_id);
            if (it == m_resources.end()) {
                return false;
            }

            if (it->second.refCount > 0) {
                --it->second.refCount;
            }

            if (it->second.refCount == 0) {
                m_resources.erase(it);
            }
            return true;
        }

        bool forceRelease(const std::string& l_id) {
            return m_resources.erase(l_id) > 0;
        }

        [[nodiscard]] bool has(const std::string& l_id) const {
            return m_resources.contains(l_id);
        }

        [[nodiscard]] std::size_t getRefCount(const std::string& l_id) const {
            auto it = m_resources.find(l_id);
            return (it != m_resources.end()) ? it->second.refCount : 0;
        }

        [[nodiscard]] std::size_t size() const {
            return m_resources.size();
        }

        [[nodiscard]] bool empty() const {
            return m_resources.empty();
        }

        void clear() {
            m_resources.clear();
        }

    private:
        std::unordered_map<std::string, ResourceEntry> m_resources;
    };

    using TextureCache = ResourceCache<sf::Texture>;
    using SoundCache = ResourceCache<sf::SoundBuffer>;
    using ImageCache = ResourceCache<sf::Image>;
    using FontCache = ResourceCache<sf::Font>;

    class ResourceManager {
    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) noexcept = default;
        ResourceManager& operator=(ResourceManager&&) noexcept = default;

        template<typename T>
        bool load(const std::string& l_id, const std::filesystem::path& l_path) {
            return getCache<T>().load(l_id, l_path);
        }

        template<typename T>
        [[nodiscard]] T& get(const std::string& l_id) {
            return getCache<T>().get(l_id);
        }

        template<typename T>
        [[nodiscard]] const T& get(const std::string& l_id) const {
            return getCache<T>().get(l_id);
        }

        template<typename T>
        bool release(const std::string& l_id) {
            return getCache<T>().release(l_id);
        }

        template<typename T>
        bool forceRelease(const std::string& l_id) {
            return getCache<T>().forceRelease(l_id);
        }

        template<typename T>
        [[nodiscard]] bool has(const std::string& l_id) const {
            return getCache<T>().has(l_id);
        }

        template<typename T>
        [[nodiscard]] std::size_t getRefCount(const std::string& l_id) const {
            return getCache<T>().getRefCount(l_id);
        }

        // Overload to automatically deduce resource type from file extension
        bool load(const std::string& l_id, const std::filesystem::path& l_path) {
            std::string ext = l_path.extension().string();
            std::ranges::transform(ext, ext.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                ext == ".bmp" || ext == ".tga" || ext == ".gif" ||
                ext == ".pnm" || ext == ".psd" || ext == ".hdr") {
                return load<sf::Texture>(l_id, l_path);
            }

            if (ext == ".wav" || ext == ".ogg" || ext == ".flac" ||
                ext == ".mp3" || ext == ".aiff") {
                return load<sf::SoundBuffer>(l_id, l_path);
            }

            if (ext == ".ttf" || ext == ".otf" || ext == ".woff" || ext == ".woff2") {
                return load<sf::Font>(l_id, l_path);
            }

            return false;
        }

        // Overload to release by ID across all resource pools
        bool release(const std::string& l_id) {
            if (m_textures.has(l_id)) return m_textures.release(l_id);
            if (m_sounds.has(l_id)) return m_sounds.release(l_id);
            if (m_images.has(l_id)) return m_images.release(l_id);
            if (m_fonts.has(l_id)) return m_fonts.release(l_id);
            return false;
        }

        // Texture-specific helpers
        bool loadTexture(const std::string& l_id, const std::filesystem::path& l_path) {
            return load<sf::Texture>(l_id, l_path);
        }
        [[nodiscard]] sf::Texture& getTexture(const std::string& l_id) {
            return get<sf::Texture>(l_id);
        }
        [[nodiscard]] const sf::Texture& getTexture(const std::string& l_id) const {
            return get<sf::Texture>(l_id);
        }
        bool releaseTexture(const std::string& l_id) {
            return release<sf::Texture>(l_id);
        }

        // Audio/SoundBuffer-specific helpers
        bool loadAudio(const std::string& l_id, const std::filesystem::path& l_path) {
            return load<sf::SoundBuffer>(l_id, l_path);
        }
        [[nodiscard]] sf::SoundBuffer& getAudio(const std::string& l_id) {
            return get<sf::SoundBuffer>(l_id);
        }
        [[nodiscard]] const sf::SoundBuffer& getAudio(const std::string& l_id) const {
            return get<sf::SoundBuffer>(l_id);
        }
        bool releaseAudio(const std::string& l_id) {
            return release<sf::SoundBuffer>(l_id);
        }

        // Sound aliases
        bool loadSound(const std::string& l_id, const std::filesystem::path& l_path) {
            return loadAudio(l_id, l_path);
        }
        [[nodiscard]] sf::SoundBuffer& getSound(const std::string& l_id) {
            return getAudio(l_id);
        }
        [[nodiscard]] const sf::SoundBuffer& getSound(const std::string& l_id) const {
            return getAudio(l_id);
        }
        bool releaseSound(const std::string& l_id) {
            return releaseAudio(l_id);
        }

        // Image-specific helpers
        bool loadImage(const std::string& l_id, const std::filesystem::path& l_path) {
            return load<sf::Image>(l_id, l_path);
        }
        [[nodiscard]] sf::Image& getImage(const std::string& l_id) {
            return get<sf::Image>(l_id);
        }
        [[nodiscard]] const sf::Image& getImage(const std::string& l_id) const {
            return get<sf::Image>(l_id);
        }
        bool releaseImage(const std::string& l_id) {
            return release<sf::Image>(l_id);
        }

        // Font-specific helpers
        bool loadFont(const std::string& l_id, const std::filesystem::path& l_path) {
            return load<sf::Font>(l_id, l_path);
        }
        [[nodiscard]] sf::Font& getFont(const std::string& l_id) {
            return get<sf::Font>(l_id);
        }
        [[nodiscard]] const sf::Font& getFont(const std::string& l_id) const {
            return get<sf::Font>(l_id);
        }
        bool releaseFont(const std::string& l_id) {
            return release<sf::Font>(l_id);
        }

        void clear() {
            m_textures.clear();
            m_sounds.clear();
            m_images.clear();
            m_fonts.clear();
        }

        [[nodiscard]] TextureCache& getTextures() { return m_textures; }
        [[nodiscard]] const TextureCache& getTextures() const { return m_textures; }

        [[nodiscard]] SoundCache& getSounds() { return m_sounds; }
        [[nodiscard]] const SoundCache& getSounds() const { return m_sounds; }

        [[nodiscard]] ImageCache& getImages() { return m_images; }
        [[nodiscard]] const ImageCache& getImages() const { return m_images; }

        [[nodiscard]] FontCache& getFonts() { return m_fonts; }
        [[nodiscard]] const FontCache& getFonts() const { return m_fonts; }

    private:
        template<typename T>
        auto& getCache() {
            if constexpr (std::is_same_v<T, sf::Texture>) {
                return m_textures;
            } else if constexpr (std::is_same_v<T, sf::SoundBuffer>) {
                return m_sounds;
            } else if constexpr (std::is_same_v<T, sf::Image>) {
                return m_images;
            } else if constexpr (std::is_same_v<T, sf::Font>) {
                return m_fonts;
            } else {
                static_assert(!sizeof(T*), "Unsupported resource type in ResourceManager");
            }
        }

        template<typename T>
        const auto& getCache() const {
            if constexpr (std::is_same_v<T, sf::Texture>) {
                return m_textures;
            } else if constexpr (std::is_same_v<T, sf::SoundBuffer>) {
                return m_sounds;
            } else if constexpr (std::is_same_v<T, sf::Image>) {
                return m_images;
            } else if constexpr (std::is_same_v<T, sf::Font>) {
                return m_fonts;
            } else {
                static_assert(!sizeof(T*), "Unsupported resource type in ResourceManager");
            }
        }

        TextureCache m_textures;
        SoundCache m_sounds;
        ImageCache m_images;
        FontCache m_fonts;
    };

} // namespace core
