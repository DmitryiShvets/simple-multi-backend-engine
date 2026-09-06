#version 460 core
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(binding = 3) uniform sampler2D albedoTex;

void main() {
    outColor = texture(albedoTex, fragTexCoord) * vec4(fragColor, 1.0);
}
