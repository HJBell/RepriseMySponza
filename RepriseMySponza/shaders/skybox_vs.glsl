#version 330


//----------------------Uniforms----------------------

layout(std140) uniform cpp_SkyboxUniforms
{
	mat4 cpp_ViewProjectionXform;
	vec3 cpp_CameraPos;
};


//----------------------In Variables----------------------

in vec3 cpp_VertexPosition;


//----------------------Out Variables----------------------

out vec3 vs_TextureCoord;


//----------------------Main Function----------------------

void main(void)
{
	vs_TextureCoord = vec3(cpp_VertexPosition.x, -cpp_VertexPosition.yz);
	gl_Position = cpp_ViewProjectionXform * vec4(cpp_VertexPosition + cpp_CameraPos, 1.0);
}

