#define FLAGS i32
//#define EPSILON 0.0000000000000001f

#define MAX_VERTICES 8
#define MAX_COLLISIONS 32

enum Flags{
    GROUND = 1 << 0,
    DRAW_TEXT = 1 << 1,
    PLAYER = 1 << 2,
    ENEMY = 1 << 3,
    SWORD = 1 << 4
};

struct Ground{
      
};

struct Enemy{
    b32 dead_man = false;  
};

struct Color_Changer{
    b32 changing = false;
    b32 interpolating = false;
    
    f32 progress = 0;

    Color start_color;
    Color target_color;
    
    f32 change_time = 2.0f;
};

struct Text_Drawer{
    const char *text;  
    f32 size = 30;
};

struct Entity;

struct Collision{
    b32 collided;
    f32 overlap;
    Entity *other_entity;
    
    Vector2 normal;    
    Vector2 point;
    Vector2 dir_to_first;
};

struct Player{
    Array<Collision, MAX_COLLISIONS> collisions = Array<Collision, MAX_COLLISIONS>();
    
    f32 max_ground_angle = 45;
    
    f32 base_move_speed = 30.0f;  
    f32 ground_acceleration = 30;
    f32 ground_deceleration = 3;
    f32 air_acceleration    = 15;
    f32 air_deceleration    = 3;
    f32 friction = 30;
    f32 jump_force = 60;
    f32 gravity = 100;
    f32 gravity_mult = 1;
    
    Vector2 sword_start_scale = {1.5f, 7};
    Vector2 plane_vector = {0, 1};
    Vector2 ground_normal = {0, 1};
    b32 grounded = false;
    
    int ground_checker_index_offset = -1;
    
    Vector2 velocity = {0, 0};
    
    f32 since_jump_timer = 0;
    f32 since_airborn_timer = 0;
    //Sword
    f32 sword_rotation_speed = 5.0f;
    f32 sword_attack_time = 0.15f;
    f32 sword_cooldown = 0.5f;
    f32 sword_spin_direction = 0;
    
    int sword_entity_index_offset = -1;
    
    f32 sword_attack_countdown;
    f32 sword_cooldown_countdown;
    f32 sword_angular_velocity = 0;  
    f32 sword_spin_speed_progress = 0;
    
    f32 current_move_speed;
};

struct Entity{
    Entity();
    Entity(Vector2 _pos);
    Entity(Vector2 _pos, Vector2 _scale);
    Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags);
    Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);
    Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);
    Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags, Array<Vector2, MAX_VERTICES> _vertices);
    Entity(Entity *copy);

    i32 id = -1;
    i32 index = -1;

    b32 enabled = 1;
    
    b32 destroyed = 0;
    
    Array<Vector2, MAX_VERTICES> vertices = Array<Vector2, MAX_VERTICES>();
    
    Vector2 up = {0, 1};
    Vector2 right = {1, 0};
    
    FLAGS flags;
    //b32 need_to_destroy = 0;
    
    //lower - closer to camera
    i32 draw_order = 1;
    
    Vector2 position;
    Vector2 scale = {1, 1};
    Vector2 bounds = {1, 1};
    Vector2 pivot = {0.5f, 0.5f};
    f32 rotation = 0;
    
    Color color = WHITE;
    
    Color_Changer color_changer;
    
    Player player;
    Ground ground;
    Text_Drawer text_drawer;
    Enemy enemy;
};

//scale 150 should be full screen;

struct Cam{
    Vector2 position;
    float rotation;
    
    Camera2D cam2D = {};
};

//definition in particles.hpp
struct Particle;
struct Particle_Emitter;

struct Context{
    Dynamic_Array<Entity>           entities  = Dynamic_Array<Entity>(1000);
    Dynamic_Array<Particle>         particles = Dynamic_Array<Particle>(100000);
    Dynamic_Array<Particle_Emitter> emitters  = Dynamic_Array<Particle_Emitter>(1000);

    Vector2 unit_screen_size;
    
    Cam cam = {};
};

struct Input{
    Vector2 direction;
    Vector2 mouse_position;
    Vector2 mouse_delta;
    f32     mouse_wheel;
};

struct Level{
    Context *context;  
};

struct Circle{
    Vector2 position;  
    f32 radius;
};

struct Editor{
    Entity  *selected_entity;
    int selected_entity_id;
    Entity  *dragging_entity;
    int dragging_entity_id;
    Entity  *moving_vertex_entity;
    int moving_vertex_entity_id;
    Entity  *cursor_entity;
    
    Entity copied_entity;
    b32 is_copied;
    
    b32 need_validate_entity_pointers = false;
    
    Vector2 *last_selected_vertex;
    int last_selected_vertex_index;
    Vector2 *moving_vertex;
    int moving_vertex_index;
    
    b32 ruler_active = false;
    Vector2 ruler_start_position;
    
    b32 objets_selector_active = false;
    
    b32 selected_this_click = 0;
    
    f32 dragging_time = 0;
    
    Collision last_collision;
    
    Vector2 player_spawn_point = {0, 0};
};

struct Debug{
    b32 draw_player_collisions = false;  
    b32 draw_player_speed = false;
    b32 draw_fps = true;
};