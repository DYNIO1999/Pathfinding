#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

layout( push_constant ) uniform push
{
	mat4 projection;
} PushConstants;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * PushConstants.projection *  vec4(inPosition, 1.0);
    fragColor = inColor;
}