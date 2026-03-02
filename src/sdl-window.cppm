export module sdl:window;

import std;
import :base;

export namespace sdl::window
{
	using window_ptr = std::unique_ptr<SDL_Window, sdl::deleter<SDL_DestroyWindow>>;

	struct description
	{
		uint32_t width;
		uint32_t height;
		std::string_view title;
		SDL_WindowFlags flags = {};
	};

	auto make_window(const description &desc) -> window_ptr
	{
		auto pWnd = SDL_CreateWindow(desc.title.data(),
		                             static_cast<int>(desc.width),
		                             static_cast<int>(desc.height),
		                             desc.flags);
		assert(pWnd != nullptr and "Window could not be created.");

		return window_ptr{ pWnd };
	}

	enum class mouse_mode : uint8_t
	{
		relative,
		absolute,
	};

	auto set_mouse_mode(SDL_Window *wnd, mouse_mode mode) -> bool
	{
		using enum mouse_mode;

		switch (mode)
		{
		case relative:
			return SDL_SetWindowRelativeMouseMode(wnd, true);
			break;
		case absolute:
			return SDL_SetWindowRelativeMouseMode(wnd, false);
			break;
		default:
			assert(false and "Unhandled mouse mode.");
			break;
		}

		return false;
	}
}