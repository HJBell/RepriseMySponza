#version 330

#define MAX_INSTANCE_COUNT 64


//----------------------Structures----------------------

struct InstanceData
{
	mat4 mvpXform;
	mat4 modelXform;
	vec3 diffuse;
	float shininess;
	vec3 specular;
	int isShiny;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 intensity;
};


//----------------------Uniforms----------------------

layout(std140) uniform cpp_PerFrameUniforms
{
	vec3 cpp_CameraPos;
vec3 cpp_AmbientIntensity;
};

layout(std140) uniform cpp_PerModelUniforms
{
	InstanceData cpp_Instances[MAX_INSTANCE_COUNT];
};

layout(std140) uniform cpp_DirectionalLightUniforms
{
	DirectionalLight cpp_Light;
};

uniform sampler2D cpp_Texture;


//----------------------In Variables----------------------

in vec3 vs_Position;
in vec3 vs_Normal;
in vec2 vs_TextureCoord;
flat in int vs_InstanceID;


//----------------------Out Variables----------------------

out vec4 fs_Colour;


//----------------------Main Function----------------------

void main(void)
{
	// Creating a colour variable and beginning by adding the ambient light.
	vec4 colour = vec4(0.0);

	// Calculating the intensity of the light due to the angle it hits the fragment.
	float angleIntensity = max(0.0, dot(cpp_Light.direction, vs_Normal));

	// Calculating and returning the final colour.
	colour += vec4(cpp_Instances[vs_InstanceID].diffuse * angleIntensity * cpp_Light.intensity, 1.0);

	// Applying the texture for the fragment.
	colour *= texture(cpp_Texture, vs_TextureCoord);

	// Passing the fragment colour to OpenGL.
	fs_Colour = clamp(colour, 0.0, 1.0);
}