#version 450

layout(location=0) out vec4 outColor;
layout(location=2) in vec4 fragColor;

vec4 warm_tone = vec4(0.6, 0.2,  0.2, 1.0);
vec4 cold_tone = vec4(0.1, 0.2, 0.6, 1.0);

void main() {
    outColor = fragColor;
}
