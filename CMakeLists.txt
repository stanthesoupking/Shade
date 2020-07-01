cmake_minimum_required(VERSION 3.7)

project(ShadeApp)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)

# Default to release build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# If release build, enable full compiler optimisation
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(include)

# Check if library was found
if(NOT glfw3_FOUND)
	message("Failed to find local copy of GLFW, downloading copy of Github...")
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

	FetchContent_Declare(
		glfw3
		GIT_REPOSITORY https://github.com/glfw/glfw
		GIT_TAG 3.3
	)

	FetchContent_MakeAvailable(glfw3)
endif()

# Get GLM
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm
)

set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

find_package(Vulkan REQUIRED)

file(GLOB Shade_SRC
    "src/*.cpp"
    "src/*/*.cpp"
	"src/*/*/*.cpp"
)

file(GLOB Shade_INC
    "include/*.hpp"
    "include/*/*.hpp"
	"include/*/*/*.hpp"
)

## Note: May use in future for GLSL integration
#FetchContent_Declare(
#    SPIRV_Headers
#    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers
#)
#
#FetchContent_Declare(
#    SPIRV_Tools
#    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools
#)
#
#FetchContent_Declare(
#    glslang
#    GIT_REPOSITORY https://github.com/KhronosGroup/glslang
#)
#
#FetchContent_Declare(
#    shaderc
#    GIT_REPOSITORY https://github.com/google/shaderc
#)
#
#FetchContent_MakeAvailable(SPIRV_Headers)
#FetchContent_MakeAvailable(SPIRV_Tools)
#FetchContent_MakeAvailable(glslang)
#
#set(SHADERC_SKIP_TESTS on)
#FetchContent_MakeAvailable(shaderc)
#FetchContent_MakeAvailable(shaderc)

add_library(Shade ${Shade_SRC} ${Shade_INC})
target_link_libraries(Shade glfw glm Vulkan::Vulkan)
target_include_directories(Shade INTERFACE include)

# Example Programs:
add_subdirectory("examples" "${CMAKE_BINARY_DIR}/examples")