#version 330

// Output vertex attributes (to fragment shader)
//in vec4 fragTangent;
//in vec3 fragNormal;

in vec2 fragTexCoord;
in vec4 fragColor;
// in vec3 fragPosition;

uniform sampler2D texture0;
// uniform vec4 colDiffuse;
//uniform vec3 lightPos;          // light position
//uniform mat4 matModel;          // pos, rotation and scaling of object
// uniform vec3 viewPos;           // eyes position
//uniform sampler2D normalMap;

// Output fragment color
out vec4 finalColor;

uniform float alpha = 0.4;

uniform vec3 light = {0, 0, 0.5};

void main()
{
    vec4 col = texture(texture0, fragTexCoord) * fragColor;
    
    float len = distance(light.xy, fragTexCoord);
    
    vec4 lightColor = {255, 255, 255, 255};
    col = {len * 255, len* 255, len* 255, len* 255};
    
    finalColor = col;
}