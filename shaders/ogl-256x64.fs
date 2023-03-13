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
	tc2.x = TexCoord.x*256;
	tc2.y = TexCoord.y*256;

	vec4 alphaColor = texture(texture2, tc2);
    FragColor = mix(texture(texture1, TexCoord), alphaColor, alphaColor.a);
}