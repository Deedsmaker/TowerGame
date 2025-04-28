#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// uniforms
uniform sampler2D u_gi_data;
uniform float u_emission_multi = 1.0;

out vec4 finalColor;

void get_surface(vec2 uv, out float emissive, out vec4 colour)
{	
    vec2 gi_uv = vec2(uv.x, uv.y);
    vec4 emissive_data = texture(u_gi_data, gi_uv);
    emissive = max(emissive_data.r, max(emissive_data.g, emissive_data.b)) * u_emission_multi;
    colour = emissive_data;
}

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
    vec4 current_color = texture(texture0, fragTexCoord);

    float pixel_emis = 1;
    vec4 color = vec4(0);
    
    get_surface(fragTexCoord, pixel_emis, color);
    
    float light_intencity = sqrt(color.r * color.r + color.g * color.g + color.b * color.b);
    
    // finalColor = current_color * color;
    // finalColor.rgb = pow(current_color + color, 2.2);
    finalColor = (current_color * light_intencity * current_color.a) + color * current_color;
    finalColor.a = 1;
    
    // finalColor = current_color * color;
    // finalColor = mix(finalColor, finalColor * 2, color.a);
    // finalColor.a = 1;
}