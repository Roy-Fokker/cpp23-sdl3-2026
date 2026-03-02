import std;

import sdl;

void test_window(SDL_Window *pWnd)
{
	auto renderer = SDL_CreateRenderer(pWnd, nullptr);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	auto evt  = SDL_Event{};
	auto quit = false;
	while (not quit)
	{
		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_EVENT_QUIT)
			{
				quit = true;
				break;
			}
		}
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
}

using namespace std::literals;

auto main() -> int
{
	std::println("Hello World!");

	constexpr auto WIDTH  = 1000;
	constexpr auto HEIGHT = WIDTH * 10 / 16;
	constexpr auto TITLE  = "SDL Window"sv;

	auto sdl_o = sdl::sdl_base();
	auto pWnd  = sdl::window::make_window({
		 .width  = WIDTH,
		 .height = HEIGHT,
		 .title  = TITLE,
    });

	test_window(pWnd.get());

	return 0;
}