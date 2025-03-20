#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
//in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 sceneColor = texture(texture0, fragTexCoord);
    finalColor = vec4(fragTexCoord.x * sceneColor.a, fragTexCoord.y * sceneColor.a, 0, sceneColor.a);
}