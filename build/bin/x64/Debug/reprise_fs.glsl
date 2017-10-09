#version 330

struct DirectionalLight
{
	vec3 direction;
	vec3 intensity;
};

struct PointLight
{
	vec3 position;
	vec3 intensity;
	float range;
};

struct SpotLight
{
	vec3 position;
	vec3 intensity;
	float range;
	float angle;
	vec3 direction;
};

uniform int cpp_PointLightCount;
uniform PointLight cpp_PointLights[22];

uniform int cpp_DirectionalLightCount;
uniform DirectionalLight cpp_DirectionalLights[22];

uniform int cpp_SpotLightCount;
uniform SpotLight cpp_SpotLights[22];

uniform vec3 cpp_AmbientIntensity;
uniform vec3 cpp_CameraPos;
uniform vec3 cpp_Diffuse;
uniform vec3 cpp_Specular;
uniform float cpp_Shininess;
uniform sampler2D cpp_Texture;

in vec3 vs_Position;
in vec3 vs_Normal;
in vec2 vs_TextureCoord;

out vec4 fs_Colour;



vec4 ApplyDirectionalLight(DirectionalLight light, vec3 N)
{
	float lDotN = max(0.0, dot(light.direction, N));

	return vec4(cpp_Diffuse * lDotN * light.intensity, 1.0);
}

vec4 ApplyPointLight(PointLight light, vec3 N)
{
	// Creating a colour variable for the light.
	vec4 colour = vec4(0.0);

	vec3 L = light.position - vs_Position;

	//Calculating the distance between the light and the fragment position.
	float distanceToLight = length(L);

	//Calculating the intensity of the light based its distance from the fragment position.
	float rangeIntensity = (1.0 - smoothstep(0, light.range, distanceToLight));

	//Checking the fragment position is within range of the light.
	if (rangeIntensity > 0.0)
	{
		//Calculating the dot product of L and N and ensuring it doesn't go below zero.
		float lDotN = max(0.0, dot(normalize(L), N));

		//Calculating the diffuse colour and returning it.
		colour = vec4(cpp_Diffuse * lDotN * light.intensity * rangeIntensity, 1.0);
	}

	return colour;
}

vec4 ApplySpotLight(SpotLight light, vec3 N)
{
	// Creating a colour variable for the light.
	vec4 colour = vec4(0.0);



	vec3 lightDirection = normalize(light.direction);
	vec3 rayDirection = normalize(vs_Position - light.position);

	float angleBetweenLightAndRay = degrees(abs(dot(lightDirection, rayDirection)));

	if (angleBetweenLightAndRay > light.angle * 0.5)
	{


		// Point light calculation
		vec3 L = light.position - vs_Position;
		float distanceToLight = length(L);
		float rangeIntensity = (1.0 - smoothstep(0, light.range, distanceToLight));
		if (rangeIntensity > 0.0)
		{
			float lDotN = max(0.0, dot(normalize(L), N));
			colour = vec4(cpp_Diffuse * lDotN * light.intensity * rangeIntensity, 1.0);
		}



	}



	return colour;
}


//----------------------Main Function----------------------

void main(void)
{
	// Creating a colour variable and beginning by adding the ambient light.
	vec4 colour = vec4(cpp_AmbientIntensity, 0.0);

	// Calculating the normal and view vectors needed for the phong reflection model.
	vec3 N = vs_Normal;

	// Applying the directional lights in the scene.
	for (int i = 0; i < cpp_DirectionalLightCount; i++)
		colour += ApplyDirectionalLight(cpp_DirectionalLights[i], N);

	// Applying the point lights in the scene.
	for (int i = 0; i < cpp_PointLightCount; i++)
		colour += ApplyPointLight(cpp_PointLights[i], N);

	// Applying the spot lights in the scene.
	for (int i = 0; i < cpp_SpotLightCount; i++)
		colour += ApplySpotLight(cpp_SpotLights[i], N);

	// Applying the texture for the fragment.
	colour *= texture(cpp_Texture, vs_TextureCoord);

	// Passing the fragment colour to OpenGL.
	fs_Colour = clamp(colour, 0.0, 1.0);
}