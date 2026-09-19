module;
#include <SFML/Graphics/Sprite.hpp>

export module core:MenuState;
import std;
import :State;
export namespace core {
class MenuState final : public State {
public:
  MenuState(StateManager *l_stateManager, SharedContext *l_context);

  ~MenuState() = default;

  void onCreate() override;

  void onActivate() override;

  void onDeactivate() override;

  void onDestroy() override;

  void update(const float l_dt) override;

  void draw() override;

private:
  std::optional<sf::Sprite> m_menuBackgroundSprite;
};
} // namespace core
