#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

layout( push_constant ) uniform push
{
	mat4 projection;
} PushConstants;

void main() {
    gl_Position = PushConstants.projection *  vec4(inPosition, 1.0);
    fragColor = inColor;
}
