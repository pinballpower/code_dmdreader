#version 300 es
precision mediump float;

// Input from vertex shader
in vec3 v_color;
in vec2 TexCoord;

// Output of fragment shader
layout(location = 0) out vec4 fragColor;

uniform sampler2D texture1;
uniform sampler2D texture2; // the texture that will be rendered on top of the DMD

// varying vec3 ourColor;

void main()
{
	// Multiple pixel overlay
	vec2 tc2;
	tc2.x = TexCoord.x * 128.0;
	tc2.y = TexCoord.y * 128.0;

	vec4 alphaColor = texture(texture2, tc2);
    fragColor = mix(texture(texture1, TexCoord), alphaColor, alphaColor.a);
}