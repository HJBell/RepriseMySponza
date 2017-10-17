#version 330

#define MAX_INSTANCE_COUNT 64

struct InstanceData
{
	mat4 mvpXform;
	mat4 modelXform;
	vec3 diffuse;
	float shininess;
	vec3 specular;
	int isShiny;
};

layout(std140) uniform cpp_PerModelUniforms
{
	InstanceData cpp_Instances[MAX_INSTANCE_COUNT];
};

in vec3 cpp_VertexPosition;
in vec3 cpp_VertexNormal;
in vec2 cpp_TextureCoord;

out vec3 vs_Position;
out vec3 vs_Normal;
out vec2 vs_TextureCoord;
flat out int vs_InstanceID;

void main(void)
{
	vs_Position = (cpp_Instances[gl_InstanceID].modelXform * vec4(cpp_VertexPosition, 1.0)).xyz;
	vs_Normal = normalize(cpp_Instances[gl_InstanceID].modelXform * vec4(cpp_VertexNormal, 0.0)).xyz;
	vs_TextureCoord = cpp_TextureCoord;
	gl_Position = cpp_Instances[gl_InstanceID].mvpXform * vec4(cpp_VertexPosition, 1.0);
	vs_InstanceID = gl_InstanceID;
}
