#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec2 TexCoords;


uniform sampler2D environmentMap;



void main()
{	

    vec3 color = texture(environmentMap, TexCoords).rgb;
    FragColor = vec4(color, 1.0);

}





