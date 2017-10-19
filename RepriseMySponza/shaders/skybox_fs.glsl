#version 330


//----------------------Uniforms----------------------

uniform samplerCube cpp_CubeMap;


//----------------------In Variables----------------------

in vec3 vs_TextureCoord;


//----------------------Out Variables----------------------

out vec4 fs_Colour;


//----------------------Main Function----------------------

void main()
{
	fs_Colour = texture(cpp_CubeMap, vs_TextureCoord);
}