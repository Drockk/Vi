#include "vipch.hpp"
#include "Vi/Core/Window.hpp"

#ifdef VI_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.hpp"
#endif

namespace Vi {
	Scope<Window> Window::create(const WindowProperties& props) {
#ifdef VI_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		VI_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}
}
