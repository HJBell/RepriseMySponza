#version 330

layout(std140) uniform cpp_PerModelUniforms
{
	mat4 cpp_MVPXform;
	mat4 cpp_ModelXform;
	vec3 cpp_Diffuse;
	float cpp_Shininess;
	vec3 cpp_Specular;
};

in vec3 cpp_VertexPosition;
in vec3 cpp_VertexNormal;
in vec2 cpp_TextureCoord;

out vec3 vs_Position;
out vec3 vs_Normal;
out vec2 vs_TextureCoord;

void main(void)
{
	vs_Position = (cpp_ModelXform * vec4(cpp_VertexPosition, 1.0)).xyz;
	vs_Normal = normalize(cpp_ModelXform * vec4(cpp_VertexNormal, 0.0)).xyz;
	vs_TextureCoord = cpp_TextureCoord;
	gl_Position = cpp_MVPXform * vec4(cpp_VertexPosition, 1.0);
}
