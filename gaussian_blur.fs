#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

// uniforms
// precision highp float;
uniform vec2 u_pixel; // Size of a pixel

#define gaussian_blur mat3(1, 2, 1, 2, 4, 2, 1, 2, 1) * 0.0625

out vec4 finalColor;

// Iterate over all pixels in 3x3 kernel area
vec4 convolute(vec2 uv, mat3 kernel) {
    vec4 color = vec4(0);
    
    float direction[3];
		direction[0] = -1.;
		direction[1] = 0.;
		direction[2] = 1.;
    for (int x = 0; x < 3; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            vec2 offset = vec2(direction[x], direction[y]) * u_pixel;
            color += texture2D(texture0, uv+offset) * kernel[x][y];
        }
    }
    return color;
}

void main() {
	finalColor = convolute(fragTexCoord, gaussian_blur);
	// vec4 color = vec4(0.);
	// for (int y = -1; y <= 1; ++y) {
	// for (int x = -1; x <= 1; ++x) { 
	// 		vec2 uv = vTexCoord + vec2(x,y) * u_pixel;
	// 		color += texture2D (texture0, uv);
	// 	}
	// }
	// gl_FragColor = color/9.0;
}