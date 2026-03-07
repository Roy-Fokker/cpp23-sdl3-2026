export module sdl:gpu;

import std;
import :base;

export namespace sdl::gpu
{
	// Special deleter for gpu.
	// it will release window on destruction
	struct gpu_window_deleter
	{
		SDL_Window *window;
		constexpr void operator()(auto *gpu)
		{
			SDL_ReleaseWindowFromGPUDevice(gpu, window);
			SDL_DestroyGPUDevice(gpu);
		}
	};
	// Define GPU type with std::unique_ptr and custom deleter
	using gpu_ptr = std::unique_ptr<SDL_GPUDevice, gpu_window_deleter>;

	enum class presentation_mode : uint8_t
	{
		vsync     = SDL_GPU_PRESENTMODE_VSYNC,
		immediate = SDL_GPU_PRESENTMODE_IMMEDIATE,
		mailbox   = SDL_GPU_PRESENTMODE_MAILBOX,
	};

	enum class swapchain_composition : uint8_t
	{
		sdr                 = SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
		sdr_linear          = SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR,
		hdr_extended_linear = SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR,
		hdr10_st2048        = SDL_GPU_SWAPCHAINCOMPOSITION_HDR10_ST2084,
	};

	consteval auto SHADER_FORMAT() -> SDL_GPUShaderFormat
	{
		return
#ifdef SPIRV
			SDL_GPU_SHADERFORMAT_SPIRV;
#elifdef DXIL
			SDL_GPU_SHADERFORMAT_DXIL;
#else
			SDL_GPU_SHADERFORMAT_INVALID;
#endif
	}

	struct description
	{
		SDL_GPUShaderFormat preferred_shader_format = SHADER_FORMAT();
		presentation_mode mode                      = presentation_mode::mailbox;
		swapchain_composition composition           = swapchain_composition::sdr;
	};

	auto make_gpu(SDL_Window *wnd, const description &desc = {}) -> gpu_ptr
	{
		auto gpu = SDL_CreateGPUDevice(desc.preferred_shader_format, IS_DEBUG, nullptr);
		assert(gpu != nullptr and "GPU device could not be created.");

		auto result = SDL_ClaimWindowForGPUDevice(gpu, wnd);
		assert(result == true and "Could not claim window for GPU.");

		auto mode        = to_sdl<SDL_GPUPresentMode>(desc.mode);
		auto composition = to_sdl<SDL_GPUSwapchainComposition>(desc.composition);

		result = SDL_WindowSupportsGPUPresentMode(gpu, wnd, mode);
		assert(result == true and "Window does not support presentation mode.");

		result = SDL_WindowSupportsGPUSwapchainComposition(gpu, wnd, composition);
		assert(result == true and "Window does not support swapchain composition.");

		result = SDL_SetGPUSwapchainParameters(gpu, wnd, composition, mode);
		assert(result == true and "Unable to set GPU swapchain parameters.");

		return { gpu, { wnd } };
	}

	auto next_swapchain_image(SDL_Window *wnd, SDL_GPUCommandBuffer *cb) -> SDL_GPUTexture *
	{
		auto texture = (SDL_GPUTexture *)nullptr;

		auto result = SDL_WaitAndAcquireGPUSwapchainTexture(cb, wnd, &texture, nullptr, nullptr);
		assert(result == true and "Wait and acquire GPU swapchain texture failed.");
		assert(texture != nullptr and "Swapchain texture is null.");

		return texture;
	}
}