export module sdl:gpu;

import std;
import io;
import :base;

namespace rg = std::ranges;
namespace vw = std::ranges::views;

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

	// Deleter for all gpu objects in SDL
	template <auto fn>
	struct gpu_deleter
	{
		SDL_GPUDevice *gpu = nullptr;
		constexpr void operator()(auto *arg)
		{
			fn(gpu, arg);
		}
	};
	// Define SDL GPU types with std::unique_ptr and custom deleter
	using free_gfx_pipeline  = gpu_deleter<SDL_ReleaseGPUGraphicsPipeline>;
	using gfx_pipeline_ptr   = std::unique_ptr<SDL_GPUGraphicsPipeline, free_gfx_pipeline>;
	using free_comp_pipeline = gpu_deleter<SDL_ReleaseGPUComputePipeline>;
	using comp_pipeline_ptr  = std::unique_ptr<SDL_GPUComputePipeline, free_comp_pipeline>;
	using free_gfx_shader    = gpu_deleter<SDL_ReleaseGPUShader>;
	using gfx_shader_ptr     = std::unique_ptr<SDL_GPUShader, free_gfx_shader>;
	using free_gpu_buffer    = gpu_deleter<SDL_ReleaseGPUBuffer>;
	using gpu_buffer_ptr     = std::unique_ptr<SDL_GPUBuffer, free_gpu_buffer>;
	using free_gpu_texture   = gpu_deleter<SDL_ReleaseGPUTexture>;
	using gpu_texture_ptr    = std::unique_ptr<SDL_GPUTexture, free_gpu_texture>;
	using free_gpu_sampler   = gpu_deleter<SDL_ReleaseGPUSampler>;
	using gfx_sampler_ptr    = std::unique_ptr<SDL_GPUSampler, free_gpu_sampler>;

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

	auto next_swapchain_image(SDL_Window *wnd, SDL_GPUCommandBuffer *cbo) -> SDL_GPUTexture *
	{
		auto texture = (SDL_GPUTexture *)nullptr;

		auto result = SDL_WaitAndAcquireGPUSwapchainTexture(cbo, wnd, &texture, nullptr, nullptr);
		assert(result == true and "Wait and acquire GPU swapchain texture failed.");
		assert(texture != nullptr and "Swapchain texture is null.");

		return texture;
	}

	enum class shader_stage : uint8_t
	{
		invalid,
		vertex,
		fragment,
		compute,
	};

	enum class raster_type : uint8_t
	{
		none_fill,
		none_wire,
		front_ccw_fill,
		front_ccw_wire,
		back_ccw_fill,
		back_ccw_wire,
		front_cw_fill,
		front_cw_wire,
		back_cw_fill,
		back_cw_wire,
	};

	enum class blend_type : uint8_t
	{
		none,
		opaque,
		alpha,
		additive,
		non_premultiplied,
	};

	auto get_gpu_supported_sampled_count(SDL_GPUDevice *gpu, SDL_GPUTextureFormat format) -> SDL_GPUSampleCount
	{
		constexpr auto sample_counts = std::array{
			SDL_GPU_SAMPLECOUNT_8,
			SDL_GPU_SAMPLECOUNT_4,
			SDL_GPU_SAMPLECOUNT_2,
			SDL_GPU_SAMPLECOUNT_1,
		};

		auto check_sample_count = [&](auto count) {
			return SDL_GPUTextureSupportsSampleCount(gpu, format, count);
		};

		auto rng = sample_counts | vw::take_while(check_sample_count);
		assert(not rng.empty() and "None of the sample counts are supported");
		return rng.front();
	}

	auto get_gpu_supported_depth_stencil_format(SDL_GPUDevice *gpu) -> SDL_GPUTextureFormat
	{
		// Order Matters, biggest to smallest
		constexpr auto depth_formats = std::array{
			SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
			SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
			SDL_GPU_TEXTUREFORMAT_D24_UNORM,
			SDL_GPU_TEXTUREFORMAT_D16_UNORM,
		};

		auto check_depth_format = [&](const auto fmt) {
			return SDL_GPUTextureSupportsFormat(gpu, fmt, SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET);
		};

		auto rng = depth_formats | vw::take_while(check_depth_format);
		assert(not rng.empty() and "None of the depth formats are supported.");
		return rng.front();
	}

	struct shader_builder
	{
		io::byte_span shader_binary;
		shader_stage stage             = shader_stage::invalid;
		uint32_t sampler_count         = 0;
		uint32_t uniform_buffer_count  = 0;
		uint32_t storage_uniform_count = 0;
		uint32_t storage_texture_count = 0;

		auto build(SDL_GPUDevice *gpu) const -> gfx_shader_ptr;
	};

	enum class topology_type : uint8_t
	{
		triangle_list  = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		triangle_strip = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
		line_list      = SDL_GPU_PRIMITIVETYPE_LINELIST,
		line_strip     = SDL_GPU_PRIMITIVETYPE_LINESTRIP,
		point_list     = SDL_GPU_PRIMITIVETYPE_POINTLIST,
	};

	struct gfx_pipeline_builder
	{
		gfx_shader_ptr vertex_shader   = nullptr;
		gfx_shader_ptr fragment_shader = nullptr;

		std::span<const SDL_GPUVertexAttribute> vertex_attributes;
		std::span<const SDL_GPUVertexBufferDescription> vertex_buffer_descriptions;

		SDL_GPUTextureFormat color_format = {};

		bool enable_depth_stencil = false;

		raster_type raster     = {};
		blend_type blend       = {};
		topology_type topology = {};

		auto build(SDL_GPUDevice *gpu) -> gfx_pipeline_ptr;
	};
}

namespace sdl
{
	using namespace gpu;

	auto to_sdl(shader_stage stage) -> SDL_GPUShaderStage
	{
		using enum shader_stage;

		switch (stage)
		{
		case vertex:
			return SDL_GPU_SHADERSTAGE_VERTEX;
		case fragment:
			return SDL_GPU_SHADERSTAGE_FRAGMENT;
		default:
			break;
		}
		assert(false and "Unhandled shader stage.");
		return {};
	}

	auto to_sdl(raster_type type) -> SDL_GPURasterizerState
	{
		using enum raster_type;
		switch (type)
		{
		case none_fill:
			return {
				.fill_mode = SDL_GPU_FILLMODE_FILL,
				.cull_mode = SDL_GPU_CULLMODE_NONE,
			};
		case none_wire:
			return {
				.fill_mode = SDL_GPU_FILLMODE_LINE,
				.cull_mode = SDL_GPU_CULLMODE_NONE,
			};
		case front_ccw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case front_ccw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case back_ccw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case back_ccw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case front_cw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		case front_cw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		case back_cw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		case back_cw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		}
		assert(false and "Unhandled raster type");
		return {};
	}

	auto to_sdl(blend_type type) -> SDL_GPUColorTargetBlendState
	{
		// TODO: verify and fix blend ops for different types.

		auto src    = SDL_GPUBlendFactor{ SDL_GPU_BLENDFACTOR_ONE };
		auto dst    = SDL_GPUBlendFactor{ SDL_GPU_BLENDFACTOR_ONE };
		auto op     = SDL_GPUBlendOp{ SDL_GPU_BLENDOP_ADD };
		auto enable = false;

		using enum blend_type;
		switch (type)
		{
		case opaque:
			src = SDL_GPU_BLENDFACTOR_ONE;
			dst = SDL_GPU_BLENDFACTOR_ZERO;
			break;
		case alpha:
			src = SDL_GPU_BLENDFACTOR_ONE;
			dst = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case additive:
			src = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst = SDL_GPU_BLENDFACTOR_ONE;
			break;
		case non_premultiplied:
			src = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		default:
			break;
		}

		enable = ((src != SDL_GPU_BLENDFACTOR_ONE) or (dst != SDL_GPU_BLENDFACTOR_ONE));

		return {
			.src_color_blendfactor = src,
			.dst_color_blendfactor = dst,
			.color_blend_op        = op,
			.src_alpha_blendfactor = src,
			.dst_alpha_blendfactor = dst,
			.alpha_blend_op        = op,
			.enable_blend          = enable,
		};
	}
}

auto sdl::gpu::shader_builder::build(SDL_GPUDevice *gpu) const -> sdl::gpu::gfx_shader_ptr
{
	assert(shader_binary.size() != 0 and "Shader Binary is empty");

	auto shader_info = SDL_GPUShaderCreateInfo{
		.code_size            = shader_binary.size(),
		.code                 = std::bit_cast<uint8_t *>(shader_binary.data()), // reinterpret_cast<const uint8_t *>(shader_binary.data()),
		.entrypoint           = "main",
		.format               = SHADER_FORMAT(),
		.stage                = to_sdl(stage),
		.num_samplers         = sampler_count,
		.num_storage_textures = storage_texture_count,
		.num_storage_buffers  = storage_uniform_count,
		.num_uniform_buffers  = uniform_buffer_count,
	};

	auto shader = SDL_CreateGPUShader(gpu, &shader_info);
	assert(shader != nullptr and "Failed to create shader.");

	return { shader, { gpu } };
}

auto sdl::gpu::gfx_pipeline_builder::build(SDL_GPUDevice *gpu) -> sdl::gpu::gfx_pipeline_ptr
{

	auto vertex_input_state = SDL_GPUVertexInputState{
		.vertex_buffer_descriptions = vertex_buffer_descriptions.data(),
		.num_vertex_buffers         = static_cast<uint32_t>(vertex_buffer_descriptions.size()),
		.vertex_attributes          = vertex_attributes.data(),
		.num_vertex_attributes      = static_cast<uint32_t>(vertex_attributes.size()),
	};

	auto depth_stencil_state  = SDL_GPUDepthStencilState{};
	auto depth_stencil_format = SDL_GPUTextureFormat{};

	// Surpringly AMD doesn't support D24 on Vulkan, it does on DirectX??
	if (enable_depth_stencil)
	{
		depth_stencil_state = SDL_GPUDepthStencilState{
			.compare_op          = SDL_GPU_COMPAREOP_LESS,
			.write_mask          = std::numeric_limits<uint8_t>::max(),
			.enable_depth_test   = true,
			.enable_depth_write  = true,
			.enable_stencil_test = false, // TODO: figure out how to enable stencil under current api
		};

		depth_stencil_format = get_gpu_supported_depth_stencil_format(gpu);
	}

	auto color_targets = std::array{
		SDL_GPUColorTargetDescription{
			.format      = color_format,
			.blend_state = to_sdl(blend),
		},
	};

	auto target_info = SDL_GPUGraphicsPipelineTargetInfo{
		.color_target_descriptions = color_targets.data(),
		.num_color_targets         = static_cast<uint32_t>(color_targets.size()),
		.depth_stencil_format      = depth_stencil_format,
		.has_depth_stencil_target  = enable_depth_stencil,
	};

	auto pipeline_info = SDL_GPUGraphicsPipelineCreateInfo{
		.vertex_shader       = vertex_shader.get(),
		.fragment_shader     = fragment_shader.get(),
		.vertex_input_state  = vertex_input_state,
		.primitive_type      = to_sdl<SDL_GPUPrimitiveType>(topology),
		.rasterizer_state    = to_sdl(raster),
		.depth_stencil_state = depth_stencil_state,
		.target_info         = target_info,
	};

	auto pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &pipeline_info);
	assert(pipeline != nullptr and "Failed to create graphics pipeline.");

	return { pipeline, { gpu } };
}