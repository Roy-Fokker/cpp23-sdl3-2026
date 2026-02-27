# name of this application/library
set(PRJ_APP_NAME "sdl3-cpp-app")

# create executable for initial setup
add_executable(${PRJ_APP_NAME})

# ensure it's Standard C++ 23
target_compile_features(${PRJ_APP_NAME} PRIVATE cxx_std_23)

# set preprocessor defines
target_compile_definitions(${PRJ_APP_NAME}
	PRIVATE
		$<$<CONFIG:Debug>:DEBUG>    # Ensure DEBUG macro is defined in debug build
)

# source files for this application
target_sources(${PRJ_APP_NAME}
	# Non C++ module source files
	PRIVATE
		main.cpp

	# magic encantations for C++ Modules Support in CMake
	# PRIVATE FILE_SET app_modules TYPE CXX_MODULES FILES
)