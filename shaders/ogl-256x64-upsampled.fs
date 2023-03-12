#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2; // the texture that will be rendered on top of the DMD

#define WIDTH 256

void main()
{
	// Multiple pixel overlay
	vec2 tc2;
	tc2.x = TexCoord.x*256;
	tc2.y = TexCoord.y*256;
	vec4 alphaColor = texture(texture2, tc2);

	// double resolution
	vec2 shift_bottom_left = vec2(-1.0f/WIDTH/2,-1.0f/WIDTH/2);
	vec2 c_bottom_left = TexCoord+shift_bottom_left;

	vec2 shift_bottom_right = vec2(-1.0f/WIDTH/2,1.0f/WIDTH/2);
	vec2 c_bottom_right = TexCoord+shift_bottom_right;

	vec2 shift_top_left = vec2(1.0f/WIDTH/2,-1.0f/WIDTH/2);
	vec2 c_top_left = TexCoord+shift_top_left;

	vec2 shift_top_right = vec2(1.0f/WIDTH/2,1.0f/WIDTH/2);
	vec2 c_top_right = TexCoord+shift_top_right;


    vec4 color_bottom_left = texture(texture1, c_bottom_left);
    vec4 color_bottom_right = texture(texture1, c_bottom_right);
    vec4 color_top_left = texture(texture1, c_top_left);
    vec4 color_top_right = texture(texture1, c_top_right);
	vec4 color = (color_bottom_left+color_bottom_right+color_top_left+color_top_right)/4;

	FragColor = mix(color, alphaColor, alphaColor.a);
}