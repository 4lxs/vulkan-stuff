#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec4 inColor;
layout(location = 4) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;
layout (location = 2) out vec2 outTexCoord;

layout( push_constant ) uniform constants
{
 mat4 mvp;
 vec3 col;
} PushConstants;

void main() 
{
	//output the position of each vertex
	// gl_Position = PushConstants.mvp * vec4(inPosition, 1.0f);
	gl_Position = vec4(inPosition, 1.0f);
	outColor = inColor;
	outTexCoord = inTexCoord;
}
