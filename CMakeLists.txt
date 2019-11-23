cmake_minimum_required(VERSION 3.7)

project(ShadeApp)

include(FetchContent)

set (CMAKE_CXX_STANDARD 17)

include_directories(include)

# Get GLFW3
find_package(glfw3 3.3)

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

find_package(vulkan REQUIRED)

file(GLOB ShadeApp_SRC
    "src/*.cpp"
    "src/*/*.cpp"
)

file(GLOB ShadeApp_INC
    "include/*.hpp"
    "include/*/*.hpp"
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

add_executable(ShadeApp ${ShadeApp_SRC} ${ShadeApp_INC})
target_link_libraries(ShadeApp glfw glm Vulkan::Vulkan)
