#pragma once

global_variable u32 rnd_state = 0;

//lehmer
u32 rnd(u32 state)
{
    state += 0xe120fc15;
    u64 tmp;
    tmp = (u64)state * 0x4a39b70d;
    u32 m1 = (tmp >> 32) ^ tmp;
    tmp = (u64)m1 * 0x12fad5c9;
    u32 m2 = (tmp >> 32) * tmp;
    return m2;
	//return (u64)state * 48271 % 0x7fffffff;
}

u32 rnd()
{
    rnd_state = (1 + rnd_state) * 5;
	return (u64)rnd_state * 48271 % 0x7fffffff;
}

int rnd(int min, int max){
    rnd_state++;
    if (min == max) return max;
    return (rnd() % (max - min)) + min;
}

f32 rnd(f32 min, f32 max){
    rnd_state = (rnd_state + 1) * 3;
    if (min == max) return max;
    return ((f32)rnd() / (f32)(0x7FFFFFFF)) * (max - min) + min;
}

f32 rnd01(){
    return rnd(0.0f, 1.0f);
}

Vector2 rnd_on_circle(){
    f32 theta = rnd01() * 2 * PI;
    return {cosf(theta), sinf(theta)};
}

Vector2 rnd_in_circle(){
    f32 r = sqrtf(rnd01());
    f32 theta = rnd01() * 2 * PI;
    return {r * cosf(theta), r * sinf(theta)};
}

Vector2 rnd_in_box(Vector2 box){
    return {rnd(-box.x * 0.5f, box.x * 0.5f), rnd(-box.y * 0.5f, box.y * 0.5f)};
}
