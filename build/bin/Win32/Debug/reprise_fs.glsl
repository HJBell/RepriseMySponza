#version 330

struct LightSource
{
	vec3 position;
	vec3 intensity;
	float range;
};

uniform LightSource cpp_Lights[22];
uniform int cpp_LightCount;

uniform float cpp_AmbientIntensity;
uniform vec3 cpp_CameraPos;
uniform vec3 cpp_Diffuse;
uniform vec3 cpp_Specular;
uniform float cpp_Shininess;
uniform sampler2D cpp_Texture;

in vec3 vs_Position;
in vec3 vs_Normal;
in vec2 vs_TextureCoord;

out vec4 fs_Colour;




//-----------------Get Diffuse Function-----------------

vec4 GetDiffuse(vec3 L, vec3 N, vec3 lightIntensity, float rangeIntensity)
{
	//Calculating the dot product of L and N and ensuring it doesn't go below zero.
	float lDotN = max(0, dot(L, N));

	//If the material has a diffuse texture, getting its colour at the current texture coord.
	vec4 textureColour = texture(cpp_Texture, vs_TextureCoord);

	//Calculating the diffuse colour and returning it.
	return textureColour * vec4(cpp_Diffuse * lDotN * lightIntensity * rangeIntensity, 1.0);
}


//-----------------Get Specular Function-----------------

vec4 GetSpecular(vec3 L, vec3 N, vec3 V, float rangeIntensity)
{
	//Checking the material has a shininess value.
	if (cpp_Shininess <= 0) return vec4(0.0);

	//Calculating specular term of the fragment.
	float specularTerm = 0;
	if (dot(N, L) > 0)
	{
		//Calculating the half vector.
		vec3 H = normalize(L + V);

		//Calculating specular term of the fragment.
		if (dot(N, H) > 0) specularTerm = pow(max(dot(N, H), 0), cpp_Shininess);
	}

	//Calculating the specular colour and returning it.
	return vec4(cpp_Shininess * specularTerm, cpp_Shininess * specularTerm, cpp_Shininess * specularTerm, 1.0) * rangeIntensity;
}


//----------------------Main Function----------------------

void main(void)
{
	//Creating a colour variable and beginning by simply adding the ambient light.
	vec4 colour = vec4(cpp_AmbientIntensity, cpp_AmbientIntensity, cpp_AmbientIntensity, 1.0);

	//Calculating the normal and view vectors needed for the phong reflection model.
	vec3 N = normalize(vs_Normal);
	vec3 V = normalize(cpp_CameraPos - vs_Position);

	//Looping through the lights in the scene.
	for (int i = 0; i < cpp_LightCount; i++)
	{
		//Calculating the distance between the light and the fragment position.
		float distanceToLight = length(cpp_Lights[i].position - vs_Position);

		//Checking the fragment position is within range of the light.
		if (distanceToLight <= cpp_Lights[i].range)
		{
			//Calculating the light vector.
			vec3 L = normalize(cpp_Lights[i].position - vs_Position);

			//Calculating the intensity of the light based its distance from the fragment position.
			float rangeIntensity = (1.0 - smoothstep(0, cpp_Lights[i].range, distanceToLight));

			//Adding the diffuse colour to the fragment colour.
			colour += GetDiffuse(L, N, cpp_Lights[i].intensity, rangeIntensity);

			//Adding the specular colour to the fragment colour.
			//colour += GetSpecular(L, N, V, rangeIntensity);
		}
	}

	//Passing the fragment colour to OpenGL.
	fs_Colour = clamp(colour, 0.0, 1.0);
}