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

struct PointLight
{
	vec3 position;
	float range;
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

layout(std140) uniform cpp_PointLightUniforms
{
	PointLight cpp_Light;
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

vec4 ApplyPointLight(PointLight light)
{
	// Creating an empty colour variable for the light.
	vec4 colour = vec4(0.0);

	// Calculting the vector from the fragment to the light.
	vec3 fragmentToLight = light.position - vs_Position;

	// Calculating the distance between the light and the fragment.
	float distanceToLight = length(fragmentToLight);

	// Using smoothstep to calculate the intensity of the light based on its range.
	float rangeIntensity = (1.0 - smoothstep(0, light.range, distanceToLight));

	// Checking the fragment is within range of the light.
	if (rangeIntensity > 0.0)
	{
		// Calculating the intensity of the light due to the angle it hits the fragment.
		float angleIntensity = max(0.0, dot(normalize(fragmentToLight), vs_Normal));

		// Using the diffuse as the base colour.
		vec3 baseColour = cpp_Instances[vs_InstanceID].diffuse;

		// Calculating specular.
		if (cpp_Instances[vs_InstanceID].isShiny == 1 && cpp_Instances[vs_InstanceID].shininess > 0.0)
		{
			// Calculating the vector from the fragment to the camera.
			vec3 fragmentToCamera = cpp_CameraPos - vs_Position;

			// Calculating the specular intensity on the fragment.
			float specularIntensity = 0.0;
			if (dot(vs_Normal, fragmentToLight) > 0.0)
			{
				vec3 resultantVector = normalize(normalize(fragmentToLight) + normalize(fragmentToCamera));
				if (dot(vs_Normal, resultantVector) > 0)
					specularIntensity = pow(max(dot(vs_Normal, resultantVector), 0), cpp_Instances[vs_InstanceID].shininess);
			}

			// Calculating the final specular colour.
			vec3 specular = cpp_Instances[vs_InstanceID].specular * specularIntensity;

			// Adding the specular colour to the base colour.
			baseColour += specular;
		}

		// Calculating the final colour value.
		colour = vec4(baseColour * angleIntensity * light.intensity * rangeIntensity, 1.0);
	}

	// Returning the colour.
	return colour;
}

void main(void)
{
	// Creating a colour variable and beginning by adding the ambient light.
	vec4 colour = vec4(0.0);

	colour += ApplyPointLight(cpp_Light);

	// Applying the texture for the fragment.
	colour *= texture(cpp_Texture, vs_TextureCoord);

	// Passing the fragment colour to OpenGL.
	fs_Colour = clamp(colour, 0.0, 1.0);
}