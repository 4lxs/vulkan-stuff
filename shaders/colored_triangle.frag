#version 450

layout(binding = 0) uniform sampler2D texSampler;

//shader input
layout (location = 0) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;

//output write
layout (location = 0) out vec4 outColor;

void main() 
{
	// outColor = vec4(inTexCoord, 0., 1.);
	// outColor = inColor;
	outColor = texture(texSampler, inTexCoord);
}
