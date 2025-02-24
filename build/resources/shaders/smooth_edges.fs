#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// uniforms
//We think that gi texture pos is {0, 0} and this is left-down corner. So my pos will be relative to that and it's also left-down.
uniform float light_power = 1;
uniform vec4 light_color;
uniform vec2 my_pos;
uniform vec2 my_size;
uniform vec2 gi_size;
uniform sampler2D gi_texture;
uniform sampler2D geometry_texture;
uniform sampler2D light_texture;
uniform sampler2D backshadows_texture;

out vec4 finalColor;

vec3 lin_to_srgb(vec4 color)
{
   vec3 x = color.rgb * 12.92;
   vec3 y = 1.055 * pow(clamp(color.rgb, 0.0, 1.0), vec3(0.4166667)) - 0.055;
   vec3 clr = color.rgb;
   clr.r = (color.r < 0.0031308) ? x.r : y.r;
   clr.g = (color.g < 0.0031308) ? x.g : y.g;
   clr.b = (color.b < 0.0031308) ? x.b : y.b;
   return clr.rgb;
}

void main()
{
    vec2 uv = fragTexCoord;
    vec2 my_uv_pos = vec2(my_pos.x + uv.x * my_size.x, my_pos.y + uv.y * my_size.y);
    vec2 gi_uv = vec2(my_uv_pos.x / gi_size.x, my_uv_pos.y / gi_size.y);

    vec4 light_texture_color = texture(light_texture, uv);
    vec4 current_color  = texture(texture0, uv);
    current_color *= light_texture_color;
    // current_color.a = alpha + light_texture_color.a;
    vec4 gi_color          = texture(gi_texture, gi_uv);
    vec4 geometry_color    = texture(geometry_texture, gi_uv) * light_texture_color;
    vec4 backshadows_color = texture(backshadows_texture, uv) * light_texture_color + geometry_color;

    float distance_to_center = distance(uv, vec2(0.5)) * 2;
    float t = clamp(1.0 - (sqrt(distance_to_center)), 0, 1);
    // current_color.a *= t;
    
    // current_color = vec4(1.0 - distance_to_center * 2 * distance_to_center * 2);
    // current_color.rgb = mix(current_color.rgb, vec3(1, 1, 1), t);
    current_color += geometry_color;
    finalColor = mix(gi_color, gi_color + current_color, t);
    finalColor *= mix(vec4(1), backshadows_color, 1.0 - texture(texture0, uv).a);
    finalColor *= light_color;
    
    finalColor = mix(gi_color, finalColor * light_power, light_color.a);
    finalColor.a = t;
}