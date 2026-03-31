# Function takes Slang shader files and adds them as dependency of the program
# as well as compiles the files listed.

# Files with no special names will be treated as containing both Vertex and Fragment/Pixel shaders.
# Files with ".cs." in the filename will be treated as Compute shaders.

# Output file name will be appended with "<shader stage>.cso" where shader stages are
# .vs == vertex
# .fs == fragment/pixel
# .cs == compute

function(target_shader_sources TARGET SHADER_FORMAT)
	# make sure slang package was found before including this file.
	if (NOT TARGET slang::slangc)
		message(FATAL_ERROR "[Error]: Cannot find slang compiler")
	endif()

	# what output format do we want, set appropriate flags for desired format
	if(SHADER_FORMAT MATCHES "SPIRV")
		set(compile_target "spirv")
		set(compile_profile "spirv_1_4")
		set(compile_flags "-emit-spirv-directly" "-fvk-use-entrypoint-name")
	elseif(SHADER_FORMAT MATCHES "DXIL")
		set(compile_target "dxil")
		set(compile_profile "sm_6_4")
		set(compile_flags "")
	else()
		message(FATAL_ERROR "[Error] Unknown Shader Format ${SHADER_FORMAT}")
	endif()

	set(SHADER_FILES ${ARGN})         # shader source files to compile
	list(APPEND gfx_stage "vs" "fs")  # graphics pipeline stages (vs:vertex, fs:fragment/pixel)
	list(APPEND cmp_stage "cs")       # compute stage

	# based on the file name determine what shader stages to compile
	macro(set_stage_type file_name)
		if (${file_name} MATCHES ".cs.")
			set(stages ${cmp_stage})
		else()
			set(stages ${gfx_stage})
		endif()
	endmacro()

	# based on short stage name, expand to full stage name
	macro(set_stage_name short_name)
		if (${short_name} MATCHES "vs")
			set(stage_name "vertex")
		elseif (${short_name} MATCHES "fs")
			set(stage_name "fragment")
		elseif (${short_name} MATCHES "cs")
			set(stage_name "compute")
		endif()
	endmacro()

	# loop through the list of source files
	foreach(SHADER ${SHADER_FILES})
		file(REAL_PATH ${SHADER} SHADER_ABS_PATH)                               # figure out absolute path to file
		cmake_path(GET SHADER_ABS_PATH FILENAME file_name)                      # get only the file name with extension
		cmake_path(GET SHADER_ABS_PATH STEM file_base)                          # get only the file name without extension
		get_filename_component(file_folder ${SHADER_ABS_PATH} DIRECTORY)        # figure what folder the file is in
		get_filename_component(file_folder ${file_folder} NAME)                 # output will be relatively placed in same
		set(shader_output_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file_folder}) # where compiled file will be saved

		set_stage_type(${file_name})

		# for each stage to compile
		foreach(stage ${stages})
			set(output_file ${shader_output_dir}/${file_base}.${stage}.cso)   # absolute path to output file
			
			set_stage_name(${stage})

			# make relative directory, if not exists
			# call slangc with various compile options
			add_custom_command(
				OUTPUT "${output_file}"
				COMMAND ${CMAKE_COMMAND} -E make_directory ${shader_output_dir}
				COMMAND ${CMAKE_COMMAND} -E echo "Started compiling Slang ${stage_name} shader for ${file_name}"
				COMMAND slang::slangc ${SHADER_ABS_PATH} -o ${output_file} -entry ${stage}Main -stage ${stage_name} -target ${compile_target} -profile ${compile_profile} ${compile_flags}
				COMMAND ${CMAKE_COMMAND} -E echo "${stage_name} compilation finished for ${file_name}"
				DEPENDS ${SHADER_ABS_PATH}
				COMMENT "Compiling slang shader ${file_name}"
				VERBATIM
			)

			# collect names of all compiled files
			list(APPEND all_shader_outputs "${output_file}")
		endforeach()
	endforeach()

	if (all_shader_outputs)
		# Create a new Target for CMake/Ninja to process
		add_custom_target(${TARGET}_shaders DEPENDS ${all_shader_outputs})
		# Add it as dependency to primay target. so if compilation failed main target also fails to compile
		add_dependencies(${TARGET} ${TARGET}_shaders)
	endif()
endfunction()