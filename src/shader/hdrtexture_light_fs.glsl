
#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec2 TexCoords;

uniform sampler2D environmentMap;


//这个等下在思考 先想想thickness怎么用吧 如何生成thicknessmap

void main()
{	

    vec3 color = texture(environmentMap, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness <= 1.0) color = vec3(0.0);

    FragColor = vec4(color, 1.0);

}





