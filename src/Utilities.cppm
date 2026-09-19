export module core:Utilities;

import std;

export namespace core::utils {

    /**
     * @brief Finds the project root directory by searching upwards for the "resources" folder
     *        starting from the current working directory.
     * @return Absolute path to the project root directory.
     */
    [[nodiscard]] inline const std::filesystem::path& getRootPath() {
        static const std::filesystem::path root = []() {
            auto current = std::filesystem::current_path();
            while (true) {
                if (std::filesystem::exists(current / "resources")) {
                    return current;
                }
                if (!current.has_parent_path() || current == current.parent_path()) {
                    break;
                }
                current = current.parent_path();
            }
            return std::filesystem::current_path();
        }();
        return root;
    }

    /**
     * @brief Resolves a path relative to the project root directory.
     * @param subPath Path relative to root (e.g. "resources/graphics/main_menu.png")
     * @return Full path.
     */
    [[nodiscard]] inline std::filesystem::path getPath(const std::filesystem::path& subPath) {
        return getRootPath() / subPath;
    }

    /**
     * @brief Resolves a path relative to the "resources" directory.
     * @param subPath Path relative to resources (e.g. "graphics/main_menu.png")
     * @return Full path to the resource.
     */
    [[nodiscard]] inline std::filesystem::path getResourcePath(const std::filesystem::path& subPath = "") {
        return getRootPath() / "resources" / subPath;
    }

} // namespace core::utils
