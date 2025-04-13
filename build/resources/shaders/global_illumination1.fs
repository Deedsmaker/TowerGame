#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
//in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// constants
uniform float PI = 3.141596;

#define MAX_CONNECTED_LIGHTMAPS 6

struct Lightmap_Data{
   sampler2D distance_texture; 
   sampler2D emitters_occluders_texture;
};

uniform Lightmap_Data lightmaps_data[MAX_CONNECTED_LIGHTMAPS];
uniform int my_lightmap_index;

// uniforms
uniform float u_time;
uniform int u_rays_per_pixel = 32;
// uniform sampler2D u_distance_data;
// uniform sampler2D u_scene_data;
// uniform sampler2D current_distance_texture;
// uniform sampler2D current_emitters_occluders_texture;
uniform float u_emission_multi = 1.0;
uniform int u_max_raymarch_steps = 128;
uniform float u_dist_mod = 1;
uniform vec2 u_screen_pixel_size;

out vec4 finalColor;

float random (vec2 st){
   return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void get_surface(vec4 surface_color, out float emissive, out vec3 colour)
{	
    // vec4 emissive_data = texture(lightmaps_data[lightmap_index].emitters_occluders_texture, uv);
    emissive = max(surface_color.r, max(surface_color.g, surface_color.b)) * u_emission_multi;
    colour = surface_color.rgb * u_emission_multi * surface_color.a;
}

bool raymarch(vec2 origin, vec2 dir, float aspect, out float mat_emissive, out vec3 mat_colour)
{
   float current_dist = 0.0;
   
   // current_distance_texture = lightmaps_data[my_lightmap_index].distance_texture;
   
    for (int i = 0; i < u_max_raymarch_steps; i++){
        int lightmap_index = my_lightmap_index;
        vec2 sample_point = origin + dir * current_dist;
        sample_point.x /= aspect; // when we sample the distance field we need to convert back to uv space.

        // early exit if we hit the edge of the screen.
        if(sample_point.y > 1.0 || sample_point.y < 0.0)
            return false;
           
        if (sample_point.x > 1.0){
           // if (lightmap_index < MAX_CONNECTED_LIGHTMAPS){
                while (sample_point.x > 1){
                    sample_point.x -= 1;
                    lightmap_index += 1;
                }
              // lightmap_index = my_lightmap_index + int(sample_point.x);
              // sample_point.x -= int(sample_point.x);
           // } else{
              // return false;
           // }
        }
        if (sample_point.x < 0.0){
           // if (lightmap_index < MAX_CONNECTED_LIGHTMAPS){
                while (sample_point.x < 0.0){
                    sample_point.x += 1;
                    lightmap_index -= 1;
                }
              // lightmap_index = my_lightmap_index - int(sample_point.x);
              // sample_point.x += int(sample_point.x) - 1;
           // } else{
              // return false;
           // }
        }
        
        vec4 distance_data           = texture(lightmaps_data[lightmap_index].distance_texture, sample_point);
    
        float dist_to_surface = distance_data.r / u_dist_mod;

        if (distance_data.a == 0){
            vec4 emitters_occluders_data = texture(lightmaps_data[lightmap_index].emitters_occluders_texture, sample_point);
            // hit_pos = sample_point;
            get_surface(emitters_occluders_data, mat_emissive, mat_colour);
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
   
   return false;
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
    float pixel_emis = 0.0;
    vec3 pixel_col = vec3(0.0);
   
    //vec2 screen_pixel_size = vec2(4.0 / 1600.0, 4.0 / 900.0);
   
    // convert from uv aspect to world aspect.
    vec2 uv = fragTexCoord;
    float aspect = u_screen_pixel_size.y / u_screen_pixel_size.x;
    uv.x *= aspect;
    
    float rand2pi = random(fragTexCoord * vec2(u_time, -u_time)) * 2.0 * PI;
    float golden_angle = PI * 0.7639320225; // magic number that gives us a good ray distribution.
   
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
        }
    }
    
    pixel_col /= pixel_emis;
    pixel_emis /= float(u_rays_per_pixel);

    // vec4 scene_color = texture(u_distance_data, fragTexCoord);
    // finalColor = vec4(scene_color.x * uv.x, scene_color.y * uv.y, 0, 1);
    // finalColor = scene_color;
    
    // finalColor = vec4(lin_to_srgb(vec4(pixel_emis * pixel_col, 1)), 1.0);
    finalColor = texture(lightmaps_data[my_lightmap_index].distance_texture, fragTexCoord);
    // if (my_lightmap_index == 2){
    //     // finalColor = mix(finalColor, vec4(1, 0, 0, 1), fragTexCoord.x);
    //     finalColor = texture(lightmaps_data[my_lightmap_index].emitters_occluders_texture, fragTexCoord);
    // }
}