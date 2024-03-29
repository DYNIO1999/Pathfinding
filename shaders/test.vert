#version 450
#extension GL_EXT_debug_printf : enable


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 fragColor;

layout(set = 0 ,binding = 0) uniform CameraUniformBuffer {
    mat4 view;
    mat4 proj;
} cameraUbo;

layout(set = 1 ,binding = 0) uniform ModelUniformBuffer {
    mat4 model;
} modelUbo;

void main() {
    gl_Position = cameraUbo.proj * cameraUbo.view * modelUbo.model *vec4(inPosition, 1.0);
    fragColor = inColor;
}