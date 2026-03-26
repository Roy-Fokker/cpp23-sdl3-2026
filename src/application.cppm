export module application;

import std;
import sdl;

using namespace std::string_view_literals;
namespace s_win = sdl::window;
namespace s_gpu = sdl::gpu;

namespace
{
	constexpr auto ASPECT_RATIO = 16.f / 9.f;                                  // Window Aspect Ratio
	constexpr auto WIDTH        = 1000u;                                       // Width of the window
	constexpr auto HEIGHT       = static_cast<uint32_t>(WIDTH / ASPECT_RATIO); // Height of the Window based on width and aspect ratio
	constexpr auto TITLE        = "SDL3 GPU C++23 Project Template"sv;         // Window title
}

export namespace project
{
	class application
	{
	public:
		auto run() -> int;

	private:
		void prepare_scene();
		void process_events();
		void update_state();
		void draw();

		// Private Members
		sdl::sdl_base _sdl;                                                    // SDL base object
		s_win::window_ptr wnd = s_win::make_window({ WIDTH, HEIGHT, TITLE });  // SDL Window object
		s_gpu::gpu_ptr gpu    = s_gpu::make_gpu(wnd.get());                    // SDL GPU object
		SDL_Event evt         = {};                                            // SDL Event object

		bool quit = false; // Loop control
	};
}

using namespace project;

namespace 
{
	constexpr auto CLEAR_COLOR = SDL_FColor{ 0.2f, 0.2f, 0.4f, 1.0f };
}

auto application::run() -> int
{
	prepare_scene();

	while (not quit)
	{
		process_events();
		update_state();
		draw();
	}

	return 0;
}

void application::prepare_scene()
{

}

void application::process_events()
{
	while (SDL_PollEvent(&evt))
	{
		if (evt.type == SDL_EVENT_QUIT)
		{
			quit = true;
			break;
		}

		if (evt.key.scancode == SDL_SCANCODE_ESCAPE)
		{
			quit = true;
		}
	}
}

void application::update_state()
{

}

void application::draw()
{
	auto cb     = SDL_AcquireGPUCommandBuffer(gpu.get());
	auto sc_img = sdl::gpu::next_swapchain_image(wnd.get(), cb);

	auto color_target = SDL_GPUColorTargetInfo{
		.texture     = sc_img,
		.clear_color = CLEAR_COLOR,
		.load_op     = SDL_GPU_LOADOP_CLEAR,
		.store_op    = SDL_GPU_STOREOP_STORE,
	};

	auto render_pass = SDL_BeginGPURenderPass(cb, &color_target, 1, nullptr);
	SDL_EndGPURenderPass(render_pass);

	SDL_SubmitGPUCommandBuffer(cb);
}