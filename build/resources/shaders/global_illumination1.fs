#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
//in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// constants
uniform float PI = 3.141596;

uniform sampler2D distance_texture;
uniform sampler2D emitters_occluders_texture;

uniform sampler2D perlin_texture;

uniform float bake_progress = -1;

// uniforms
uniform float u_time;
uniform int u_rays_per_pixel = 32;
uniform float u_emission_multi = 1.0;
uniform int u_max_raymarch_steps = 128;
uniform float u_dist_mod = 1;
uniform vec2 u_screen_pixel_size;

// r - left (?)
// g - up
// b - to us
uniform sampler2D u_normal_texture;

out vec4 finalColor;

float random (vec2 st){
   return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(vec2 st){
    vec2 i = floor(st);
    vec2 f = fract(st);
    
    // Four corners in 2D of a tile.
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    
    // Smooth interpolation.
    
    // Cubic Hernie Curve. Save as SmoothStep().
    vec2 u = f * f * (3.0 - 2.0 * f);
    // u = smoothstep(0.0, 1.0, f);
    
    // Mix 4 corners percentages
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float perlin(vec2 st){
    st.x -= int(st.x);
    st.y -= int(st.y);
    
    vec4 color = texture(perlin_texture, st);
    return (color.r + color.g + color.b) / 3.0;
}

void get_surface(vec4 surface_color, out float emissive, out vec3 colour)
{	
    // vec4 emissive_data = texture(lightmaps_data[lightmap_index].emitters_occluders_texture, uv);
    if (surface_color.a <= 0){
        emissive = 0;
        colour = vec3(0);
        return;
    }
    
    emissive = max(surface_color.r, max(surface_color.g, surface_color.b)) * u_emission_multi * surface_color.a;
    colour = surface_color.rgb * u_emission_multi;
    // colour = vec3(1, 0, 0);
}

vec2 reflected_vector(vec2 vec, vec2 normal){
    vec2 result = vec2(0);

    float dot_product = dot(vec, normal);//(v.x*normal.x + v.y*normal.y); // Dot product

    result.x = vec.x - (2.0f*normal.x)*dot_product;
    result.y = vec.y - (2.0f*normal.y)*dot_product;

    return result;
}

float sign(float num){
    if (num > 0){
        return 1;
    }
    return -1;
}

bool raymarch(vec2 origin, vec2 dir, float aspect, out float mat_emissive, out vec3 mat_colour)
{
    float current_dist = 0.0;
    int bounce_count = 0;
    mat_emissive = 0;    
    mat_colour = vec3(0);
   
    for (int i = 0; i < u_max_raymarch_steps; i++){
        vec2 sample_point = origin + dir * current_dist;
        sample_point.x /= aspect; // when we sample the distance field we need to convert back to uv space.

        // early exit if we hit the edge of the screen.
        if(sample_point.y > 1.0 || sample_point.y < 0.0 || sample_point.x > 1.0 || sample_point.x < 0.0){
            return false;
        }
            
        vec4 distance_data = texture(distance_texture, sample_point);
        vec4 emitters_occluders_data = texture(emitters_occluders_texture, sample_point);
        
        float dist_to_surface = distance_data.r / u_dist_mod;

        if (distance_data.a == 0){
            // hit_pos = sample_point;
            float emissive = 0;
            vec3 color = vec3(0);
            get_surface(emitters_occluders_data, emissive, color);
            
            bounce_count += 1;
            
            emissive /= bounce_count;
            color /= bounce_count;
            
            mat_emissive += emissive;
            mat_colour   += color;
            
            return true;
        }

        // we've hit a surface if distance field returns 0 or close to 0 (due to our distance field using a 16-bit float
        // the precision isn't enough to just check against 0).
        // if(dist_to_surface <= 0.00001){
        //    hit_pos = sample_point;
        //    return true;
        // }

        // if we don't hit a surface, continue marching along the ray.
        if (dist_to_surface < 0.001){
            dist_to_surface = 0.001;
        }
      
        current_dist += dist_to_surface;
   }
   
   return bounce_count > 0;
}

// Don't think that we should use that as it flattens all the colors. We're want to apply hdr stuff afterwards.
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
    if (bake_progress > -1){
        // baking in chunks of 0.05 of uv width.
        bool we_are_ahead = fragTexCoord.x > bake_progress;
        bool we_are_on_baked_stuff = fragTexCoord.x < bake_progress - 0.05;
        if (we_are_ahead || we_are_on_baked_stuff){
            return;    
        }
    }

    float pixel_emis = 0.0;
    vec3 pixel_col = vec3(0.0);
   
    //vec2 screen_pixel_size = vec2(4.0 / 1600.0, 4.0 / 900.0);
   
    // convert from uv aspect to world aspect.
    vec2 uv = fragTexCoord;
    float aspect = u_screen_pixel_size.y / u_screen_pixel_size.x;
    uv.x *= aspect;
    
    float rand2pi = perlin(fragTexCoord * vec2(u_time, -u_time)) * 2.0 * PI;
    float golden_angle = PI * 0.7639320225; // magic number that gives us a good ray distribution.
   
    vec2 average_light_direction = vec2(0);
   
    // cast our rays.
    for (int i = 0; i < u_rays_per_pixel; i++){
        // get our ray dir by taking the random angle and adding golden_angle * ray number.
        float cur_angle = rand2pi + golden_angle * float(i);
        vec2 ray_dir = normalize(vec2(cos(cur_angle), sin(cur_angle)));
        vec2 ray_origin = uv;
        
        float mat_emissive;
        vec3 mat_colour;
        // vec2 hit_pos;
        
        bool hit = raymarch(ray_origin, ray_dir, aspect, mat_emissive, mat_colour);
        if (hit){
            // get_surface(hit_pos, mat_emissive, mat_colour);
            pixel_emis += mat_emissive;
            pixel_col += mat_colour;
            
            if (mat_emissive > 0){
                average_light_direction += ray_dir * (mat_emissive / u_emission_multi);
            }
        }
    }
    
    
    pixel_col /= float(u_rays_per_pixel);
    pixel_emis /= float(u_rays_per_pixel);
    pixel_emis *= u_emission_multi;
    
    vec4 normal_color = texture(u_normal_texture, uv);
    if (normal_color.a > 0){
        average_light_direction = normalize(average_light_direction);
            
        vec2 normal_direction = vec2(((normal_color.r - 0.5) * 2) * -1, (normal_color.g - 0.5) * 2) * 1;
        
        float mult = dot(average_light_direction, normal_direction);
        
        mult = max(mult, -1);
        mult = (mult + 1) * 0.5;
        
        pixel_emis *= mult;
    }

    vec4 current_color = texture(texture0, fragTexCoord);
    // finalColor = current_color + vec4(pixel_emis * pixel_col, 1);
    finalColor = current_color + vec4(pixel_col * pixel_emis, 1);
    finalColor.a = 1.0;
}