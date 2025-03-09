#define FLAGS u64
//#define EPSILON 0.0000000000000001f

#define FIXED_FPS 240
#define TARGET_FRAME_TIME (1.0f / FIXED_FPS)

#define MAX_VERTICES 8
// #define MAX_COLLISIONS 128
#define MAX_ENTITY_EMITTERS 8
#define GRAVITY 100
#define PLAYER_MASS 10

struct Level_Context;

#define LINE_TRAIL_MAX_POINTS 128

struct Line_Trail{
    b32 occupied = false;
    Array<Vector2, LINE_TRAIL_MAX_POINTS> positions = Array<Vector2, LINE_TRAIL_MAX_POINTS>();  
    Vector2 last_added_position = Vector2_zero;
    i32 start_index = 0;
};

enum Particle_Shape{
    SQUARE,
    PARTICLE_TEXTURE,
    PARTICLE_LINE
};

struct Particle{
    b32 enabled = false;
    Particle_Shape shape = SQUARE;
    Vector2 position = Vector2_zero;
    Vector2 scale = {1, 1};
    Vector2 velocity = Vector2_zero;
    Vector2 original_scale = Vector2_one;
    f32 lifetime = 0;
    f32 max_lifetime = 0;
    
    i32 line_trail_index = -1;
    i32 trail_emitter_index = -1;
    
    f32 rotation = 0;
    
    Color start_color = PINK;
    Color color = YELLOW;
};

enum Particle_Spawn_Area{
    CIRCLE = 0,  
    BOX = 1
};

#define MAX_SMALL_COUNT_PARTICLE_EMITTERS 1024
#define MAX_MEDIUM_COUNT_PARTICLE_EMITTERS 256
#define MAX_BIG_COUNT_PARTICLE_EMITTERS 8

#define MAX_SMALL_COUNT_PARTICLES 128
#define MAX_MEDIUM_COUNT_PARTICLES 256
#define MAX_BIG_COUNT_PARTICLES 2048

#define SMALL_COUNT_PARTICLES_START_INDEX 0
constexpr i32 MEDIUM_COUNT_PARTICLES_START_INDEX = MAX_SMALL_COUNT_PARTICLES * MAX_SMALL_COUNT_PARTICLE_EMITTERS;
constexpr i32 BIG_COUNT_PARTICLES_START_INDEX    = MEDIUM_COUNT_PARTICLES_START_INDEX + MAX_MEDIUM_COUNT_PARTICLES * MAX_MEDIUM_COUNT_PARTICLE_EMITTERS;

constexpr i32 MAX_PARTICLES = BIG_COUNT_PARTICLES_START_INDEX + MAX_BIG_COUNT_PARTICLES * MAX_BIG_COUNT_PARTICLE_EMITTERS;

// @OPTIMIZATION:
// Right now particles in emitters are stored as simple static array so Particle_Emmiter occupies many memory, which
// goes heavily on cache. When we'll come to memory arenas we can allocate memory for all particles and in emitter
// just keep pointer to start, so he can access that without storing all in himself.
// That generally makes it harder for single emmiter to get his particles, but in the broad sceme - emitter most likely
// will not be active, so reduced memory capacity will be good.
//
//!!!!!!!!!!!!
// Tried that. There's huge problem. During gameplay we *will* be creating and destroying many emitters and that approach
// allows us only clear full array of emitters by zero-ing watermark. 
// Solution would be to initialize in beginning all dynamic emitters that we will create and destroy. 
// That's obviously adds a lot of friction and things to remember.
// Actually... All we 'really' have to do is after initializing particle call a function that will add some amount of it
// to emitters array (and up watermark in level context). Next we only need to know how to find them on add 
//
// Right now we'll go with static arrays per emitter. We'll see if i can figure out something simple so we don't keep 
// everything in emitter. If that will happen - that will be after i'll work with arenas.
//
// Actually... We'll go with one particles array and three groups (big_count, medium_count, small_count). 
// That will be the equivalent of three static arrays, but in one block in memory with just marks.
// Also we will have small emitters to go through.
//

enum Particle_Emitter_Count{
    SMALL_PARTICLE_COUNT,  
    MEDIUM_PARTICLE_COUNT,
    BIG_PARTICLE_COUNT
};

struct Particle_Emitter{
    char tag_16[16] = "Untagged";    

    // These just 'copy' emitters.
    Array<Particle_Emitter*, 6> additional_emitters = Array<Particle_Emitter*, 6>();
    Particle_Emitter *particle_trail_emitter = NULL;

    i32 connected_entity_id = -1;
    i32 index = -1;

    b32 occupied = false;
    Particle_Shape shape = SQUARE;
    Particle_Emitter_Count count_type = SMALL_PARTICLE_COUNT;
    
    b32 should_extinct = false;
    
    Texture texture = {};
    f32 line_length_multiplier = 1.0f;
    f32 line_width = 1.0f;
    
    b32 individual_noise_movement = false;
    f32 noise_speed = 1.0f;
    f32 noise_power = 10;
    b32 random_movement = true;
    
    b32 stop_before_death = false;
    f32 lifetime_t_to_start_stopping = 0.5f;
    
    b32 fade_till_death = false;
    
    b32 grow_till_death = false;
    b32 grow_after_birth    = false;
    b32 shrink_before_death = true;
    
    i32 alive_particles_count = 0;
    
    i32 particles_start_index  = -1;
    i32 particles_max_index    = -1;
    i32 last_added_index = -1;
    
    b32 particle_line_trail = false;
    
    b32 just_born = true;
    b32 enabled = false;
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
    
    f32 emitting_timer = 0;
    f32 over_time = 0;
    f32 over_distance = 0;
    b32 direction_to_move = false;
    
    b32 should_collide = false;
    
    f32 rotation_multiplier = 1.0f;
    
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
    // f32 emitter_max_lifetime = 0;
    
    Color color = YELLOW;
};

struct Texture_Data{
    char name[64] = "\0";  
    Texture texture = {};
};

#define MAX_SINGLE_SOUND 16

struct Sound_Handler{
    char name[64] = "\0";
    Array<Sound, MAX_SINGLE_SOUND> buffer = Array<Sound, MAX_SINGLE_SOUND>();
    
    i32 current_index = 0;
    
    f32 base_volume = 0.7f;  
    f32 base_pitch = 1.0f;
    
    f32 volume_variation = 0.1f;
    f32 pitch_variation = 0.3f;
};

enum Flags : u64{
    GROUND              = 1 << 0,
    DRAW_TEXT           = 1 << 1,
    PLAYER              = 1 << 2,
    ENEMY               = 1 << 3,
    SWORD               = 1 << 4,
    BIRD_ENEMY          = 1 << 5,
    TEXTURE             = 1 << 6,
    PROJECTILE          = 1 << 7,
    PARTICLE_EMITTER    = 1 << 8,
    WIN_BLOCK           = 1 << 9,
    SWORD_SIZE_REQUIRED = 1 << 10,
    EXPLOSIVE           = 1 << 11,
    BLOCKER             = 1 << 12,
    STICKY_TEXTURE      = 1 << 13,
    NOTE                = 1 << 14,
    PROPELLER           = 1 << 15,
    SHOOT_BLOCKER       = 1 << 16,
    DOOR                = 1 << 17,
    TRIGGER             = 1 << 18,
    SPIKES              = 1 << 19,
    PLATFORM            = 1 << 20,
    MOVE_SEQUENCE       = 1 << 21,
    DUMMY               = 1 << 22,
    CENTIPEDE           = 1 << 23,
    CENTIPEDE_SEGMENT   = 1 << 24,
    SHOOT_STOPER        = 1 << 25,
    PHYSICS_OBJECT      = 1 << 26,
    BLOCK_ROPE          = 1 << 27,
    ROPE_POINT          = 1 << 28,
    JUMP_SHOOTER        = 1 << 29,
    LIGHT               = 1 << 30
};

struct Physics_Object{
    b32 simulating = true;

    Vector2 velocity = Vector2_zero;  
    Vector2 moved_last_frame = Vector2_zero;
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
    
    // Only if SWORD_SIZE_REQUIRED flag
    b32 big_sword_killable = true;
    i32 sword_required_sticky_id = -1;
    
    b32 gives_ammo = true;
    b32 gives_full_ammo = false;
    
    Vector2 original_scale = {1, 1};
    
    b32 shoot_blocker_immortal = false;
    Vector2 shoot_blocker_direction = Vector2_up;
    
    // Sound_Handler *explosion_sound = NULL;
    // Sound_Handler *big_explosion_sound = NULL;
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
    
    f32 not_found_ground_timer = 0;
    
    Vector2 velocity = Vector2_zero;
    i32 current_index = 0;

    i32 shots_count = 10;  
    f32 spread = 45;
    
    // Internal, used to randomize and decide what we'll shoot if shoot_sword_blockers is picked
    b32 blocker_clockwise = false;
    f32 last_visual_blocker_direction_change_time = 0;
    
    i32 explosive_count                = 0;
    
    b32 shoot_sword_blockers           = false;
    b32 shoot_sword_blockers_immortal  = false;
    
    b32 shoot_bullet_blockers = false;
    
    Dynamic_Array<Move_Point> move_points;
    
    i32 trail_emitter_index = -1;
    i32 flying_emitter_index = -1;
};

struct Move_Sequence{
    Dynamic_Array<Vector2> points;  
    b32 moving = false;
    f32 speed = 10;
    Vector2 velocity = Vector2_zero;
    Vector2 wish_velocity = Vector2_zero;
    Vector2 wish_position = Vector2_zero;
    b32 just_born = true;
    
    b32 speed_related_player_distance = false;
    f32 min_distance = 100;
    f32 max_distance = 300;
    f32 max_distance_speed = 20;
    
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
    
    // Sound_Handler *open_sound = NULL;
};

enum Trigger_Action_Type{
    TRIGGER_SOME_ACTION = 1 << 0,  
    TRIGGER_LEVEL_LOAD  = 1 << 1
};

struct Trigger{
    Dynamic_Array<int> connected;
    Dynamic_Array<int> tracking;
    
    b32 die_after_trigger = false;
    
    b32 player_touch             = true;
    b32 kill_player              = false;
    b32 kill_enemies             = false;
    b32 open_doors               = true;
    b32 start_physics_simulation = true;
    b32 track_enemies            = false;
    b32 draw_lines_to_tracked    = false;
    b32 agro_enemies             = true;
    
    b32 shows_entities = true;
    b32 starts_moving_sequence = true;
    
    b32 start_cam_rails_horizontal = false;
    b32 start_cam_rails_vertical = false;
    b32 stop_cam_rails = false;
    Dynamic_Array<Vector2> cam_rails_points = Dynamic_Array<Vector2>();
    
    b32 change_zoom = false;
    f32 zoom_value = 0.35f;
    
    b32 play_sound = false;
    char sound_name[128] = "\0";
    
    b32 load_level = false;
    char level_name[128] = "\0";
    
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
    b32 spin_sensitive = false;
    
    i32 air_emitter_index = -1;
};

//highest - 80. sides - 100
#define MAX_BIRD_POSITIONS 8
Vector2 bird_formation_positions[MAX_BIRD_POSITIONS] = {
        {0, 80}, {-30, 50}, {30, 50}, {0, 40}, {-100, 30}, {100, 30}, {-80, 50}, {80, 50}
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
    f32 attacked_time = -12;
    f32 max_attack_time = 3.0f;

    //Charging attack state
    b32 charging = false;
    f32 charging_start_time = 0;
    f32 max_charging_time = 3.0f;

    //Roam state
    b32 roaming = true;
    f32 roam_start_time = 0;
    f32 max_roam_time = 6.0f;
    f32 max_roam_speed = 300;
    f32 roam_acceleration = 10;

    Vector2 target_position;  
    Vector2 velocity = Vector2_zero;
    
    i32 attack_emitter_index = -1;
    i32 trail_emitter_index = -1;
    i32 fire_emitter_index = -1;
    i32 collision_emitter_index = -1;
    
    // Sound_Handler *attack_sound = NULL;
};

#define MAX_CENTIPEDE_SEGMENTS 64

struct Centipede_Segment{
    i32 head_id = -1;
    i32 previous_id = -1;
};

struct Centipede{
    Array<i32, MAX_CENTIPEDE_SEGMENTS> segments_ids = Array<i32, MAX_CENTIPEDE_SEGMENTS>();
    
    i32 segments_count = 32;
    b32 spikes_on_right = true;
    b32 spikes_on_left = false;
    
    b32 all_segments_dead = false;
};

struct Sticky_Texture{
    b32 need_to_follow = false;
    i32 follow_id = -1;  
    f32 max_lifetime = 2.0f;
    
    b32 should_draw_texture = true;
    b32 should_draw_until_expires = false;
    
    b32 draw_line = false;
    Color line_color = SKYBLUE;
    
    f32 alpha = 1.0f;
    Vector2 base_size = {3, 3};
    
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
    // Array<Collision, MAX_COLLISIONS> collisions = Array<Collision, MAX_COLLISIONS>();
    i32 stun_emitter_index = -1;
    
    struct Timers{
        f32 died_time = -12;
        f32 jump_press_time = -12;
        f32 air_jump_press_time = -12;
        f32 wall_jump_time = -12;
        f32 since_jump_timer = 0;
        f32 since_airborn_timer = 0;
        f32 rifle_shake_start_time = 0;
        f32 rifle_activate_time = 0;
        f32 rifle_shoot_time = 0;
    };
    
    Timers timers = {};
    
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
    
    f32 coyote_time = 0.3f;
    f32 jump_buffer_time = 0.2f;
    f32 wall_jump_buffer_time = 0.2f;
    
    f32 heavy_collision_time = 0;
    Vector2 heavy_collision_velocity = Vector2_zero;
    
    Vector2 sword_start_scale = {1.5f, 6};
    b32 is_sword_big = false;
    f32 blood_amount = 0;
    f32 blood_progress = 0;
    
    Vector2 plane_vector = {0, 1};
    Vector2 ground_normal = {0, 1};
    Vector2 ground_point = {0, 0};
    b32 grounded = false;
    
    b32 on_moving_object = false;
    Vector2 moving_object_velocity = Vector2_zero;
    
    struct Connected_Entities_Ids{
        i32 ground_checker_id = -1;
        i32 left_wall_checker_id = -1;
        i32 right_wall_checker_id = -1;
        i32 sword_entity_id = -1;
    };
    Connected_Entities_Ids connected_entities_ids = {};
    
    Vector2 velocity = {0, 0};
    
    //Sword
    f32 sword_rotation_speed = 5.0f;
    
    b32 is_sword_will_hit_explosive = false;
    
    f32 sword_spin_direction = 0;
    f32 sword_angular_velocity = 0;  
    f32 sword_spin_progress = 0;
    
    b32 sword_hit_ground = false;
    
    //Rifle
    b32 rifle_active = false;
    b32 rifle_perfect_shot_avaliable = false;
    f32 rifle_weak_speed = 800;
    f32 rifle_strong_speed = 1400;
    f32 rifle_max_active_time = 3.0f;
    
    i32 rifle_perfect_shots_made = 0;
    i32 rifle_max_perfect_shots = 3;
    
    i32 ammo_count = 0;
    i32 ammo_charges = 0;
    i32 ammo_charges_for_count = 5;
    
    // f32 strong_recoil_stun_start_time = -190321;
    f32 weak_recoil_stun_start_time = -12;
    b32 in_stun = false;
    
    f32 current_move_speed = 0;
    
    i32 rifle_trail_emitter_index = -1;
    i32 tires_emitter_index = -1;
    
    // Sound_Handler *rifle_hit_sound    = NULL;
    // Sound_Handler *player_death_sound = NULL;
    // Sound_Handler *sword_kill_sound   = NULL;
    // Sound_Handler *sword_block_sound  = NULL;
    // Sound_Handler *bullet_block_sound = NULL;
    // Sound_Handler *rifle_switch_sound = NULL;
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
    
    i32 trail_emitter_index = -1;
    
    Array<i32, 8> already_hit_ids = Array<i32, 8>();
    f32 last_light_spawn_time = -112;
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
    Entity(Entity *copy, b32 keep_id, Level_Context *copy_entity_level_context = NULL, b32 should_init_entity = true);

    i32 id = -1;
    b32 need_to_save = true;
    b32 visible = true;
    b32 hidden = false;
    
    Level_Context *level_context;
    
    char name[128] = "unknown_name";

    b32 enabled = 1;
    
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
    
    Text_Drawer text_drawer;
    Enemy enemy;
    Bird_Enemy bird_enemy;
    Projectile projectile;
    Array<i32, MAX_ENTITY_EMITTERS> particle_emitters_indexes = Array<i32, MAX_ENTITY_EMITTERS>();
    Sticky_Texture sticky_texture;
    Propeller propeller;
    Door door;
    Trigger trigger;
    Move_Sequence move_sequence;
    Centipede centipede;
    Physics_Object physics_object;
    Jump_Shooter jump_shooter;
    i32 note_index = -1;
    
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
    
    b32 fire_effect = false;
    
    f32 opacity = 1.0f;
    f32 start_opacity = 1.0f;
    f32 power = 1.0f;
    f32 radius           = 150.0f;
    b32 make_shadows     = false;
    b32 make_backshadows = false;
    
    b32 bake_shadows = false;
    b32 baked = false;
    
    f32 last_bake_time = -12;
    
    //temp light options
    f32 birth_time = -12;
    f32 target_radius = 150;
    f32 target_power = 4;
    f32 grow_time = 0;
    f32 shrink_time = 0;
    
    RenderTexture shadowmask_rt;
    RenderTexture backshadows_rt;
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
    Vector2 size = {10000, 6000};
    Vector2 cell_size = {80, 80};
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
    f32 unit_size;
    
    //Shake
    Camera2D cam2D = {};
    f32 target_zoom = 0.35f;
};

//definition in particles.hpp
struct Particle;
struct Particle_Emitter;

struct Time{      
    f32 target_dt = TARGET_FRAME_TIME;
    f32 previous_dt = 0;
    f32 dt = 0;  
    f32 not_updated_accumulated_dt = 0;
    f32 fixed_dt = 0;
    f32 unscaled_dt = 0;
    f32 real_dt = 0;
    f32 time_scale = 1;
    f32 debug_target_time_scale = 1;
    f32 target_time_scale = 1;
    f32 game_time = 0;
    f32 app_time = 0;
    f32 hitstop = 0;
};

struct Core{
    Time time;
};

#define MAX_ENTITIES 10000

struct Speedrun_Timer{
    b32 level_timer_active = false;    
    b32 game_timer_active  = false;
    
    b32 paused = false;
    
    f32 time = 0;
};

struct Note{
    b32 occupied = false;
    char content[2048] = "\0";
    
    b32 draw_in_game = false;
    Color in_game_color = WHITE;
};

enum Death_Instinct_Reason{
    ENEMY_ATTACKING = 0,
    SWORD_WILL_EXPLODE = 1
};

#define MAX_LINE_TRAILS 128

struct Level_Context{
    b32 inited = false;
    char name[64] = "\0";

    Hash_Table_Int<Entity>          entities  = Hash_Table_Int<Entity>();
      
    Dynamic_Array<Particle>         particles = Dynamic_Array<Particle>(MAX_PARTICLES);
    Dynamic_Array<Particle_Emitter> particle_emitters  = Dynamic_Array<Particle_Emitter>(MAX_SMALL_COUNT_PARTICLE_EMITTERS + MAX_MEDIUM_COUNT_PARTICLE_EMITTERS + MAX_BIG_COUNT_PARTICLE_EMITTERS);
    Dynamic_Array<Line_Trail> line_trails = Dynamic_Array<Line_Trail>(MAX_LINE_TRAILS);
    
    Dynamic_Array<Note> notes = Dynamic_Array<Note>(128);
    
    Bird_Slot bird_slots[MAX_BIRD_POSITIONS];
    
    Dynamic_Array<Light> lights = Dynamic_Array<Light>(1024);
};

struct State_Context{
    struct Timers{
        f32 last_bird_attack_time = -11110;
        f32 last_jump_shooter_attack_time = -11110;
        f32 background_flash_time = -21;
        f32 last_collision_cells_clear_time = -2;
        f32 last_projectile_hit_time = -12;
    };
    Timers timers = {};
    
    struct Death_Instinct{
        f32 allowed_duration_without_cooldown = 0.5f;
        f32 duration = 2;
        f32 cooldown = 12;
        
        f32 start_time = -12;
        f32 cooldown_start_time = -12;
        f32 stop_time = -12;
        i32 threat_entity_id = -1;
        b32 played_effects = false;
        
        b32 was_in_cooldown = false;
        
        f32 angle_till_explode = 0;
        
        Death_Instinct_Reason last_reason = ENEMY_ATTACKING;
    };
    Death_Instinct death_instinct = {};
    
    struct Cam_State{
        f32 trauma = 0;
        f32 trauma_decrease_rate = 1.5f;
        b32 locked = false;
        b32 on_rails_horizontal = false;
        b32 on_rails_vertical   = false;
        i32 rails_trigger_id    = -1;
    };
    Cam_State cam_state = {};
      
    b32 we_got_a_winner = false;
    
    b32 in_pause_editor = false;
    b32 free_cam = false;
    
    f32 explosion_trauma = 0;
    i32 shoot_stopers_count = 0;
    
    i32 contiguous_projectile_hits_count = 0;
};

struct Session_Context{
    Dynamic_Array<Entity> entities_draw_queue = Dynamic_Array<Entity>(10000);
    
    // Dynamic_Array<Light> temp_lights = Dynamic_Array<Light>(512);
    
    i32 temp_lights_count = 512;
    //big lights are also in temp lights, it's first N 
    i32 big_temp_lights_count = 8;
    i32 huge_temp_lights_count = 4;
    // We should set it in beginning 
    i32 entity_lights_start_index = -1;
    
    i32 game_frame_count = 0;
    b32 playing_replay = false;
    
    b32 just_entered_game_state = false;
    b32 baked_shadows_this_frame = false;
    
    b32 updated_today = false;
    
    // Death instinct timers is in app time. (Because it's manipulating time!!).
    
    char current_level_name[256] = "\0";
    char previous_level_name[256] = "\0";
    
    Collision_Grid collision_grid;
    i32 collision_grid_cells_count = 0;
    
    
    Cam cam = {};
    
    Speedrun_Timer speedrun_timer = {};
};

struct Line{
    Vector2 start_position  = Vector2_zero;  
    Vector2 target_position = Vector2_zero;
    f32 thick = 0;
    Color color = PINK;
};

struct Ring_Lines{
    Vector2 center = Vector2_zero;  
    f32 inner_radius = 0;  
    f32 outer_radius = 0;  
    i32 segments = 12;  
    Color color = PINK;
};

struct Rect_Lines{
    Vector2 position = Vector2_zero;  
    Vector2 scale = Vector2_one;
    Vector2 pivot = {0.5f, 0.5f};
    f32 thick = 0;
    Color color = PINK;
};

struct Immediate_Texture{
    Texture texture = {};
    Vector2 position = Vector2_zero;
    Vector2 scale = Vector2_one;
    Vector2 pivot = {0.5f, 0.5f};
    f32 rotation = 0;
    Color color = WHITE;
};

struct Render{
    Dynamic_Array<Line> lines_to_draw = Dynamic_Array<Line>(128);
    Dynamic_Array<Ring_Lines> ring_lines_to_draw = Dynamic_Array<Ring_Lines>(32);
    Dynamic_Array<Rect_Lines> rect_lines_to_draw = Dynamic_Array<Rect_Lines>(32);
    Dynamic_Array<Immediate_Texture> textures_to_draw   = Dynamic_Array<Immediate_Texture>(32);
    
    Dynamic_Array<Light> lights_draw_queue = Dynamic_Array<Light>(128);

    Shader lights_shader;
    Shader test_shader;
    
    RenderTexture lights_buffer_render_texture;
    RenderTexture main_render_texture;
};

enum Hold_Flags{
    UP         = 1 << 1,
    DOWN       = 1 << 2,
    RIGHT      = 1 << 3,
    LEFT       = 1 << 4,
    SPIN_DOWN  = 1 << 8,
    SHOOT_DOWN = 1 << 9,
    SWORD_BIG_DOWN = 1 << 10
};

enum Press_Flags{
    JUMP           = 1 << 1,
    SHOOT          = 1 << 2,
    SPIN           = 1 << 3,
    SPIN_RELEASED  = 1 << 4,
    SWORD_BIG      = 1 << 5,
    SHOOT_RELEASED = 1 << 6  
};

struct Input{
    Vector2 direction             = Vector2_zero;
    Vector2 tap_direction         = Vector2_zero;
    Vector2 screen_mouse_position = Vector2_zero;
    Vector2 mouse_position        = Vector2_zero;
    Vector2 mouse_delta           = Vector2_zero;
    f32     mouse_wheel           = 0;
    
    Vector2 sum_mouse_delta = Vector2_zero;
    f32     sum_mouse_wheel = 0;
    Vector2 sum_direction   = Vector2_zero;
    FLAGS hold_flags        = 0;
    FLAGS press_flags       = 0;
    
    u32 rnd_state           = 0;
};

#define MAX_INPUT_RECORDS (FIXED_FPS * 60)
struct Replay_Frame_Data{
    Input frame_input = {};  
    // Time frame_time_data = {};
};

struct Level_Replay{
    Dynamic_Array<Replay_Frame_Data> input_record = Dynamic_Array<Replay_Frame_Data>(MAX_INPUT_RECORDS);
};

// struct Level{
//     Context context;  
// };

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

    f32 create_box_scrolled = 0;
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
    f32 last_zoom = 0.35f;

    b32 full_light = false;

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
    
    b32 drawing_stopped = false;
    
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
    void (*func_arg)(const char*) = NULL;
};

struct Console{   
    b32 is_open = false;
    Dynamic_Array<Console_Command> commands;
    Dynamic_Array<Medium_Str> args;
    
    Dynamic_Array<Medium_Str> level_files;
    
    Dynamic_Array<Medium_Str> history;
    int history_max = 0;
    
    String str = String();
    f32 closed_time = -12;
    f32 opened_time = -12;
    f32 open_progress = 0;
};