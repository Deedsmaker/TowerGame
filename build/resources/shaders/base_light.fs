#version 330

// Input vertex attributes
//in vec3 vertexPosition;
//in vec2 vertexTexCoord;
//in vec3 vertexNormal;
//in vec4 vertexColor;
//in vec4 vertexTangent;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
//out vec2 fragTexCoord;
//out vec4 fragColor;
//out vec3 fragPosition;
//in vec4 fragTangent;
//in vec3 fragNormal;

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
// in vec3 fragPosition;

// Input uniform values
uniform sampler2D texture0;
// uniform vec4 colDiffuse;
//uniform vec3 lightPos;          // light position
//uniform mat4 matModel;          // pos, rotation and scaling of object
// uniform vec3 viewPos;           // eyes position
//uniform sampler2D normalMap;

// Output fragment color
out vec4 finalColor;

//uniform
uniform float alpha = 0.4;
uniform vec2 light_dir = {-0.5, 1};

void main()
{
    vec4 col = texture(texture0, fragTexCoord) * fragColor;
    //vec4 col = fragColor;
    //col.r = 0;
    float d = dot(light_dir, fragTexCoord);
    d = clamp(d, 0.3, 1);
    //col.a = col.a * alpha;
    float a = col.a;
    col *= d;
    col.a = a;
    finalColor = col;
}