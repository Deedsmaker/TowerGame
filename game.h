#pragma once

void update_editor();
void update_entities();

void update_color_changer(Entity *entity);

void draw_game();

void draw_entities();

Vector2 global(Entity e, Vector2 local_pos);
Vector2 local(Entity e, Vector2 global_pos);

Vector2 world_to_screen(Vector2 pos);
void draw_game_circle(Vector2 position, f32 radius, Color color);
void draw_game_triangle_strip(Entity entity);
void draw_game_rect(Vector2 pos, Vector2 scale, Vector2 pivot, Color color);
void draw_game_rect(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color);
void draw_game_texture(Texture tex, Vector2 pos, Vector2 scale, Vector2 pivot, Color color);
void draw_game_line(Vector2 start, Vector2 end, float thick, Color color);

void draw_game_text(Vector2 position, const char *text, f32 size, Color color);

//void draw_anim(Anim *anim, Texture *textures);
void load_anim(Array<Texture> *frames, const char *name);

Entity* add_text(Vector2 pos, f32 size, const char *text);

Entity* add_entity(Vector2 pos, Vector2 scale, f32 rotation, FLAGS flags);
Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags);
Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags);
Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags);
Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags, Array<Vector2> vertices);

f32 zoom_unit_size();

Vector2 get_left_up_no_rot(Entity e);
Vector2 get_left_up(Entity e);
Vector2 get_right_down_no_rot(Entity e);
Vector2 get_right_down(Entity e);
Vector2 get_left_down_no_rot(Entity e);
Vector2 get_left_down(Entity e);
Vector2 get_right_up_no_rot(Entity e);
Vector2 get_right_up(Entity e);

void change_scale(Entity *entity, Vector2 new_scale);
void add_scale(Entity *entity, Vector2 added);

void change_up(Entity *entity, Vector2 new_up);
void change_right(Entity *entity, Vector2 new_right);
void rotate_to(Entity *entity, f32 new_rotation);
void rotate(Entity *entity, f32 rotation);
