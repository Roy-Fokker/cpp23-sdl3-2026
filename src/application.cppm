export module application;

import std;
import io;
import sdl;

using namespace std::string_view_literals;

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
		sdl::sdl_base _sdl; // SDL base object
		sdl::window::window_ptr wnd = sdl::window::make_window({ .width  = WIDTH,
		                                                         .height = HEIGHT,
		                                                         .title  = TITLE }); // SDL Window object
		sdl::gpu::gpu_ptr gpu       = sdl::gpu::make_gpu(wnd.get());                // SDL GPU object
		SDL_Event evt               = {};                                           // SDL Event object

		bool quit = false; // Loop control

		struct scene
		{
			sdl::gpu::gfx_pipeline_ptr pipeline;
		};
		scene scn;
	};
}

using namespace project;

namespace
{
	constexpr auto CLEAR_COLOR = SDL_FColor{ 0.2f, 0.2f, 0.4f, 1.0f };

	auto basic_pipeline(SDL_GPUDevice *gpu, SDL_Window *wnd) -> sdl::gpu::gfx_pipeline_ptr
	{
		return {};
	}
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
	scn.pipeline = basic_pipeline(gpu.get(), wnd.get());
}

void application::process_events()
{
	auto handle_keyboard = [&](const SDL_KeyboardEvent &key_evt) {
		switch (key_evt.scancode)
		{
		case SDL_SCANCODE_ESCAPE:
			quit = true;
			break;
		default:
			break;
		}
	};

	auto handle_mouse_motion = [&]([[maybe_unused]] const SDL_MouseMotionEvent &mouse_evt) {
	};

	auto handle_mouse_wheel = [&]([[maybe_unused]] const SDL_MouseWheelEvent &wheel_evt) {
	};

	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_EVENT_QUIT:
			quit = true;
			break;
		case SDL_EVENT_KEY_DOWN:
			handle_keyboard(evt.key);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			handle_mouse_motion(evt.motion);
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			handle_mouse_wheel(evt.wheel);
			break;
		}
	}
}

void application::update_state()
{
}

void application::draw()
{
	auto cbo    = SDL_AcquireGPUCommandBuffer(gpu.get());
	auto sc_img = sdl::gpu::next_swapchain_image(wnd.get(), cbo);

	auto color_target = SDL_GPUColorTargetInfo{
		.texture     = sc_img,
		.clear_color = CLEAR_COLOR,
		.load_op     = SDL_GPU_LOADOP_CLEAR,
		.store_op    = SDL_GPU_STOREOP_STORE,
	};

	auto render_pass = SDL_BeginGPURenderPass(cbo, &color_target, 1, nullptr);
	SDL_EndGPURenderPass(render_pass);

	SDL_SubmitGPUCommandBuffer(cbo);
}