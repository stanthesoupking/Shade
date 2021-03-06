#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformMVP
{
    mat4 mvp;
    mat4 mv;
    mat4 m;
} uniformMVP;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;

void main() {
    vec4 transformedPosition =  uniformMVP.mvp * vec4(inPosition, 1.0);

    gl_Position = transformedPosition;

    fragTexCoord = inTexCoord;
    fragNormal = vec3(uniformMVP.m * vec4(inNormal, 1.0));
}