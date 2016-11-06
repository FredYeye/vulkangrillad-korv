cmake_minimum_required(VERSION 2.8.3)
set(project_name vulkan)
project(${project_name})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -march=native") #-flto -static -fno-math-errno?

if(WIN32)
	message("bad choice")

	find_path(GLFW_INCLUDE GLFW/glfw3.h PATHS NO_DEFAULT_PATH DOC "GLFW include path")
	find_path(GLFW_LIB glfw3.dll PATHS NO_DEFAULT_PATH DOC "GLFW lib path")
	include_directories(${GLFW_INCLUDE})
	include_directories(${GLFW_LIB})
	find_library(GLFW_LIBRARY "glfw3" HINTS ${GLFW_LIB})

    find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS
        "$ENV{VULKAN_SDK}/Include"
        "$ENV{VK_SDK_PATH}/Include")
	include_directories(${VULKAN_INCLUDE_DIR})
	find_library(VULKAN_LIBRARY NAMES vulkan-1 HINTS
		"$ENV{VULKAN_SDK}/Bin"
		"$ENV{VK_SDK_PATH}/Bin")
	find_library(VULKAN_STATIC_LIBRARY NAMES vkstatic.1 HINTS
		"$ENV{VULKAN_SDK}/Bin"
		"$ENV{VK_SDK_PATH}/Bin")


	add_definitions(-lglfw3)

	set(source_files
		src/main.cpp
		src/vulkan.cpp
		src/file.cpp
		)

	set(header_files
		src/main.hpp
		src/vulkan.hpp
		src/file.hpp
		)

	add_executable(${project_name} ${header_files} ${source_files})
	target_link_libraries(vulkan ${GLFW_LIBRARY} ${VULKAN_LIBRARY}) # ${VULKAN_STATIC_LIBRARY}
endif(WIN32)
