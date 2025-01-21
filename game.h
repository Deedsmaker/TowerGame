#pragma once

enum Game_State{
    EDITOR,
    GAME,
    PAUSE
};

global_variable Game_State game_state = EDITOR;

void clean_up_scene();
void enter_game_state();
void enter_editor_state();

void bird_clear_formation(Bird_Enemy *bird);

void reload_level_files();

void assign_selected_entity(Entity *new_selected);

void close_create_box();

void undo_apply_vertices_change(Entity *entity, Undo_Action *undo_action);

void calculate_bounds(Entity *entity);
Bounds get_bounds(Array<Vector2, MAX_VERTICES> vertices, Vector2 pivot = {0.5f, 0.5f});

Entity *get_entity_by_id(i32 id);
Entity *get_entity_by_index(i32 index);

void update_editor_ui();
void update_editor();
void update_editor_entity(Entity *e);
void update_all_collision_cells();
void update_entity_collision_cells(Entity *entity);
void update_entities(f32 dt);

void activate_door(Entity *entity, b32 is_open);

void add_hitstop(f32 added, b32 can_go_over_limit = false);

void resolve_physics_collision(Vector2 *my_velocity, f32 my_mass, Vector2 &their_velocity, f32 their_mass, Vector2 normal = Vector2_zero);
f32 apply_physics_force(Vector2 velocity, f32 mass, Physics_Object *to_whom, Vector2 normal = Vector2_zero);

void move_vec_towards(Vector2 *current, Vector2 target, f32 speed, f32 dt);
Vector2 move_towards(Vector2 current, Vector2 target, f32 speed, f32 dt);
Vector2 move_by_velocity(Vector2 position, Vector2 target, Velocity_Move* settings, f32 dt);
//void rotate_towards(f32 *rotate_angle, f32 target, f32 speed, f32 dt);

b32 should_kill_player(Entity *entity);

inline Vector2 get_shoot_stoper_cross_position(Entity *entity);
void agro_enemy(Entity *entity);
void destroy_enemy(Entity *entity);
inline b32 is_enemy_can_take_damage(Entity *enemy_entity);
void kill_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, f32 particles_speed_modifier = 1);
void stun_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, b32 serious = false);

void setup_color_changer(Entity *entity);
void update_color_changer(Entity *entity);

void draw_game();

void draw_ui(const char *tag);
void draw_entities();
void draw_entity(Entity *e);

inline Vector2 global(Entity *e, Vector2 local_pos);
inline Vector2 global(Vector2 position, Vector2 local_pos);
Vector2 local (Entity *e, Vector2 global_pos);

Vector2 get_rotated_vector_90(Vector2 v, f32 clockwise);

Array<Vector2, MAX_VERTICES> get_normals(Array<Vector2, MAX_VERTICES> vertices);
void fill_arr_with_normals(Array<Vector2, MAX_VERTICES> *normals, Array<Vector2, MAX_VERTICES> vertices);

inline void calculate_collisions(void (respond_func)(Entity*, Collision), Entity *entity);

void resolve_collision(Entity *entity, Collision col);
//Array<Collision> get_collisions(Entity *entity);
void fill_collisions(Entity *entity, Array<Collision, MAX_VERTICES> *result);
Collision check_rectangles_col(Entity *entity1, Entity *entity2);
Collision get_nearest_ground_collision(Vector2 point, f32 radius);
b32 check_col_circles(Circle a, Circle b);

void print_hotkeys_to_console();

inline int table_next_avaliable(Hash_Table_Int<Entity> table, int index, FLAGS flags = 0);
inline int next_entity_avaliable(int index, Entity **entity, FLAGS flags);

inline Vector2 transform_texture_scale(Texture texture, Vector2 wish_scale);

void add_hitmark(Entity *entity, b32 need_to_follow, f32 scale_multiplier = 1, Color tint = WHITE);

Vector2 world_to_screen(Vector2 pos);
Vector2 world_to_screen_with_zoom(Vector2 position);
Vector2 get_texture_pixels_size(Texture texture, Vector2 game_scale);
Vector2 get_left_down_texture_screen_position(Texture texture, Vector2 world_position, Vector2 game_scale);
void draw_game_circle(Vector2 position, f32 radius, Color color);
void draw_game_triangle_strip(Array<Vector2, MAX_VERTICES> vertices, Vector2 position, Color color);
inline void draw_game_triangle_strip(Entity *entity);
inline void draw_game_triangle_strip(Entity *entity, Color color);
void draw_game_rect(Vector2 pos, Vector2 scale, Vector2 pivot, Color color);
void draw_game_rect(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color);
inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color);
inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, Color color);
void draw_game_line_strip(Entity *entity, Color color);
void draw_game_line_strip(Vector2 *points, int count, Color color);
void draw_game_texture(Texture tex, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, b32 flip = false);
void draw_game_line(Vector2 start, Vector2 end, float thick, Color color);
void draw_game_line(Vector2 start, Vector2 end, Color color);
void draw_game_ring_lines(Vector2 center, f32 inner_radius, f32 outer_radius, i32 segments, Color color, f32 start_angle = 0, f32 end_angle = 360);
void draw_game_triangle_lines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);

void draw_game_text(Vector2 position, const char *text, f32 size, Color color);

//void draw_anim(Anim *anim, Texture *textures);
void load_anim(Dynamic_Array<Texture> *frames, const char *name);

Entity* add_text(Vector2 pos, f32 size, const char *text);

void copy_entity(Entity *dest, Entity *src);

void init_light(Light *light);
i32 init_entity_light(Entity *entity, Light *copy_light = NULL, b32 free_light = false);

inline void loop_entities(void (func)(Entity*));
inline void init_loaded_entity(Entity *entity);
void init_entity(Entity *entity);

void add_explosion_light(Vector2 position, f32 radius, f32 grow_time, f32 shrink_time, Color color, i32 size = SMALL_LIGHT);

void add_rifle_projectile(Vector2 start_position, Vector2 velocity, Projectile_Type type);

void check_avaliable_ids_and_set_if_found(i32 *id);

Entity* add_entity(Entity *copy, b32 keep_id = false);
//Entity* add_entity(Vector2 pos, Vector2 scale, f32 rotation, FLAGS flags);
Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags);
Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Texture texture, FLAGS flags);
Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags);
Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags);
Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags);
Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags, Array<Vector2, MAX_VERTICES> vertices);

Particle_Emitter* add_emitter();
Particle_Emitter* add_emitter(Particle_Emitter *copy);

f32 zoom_unit_size();

inline Vector2 get_perlin_in_circle(f32 speed);

inline f32 get_explosion_radius(Entity *entity, f32 base_radius = 40);

Vector2 get_left_up_no_rot(Entity *e);
Vector2 get_left_up(Entity *e);
Vector2 get_right_down_no_rot(Entity *e);
Vector2 get_right_down(Entity *e);
Vector2 get_left_down_no_rot(Entity *e);
Vector2 get_left_down(Entity *e);
Vector2 get_right_up_no_rot(Entity *e);
Vector2 get_right_up(Entity *e);

void change_scale(Entity *entity, Vector2 new_scale);
void change_color(Entity *entity, Color new_color);
void add_scale(Entity *entity, Vector2 added);

void change_up(Entity *entity, Vector2 new_up);
void change_right(Entity *entity, Vector2 new_right);
void rotate_to(Entity *entity, f32 new_rotation);
void rotate(Entity *entity, f32 rotation);