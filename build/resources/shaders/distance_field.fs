#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
//in vec4 fragColor;

// Input uniform values
// uniform sampler2D texture0;
uniform sampler2D u_tex;
uniform sampler2D obstacles_texture;
uniform float u_dist_mod = 1.0;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 tex = texture(u_tex, fragTexCoord);
    float dist = distance(tex.xy, fragTexCoord);
    float mapped = clamp(dist * u_dist_mod, 0.0, 1.0);
    float a = 1.0;
    if (texture(obstacles_texture, fragTexCoord).a > 0){
        a = 0;
    }
    
    finalColor = vec4(vec3(mapped), a);
}