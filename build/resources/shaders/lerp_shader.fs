#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D texture1;

// Output fragment color
out vec4 finalColor;

//uniform
uniform float interp;

void main()
{
    vec4 from = texture2D(texture0, fragTexCoord.xy) ;
    vec4 to = texture2D(texture1, fragTexCoord.xy);

    finalColor = mix(from, to, interp);
}