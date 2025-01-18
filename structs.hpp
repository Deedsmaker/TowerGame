#define FLAGS u64
//#define EPSILON 0.0000000000000001f

#define FIXED_FPS 240
#define TARGET_FRAME_TIME (1.0f / FIXED_FPS)

#define MAX_VERTICES 8
#define MAX_COLLISIONS 128
#define MAX_ENTITY_EMITTERS 4
#define GRAVITY 100
#define PLAYER_MASS 10

enum Particle_Shape{
    SQUARE
};

struct Particle{
    b32 enabled = false;
    Particle_Shape shape = SQUARE;
    Vector2 position;
    Vector2 scale = {1, 1};
    Vector2 velocity;
    Vector2 original_scale;
    f32 lifetime;
    f32 max_lifetime;
    f32 gravity_multiplier = 1;
    
    //b32 colliding;
    
    Color color = YELLOW;
};

enum Particle_Spawn_Area{
    CIRCLE = 0,  
    BOX = 1
};

struct Particle_Emitter{
    Particle_Shape shape = SQUARE;
    
    b32 just_born = true;
    b32 enabled = true;
    b32 destroyed = false;
    b32 follow_entity = true;
    
    Particle_Spawn_Area spawn_type = CIRCLE;
    
    Vector2 local_position = {0, 0};
    Vector2 position = {0, 0};
    Vector2 last_emitted_position = {0, 0};
    Vector2 direction = Vector2_up;
    
    Vector2 spawn_offset = Vector2_zero;
    
    union{
        f32 spawn_radius;
        Vector2 spawn_area;
    };
    f32 gravity_multiplier = 1;
    
    b32 emitting;
    f32 emitting_timer;
    f32 over_time;
    f32 over_distance;
    b32 direction_to_move = false;
    
    u32 count_min = 10;
    u32 count_max = 50;
    f32 count_multiplier = 1;
    
    f32 speed_min = 10;
    f32 speed_max = 50;  
    f32 speed_multiplier = 1;
    
    f32 scale_min = 0.1f;
    f32 scale_max = 0.5f;
    f32 size_multiplier = 1;
    
    f32 spread = 0.2f;
    
    f32 lifetime_min = 0.5f;
    f32 lifetime_max = 2;
    f32 lifetime_multiplier = 1;
    
    f32 emitter_lifetime = 0;
    
    //f32 colliding_chance = 1.0f;
    
    Color color = YELLOW;
};

#define MAX_SINGLE_SOUND 16

struct Sound_Handler{
    Array<Sound, MAX_SINGLE_SOUND> buffer = Array<Sound, MAX_SINGLE_SOUND>();
    
    i32 current_index = 0;
    
    f32 base_volume = 0.7f;  
    f32 base_pitch = 1.0f;
    
    f32 volume_variation = 0.1f;
    f32 pitch_variation = 0.3f;
};

enum Flags : i64{
    GROUND            = 1 << 0,
    DRAW_TEXT         = 1 << 1,
    PLAYER            = 1 << 2,
    ENEMY             = 1 << 3,
    SWORD             = 1 << 4,
    BIRD_ENEMY        = 1 << 5,
    TEXTURE           = 1 << 6,
    PROJECTILE        = 1 << 7,
    PARTICLE_EMITTER  = 1 << 8,
    WIN_BLOCK         = 1 << 9,
    EXPLOSIVE         = 1 << 11,
    BLOCKER           = 1 << 12,
    STICKY_TEXTURE    = 1 << 13,
    PROPELLER         = 1 << 15,
    SHOOT_BLOCKER     = 1 << 16,
    DOOR              = 1 << 17,
    TRIGGER           = 1 << 18,
    SPIKES            = 1 << 19,
    PLATFORM          = 1 << 20,
    MOVE_SEQUENCE     = 1 << 21,
    DUMMY             = 1 << 22,
    CENTIPEDE         = 1 << 23,
    CENTIPEDE_SEGMENT = 1 << 24,
    SHOOT_STOPER      = 1 << 25,
    PHYSICS_OBJECT    = 1 << 26,
    BLOCK_ROPE        = 1 << 27,
    ROPE_POINT        = 1 << 28,
    JUMP_SHOOTER      = 1 << 29,
    LIGHT             = 1 << 30
};

struct Ground{
      
};

struct Physics_Object{
    Vector2 velocity = Vector2_zero;  
    f32 angular_velocity = 0;
    
    f32 mass = 100.0f;
    f32 gravity_multiplier = 2.0f;
    
    b32 rotate_by_velocity = true;
    b32 on_rope = false;
    f32 last_pick_rope_point_time = -14234423;
    
    i32 rope_id = -1;
    i32 up_rope_point_id = -1;
    i32 down_rope_point_id = -1;
    Vector2 rope_point = Vector2_zero;
};

struct Move_Point{
    Vector2 position = Vector2_zero;  
    Vector2 normal = Vector2_zero;
};

struct Enemy{
    b32 dead_man = false;  
    b32 in_agro = false;
    b32 in_stun = false;
    b32 just_awake = true;
    
    b32 was_in_stun = false;
    
    f32 died_time = 0;
    
    i32 hits_taken = 0;
    i32 max_hits_taken = 3;
    
    f32 stun_start_time = -1234;
    f32 max_stun_time = 1.0f;
    
    f32 last_hit_time = -123;
    
    f32 birth_time = 0;
    
    f32 sword_kill_speed_modifier = 1;
    
    f32 explosive_radius_multiplier = 1.0f;
    
    b32 blocker_clockwise = false;
    b32 blocker_immortal = false;
    i32 blocker_sticky_id = -1;
    
    b32 gives_ammo = true;
    b32 gives_full_ammo = false;
    
    Vector2 original_scale = {1, 1};
    
    b32 shoot_blocker_immortal = false;
    Vector2 shoot_blocker_direction = Vector2_up;
    
    Sound_Handler *explosion_sound = NULL;
    Sound_Handler *big_explosion_sound = NULL;
};

struct Jump_Shooter{
    f32 max_charging_time = 1.5f;
    f32 max_picking_point_time = 1.5f;
    
    Vector2 jump_direction = Vector2_up;

    struct shooter_states{
        f32 standing_start_time      = 0;
        f32 jump_start_time          = 0;
        f32 charging_start_time      = 0;
        f32 recoil_start_time        = 0;
        f32 picking_point_start_time = 0;
        f32 flying_start_time        = 0;
    
        b32 standing = true;
        b32 jumping = false;
        b32 charging = false;
        b32 in_recoil = false;
        b32 picking_point = false;
        b32 flying_to_point = false;
    };
    
    shooter_states states = {};
    
    Vector2 velocity = Vector2_zero;
    i32 current_index = 0;

    i32 shots_count = 10;  
    f32 spread = 45;
    
    // Internal, used to randomize and decide what we'll shoot if shoot_sword_blockers is picked
    b32 blocker_clockwise = false;
    f32 last_visual_blocker_direction_change_time = 0;
    
    i32 explosive_count                = 0;
    
    b32 shoot_sword_blockers           = false;
    // b32 shoot_sword_blockers_clockwise = false;
    // b32 shoot_sword_blockers_random_direction = false;
    b32 shoot_sword_blockers_immortal  = false;
    
    b32 shoot_bullet_blockers = false;
    
    Dynamic_Array<Move_Point> move_points;
    
    Particle_Emitter *trail_emitter;
    Particle_Emitter *flying_emitter;
};

struct Move_Sequence{
    Dynamic_Array<Vector2> points;  
    b32 moving = false;
    f32 speed = 10;
    Vector2 velocity = Vector2_zero;
    Vector2 wish_velocity = Vector2_zero;
    Vector2 wish_position = Vector2_zero;
    b32 just_born = true;
    
    b32 rotate = false;
    b32 loop = false;
    
    i32 current_index = -1;
    Vector2 moved_last_frame = Vector2_zero;
};

struct Door{
    Vector2 closed_position = Vector2_zero;
    Vector2 open_position   = Vector2_zero;
    b32 is_open = false;
    
    f32 triggered_time = -99999;
    
    f32 time_to_open = 3.0f;
    f32 time_to_close = 1.5f;
    
    Sound_Handler *open_sound = NULL;
};

enum Trigger_Action_Type{
    TRIGGER_SOME_ACTION = 1 << 0,  
    TRIGGER_LEVEL_LOAD  = 1 << 1
};

struct Trigger{
    Dynamic_Array<int> connected;
    Dynamic_Array<int> tracking;
    b32 player_touch = true;
    b32 kill_player = false;
    b32 open_doors = true;
    b32 track_enemies = false;
    b32 agro_enemies = true;
    
    b32 shows_entities = true;
    b32 starts_moving_sequence = true;
    
    b32 change_zoom = false;
    f32 zoom_value = 0.35f;
    
    b32 play_sound = false;
    char sound_name[128];
    
    b32 load_level = false;
    char level_name[128];
    
    b32 triggered = false;
    
    b32 lock_camera = false;
    b32 unlock_camera = false;
    Vector2 locked_camera_position = Vector2_zero;
};

struct Velocity_Move{
    f32 acceleration = 150;  
    f32 deceleration = 1000;
    
    f32 accel_damping = 100.0f;
    f32 decel_damping = 50.0f;
    
    f32 max_speed = 300;
    Vector2 velocity = Vector2_zero;
};

struct Propeller{
    f32 power = 1000;  
    
    Particle_Emitter *air_emitter = NULL;
};

struct Win_Block{
    Vector2 kill_direction = Vector2_up;
};

//highest - 80. sides - 200
#define MAX_BIRD_POSITIONS 12
Vector2 bird_formation_positions[MAX_BIRD_POSITIONS] = {
        {0, 80}, {-30, 50}, {30, 50}, {0, 40}, {-100, 30}, {100, 30}, {-80, 50}, {80, 50}, {-150, 20}, {150, 20}, {-200, 5}, {200, 5}
};

struct Bird_Slot{
    b32 occupied = false;  
    i32 index = -1;    
};

struct Bird_Enemy{
    i32 slot_index = -1;

    //Attacking state
    b32 attacking = false;
    f32 attack_start_time = 0;
    //f32 attack_timer = 0;
    f32 max_attack_time = 3.0f;

    //Charging attack state
    b32 charging = false;
    //f32 charging_attack_timer = 0;
    f32 charging_start_time = 0;
    f32 max_charging_time = 3.0f;

    //Roam state
    b32 roaming = true;
    //f32 roam_timer = 0;
    f32 roam_start_time = 0;
    f32 max_roam_time = 6.0f;
    f32 max_roam_speed = 300;
    f32 roam_acceleration = 10;

    Vector2 target_position;  
    Vector2 velocity = Vector2_zero;
    // Vector2 speed = V;
    
    Particle_Emitter *attack_emitter;
    Particle_Emitter *trail_emitter;
    Particle_Emitter *fire_emitter;
    
    Sound_Handler *attack_sound = NULL;
};

#define MAX_CENTIPEDE_SEGMENTS 64

struct Centipede_Segment{
    i32 previous_id = -1;
};

struct Centipede{
    Array<i32, MAX_CENTIPEDE_SEGMENTS> segments_ids = Array<i32, MAX_CENTIPEDE_SEGMENTS>();
    
    i32 segments_count = 32;
    b32 spikes_on_right = true;
    b32 spikes_on_left = false;
};

struct Sticky_Texture{
    b32 need_to_follow = false;
    i32 follow_id = -1;  
    Vector2 texture_position = Vector2_zero;
    f32 max_lifetime = 2.0f;
    Color line_color = SKYBLUE;
    
    f32 max_distance = 400;
    f32 birth_time = 0;
};

struct Color_Changer{
    b32 changing = false;
    b32 interpolating = false;
    
    f32 progress = 0;

    Color start_color = BLACK;
    Color target_color = BLACK;
    
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

struct Bounds{
    Vector2 size;  
    Vector2 offset;
};

struct Player{
    Array<Collision, MAX_COLLISIONS> collisions = Array<Collision, MAX_COLLISIONS>();
    
    Particle_Emitter *stun_emitter;
    
    b32 dead_man = false;
    
    f32 max_ground_angle = 60;
    
    f32 base_move_speed = 30.0f;  
    f32 ground_acceleration = 30;
    f32 ground_deceleration = 3;
    f32 air_acceleration    = 15;
    f32 air_deceleration    = 3;
    f32 friction = 30;
    f32 jump_force = 80;
    f32 gravity = 100;
    f32 gravity_mult = 1;
    f32 max_blood_amount = 100;
    
    f32 heavy_collision_time = 0;
    Vector2 heavy_collision_velocity = Vector2_zero;
    
    Vector2 sword_start_scale = {1.5f, 6};
    b32 is_sword_big = false;
    f32 blood_amount = 0;
    f32 blood_progress = 0;
    
    Vector2 plane_vector = {0, 1};
    Vector2 ground_normal = {0, 1};
    b32 grounded = false;
    
    b32 on_moving_object = false;
    Vector2 moving_object_velocity = Vector2_zero;
    
    //int ground_checker_index_offset = -1;
    int ground_checker_id = -1;
    int left_wall_checker_id = -1;
    int right_wall_checker_id = -1;
    
    Vector2 velocity = {0, 0};
    
    f32 since_jump_timer = 0;
    f32 since_airborn_timer = 0;
    //Sword
    int sword_entity_id = -1;
    f32 sword_rotation_speed = 5.0f;
    
    f32 sword_spin_direction = 0;
    f32 sword_angular_velocity = 0;  
    f32 sword_spin_progress = 0;
    
    b32 sword_hit_ground = false;
    
    //int sword_entity_index_offset = -1;
    
    //Rifle
    b32 rifle_active = false;
    b32 rifle_perfect_shot_avaliable = false;
    f32 rifle_weak_speed = 800;
    f32 rifle_strong_speed = 1400;
    //f32 rifle_current_power = 0;
    //f32 rifle_max_power = 100;
    //f32 rifle_power_progress = 0;
    f32 rifle_shake_start_time = 0;
    f32 rifle_activate_time = 0;
    f32 rifle_max_active_time = 3.0f;
    f32 rifle_shoot_time = 0;
    
    i32 rifle_perfect_shots_made = 0;
    i32 rifle_max_perfect_shots = 3;
    
    
    i32 ammo_count = 0;
    i32 ammo_charges = 0;
    i32 ammo_charges_for_count = 5;
    
    f32 strong_recoil_stun_start_time = -190321;
    f32 weak_recoil_stun_start_time = -123123;
    b32 in_stun = false;
    
    f32 current_move_speed = 0;
    
    Particle_Emitter *rifle_trail_emitter = NULL;
    
    Sound_Handler *rifle_hit_sound    = NULL;
    Sound_Handler *player_death_sound = NULL;
    Sound_Handler *sword_kill_sound   = NULL;
    Sound_Handler *sword_block_sound  = NULL;
    Sound_Handler *bullet_block_sound = NULL;
    Sound_Handler *rifle_switch_sound = NULL;
};

enum Projectile_Flags{
    DEFAULT = 1 << 1,  
    PLAYER_RIFLE = 1 << 2,
    JUMP_SHOOTER_PROJECTILE = 1 << 3
};

enum Projectile_Type{  
    WEAK = 0,
    MEDIUM = 1,
    STRONG = 2
};

struct Projectile{
    FLAGS flags;
    Projectile_Type type = WEAK;
    Vector2 velocity = {0, 0};
    f32 birth_time = 0;
    f32 max_lifetime = 5;
    b32 dying = false;
    
    f32 last_light_spawn_time = -112;
    //Particle_Emitter trail_emitter;
};

struct Entity{
    Entity();
    Entity(Vector2 _pos);
    Entity(Vector2 _pos, Vector2 _scale);
    Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags);
    Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);
    Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, Texture texture, FLAGS _flags);
    Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);
    Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags, Array<Vector2, MAX_VERTICES> _vertices);
    Entity(Entity *copy, b32 keep_id);

    i32 id = -1;
    b32 need_to_save = true;
    b32 visible = true;
    b32 hidden = false;
    //i32 index = -1;
    
    char name[128] = "unknown_name";

    b32 enabled = 1;
    
    f32 volume_multiplier = 1;
    
    Texture texture;
    char texture_name[64];
    Vector2 scaling_multiplier = {1, 1};
    
    b32 destroyed = 0;
    
    Array<Vector2, MAX_VERTICES> unscaled_vertices = Array<Vector2, MAX_VERTICES>();
    Array<Vector2, MAX_VERTICES> vertices = Array<Vector2, MAX_VERTICES>();
    
    Vector2 up = {0, 1};
    Vector2 right = {1, 0};
    
    FLAGS flags;
    FLAGS collision_flags = 0;
    //b32 need_to_destroy = 0;
    
    //lower - closer to camera
    i32 draw_order = 100;
    
    Vector2 position;
    Vector2 scale = {1, 1};
    Bounds bounds = {{0, 0}, {0, 0}};
    Vector2 pivot = {0.5f, 0.5f};
    f32 rotation = 0;
    
    Color color = WHITE;
    
    Color_Changer color_changer;
    
    b32 spawn_enemy_when_no_ammo = false;
    i32 spawned_enemy_id = -1;
    
    Entity *centipede_head;
    
    //Player player;
    Ground ground;
    Text_Drawer text_drawer;
    Enemy enemy;
    Bird_Enemy bird_enemy;
    Projectile projectile;
    Array<Particle_Emitter, MAX_ENTITY_EMITTERS> emitters = Array<Particle_Emitter, MAX_ENTITY_EMITTERS>();
    Win_Block win_block;
    Sticky_Texture sticky_texture;
    Propeller propeller;
    Door door;
    Trigger trigger;
    Move_Sequence move_sequence;
    Centipede centipede;
    Physics_Object physics_object;
    Jump_Shooter jump_shooter;
    
    i32 light_index = -1;
};

enum Light_Size_Flags{
    ULTRA_SMALL_LIGHT = 1 << 0,
    SMALL_LIGHT       = 1 << 1,  
    MEDIUM_LIGHT      = 1 << 2,
    BIG_LIGHT         = 1 << 3,
    HUGE_LIGHT        = 1 << 4,
    GIANT_LIGHT       = 1 << 5
};

struct Light{
    b32 exists = false;
    Vector2 position = Vector2_zero;


    i32 connected_entity_id = -1;
    
    FLAGS additional_shadows_flags = 0;
    
    i32 shadows_size_flags     = SMALL_LIGHT;
    i32 backshadows_size_flags = SMALL_LIGHT;
    
    Color color = WHITE;
    
    i32 shadows_size     = 256;
    i32 backshadows_size = 256;
    // i32 geometry_size    = 1024;
    
    f32 opacity = 1.0f;
    f32 power = 1.0f;
    f32 radius           = 150.0f;
    b32 make_shadows     = true;
    b32 make_backshadows = true;
    
    b32 bake_shadows = false;
    f32 last_bake_time = -12;
    
    //temp light options
    f32 birth_time = -12;
    f32 target_radius = 150;
    f32 grow_time = 0;
    f32 shrink_time = 0;
    
    RenderTexture shadowmask_rt;
    RenderTexture backshadows_rt;
    // RenderTexture geometry_rt;
};

global_variable Player player_data;

struct Spawn_Object{
    char name[64];
    Entity entity;
};

#define MAX_COLLISION_CELL_OBJECTS 256

struct Collision_Grid_Cell{
    Array<i32, MAX_COLLISION_CELL_OBJECTS> entities_ids = Array<i32, MAX_COLLISION_CELL_OBJECTS>();
};

struct Collision_Grid{  
    Vector2 origin = Vector2_zero;
    Vector2 size = {4000, 2000};
    Vector2 cell_size = {40, 40};
    Collision_Grid_Cell *cells = NULL;
};

//scale 150 should be full screen;

struct Cam{
    Vector2 position = Vector2_zero;
    Vector2 target = Vector2_zero;
    //For culling
    Vector2 view_position = Vector2_zero;
    float rotation = 0;
    
    i32 width = 1600;
    i32 height = 900;
    
    //Shake
    f32 trauma = 0;
    f32 trauma_decrease_rate = 1.5f;
    
    b32 locked = false;
    
    Camera2D cam2D = {};
    f32 target_zoom = 0.35f;
    f32 unit_size;
    
    Velocity_Move move_settings = {};
};

//definition in particles.hpp
struct Particle;
struct Particle_Emitter;

struct Core{
    struct Time{      
        f32 target_dt = TARGET_FRAME_TIME;
        f32 previous_dt = 0;
        f32 dt = 0;  
        f32 fixed_dt = 0;
        f32 unscaled_dt = 0;
        f32 real_dt = 0;
        f32 time_scale = 1;
        f32 target_time_scale = 1;
        f32 game_time = 0;
        f32 app_time = 0;
        f32 hitstop = 0;
    };
    
    Time time;
};

#define MAX_ENTITIES 10000

struct Context{
    Hash_Table_Int<Entity>          entities  = Hash_Table_Int<Entity>();
    Dynamic_Array<Entity>           entities_draw_queue = Dynamic_Array<Entity>(10000);
    Dynamic_Array<Particle>         particles = Dynamic_Array<Particle>(20000);
    Dynamic_Array<Particle_Emitter> emitters  = Dynamic_Array<Particle_Emitter>(1000);
    
    Dynamic_Array<Light> lights = Dynamic_Array<Light>(256);
    i32 temp_lights_count = 64;
    // We should set it in beginning 
    i32 entity_lights_start_index = -1;
    
    Bird_Slot bird_slots[MAX_BIRD_POSITIONS];
    f32 last_bird_attack_time = -11110;
    f32 last_jump_shooter_attack_time = -11110;
    f32 last_collision_cells_clear_time = -2;
    
    b32 we_got_a_winner = false;
    b32 just_entered_game_state = false;
    b32 baked_shadows_this_frame = false;
    // Vector2 unit_screen_size;
    
    char current_level_name[256];
    
    Collision_Grid collision_grid;
    i32 collision_grid_cells_count = 0;
    
    i32 shoot_stopers_count = 0;
    
    Cam cam = {};
};

struct Render{
    // Shader base_light_shader;  
    // Shader ray_tracer_shader;  
    // Shader light_sampler_shader;  
    Shader lights_shader;
    Shader test_shader;
    
    // RenderTexture ray_collision_render_texture;
    // RenderTexture rays_render_texture;
    RenderTexture lights_buffer_render_texture;
    RenderTexture main_render_texture;
};

enum Hold_Flags{
    UP        = 1 << 1,
    DOWN      = 1 << 2,
    RIGHT     = 1 << 3,
    LEFT      = 1 << 4,
    SPIN_DOWN = 1 << 8,
};

enum Press_Flags{
    JUMP          = 1 << 1,
    SHOOT         = 1 << 2,
    SPIN          = 1 << 3,
    SPIN_RELEASED = 1 << 4,
    SWORD_BIG     = 1 << 5
};

struct Input{
    Vector2 direction;
    Vector2 tap_direction;
    Vector2 mouse_position;
    Vector2 mouse_delta;
    f32     mouse_wheel;
    
    Vector2 sum_mouse_delta = Vector2_zero;
    f32     sum_mouse_wheel = 0;
    Vector2 sum_direction = Vector2_zero;
    FLAGS hold_flags = 0;
    FLAGS press_flags = 0;
};

struct Level{
    Context context;  
};

struct Circle{
    Vector2 position;  
    f32 radius;
};

#define MAX_UNDOS 256

struct Undo_Action{
    //Entity *entity;
    int entity_id = -1;

    Entity deleted_entity;
    b32    entity_was_deleted = false;
    
    Entity spawned_entity;
    b32    entity_was_spawned = false;
    
    Vector2 position_change = {0, 0};  
    Vector2 scale_change = {0, 0};
    Array<Vector2, MAX_VERTICES> vertices_change = Array<Vector2, MAX_VERTICES>();
    Array<Vector2, MAX_VERTICES> unscaled_vertices_change = Array<Vector2, MAX_VERTICES>();
    f32 rotation_change = 0;
    i32 draw_order_change = 0;
};

struct Editor{
    f32 in_editor_time = 0;

    Array<Entity*, 30> place_cursor_entities = Array<Entity*, 30>();
    
    Array<Undo_Action, MAX_UNDOS> undo_actions = Array<Undo_Action, MAX_UNDOS>();
    int max_undos_added;

    b32 update_cam_view_position = true;

    b32 create_box_active = false;
    b32 create_box_closing = false;
    i32 create_box_selected_index = 0;
    f32 create_box_lifetime = 0;
    f32 create_box_slide_time = 0.25f;
    Vector2 create_box_open_mouse_position = {0, 0};

    Vector2 dragging_start;
    Vector2 scaling_start;
    f32     rotating_start;
    Array<Vector2, MAX_VERTICES> vertices_start = Array<Vector2, MAX_VERTICES>();
    Array<Vector2, MAX_VERTICES> unscaled_vertices_start = Array<Vector2, MAX_VERTICES>();
    
    b32 is_scaling_entity = false;
    b32 is_rotating_entity = false;

    Entity  *selected_entity = NULL;
    int selected_entity_id;
    Entity  *dragging_entity = NULL;
    int dragging_entity_id;
    Entity  *moving_vertex_entity = NULL;
    int moving_vertex_entity_id;
    Entity  *cursor_entity = NULL;
    
    Entity copied_entity;
    b32 is_copied;
    
    b32 need_validate_entity_pointers = false;
    
    // Vector2 *last_selected_vertex;
    // int last_selected_vertex_index;
    Vector2 *moving_vertex = NULL;
    int moving_vertex_index;
    
    b32 ruler_active = false;
    Vector2 ruler_start_position;
    
    b32 objets_selector_active = false;
    
    b32 selected_this_click = 0;
    
    f32 dragging_time = 0;
    
    Collision last_collision;
    
    f32 last_autosave_time = 0;
    
    Vector2 player_spawn_point = {0, 0};
    Vector2 last_click_position = {0, 0};
    f32 last_click_time = 0;
    
    //----------ui-----------------
    b32 draw_entity_settings = true;
    b32 draw_trigger_settings = false;
    b32 draw_enemy_settings = false;
    b32 draw_centipede_settings = false;
    b32 draw_jump_shooter_settings = false;
    b32 draw_door_settings = false;
    b32 draw_light_settings = false;
};

struct Debug{
    b32 free_cam = false;
    f32 last_zoom = 0.35f;

    b32 draw_player_collisions = false;  
    b32 draw_player_speed = false;
    b32 draw_rotation = false;
    b32 draw_scale = false;
    b32 draw_directions = false;
    b32 draw_up_right = false;
    b32 draw_bounds = false;
    b32 draw_position = false;
    b32 draw_areas_in_game = false;
    b32 draw_collision_grid = false;
    
    b32 info_fps = true;
    b32 info_spin_progress = true;
    b32 info_blood_progress = true;
    b32 info_particle_count = true;
    b32 info_emitters_count = false;
    b32 info_player_speed = true;
    
    b32 infinite_ammo = false;
    b32 enemy_ai = true;
    b32 god_mode = false;
};

struct Console_Command{
    char name[MEDIUM_STR_LEN];
    void (*func)() = NULL;
    void (*func_arg)(char*) = NULL;
};

struct Console{   
    b32 is_open = false;
    Dynamic_Array<Console_Command> commands;
    Dynamic_Array<Medium_Str> args;
    
    Dynamic_Array<Medium_Str> level_files;
    
    Dynamic_Array<Medium_Str> history;
    int history_max = 0;
    
    String str = String();
    f32 closed_time = 0;
    f32 opened_time = 0;
    f32 open_progress = 0;
};
