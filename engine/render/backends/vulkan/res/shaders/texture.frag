#version 450
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

//layout(set = 0, binding = 1) uniform sampler2D texSampler;
layout(set = 0, binding = 1) uniform sampler samp;
layout(set = 0, binding = 2) uniform texture2D textures[2];
layout(push_constant) uniform Push {
  mat4 modelMatrix; 
  int index;
} push;

void main() {
    //outColor = outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);;
    //outColor = texture(texSampler, fragTexCoord);
    outColor = texture(sampler2D(textures[push.index], samp), fragTexCoord);
    //outColor = vec4(fragColor,1.0f);
}