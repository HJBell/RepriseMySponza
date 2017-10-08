#version 330

uniform mat4 cpp_MVPXform;
uniform mat4 cpp_ModelXform;

in vec3 cpp_VertexPosition;
in vec3 cpp_VertexNormal;
in vec2 cpp_TextureCoord;

out vec3 vs_Position;
out vec3 vs_Normal;
out vec2 vs_TextureCoord;

void main(void)
{
	vs_Position = mat3(cpp_ModelXform) * cpp_VertexPosition;
	vs_Normal = normalize(mat3(cpp_ModelXform) * cpp_VertexNormal);
	vs_TextureCoord = cpp_TextureCoord;
	gl_Position = cpp_MVPXform * vec4(cpp_VertexPosition, 1.0);
}
