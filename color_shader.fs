#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

//uniform
uniform float alpha = 0.5;

void main()
{
    vec4 col = texture(texture0, fragTexCoord) * fragColor;
    //vec4 col = fragColor;
    //col.r = 0;
    col.a = col.a * alpha;
    finalColor = col;
}