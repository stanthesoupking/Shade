include_directories("include")

file(GLOB Project_SRC
    "src/*.cpp"
    "src/*/*.cpp"
    "src/*/*/*.cpp"
)

file(GLOB Project_INC
    "include/*.hpp"
    "include/*/*.hpp"
    "include/*/*/*.hpp"
)

add_executable(SpinningTexture ${Project_SRC} ${Project_INC})
target_link_libraries(SpinningTexture Shade glfw glm Vulkan::Vulkan)
file(COPY "assets" DESTINATION "./")