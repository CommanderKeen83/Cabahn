export module core:SharedContext;

export namespace core {

class Window;
class EventManager;
class ResourceManager;

struct SharedContext {
  Window *m_window = nullptr;
  EventManager *m_eventmanager = nullptr;
  ResourceManager *m_resourceManager = nullptr;
};

} // namespace core