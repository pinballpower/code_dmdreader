#pragma once

const string fragmentShader128x32 = R"""(

#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2; // the texture that will be rendered on top of the DMD

void main()
{
	// Multiple pixel overlay
	vec2 tc2;
	tc2.x = TexCoord.x*128;
	tc2.y = TexCoord.y*128;

	vec4 alphaColor = texture(texture2, tc2);
    FragColor = mix(texture(texture1, TexCoord), alphaColor, alphaColor.a);
}

)""";

const string fragmentShader192x64 = R"""(

#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	// Multiple pixel overlay
	vec2 tc2;
	tc2.x = TexCoord.x*192;
	tc2.y = TexCoord.y*192;

	vec4 alphaColor = texture(texture2, tc2);
    FragColor = mix(texture(texture1, TexCoord), alphaColor, alphaColor.a);
}

)""";


const string vertexShader = R"""(
	
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	ourColor = aColor;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)""";

const string fragmentShader128x32EGL = R"""(

out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2; // the texture that will be rendered on top of the DMD

void main()
{
	// Multiple pixel overlay
	vec2 tc2;
	tc2.x = TexCoord.x*128;
	tc2.y = TexCoord.y*128;

	vec4 alphaColor = texture(texture2, tc2);
    FragColor = mix(texture(texture1, TexCoord), alphaColor, alphaColor.a);
}

)""";

const string vertexShaderEGL = R"""(
	
attribute vec3 position;

void main()
{
	gl_Position = vec4(position, 1.0);
}
)""";
