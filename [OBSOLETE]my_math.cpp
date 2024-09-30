#pragma once

#include <math.h>

#define PI 3.14159265358979323846f
#define DEG2RAD (PI/180.0f)
#define RAD2DEG (180.0f/PI)
#define EPSILON 0.000001f

struct Vector2{
    f32 x;
    f32 y;
    
    Vector2 operator+(Vector2 b){
        return {x + b.x, y + b.y};
    }
    Vector2 operator-(Vector2 b){
        return {x - b.x, y - b.y};
    }
    Vector2 operator*(f32 value){
        return {x * value, y * value};
    }
    Vector2 operator/(f32 value){
        return {x / value, y / value};
    }
    void operator+=(Vector2 b){
        x += b.x;
        y += b.y;
    }
    void operator-=(Vector2 b){
        x -= b.x;
        y -= b.y;
    }
    void operator*=(f32 value){
        x *= value;
        y *= value;
    }
    void operator/=(f32 value){
        x /= value;
        y /= value;
    }
    
    b32 operator==(Vector2 b){
        return x == b.x && y == b.y;
    }
    b32 operator!=(Vector2 b){
        return x != b.x || y != b.y;
    }
};

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    
    Color operator*(f32 value){
        return {(unsigned char)(r * value), (unsigned char)(g * value), (unsigned char)(b * value), (unsigned char)(a * value)};
    }
    Color operator+(Color other){
        return {(unsigned char)(r + other.r), (unsigned char)(g + other.g), (unsigned char)(b + other.b), (unsigned char)(a + other.a)};
    }
};

#define LIGHTGRAY  (Color){ 200, 200, 200, 255 }   
#define GRAY       (Color){ 130, 130, 130, 255 }   
#define DARKGRAY   (Color){ 80, 80, 80, 255 }      
#define YELLOW     (Color){ 253, 249, 0, 255 }     
#define GOLD       (Color){ 255, 203, 0, 255 }     
#define ORANGE     (Color){ 255, 161, 0, 255 }     
#define PINK       (Color){ 255, 109, 194, 255 }   
#define RED        (Color){ 230, 41, 55, 255 }     
#define MAROON     (Color){ 190, 33, 55, 255 }     
#define GREEN      (Color){ 0, 228, 48, 255 }      
#define LIME       (Color){ 0, 158, 47, 255 }      
#define DARKGREEN  (Color){ 0, 117, 44, 255 }      
#define SKYBLUE    (Color){ 102, 191, 255, 255 }   
#define BLUE       (Color){ 0, 121, 241, 255 }     
#define DARKBLUE   (Color){ 0, 82, 172, 255 }      
#define PURPLE     (Color){ 200, 122, 255, 255 }   
#define VIOLET     (Color){ 135, 60, 190, 255 }    
#define DARKPURPLE (Color){ 112, 31, 126, 255 }    
#define BEIGE      (Color){ 211, 176, 131, 255 }   
#define BROWN      (Color){ 127, 106, 79, 255 }    
#define DARKBROWN  (Color){ 76, 63, 47, 255 }      
#define WHITE      (Color){ 255, 255, 255, 255 }   
#define BLACK      (Color){ 0, 0, 0, 255 }         
#define BLANK      (Color){ 0, 0, 0, 0 }           
#define MAGENTA    (Color){ 255, 0, 255, 255 }     
#define RAYWHITE   (Color){ 245, 245, 245, 255 }   

global_variable Vector2 Vector2_one   = {1, 1};
global_variable Vector2 Vector2_up    = {0, 1};
global_variable Vector2 Vector2_right = {1, 0};
global_variable Vector2 Vector2_zero  = {0, 0};

struct Gradient{
    u32 *colors;
    u32 colors_count;
};


f32 normalized(f32 number){
    if (number == 0) return 0;
    if (number > 0) return 1;
    return -1;
}

void normalize(f32 *number){
    if (*number == 0) *number = 0;
    if (*number > 0)  *number = 1;
    *number = -1;
}

f32 abs(f32 number){
    if (number < 0) return -number;
    return number;
}

i32 clamp(i32 value, i32 min, i32 max){
    if (value <= min){
        return min;
    }
    
    if (value >= max){
        return max;
    }
    
    return value;
}

f32 clamp(f32 value, f32 min, f32 max){
    if (value <= min){
        return min;
    }
    
    if (value >= max){
        return max;
    }
    
    return value;
}

f32 clamp01(f32 value){
    return clamp(value, 0.0f, 1.0f);
}

void clamp(f32 *value, f32 min, f32 max){
    if (*value <= min){
        *value = min;
    } else if (*value >= max){
        *value = max;
    }
}

Vector2 add(Vector2 first, Vector2 second){
    return {first.x + second.x, first.y + second.y};
}

void add(Vector2 *first, Vector2 second){
    first->x += second.x;
    first->y += second.y;
}

Vector2 subtract(Vector2 first, Vector2 second){
    return {first.x - second.x, first.y - second.y};
}

void subtract(Vector2 *first, Vector2 second){
    first->x -= second.x;
    first->y -= second.y;
}

Vector2 multiply(Vector2 vector, f32 value){
    return {vector.x * value, vector.y * value};
}

void multiply(Vector2 *vector, f32 value){
    vector->x *= value;
    vector->y *= value;
}

Vector2 multiply(Vector2 first, Vector2 second){
    return {first.x * second.x, first.y * second.y};
}

Vector2 divide(Vector2 vector, f32 value){
    return {vector.x / value, vector.y / value};
}

f32 sqr_magnitude(Vector2 vector){
    return vector.x * vector.x + vector.y * vector.y;
}

f32 magnitude(Vector2 vector){
    return sqrt(sqr_magnitude(vector));
}

Vector2 normalized(Vector2 vector){
    f32 len = magnitude(vector);
    
    if (len <= EPSILON || (abs(vector.x) <= EPSILON && abs(vector.y) <= EPSILON)){
        return {0, -1};
    }

    return divide(vector, len);
}

f32 dot(Vector2 v1, Vector2 v2){
    f32 result = (v1.x*v2.x + v1.y*v2.y);

    return result;
}

f32 fangle(Vector2 v1, Vector2 v2){
    return (acosf(dot(v1, v2) / (magnitude(v1) *  magnitude(v2)))) * RAD2DEG;
}

//angle of vector ({0, 1} it's 0)
f32 fangle(Vector2 v){
    return atan2f(v.x, v.y) * RAD2DEG;
}

// f32 min(f32 a, f32 b){
//     if (a <= b) return a;
//     return b;
// }

// f32 max(f32 a, f32 b){
//     if (a >= b) return a;
//     return b;
// }

void normalize(Vector2 *vector){
    *vector = divide(*vector, magnitude(*vector));
}

float lerp(float a, float b, float t){
    return ((float)1-t) * a + b * t;
}

int lerp(int a, int b, float t){
    return ((float)1-t) * a + b * t;
}

Vector2 lerp(Vector2 a, Vector2 b, float t){
    return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
}

Color lerp(Color c1, Color c2, f32 fraction){
    // unsigned char   r1 = (color1 >> 16) & 0xff;
    // unsigned char   r2 = (color2 >> 16) & 0xff;
    // unsigned char   g1 = (color1 >> 8) & 0xff;
    // unsigned char   g2 = (color2 >> 8) & 0xff;
    // unsigned char   b1 = color1 & 0xff;
    // unsigned char   b2 = color2 & 0xff;

    return  {(unsigned char)((c2.r - c1.r) * fraction + c1.r),
             (unsigned char)((c2.g - c1.g) * fraction + c1.g),
             (unsigned char)((c2.b - c1.b) * fraction + c1.b),
             (unsigned char)((c2.a - c1.a) * fraction + c1.a)};
}

int lerp_color(uint32_t color1, uint32_t color2, float fraction)
{
        unsigned char   r1 = (color1 >> 16) & 0xff;
        unsigned char   r2 = (color2 >> 16) & 0xff;
        unsigned char   g1 = (color1 >> 8) & 0xff;
        unsigned char   g2 = (color2 >> 8) & 0xff;
        unsigned char   b1 = color1 & 0xff;
        unsigned char   b2 = color2 & 0xff;

        return (int) ((r2 - r1) * fraction + r1) << 16 |
                (int) ((g2 - g1) * fraction + g1) << 8 |
                (int) ((b2 - b1) * fraction + b1);
}

float powe(float value, int power){
    if (power == 0) return 1;
    
    if (power & 1) return value * pow(value, power >> 1) * pow (value, power >> 1);
    else           return pow(value, power >> 1) * pow(value, power >> 1);
}

int factorial(int value){
    if (value <= 1) return 1;
    return value * factorial(value-1);
}

int bezie(int *numbers, int n, float t){
    int result = 0x00;
    for (int i = 0; i < n - 1; i++){
        result += (factorial(n) / (factorial(n - i) * factorial(i))) // Число сочетаний n по i
                  * pow((float)1 - t, n - i)
                  * pow(t, i) * numbers[i];
    }
    
    result += pow(t, n) * numbers[n - 1];
    
    return result;
}

int bezie_colors(uint32_t *colors, int colors_count, float fraction)
{
    int *reds   = (int *)malloc(colors_count * sizeof(int));    
    int *greens = (int *)malloc(colors_count * sizeof(int));    
    int *blues  = (int *)malloc(colors_count * sizeof(int));    
    
    for (int i = 0; i < colors_count; i++){
        reds  [i] = (colors[i] >> 16) & 0xff;
        greens[i] = (colors[i] >> 8)  & 0xff;
        blues [i] =  colors[i]        & 0xff;
    }
    
    int result =  bezie(reds, colors_count, fraction)   << 16 |
                  bezie(greens, colors_count, fraction) << 8  |
                  bezie(blues, colors_count, fraction);
    free(reds);
    free(greens);
    free(blues);
    return result;
}

int lerp_colors(uint32_t *colors, int colors_count, float fraction){
    //clamp(&fraction, (float)0, (float)0.9999999);
    int index  = lerp(0, colors_count-1, fraction);
    int color1 = colors[index];
    int color2 = colors[index+1];
    
    float t = (fraction - ((float)index/(colors_count-1))) * (colors_count-1);
    
    return lerp_color(color1, color2, t);
}

int lerp_gradient(Gradient gradient, float fraction){
    return lerp_colors(gradient.colors, gradient.colors_count, fraction);
}


//Ease functions
float EaseInOutQuad(float x){
    return x < 0.5f ? 2.0f * x * x : 1 - (f32)pow(-2 * x + 2, 2) / 2.0f;
}

float EaseOutQuint(float x){
    return 1.0f - pow(1.0f - x, 5);
}

float EaseOutElastic(float x){
    float c4 = (2 * PI) / 3.0f;
    
    return x == 0 ? 0
      : (x == 1
      ? 1
      : pow(2.0f, -10 * x) * sin((x * 10 - 0.75f) * c4) + 1.0f);
}

f32 EaseInCirc(f32 x){
    return 1 - sqrt(1 - pow(x, 2));
}

f32 EaseInQuint(f32 x){
    return x * x * x * x * x;
}

f32 EaseInSine(f32 x){
  return 1.0f - cos((x * PI) / 2.0f);
}
