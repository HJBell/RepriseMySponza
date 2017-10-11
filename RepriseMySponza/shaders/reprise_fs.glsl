#version 330

struct DirectionalLight
{
	vec3 direction;
	vec3 intensity;
};

struct PointLight
{
	vec3 position;
	float range;
	vec3 intensity;
};

struct SpotLight
{
	vec3 position;
	float range;
	vec3 intensity;
	float angle;
	vec3 direction;
};

layout (std140) uniform cpp_PerFrameUniforms
{
	vec3 cpp_CameraPos;
	vec3 cpp_AmbientIntensity;
	DirectionalLight cpp_DirectionalLights[32];
	int cpp_DirectionalLightCount;
	PointLight cpp_PointLights[32];
	int cpp_PointLightCount;
	SpotLight cpp_SpotLights[32];
	int cpp_SpotLightCount;
};

layout(std140) uniform cpp_PerModelUniforms
{
	mat4 cpp_MVPXform;
	mat4 cpp_ModelXform;
	vec3 cpp_Diffuse;
	float cpp_Shininess;
	vec3 cpp_Specular;
};

uniform sampler2D cpp_Texture;

in vec3 vs_Position;
in vec3 vs_Normal;
in vec2 vs_TextureCoord;

out vec4 fs_Colour;



vec4 ApplyDirectionalLight(DirectionalLight light)
{
	// Calculating the intensity of the light due to the angle it hits the fragment.
	float angleIntensity = max(0.0, dot(light.direction, vs_Normal));

	// Calculating and returning the final colour.
	return vec4(cpp_Diffuse * angleIntensity * light.intensity, 1.0);
}

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

		// Calculating the final colour value.
		colour = vec4(cpp_Diffuse * angleIntensity * light.intensity * rangeIntensity, 1.0);
	}

	// Returning the colour.
	return colour;
}

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
			colour = vec4(cpp_Diffuse * angleIntensity * light.intensity * rangeIntensity, 1.0);
		}
	}
	// Returning the colour.
	return colour;
}


//----------------------Main Function----------------------

void main(void)
{
	// Creating a colour variable and beginning by adding the ambient light.
	vec4 colour = vec4(cpp_AmbientIntensity, 0.0);

	// Applying the directional lights in the scene.
	for (int i = 0; i < cpp_DirectionalLightCount; i++)
		colour += ApplyDirectionalLight(cpp_DirectionalLights[i]);

	// Applying the point lights in the scene.
	for (int i = 0; i < cpp_PointLightCount; i++)
		colour += ApplyPointLight(cpp_PointLights[i]);

	// Applying the spot lights in the scene.
	for (int i = 0; i < cpp_SpotLightCount; i++)
		colour += ApplySpotLight(cpp_SpotLights[i]);

	// Applying the texture for the fragment.
	colour *= texture(cpp_Texture, vs_TextureCoord);

	// Passing the fragment colour to OpenGL.
	fs_Colour = clamp(colour, 0.0, 1.0);
}