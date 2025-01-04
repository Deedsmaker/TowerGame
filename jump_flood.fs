#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
//in vec4 fragColor;

// Input uniform values
uniform int u_level;
uniform int u_step;
uniform vec2 u_pixel;
uniform int u_max_steps;
uniform float u_offset;
uniform sampler2D texture0;
uniform sampler2D u_tex;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 sceneColor = texture(u_tex, fragTexCoord);
    finalColor = vec4(fragTexCoord.x * sceneColor.a, fragTexCoord.y * sceneColor.a, 0, 1);
    
    float closest_dist = 9999999.9;
    vec2 closest_pos = vec2(0.0);
    vec2 screen_pixel_size = vec2(4.0 / 1600.0, 4.0 / 900.0);

    // insert jump flooding algorithm here.
    for (float x = -1.0; x <= 1.0; x += 1.0){
        for (float y = -1.0; y <= 1.0; y += 1.0){
            vec2 voffset = fragTexCoord;
            voffset += vec2(x, y) * screen_pixel_size * u_offset;
    
            vec2 pos = texture(u_tex, voffset).xy;
            float dist = distance(pos.xy, fragTexCoord.xy);
            
            if (pos.x != 0.0 && pos.y != 0.0 && dist < closest_dist){
                closest_dist = dist;
                closest_pos = pos;
            }
        }
    }
    
    finalColor = vec4(closest_pos, 0.0, 1.0);
    
  //   vec2 closest_pixel = vec2(-1.0);
  //   float closest_dist = 9999999.9;

    
  //   vec4 p = texture(u_tex, fragTexCoord);
  //   if (p.a == -1.) {
  // 		// already visited
  // 		closest_pixel = p.rg;
  // 		closest_dist = length((p.rg-fragTexCoord)/u_pixel);
  // 	}
	
  // for (int y = -1; y <= 1; ++y) {
  //   for (int x = -1; x <= 1; ++x) { 
  // 			vec2 v = vec2(x,y) * u_step; // Vector from pos to neighbor in pixel coordinates
  // 			vec2 uv = fragTexCoord + v * u_pixel; // Texture coord of neighbor
  // 			if (uv.x < 0. || uv.x >= 1. || uv.y < 0. || uv.y >= 1.) continue;
  // 			vec4 sample = texture2D (u_tex, uv);
			
  // 			if (sample.a >= 1.) {  // An original pixel neighbor
  // 				float dist = length(v);
  // 				if (dist < closest_dist) {
  // 					closest_dist = dist;
  // 					closest_pixel = uv;
  // 				}
  // 			} else if (sample.a == -1.) { // A visited empty pixel neighbor
  // 				float dist = length((sample.rg-fragTexCoord)/u_pixel);
  // 				if (dist < closest_dist) {
  // 					closest_dist = dist;
  // 					closest_pixel = sample.rg;
  // 				}
  // 			}
  // 		}
  // 	}
  // 	if (closest_dist < 9999999) p = vec4(closest_pixel,0,-1.);
    
  //   p.a = 1;
    // finalColor = p;//vec4(closest_pos, 0.0, 1.0);
}