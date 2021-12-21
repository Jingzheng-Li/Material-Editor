#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec2 TexCoords;

uniform samplerCube environmentMap;

const float PI = 3.1415926;
const vec2 Atan = vec2(2*PI, PI);

vec3 SampleCubeMap(vec2 uv)
{
	uv -= 0.5;
	uv *= Atan;

	float x = cos(uv.y) * cos(uv.x);
	float y = sin(uv.y);
	float z = cos(uv.y) * sin(uv.x);

	vec3 samplevector = vec3(x ,y, z);

	return samplevector;
}

void main()
{
	vec3 samplevector = SampleCubeMap(TexCoords);
	vec3 color = texture(environmentMap, normalize(samplevector)).rgb;


	FragColor = vec4(color, 1.0);

}





