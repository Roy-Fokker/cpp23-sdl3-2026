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
		std::string title;
		SDL_WindowFlags flags = {};
	};

	auto make_window(const description &desc) -> window_ptr
	{
		auto pWnd = SDL_CreateWindow(desc.title.c_str(),
		                             static_cast<int>(desc.width),
		                             static_cast<int>(desc.height),
		                             desc.flags);
		assert(pWnd != nullptr and "Window could not be created.");

		return window_ptr{ pWnd };
	}
}