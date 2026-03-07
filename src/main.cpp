import std;

import sdl;

void test_gpu(SDL_Window *wnd, SDL_GPUDevice *gpu)
{
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

		auto cb     = SDL_AcquireGPUCommandBuffer(gpu);
		auto sc_img = sdl::gpu::next_swapchain_image(wnd, cb);

		auto color_target = SDL_GPUColorTargetInfo{
			.texture     = sc_img,
			.clear_color = { 0.2f, 0.2f, 0.4f, 1.0f },
			.load_op     = SDL_GPU_LOADOP_CLEAR,
			.store_op    = SDL_GPU_STOREOP_STORE,
		};

		auto render_pass = SDL_BeginGPURenderPass(cb, &color_target, 1, nullptr);
		SDL_EndGPURenderPass(render_pass);

		SDL_SubmitGPUCommandBuffer(cb);
	}
}

using namespace std::literals;

auto main() -> int
{
	using namespace sdl::window;

	std::println("Hello World!");

	constexpr auto WIDTH  = 1000;
	constexpr auto HEIGHT = WIDTH * 10 / 16;
	constexpr auto TITLE  = "SDL Window"sv;

	auto sdl_o = sdl::sdl_base();
	auto pWnd  = make_window({
		 .width  = WIDTH,
		 .height = HEIGHT,
		 .title  = TITLE,
    });

	auto gpu = sdl::gpu::make_gpu(pWnd.get());

	std::println("GPU created");

	{
		set_mouse_mode(pWnd.get(), mouse_mode::relative);
		test_gpu(pWnd.get(), gpu.get());
	}

	return 0;
}