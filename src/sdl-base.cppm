export module sdl:base;

export namespace sdl
{
	// If we are building in DEBUG mode, use this variable to enable extra messages
	// from SDL
	constexpr auto IS_DEBUG =
#ifdef DEBUG
		true;
#else
		false;
#endif

	// SDL requires call init and quit functions.
	// This class does that, it only needs to init in class.
	// Object should be created before any other SDL type.
	class sdl_base
	{
	public:
		sdl_base(sdl_base &&)            = default;
		sdl_base &operator=(sdl_base &&) = default;
		sdl_base(sdl_base &)             = delete;
		sdl_base &operator=(sdl_base &)  = delete;

		sdl_base()
		{
			auto result = SDL_Init(SDL_INIT_VIDEO);
			assert(result and "SDL could not be initialized.");
		}

		~sdl_base()
		{
			SDL_Quit();
		}
	};

	// Deleter template, for use with SDL objects.
	// Allows use of SDL Objects with C++'s smart pointers, using SDL's destroy function
	template <auto fn>
	struct deleter
	{
		constexpr void operator()(auto *arg)
		{
			fn(arg);
		}
	};
} // namespace sdl