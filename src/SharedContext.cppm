export module core:SharedContext;

export namespace core {

class Window;
class EventManager;

struct SharedContext {
  Window *m_window = nullptr;
  EventManager *m_eventmanager = nullptr;
};

} // namespace core