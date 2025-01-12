#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// uniforms
//We think that gi texture pos is {0, 0} and this is left-down corner. So my pos will be relative to that and it's also left-down.
uniform vec2 my_pos;
uniform vec2 my_size;
uniform vec2 gi_size;
uniform sampler2D gi_texture;
uniform sampler2D geometry_texture;

out vec4 finalColor;

void main()
{
    vec2 uv = fragTexCoord;
    vec2 my_uv_pos = vec2(my_pos.x + uv.x * my_size.x, -my_pos.y + uv.y * my_size.y);
    vec2 gi_uv = vec2(my_uv_pos.x / gi_size.x, my_uv_pos.y / gi_size.y);

    vec4 current_color  = texture(texture0, uv);
    vec4 gi_color       = texture(gi_texture, gi_uv);
    vec4 geometry_color = texture(geometry_texture, uv);

    float distance_to_center = distance(uv, vec2(0.5)) * 2;
    float t = 1.0 - (sqrt(distance_to_center));
    // current_color.a *= t;
    
    // current_color = vec4(1.0 - distance_to_center * 2 * distance_to_center * 2);
    // current_color.rgb = mix(current_color.rgb, vec3(1, 1, 1), t);
    current_color += geometry_color;
    finalColor = mix(gi_color, gi_color + current_color, t);
    finalColor.a = t;
    
    // finalColor = vec4(uv.x, uv.y, 0, 1);
    // finalColor.a = t;
}