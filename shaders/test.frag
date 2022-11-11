#version 450
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 fragColor;

layout( push_constant ) uniform push
{
    vec4 color;
} PushConstants;

void main() {
    outColor = PushConstants.color;
}
