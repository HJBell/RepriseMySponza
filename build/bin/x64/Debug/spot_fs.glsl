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

struct SpotLight
{
	vec3 position;
	float range;
	vec3 intensity;
	float angle;
	vec3 direction;
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

layout(std140) uniform cpp_SpotLightUniforms
{
	SpotLight cpp_Light;
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

vec4 ApplySpotLight(SpotLight light)
{
	// Creating an empty colour variable for the light.
	vec4 colour = vec4(0.0);

	// Normalising the forward direction of the light.
	vec3 lightDirection = normalize(light.direction);

	// Calculating the direction from the light to the fragment.
	vec3 lightToFragment = vs_Position - light.position;

	// Calculating the angle between the 'lightDirection' and 'lightToFragment' vectors.
	float angleBetweenLightAndRay = degrees(dot(lightDirection, normalize(lightToFragment)));

	// Checking if the fragment is in the light cone.
	if (angleBetweenLightAndRay > light.angle * 0.5)
	{
		// Calculating the distance between the fragment and the light.
		float distanceToLight = length(-lightToFragment);

		// Using smoothstep to calculate the intensity of the light based on its range.
		float rangeIntensity = (1.0 - smoothstep(0, light.range, distanceToLight));

		// Checking the fragment is within range of the light.
		if (rangeIntensity > 0.0)
		{
			// Calculating the intensity of the light due to the angle it hits the fragment.
			float angleIntensity = max(0.0, dot(normalize(-lightToFragment), vs_Normal));

			// Calculating the final colour value.
			colour = vec4(cpp_Instances[vs_InstanceID].diffuse * angleIntensity * light.intensity * rangeIntensity, 1.0);
		}
	}
	// Returning the colour.
	return colour;
}

void main(void)
{
	// Creating a colour variable and beginning by adding the ambient light.
	vec4 colour = vec4(0.0);

	colour += ApplySpotLight(cpp_Light);

	// Applying the texture for the fragment.
	colour *= texture(cpp_Texture, vs_TextureCoord);

	// Passing the fragment colour to OpenGL.
	fs_Colour = clamp(colour, 0.0, 1.0);
}