#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uData_t
{
    mat4 mvp;
} uData;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColour;


layout(location = 0) out vec3 fragColour;


void main() {
    vec4 transformedPosition =  uData.mvp * vec4(inPosition, 0.0, 1.0);

    gl_Position = transformedPosition;

    fragColour = inColour;
}