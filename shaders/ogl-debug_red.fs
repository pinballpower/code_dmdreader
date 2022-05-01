#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2; // the texture that will be rendered on top of the DMD

void main()
{
    FragColor = vec4( 1.0, 0.0, 0.0, 0.5 );
}