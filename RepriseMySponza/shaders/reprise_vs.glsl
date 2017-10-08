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
	vs_Position = (cpp_ModelXform * vec4(cpp_VertexPosition, 1.0)).xyz;
	vs_Normal = normalize(cpp_ModelXform * vec4(cpp_VertexNormal, 0.0)).xyz;
	vs_TextureCoord = cpp_TextureCoord;
	gl_Position = cpp_MVPXform * vec4(cpp_VertexPosition, 1.0);
}
