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

add_executable(UserInput ${Project_SRC} ${Project_INC})
target_link_libraries(UserInput Shade glfw glm Vulkan::Vulkan)
file(COPY "assets" DESTINATION "./")