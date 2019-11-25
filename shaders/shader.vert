#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uData_t
{
    vec3 colour;
    mat4 transformMatrix;
} uData;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec3 fragColour;


void main() {
    vec4 transformedPosition =  vec4(inPosition, 1.0, 1.0) * uData.transformMatrix;

    gl_Position = transformedPosition;

    fragColour = uData.colour;
}