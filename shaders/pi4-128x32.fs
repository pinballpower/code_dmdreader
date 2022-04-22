// #version 300 es   
// precision mediump float;

// out vec4 FragColor;

// 
// varying vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2; // the texture that will be rendered on top of the DMD

varying vec3 ourColor;

void main()
{
	// Multiple pixel overlay
	// vec2 tc2;
	// tc2.x = TexCoord.x*128;
	// tc2.y = TexCoord.y*128;

	// vec4 alphaColor = texture(texture2, tc2);
    gl_FragColor = vec4(1.0,0.0,1.0,0.0); // mix(texture(texture1, TexCoord), alphaColor, alphaColor.a);
}