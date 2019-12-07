#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform UniformLighting
{
    vec3 lightDirection;
    vec3 lightColour;
} uniformLighting;

layout(location = 0) out vec4 outColour;

void main() {
    vec4 textureColour = texture(texSampler, fragTexCoord);
    outColour = textureColour * vec4(uniformLighting.lightColour, 1.0);
}