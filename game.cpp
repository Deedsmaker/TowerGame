#pragma once

#ifndef RELEASE_BUILD
    #define RELEASE_BUILD 0
#endif

#ifndef DEBUG_BUILD
    #define DEBUG_BUILD 0
#endif

//#define assert(a) (if (!a) (i32*)void*);
//#define assert(Expression) if(!(Expression)) {*(i32 *)0 = 0;}

global_variable Dynamic_Array<Collision> collisions_buffer        = Dynamic_Array<Collision>(256);

#include "game.h"
#include "../my_libs/perlin.h"

// #define ForEntities(entityext_avaliable(table, 0);  xx < table.max_count; xx = table_next_avaliable(table, xx+0))

#define ForEntities(entity, flags) Entity *entity = NULL; for (i32 index = next_entity_avaliable(current_level_context, 0, &entity, flags); index < current_level_context->entities.max_count && entity; index = next_entity_avaliable(current_level_context, index+1, &entity, flags)) 

#define ForEntitiesInContext(context, entity, flags) Entity *entity = NULL; for (i32 index = next_entity_avaliable(context, 0, &entity, flags); index < current_level_context->entities.max_count && entity; index = next_entity_avaliable(context, index+1, &entity, flags)) 

#define ArrayOfStructsToDefaultValues(arr) for (i32 arr_index = 0; arr_index < arr.max_count; arr_index++){ (*arr.get_ptr(arr_index)) = {};}

//#define For(arr, type, value) for(i32 ii = 0; ii < arr.count; ii++){ type value = arr.get(ii);

global_variable Input input = {};
global_variable Input replay_input = {};
// global_variable Level_Context editor_level_context = {};

#define MAX_LOADED_LEVELS 2
global_variable Level_Context loaded_levels_contexts[MAX_LOADED_LEVELS];
global_variable Level_Context *editor_level_context = NULL;
i32 current_editor_level_context_index = 0;
i32 last_loaded_editor_level_context_index = 0;

global_variable Level_Context game_level_context = {};
global_variable Level_Context checkpoint_level_context = {};
global_variable Level_Context loaded_level_context = {};
global_variable Level_Context undo_level_context = {};
global_variable Level_Context copied_entities_level_context = {};
global_variable Level_Context *current_level_context = NULL;
global_variable State_Context state_context = {};
global_variable Session_Context session_context = {};

global_variable Entity *checkpoint_player_entity;
global_variable Player checkpoint_player_data;
global_variable Time checkpoint_time;
global_variable State_Context checkpoint_state_context;
global_variable i32 checkpoint_trigger_id = -1;

global_variable Level_Replay level_replay = {};
global_variable Render render = {};
global_variable Console console = {};
global_variable Editor editor  = {}; 
global_variable Debug  debug  = {};

global_variable const char *first_level_name = "new_basics1";

global_variable Array<Vector2, MAX_VERTICES> global_normals = Array<Vector2, MAX_VERTICES>();

global_variable Entity mouse_entity;

global_variable Entity *player_entity;
global_variable b32 need_destroy_player = false;

global_variable f32 frame_rnd;
global_variable Vector2 frame_on_circle_rnd;

global_variable b32 clicked_ui = false;

global_variable b32 enter_game_state_on_new_level = false;

global_variable Dynamic_Array<Texture_Data> textures_array = Dynamic_Array<Texture_Data>(512);
global_variable Dynamic_Array<Sound_Handler> sounds_array = Dynamic_Array<Sound_Handler>(128);

global_variable b32 initing_game = false;

global_variable Array<Lightmap_Data, 1> lightmaps = Array<Lightmap_Data, 1>();

#include "../my_libs/random.hpp"
#include "particles.hpp"
#include "text_input.hpp"
#include "ui.hpp"

Player last_player_data = {};
Player death_player_data = {};

Cam global_cam_data = {};

void log_short(const char *str){
    Log_Message *new_log = debug.log_messages_short.add({});
    str_copy(new_log->data, str);
    new_log->birth_time = core.time.app_time;
}

inline void log_short(f32 value){
    log_short(text_format("%f", value));
}
inline void log_short(Vector2 value){
    log_short(text_format("{%f, %f}", value.x, value.y));
}

void setup_context_cam(Level_Context *level_context){
    level_context->cam.width = global_cam_data.width;
    level_context->cam.height = global_cam_data.height;
    level_context->cam.unit_size = global_cam_data.width / SCREEN_WORLD_SIZE; 
    level_context->cam.cam2D.target = cast(Vector2){ global_cam_data.width/2.0f, global_cam_data.height/2.0f };
    level_context->cam.cam2D.offset = cast(Vector2){ global_cam_data.width/2.0f, global_cam_data.height/2.0f };
    // level_context->cam = global_cam_data;
}

void switch_current_level_context(Level_Context *target, b32 clear_stuff = false){
    if (clear_stuff){
        clear_multiselected_entities(true);
    }

    current_level_context = target;
    setup_context_cam(current_level_context);
}

inline Color color_fade(Color color, f32 alpha_multiplier){
    return {color.r, color.g, color.b, (u8)(clamp((f32)color.a * alpha_multiplier, 0.0f, 255.0f))};
}

inline Color color_opacity(Color color, f32 alpha){
    return {color.r, color.g, color.b, (u8)(clamp01(alpha) * 255)};
}

void free_light(Light *light){
    if (light->exists){
        if (light->make_shadows){
            UnloadRenderTexture(light->shadowmask_rt);
            light->shadowmask_rt = {};
        }
        // UnloadRenderTexture(light->geometry_rt);
        if (light->make_backshadows){
            UnloadRenderTexture(light->backshadows_rt);
            light->backshadows_rt = {};
        }
        
        light->exists = false;
        
        if (light->connected_entity_id != -1){
            Entity *connected_entity = get_entity_by_id(light->connected_entity_id);
            if (connected_entity){
                connected_entity->light_index = -1;
            }
            light->connected_entity_id = -1;
        }
    }
}

void free_entity_light(Entity *e){
    if (e->light_index != -1 ){
        // @OPTIMIZATION we actually don't want unload texture every time entity gets freed.
        // I think we should mark it as non existing and when the next entity will search for light - check if this one already
        // has index and size is the same and use it. We will free it when level gets unload.
        // Update: We will have lights of each size loaded in the init and will use them without more allocating. 
        // Like we already do with temp lights.
        Light *current_light = current_level_context->lights.get_ptr(e->light_index);           
        free_light(current_light);        
        e->light_index = -1;
    }
}

void free_entity(Entity *e){
    if (e->flags & TRIGGER){
        if (e->trigger.connected.max_count > 0){
            e->trigger.connected.free_arr();
        }
        
        if (e->trigger.cam_rails_points.max_count > 0){
            e->trigger.cam_rails_points.free_arr();
        }
    }
    
    if (e->flags & BIRD_ENEMY){
        bird_clear_formation(&e->bird_enemy);
    }
    
    // free kill switch
    if (e->flags & KILL_SWITCH){
        e->enemy.kill_switch.connected.free_arr();    
    }
    
    if (e->flags & CENTIPEDE){
        // free centipede
        for (i32 i = 0; i < e->centipede.segments_ids.count; i++){
            // @CLEANUP: Why we don't call free_entity on segments?
            // Probably that doesn't matter because while game-looping we're just destroy them and when we're clearing context 
            // we'll call it anyway, but nonetheless we should be able to just call free_entity without trouble in such case.
            // Will see into that when will rewrite entities.
            Entity *segment = get_entity_by_id(e->centipede.segments_ids.get(i));
            segment->destroyed = true;
            segment->enabled = false;
        }
        
        e->centipede.segments_ids.clear();
    }
    
    // free physics objctt
    if (e->flags & PHYSICS_OBJECT){
        Entity *rope_entity = get_entity_by_id(e->physics_object.rope_id);
        if (e->physics_object.on_rope){
            if (rope_entity){
                rope_entity->destroyed = true;
            }
            Entity *up_rope_point_entity = get_entity_by_id(e->physics_object.up_rope_point_id);
            if (up_rope_point_entity){
                up_rope_point_entity->destroyed = true;
            }
            Entity *down_rope_point_entity = get_entity_by_id(e->physics_object.down_rope_point_id);
            if (down_rope_point_entity){
                down_rope_point_entity->destroyed = true;
            }
        }
    }
    
    // @CLEANUP: Why exactly that? We're just leaking without hesitation? Brave. Will see into that when will rewrite entity system.
    // if (e->flags & MOVE_SEQUENCE){
    //     e->move_sequence.points.free_arr();
    // }
    
    // if (e->flags & JUMP_SHOOTER){
    //     e->jump_shooter.move_points.free_arr();
    // }
    
    // free light
    if (e->light_index != -1){
        free_entity_light(e);
    }
    
    e->color_changer.changing = false;
    
    free_entity_particle_emitters(e);
}

inline void add_rect_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    vertices->clear();
    vertices->add({1.0f - pivot.x, pivot.y});
    vertices->add({-pivot.x, pivot.y});
    vertices->add({1.0f - pivot.x, pivot.y - 1.0f});
    vertices->add({-pivot.x, pivot.y - 1.0f});
}

void add_triangle_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    vertices->clear();
    vertices->add({pivot.x, pivot.y});
    vertices->add({-pivot.x, pivot.y});
    vertices->add({pivot.x, pivot.y - 1.0f});
}

void add_sword_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    add_rect_vertices(vertices, pivot);
    vertices->get_ptr(0)->x *= 0.3f;
    vertices->get_ptr(1)->x *= 0.3f;
    
    vertices->get_ptr(2)->y += 0.15f;
    vertices->get_ptr(3)->y += 0.15f;
}

void add_prism_shaped_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot, f32 narrowing = 0.3f){
    add_rect_vertices(vertices, pivot);
    vertices->get_ptr(0)->x *= narrowing;
    vertices->get_ptr(1)->x *= narrowing;
}

void add_upsidedown_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    add_rect_vertices(vertices, pivot);
    vertices->get_ptr(2)->x *= 0.3f;
    vertices->get_ptr(3)->x *= 0.3f;
}

void add_romb_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    add_rect_vertices(vertices, pivot);
    vertices->get_ptr(0)->x *= 1.5f;
    vertices->get_ptr(3)->x *= 1.5f;
    for (i32 i = 0; i < vertices->count; i++){
        rotate_around_point(vertices->get_ptr(i), {0, 0}, -55);
    }
}

void pick_vertices(Entity *entity){
    if (entity->flags & (SWORD)){
        add_sword_vertices(&entity->vertices, entity->pivot);
        add_sword_vertices(&entity->unscaled_vertices, entity->pivot);
    } else if (entity->flags & (BIRD_ENEMY | CENTIPEDE | PROJECTILE | HIT_BOOSTER)){
        f32 narrowing = 0.3f;
        if (entity->flags & HIT_BOOSTER){
            narrowing = 0.1f;
        }
        add_prism_shaped_vertices(&entity->vertices, entity->pivot, narrowing);
        add_prism_shaped_vertices(&entity->unscaled_vertices, entity->pivot, narrowing);
    } else if (entity->flags & (JUMP_SHOOTER)){
        add_upsidedown_vertices(&entity->vertices, entity->pivot);
        add_upsidedown_vertices(&entity->unscaled_vertices, entity->pivot);
    } else if (entity->flags & GIVES_BIG_SWORD_CHARGE){
        add_romb_vertices(&entity->vertices, entity->pivot);
        add_romb_vertices(&entity->unscaled_vertices, entity->pivot);
    } else{
        add_rect_vertices(&entity->unscaled_vertices, entity->pivot);
        add_rect_vertices(&entity->vertices, entity->pivot);
    }
}

Entity::Entity(){
    calculate_bounds(this);
    setup_color_changer(this);
}

Entity::Entity(Vector2 _pos){
    flags = 0;
    position = _pos;
    
    add_rect_vertices(&vertices, pivot);

    rotation = 0;
    up = {0, 1};
    right = {1, 0};
    
    change_scale(this, {1, 1});
    setup_color_changer(this);
}

Entity::Entity(Vector2 _pos, Vector2 _scale){
    flags = 0;
    position = _pos;
    
    add_rect_vertices(&vertices, pivot);

    rotation = 0;
    up = {0, 1};
    right = {1, 0};
    change_scale(this, _scale);
    setup_color_changer(this);
}

Entity::Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags){
    flags    = _flags;
    position = _pos;
    pick_vertices(this);
    rotation = 0;
    
    rotate_to(this, _rotation);
    change_scale(this, _scale);
    setup_color_changer(this);
}

Entity::Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags){
    // *this = Entity(_pos, _scale, _rotation, _flags);
    flags    = _flags;
    position = _pos;
    pivot = _pivot;
    
    pick_vertices(this);
    
    rotation = 0;
    
    rotate_to(this, _rotation);
    
    change_scale(this, _scale);
    setup_color_changer(this);
}

Entity::Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, Texture _texture, FLAGS _flags){
    flags    = _flags;
    position = _pos;
    pivot    = _pivot;
    texture  = _texture;
    scaling_multiplier = {texture.width / current_level_context->cam.unit_size, texture.height / current_level_context->cam.unit_size};
    //scaling_multiplier = {1, 1};
    color = WHITE;
    //scale = transform_texture_scale(texture, _scale);
    
    pick_vertices(this);
    
    rotation = 0;
    
    rotate_to(this, _rotation);
    
    change_scale(this, _scale);
    setup_color_changer(this);
}

Entity::Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags){
    flags    = _flags;
    id       = _id;
    position = _pos;
    pivot    = _pivot;
    
    pick_vertices(this);
    
    rotation = 0;
    rotate_to(this, _rotation);
    change_scale(this, _scale);
    setup_color_changer(this);
}

Entity::Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags, Array<Vector2, MAX_VERTICES> _vertices){
    flags    = _flags;
    id = _id;
    position = _pos;
    pivot = _pivot;
    
    vertices = _vertices;
    
    rotation = 0;
    rotate_to(this, _rotation);
    change_scale(this, _scale);
    setup_color_changer(this);
}

Entity::Entity(Entity *copy, b32 keep_id, Level_Context *copy_level_context, b32 should_init_entity){
    if (!copy_level_context) copy_level_context = current_level_context;

    *this = *copy;
    id = copy->id;
    
    if (!keep_id){
        id = current_level_context->entities.total_added_count + core.time.app_time * 10000 + 100;
        check_avaliable_ids_and_set_if_found(&id);
    }
    
    position = copy->position;
    pivot    = copy->pivot;
    
    vertices = copy->vertices;
    unscaled_vertices = copy->unscaled_vertices;
    rotation = copy->rotation;
    scale = copy->scale;
    flags = copy->flags;
    collision_flags = copy->collision_flags;
    color = copy->color_changer.start_color;
    draw_order = copy->draw_order;
    str_copy(name, copy->name);
    hidden = copy->hidden;
    spawn_enemy_when_no_ammo = copy->spawn_enemy_when_no_ammo;
    
    if (flags & TEXTURE){
        texture = copy->texture;
        scaling_multiplier = {texture.width / current_level_context->cam.unit_size, texture.height / current_level_context->cam.unit_size};
        // This means that copy is just texture. Visual flakes.
        if (copy->scale == Vector2_one){
            scale = {texture.width / 10.0f, texture.height / 10.0f};
        }
        str_copy(texture_name, copy->texture_name);
    }
    color_changer = copy->color_changer;
    
    if (flags & DRAW_TEXT){
        text_drawer = copy->text_drawer;
    }
    if (flags & ENEMY){
        enemy = copy->enemy;
    }
    
    // copy trigger
    if (flags & TRIGGER){
        trigger = copy->trigger;
        trigger.connected = Dynamic_Array<int>();
        for (i32 i = 0; i < copy->trigger.connected.count; i++){
            trigger.connected.add(copy->trigger.connected.get(i));
        }
        trigger.tracking = Dynamic_Array<int>();
        for (i32 i = 0; i < copy->trigger.tracking.count; i++){
            trigger.tracking.add(copy->trigger.tracking.get(i));
        }
        
        if (copy->trigger.start_cam_rails_horizontal || copy->trigger.start_cam_rails_vertical){
            trigger.cam_rails_points = Dynamic_Array<Vector2>();           
            for (i32 i = 0; i < copy->trigger.cam_rails_points.count; i++){
                trigger.cam_rails_points.add(copy->trigger.cam_rails_points.get(i));               
            }
        }
    }
    
    // copy kill switch
    if (flags & KILL_SWITCH){
        Kill_Switch *kill_switch = &enemy.kill_switch;
        Kill_Switch *copy_kill_switch = &copy->enemy.kill_switch;
        *kill_switch = *copy_kill_switch;
        kill_switch->connected = Dynamic_Array<i32>();
        for (i32 i = 0; i < copy_kill_switch->connected.count; i++){
            kill_switch->connected.add(copy_kill_switch->connected.get(i));
        }
    }
    
    if (flags & NOTE){
        note_index = add_note("");
        if (note_index != -1 && copy->note_index != -1){
            (*current_level_context->notes.get_ptr(note_index)) = *copy_level_context->notes.get_ptr(copy->note_index);
        }
    }
    
    if (flags & MOVE_SEQUENCE){
        move_sequence = copy->move_sequence;
        move_sequence.points = Dynamic_Array<Vector2>();
        for (i32 i = 0; i < copy->move_sequence.points.count; i++){
            move_sequence.points.add(copy->move_sequence.points.get(i));
        }
    }
    
    if (flags & PROPELLER){
        propeller = copy->propeller;
    }
    
    if (flags & DOOR){
        door = copy->door;
    }
    
    if (copy->light_index != -1){
        light_index = -1;
        init_entity_light(this, copy_level_context->lights.get_ptr(copy->light_index));        
    }
    
    if (should_init_entity){
        particle_emitters_indexes.clear(); // Because on init entities add emitters themselves.
        rotate_to(this, rotation);
        setup_color_changer(this);
        init_entity(this);
        calculate_bounds(this);
    }
}

void parse_line(const char *line, char *result, i32 *index){ 
    assert(line[*index] == ':');
    
    i32 i;
    i32 added_count = 0;
    for (i = *index + 1; line[i] != NULL && line[i] != ':'; i++){
        result[added_count] = line[i];
        added_count++;
    }
    
    *index = i;
}

inline void set_particle_emitter_start_and_max_indexes(Particle_Emitter_Count count_type, i32 *start_index, i32 *max_index){
    switch (count_type){
        case SMALL_PARTICLE_COUNT:
            *start_index = 0;
            *max_index   = MAX_SMALL_COUNT_PARTICLE_EMITTERS;
            break;
        case MEDIUM_PARTICLE_COUNT:
            *start_index = MAX_SMALL_COUNT_PARTICLE_EMITTERS;
            *max_index   = *start_index + MAX_MEDIUM_COUNT_PARTICLE_EMITTERS;
            break;
        case BIG_PARTICLE_COUNT:
            *start_index = MAX_SMALL_COUNT_PARTICLE_EMITTERS + MAX_MEDIUM_COUNT_PARTICLE_EMITTERS;
            *max_index   = *start_index + MAX_BIG_COUNT_PARTICLE_EMITTERS;
            break;
    }
}

inline i32 get_particles_count_for_count_type(Particle_Emitter_Count count_type){
    switch (count_type){
        case SMALL_PARTICLE_COUNT:  return MAX_SMALL_COUNT_PARTICLES;
        case MEDIUM_PARTICLE_COUNT: return MAX_MEDIUM_COUNT_PARTICLES;
        case BIG_PARTICLE_COUNT:    return MAX_BIG_COUNT_PARTICLES;
        default: return -1;
    }
}

i32 add_particle_emitter(Particle_Emitter *copy, i32 entity_id){
    i32 start_index = 0;
    i32 max_index = 0;
    
    set_particle_emitter_start_and_max_indexes(copy->count_type, &start_index, &max_index);
    
    f32 particles_count = get_particles_count_for_count_type(copy->count_type);
    
    i32 emitter_index = -1;  
    i32 occupied_count = 0;
    for (i32 i = start_index; i < max_index; i++){
        Particle_Emitter *emitter = current_level_context->particle_emitters.get_ptr(i);
        if (!emitter->occupied){
            *emitter = *copy;
            emitter->occupied = true;
            emitter->index = i;
            
            // So small particles starts at 0, medium count starts at MEDIUM_COUNT_PARTICLES_START_INDEX and big at ...
            i32 particles_count_type_start_index = 0;
            if (emitter->count_type == MEDIUM_PARTICLE_COUNT){
                particles_count_type_start_index = MEDIUM_COUNT_PARTICLES_START_INDEX;
            } else if (emitter->count_type == BIG_PARTICLE_COUNT){
                particles_count_type_start_index = BIG_COUNT_PARTICLES_START_INDEX;
            }
            
            emitter->particles_start_index = particles_count_type_start_index + (occupied_count * particles_count);
            emitter->particles_max_index   = emitter->particles_start_index + particles_count;
            emitter->last_added_index      = emitter->particles_start_index;
            emitter_index = i;
            emitter->connected_entity_id = entity_id;
            break;
        }
        
        occupied_count += 1;
    }
    
    if (emitter_index == -1){
        printf("WARNING: Could not found particle emitter index to add. Copy emitter tag is: %s. And count type is: %d\n", copy->tag_16, copy->count_type);
    }
    
    return emitter_index;
}

inline i32 add_entity_particle_emitter(Entity *entity, Particle_Emitter *emitter){
    return *entity->particle_emitters_indexes.add(add_particle_emitter(emitter, entity->id));
}

inline i32 add_and_enable_entity_particle_emitter(Entity *entity, Particle_Emitter *emitter_copy, Vector2 position, b32 need_to_follow){
    i32 index = add_entity_particle_emitter(entity, emitter_copy);
    Particle_Emitter *emitter = get_particle_emitter(index);
    if (emitter){
        emitter->follow_entity = need_to_follow;
        enable_emitter(emitter, position);
    }
    
    return index;
}

Particle_Emitter *get_particle_emitter(i32 index){
    if (index < 0 || index >= current_level_context->particle_emitters.max_count){
        printf("WARNING: Tried to get particle emitter with bad index: %d\n", index);
        return NULL;
    }
    
    Particle_Emitter *emitter = current_level_context->particle_emitters.get_ptr(index);
    if (!emitter->occupied){
        print("WARNING: In get_particle_emitter we just took un-occupied emitter. Don't think that should happen");
    }
    
    return emitter;
}

i32 add_note(const char *content){
    i32 note_index = -1;
    for (i32 i = 0; i < current_level_context->notes.max_count; i++){
        if (!current_level_context->notes.get_ptr(i)->occupied){
            current_level_context->notes.get_ptr(i)->occupied = true;
            str_copy(current_level_context->notes.get_ptr(i)->content, content);
            note_index = i;
            break;
        }
    }
    
    if (note_index == -1){
        print("WARNING: Could not found note index to add");
    }
    
    return note_index;
}

void copy_level_context(Level_Context *dest, Level_Context *src, b32 should_init_entities){
    // *dest = *src;
    Level_Context *original_level_context = current_level_context;
    switch_current_level_context(dest);
    
    Game_State original_game_state = game_state;
    game_state = EDITOR;
    
    str_copy(dest->level_name, src->level_name);
    dest->player_spawn_point = src->player_spawn_point;
    dest->cam = src->cam;
    
    if (should_init_entities){
        // Particle emitters get's added on each entity init.
        // So when se init entities - we clear particle emitters, because they will be added again. 
        // When we don't init entities - we copy emitters (and entity indexes are staying the same).
        ArrayOfStructsToDefaultValues(current_level_context->particle_emitters);       
    }
    
    for (i32 i = 0; i < src->particle_emitters.max_count; i++){
        // Particle emitter could be connected to entity - in that case entity will add emitter itself if we init entities.
        // If emitter don't connected to entity - we want to copy it.
        if (src->particle_emitters.get(i).connected_entity_id == -1 || !should_init_entities){
            dest->particle_emitters.data[i] = src->particle_emitters.get(i);
        }
    }
    
    for (i32 i = 0; i < src->entities.max_count; i++){
        Table_Data<Entity> data = {};
        
        data.key = src->entities.data[i].key;
        if (data.key != -1){
            data.value = Entity(&src->entities.data[i].value, true, src, should_init_entities);
            data.value.level_context = current_level_context;
        } else{
            data.value = {};
        }
        dest->entities.data[i] = data;
    }
    
    dest->entities.max_count = src->entities.max_count;
    dest->entities.total_added_count = src->entities.total_added_count;
    dest->entities.last_added_key = src->entities.last_added_key;
    
    for (i32 i = 0; i < src->line_trails.max_count; i++){
        dest->line_trails.data[i] = src->line_trails.get(i);
    }
    
    switch_current_level_context(original_level_context);
    game_state = original_game_state;
}

void clear_level_context(Level_Context *level_context){
    Level_Context *original_level_context = current_level_context;
    switch_current_level_context(level_context);
    ForEntities(entity, 0){
        free_entity(entity);
        *entity = {};
    }

    level_context->entities.clear();
    level_context->particles.clear();
    // level_context->emitters.clear();
    
    ArrayOfStructsToDefaultValues(level_context->particle_emitters);
    ArrayOfStructsToDefaultValues(level_context->particles);
    ArrayOfStructsToDefaultValues(level_context->notes);
    
    for (i32 i = 0; i < level_context->lights.max_count; i++){
        level_context->lights.get_ptr(i)->exists = false;
        if (i >= session_context.entity_lights_start_index){
            free_light(level_context->lights.get_ptr(i));       
            *(level_context->lights.get_ptr(i)) = {};
        } else{ // So we in temp lights section
        }
    }
    
    // level_context->we_got_a_winner = false;
    // player_data = {};
    
    switch_current_level_context(original_level_context);
}

i32 save_level(const char *level_name){
    if (game_state == GAME){
        print_to_console("Will not save in game mode under any circumstances!");
        return -1;
    }

    char name[1024];
    str_copy(name, get_substring_before_symbol(level_name, '.'));

    char level_path[1024];
    str_copy(level_path, text_format("levels/%s.level", name));
    FILE *fptr;
    fptr = fopen(text_format(level_path, name), "w");
    
    if (fptr == NULL){
        return 0;
    }
    
    fprintf(fptr, "Setup Data:\n");
    
    fprintf(fptr, "player_spawn_point:{:%f:, :%f:} ", current_level_context->player_spawn_point.x, current_level_context->player_spawn_point.y);
    
    fprintf(fptr, ";\n");
    
    fprintf(fptr, "Entities:\n");
    for (i32 i = 0; i < current_level_context->entities.max_count; i++){        
        if (!current_level_context->entities.has_index(i)){
            continue;
        }
    
        Entity *e = current_level_context->entities.get_ptr(i);
        
        if (!e->need_to_save){
            continue;
        }
        
        Color color = e->color_changer.start_color;
        fprintf(fptr, "name:%s: id:%d: pos{:%f:, :%f:} scale{:%f:, :%f:} pivot{:%f:, :%f:} rotation:%f: color{:%d:, :%d:, :%d:, :%d:}, flags:%llu:, draw_order:%d: ", e->name, e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags, e->draw_order);
        
        fprintf(fptr, "vertices [ ");
        for (i32 v = 0; v < e->vertices.count; v++){
            fprintf(fptr, "{:%f:, :%f:} ", e->vertices.get(v).x, e->vertices.get(v).y); 
        }
        fprintf(fptr, "] "); 
        
        fprintf(fptr, "unscaled_vertices [ ");
        for (i32 v = 0; v < e->unscaled_vertices.count; v++){
            fprintf(fptr, "{:%f:, :%f:} ", e->unscaled_vertices.get(v).x, e->unscaled_vertices.get(v).y); 
        }
        fprintf(fptr, "] "); 
        
        fprintf(fptr, "hidden:%d: ", e->hidden);
        fprintf(fptr, "spawn_enemy_when_no_ammo:%d: ", e->spawn_enemy_when_no_ammo);
        
        if (e->light_index >= 0){
            Light *light = current_level_context->lights.get_ptr(e->light_index);
            fprintf(fptr, "light_shadows_size_flag:%d: ",     light->shadows_size_flags);
            fprintf(fptr, "light_backshadows_size_flag:%d: ", light->backshadows_size_flags);
            fprintf(fptr, "light_make_shadows:%d: ",          light->make_shadows);
            fprintf(fptr, "light_make_backshadows:%d: ",      light->make_backshadows);
            fprintf(fptr, "light_bake_shadows:%d: ",          light->bake_shadows);
            fprintf(fptr, "light_radius:%f: ",                light->radius);
            fprintf(fptr, "light_opacity:%f: ",               light->opacity);
            fprintf(fptr, "light_power:%f: ",                 light->power);
            fprintf(fptr, "light_color{:%d:, :%d:, :%d:, :%d:} ", (i32)light->color.r, (i32)light->color.g, (i32)light->color.b, (i32)light->color.a);
        }
        
        if (e->flags & TRIGGER){
            if (e->trigger.connected.count > 0){
                fprintf(fptr, "trigger_connected [ ");
                for (i32 v = 0; v < e->trigger.connected.count; v++){
                    fprintf(fptr, ":%d: ", e->trigger.connected.get(v)); 
                }
                fprintf(fptr, "] "); 
            }
            
            if (e->trigger.tracking.count > 0){
                fprintf(fptr, "trigger_tracking [ ");
                for (i32 v = 0; v < e->trigger.tracking.count; v++){
                    fprintf(fptr, ":%d: ", e->trigger.tracking.get(v)); 
                }
                fprintf(fptr, "] "); 
            }
            
            fprintf(fptr, "trigger_kill_player:%d: ",                    e->trigger.kill_player);
            fprintf(fptr, "trigger_die_after_trigger:%d: ",              e->trigger.die_after_trigger);
            fprintf(fptr, "trigger_kill_enemies:%d: ",                   e->trigger.kill_enemies);
            fprintf(fptr, "trigger_open_doors:%d: ",                     e->trigger.open_doors);
            fprintf(fptr, "trigger_start_physics_simulation:%d: ",       e->trigger.start_physics_simulation);
            fprintf(fptr, "trigger_track_enemies:%d: ",                  e->trigger.track_enemies);
            fprintf(fptr, "trigger_draw_lines_to_tracked:%d: ",          e->trigger.draw_lines_to_tracked);
            fprintf(fptr, "trigger_agro_enemies:%d: ",                   e->trigger.agro_enemies);
            fprintf(fptr, "trigger_player_touch:%d: ",                   e->trigger.player_touch);
            fprintf(fptr, "trigger_shows_entities:%d: ",                 e->trigger.shows_entities);
            fprintf(fptr, "trigger_starts_moving_sequence:%d: ",         e->trigger.starts_moving_sequence);
            fprintf(fptr, "trigger_lock_camera:%d: ",                    e->trigger.lock_camera);
            fprintf(fptr, "trigger_unlock_camera:%d: ",                  e->trigger.unlock_camera);
            fprintf(fptr, "trigger_allow_player_shoot:%d: ",               e->trigger.allow_player_shoot);
            fprintf(fptr, "trigger_forbid_player_shoot:%d: ",               e->trigger.forbid_player_shoot);
            fprintf(fptr, "trigger_locked_camera_position{:%f:, :%f:} ", e->trigger.locked_camera_position.x, e->trigger.locked_camera_position.y);
            
            fprintf(fptr, "trigger_load_level:%d: ", e->trigger.load_level);
            if (e->trigger.load_level){
                fprintf(fptr, "trigger_level_name:%s: ", e->trigger.level_name);
            }
            
            fprintf(fptr, "trigger_play_replay:%d: ", e->trigger.play_replay);
            if (e->trigger.play_replay){
                fprintf(fptr, "trigger_replay_name:%s: ", e->trigger.replay_name);
            }
            
            fprintf(fptr, "trigger_change_zoom:%d: ", e->trigger.change_zoom);
            if (e->trigger.change_zoom){
                fprintf(fptr, "trigger_zoom_value:%f: ", e->trigger.zoom_value);
            }
            
            fprintf(fptr, "trigger_play_sound:%d: ", e->trigger.play_sound);
            if (e->trigger.play_sound){
                fprintf(fptr, "trigger_sound_name:%s: ", e->trigger.sound_name);
            }
            
            fprintf(fptr, "trigger_start_cam_rails_horizontal:%d: ", e->trigger.start_cam_rails_horizontal);
            fprintf(fptr, "trigger_start_cam_rails_vertical:%d: ", e->trigger.start_cam_rails_vertical);
            fprintf(fptr, "trigger_stop_cam_rails:%d: ", e->trigger.stop_cam_rails);
            if (e->trigger.cam_rails_points.count > 0){
                fprintf(fptr, "trigger_cam_rails_points [ ");
                for (i32 v = 0; v < e->trigger.cam_rails_points.count; v++){
                    fprintf(fptr, "{:%f:, :%f:} ", e->trigger.cam_rails_points.get(v).x, e->trigger.cam_rails_points.get(v).y); 
                }
                fprintf(fptr, "] "); 
            }
        }
        
        if (e->flags & KILL_SWITCH){
            if (e->enemy.kill_switch.connected.count > 0){
                fprintf(fptr, "kill_switch_connected [ ");
                for (i32 v = 0; v < e->enemy.kill_switch.connected.count; v++){
                    fprintf(fptr, ":%d: ", e->enemy.kill_switch.connected.get(v)); 
                }
                fprintf(fptr, "] "); 
            }
        }
        
        if (e->flags & MOVE_SEQUENCE){
            if (e->move_sequence.points.count > 0){
                fprintf(fptr, "move_sequence_points [ ");
                for (i32 v = 0; v < e->move_sequence.points.count; v++){
                    fprintf(fptr, "{:%f:, :%f:} ", e->move_sequence.points.get(v).x, e->move_sequence.points.get(v).y); 
                }
                fprintf(fptr, "] "); 
            }
            
            fprintf(fptr, "move_sequence_moving:%d: ",                        e->move_sequence.moving);
            fprintf(fptr, "move_sequence_speed:%f: ",                         e->move_sequence.speed);
            fprintf(fptr, "move_sequence_loop:%d: ",                          e->move_sequence.loop);
            fprintf(fptr, "move_sequence_rotate:%d: ",                        e->move_sequence.rotate);
            fprintf(fptr, "move_sequence_speed_related_player_distance:%d: ", e->move_sequence.speed_related_player_distance);
            fprintf(fptr, "move_sequence_min_distance:%f: ",                  e->move_sequence.min_distance);
            fprintf(fptr, "move_sequence_max_distance:%f: ",                  e->move_sequence.max_distance);
            fprintf(fptr, "move_sequence_max_distance_speed:%f: ",            e->move_sequence.max_distance_speed);
        }
        
        if (e->flags & CENTIPEDE){
            fprintf(fptr, "spikes_on_right:%d: ", e->centipede.spikes_on_right);
            fprintf(fptr, "spikes_on_left:%d: ", e->centipede.spikes_on_left);
            fprintf(fptr, "segments_count:%d: ", e->centipede.segments_count);
        }
        
        if (e->flags & JUMP_SHOOTER){
            fprintf(fptr, "jump_shooter_shots_count:%d: ", e->jump_shooter.shots_count);
            fprintf(fptr, "jump_shooter_spread:%f: ", e->jump_shooter.spread);
            fprintf(fptr, "jump_shooter_explosive_count:%d: ", e->jump_shooter.explosive_count);
            fprintf(fptr, "jump_shooter_shoot_sword_blockers:%d: ", e->jump_shooter.shoot_sword_blockers);
            // fprintf(fptr, "jump_shooter_shoot_sword_blockers_clockwise:%d: ", e->jump_shooter.shoot_sword_blockers_clockwise);
            // fprintf(fptr, "jump_shooter_shoot_sword_blockers_random_direction:%d: ", e->jump_shooter.shoot_sword_blockers_random_direction);
            fprintf(fptr, "jump_shooter_shoot_sword_blockers_immortal:%d: ", e->jump_shooter.shoot_sword_blockers_immortal);
            fprintf(fptr, "jump_shooter_shoot_bullet_blockers:%d: ", e->jump_shooter.shoot_bullet_blockers);
        }
        
        if (e->flags & TURRET){
            Turret *turret = &e->enemy.turret;
            fprintf(fptr, "turret_projectile_flags:%llu: ", turret->projectile_settings.enemy_flags);
            fprintf(fptr, "turret_shoot_sword_blocker_clockwise:%d: ", turret->projectile_settings.blocker_clockwise);
            fprintf(fptr, "turret_activated:%d: ", turret->activated);
            fprintf(fptr, "turret_homing_projectiles:%d: ", turret->homing);
            fprintf(fptr, "turret_shoot_every_tick:%d: ", turret->shoot_every_tick);
            fprintf(fptr, "turret_start_tick_delay:%d: ", turret->start_tick_delay);
            fprintf(fptr, "turret_projectile_speed:%f: ", turret->projectile_settings.launch_speed);
            fprintf(fptr, "turret_projectile_max_lifetime:%f: ", turret->projectile_settings.max_lifetime);
            fprintf(fptr, "turret_shoot_width:%f: ", turret->shoot_width);
            fprintf(fptr, "turret_shoot_height:%f: ", turret->shoot_height);
        }
        
        if (e->flags & PHYSICS_OBJECT){
            fprintf(fptr, "physics_simulating:%d: ", e->physics_object.simulating);
            fprintf(fptr, "on_rope:%d: ", e->physics_object.on_rope);
            fprintf(fptr, "physics_rotate_by_velocity:%d: ", e->physics_object.rotate_by_velocity);
            fprintf(fptr, "physics_gravity_multiplier:%f: ", e->physics_object.gravity_multiplier);
            fprintf(fptr, "physics_mass:%f: ", e->physics_object.mass);
            
            if (e->physics_object.on_rope){
                fprintf(fptr, "physics_rope_point:{%f, %f}: ", e->physics_object.rope_point.x, e->physics_object.rope_point.y);
            }
        }
        
        if (e->flags & DOOR){
            fprintf(fptr, "door_open:%d: ", e->door.is_open);
        }
        
        if (e->flags & BLOCKER){
            fprintf(fptr, "blocker_clockwise:%d: ", e->enemy.blocker_clockwise);
            fprintf(fptr, "blocker_immortal:%d: ", e->enemy.blocker_immortal);
        }
        
        if (e->flags & SWORD_SIZE_REQUIRED){
            fprintf(fptr, "enemy_big_or_small_killable:%d: ", e->enemy.big_sword_killable);
        }
        
        if (e->flags & ENEMY && e->enemy.sword_kill_speed_modifier != 1){
            fprintf(fptr, "sword_kill_speed_modifier:%.1f: ", e->enemy.sword_kill_speed_modifier);
        }
        
        if (e->flags & ENEMY){
            fprintf(fptr, "enemy_gives_ammo:%d: ", e->enemy.gives_ammo);
        }
        
        if (e->flags & EXPLOSIVE){
            fprintf(fptr, "explosive_radius_multiplier:%fs: ", e->enemy.explosive_radius_multiplier);
        }
        
        if (e->flags & PROPELLER){
            fprintf(fptr, "propeller_power:%f: ", e->propeller.power);
            fprintf(fptr, "propeller_spin_sensitive:%d: ", e->propeller.spin_sensitive);
        }
        
        if (e->flags & SHOOT_BLOCKER){
            fprintf(fptr, "shoot_blocker_direction{:%f:, :%f:} ", e->enemy.shoot_blocker_direction.x, e->enemy.shoot_blocker_direction.y);
            fprintf(fptr, "shoot_blocker_immortal:%d: ", e->enemy.shoot_blocker_immortal);
        }
        
        if (e->flags & TEXTURE){
            fprintf(fptr, "texture_name:%s: ", e->texture_name);
        }
        
        if (e->flags & NOTE){
            assert(e->note_index != -1);
            fprintf(fptr, "note_content:\" %s \": ", current_level_context->notes.get_ptr(e->note_index)->content);
            fprintf(fptr, "note_draw_in_game:%d: ", current_level_context->notes.get_ptr(e->note_index)->draw_in_game);
        }
        
        fprintf(fptr, ";\n"); 
    }
    
    fclose(fptr);
    
    b32 is_temp_level = str_start_with_const(name, "temp/TEMP_");
    b32 is_autosave   = str_start_with_const(name, "autosaves/AUTOSAVE_");
    if (!is_temp_level && !is_autosave){
        // Why do we set previous level on saving??
        if (!str_equal(current_level_context->level_name, name)){
            str_copy(session_context.previous_level_name, current_level_context->level_name);
        }
        str_copy(current_level_context->level_name, name);
        reload_level_files();
        console.str += text_format("\t>Level saved: \"%s\"; App time: %.2f\n", name, core.time.app_time);
        printf("level saved: \"%s\"; \n", level_path);
    }
    
    if (is_temp_level){
        // console.str += "\t>Temp level saved: ";
        // console.str += name;
        // console.str += "\n";
        printf("Temp level saved: %s\n", level_path);
    }
    
    if (is_autosave){
        // console.str += "\t>Autosaved: ";
        // console.str += name;
        // console.str += "\n";
        printf("Temp level saved: %s\n", level_path);
    }
    
    return 1;
}

inline void save_level_by_name(const char *name){
    save_level(name);
}

void fill_string(char *dest, Dynamic_Array<Medium_Str> line_arr, i32 *index_ptr){
    assert(line_arr.get(*index_ptr + 1).data[0] == '\"');
    
    *index_ptr += 2;
    
    for (; *index_ptr < line_arr.count && line_arr.get(*index_ptr).data[0] != '\"'; *index_ptr += 1){
        Medium_Str current_str = line_arr.get(*index_ptr);
        str_copy(dest, text_format("%s %s", dest, current_str.data));
    }
}

b32 load_level(const char *level_name){
    Game_State original_game_state = game_state;
    game_state = EDITOR;
    
    session_context.playing_replay = false;
    
    char name[1024];
    str_copy(name, get_substring_before_symbol(level_name, '.'));

    const char *level_path = text_format("levels/%s.level", name);
    File file = load_file(level_path, "r");
    
    if (!file.loaded){
        console.str += "Could not load level: ";
        console.str += name;
        console.str += "\n";
        return false;
    }
    
    // game_state = EDITOR;
    clean_up_scene();
    switch_current_level_context(&loaded_level_context, true);
    clear_level_context(&loaded_level_context);
    setup_particles();
    
    Dynamic_Array<Medium_Str> splitted_line = Dynamic_Array<Medium_Str>(64);
    
    b32 parsing_setup_data = false;
    b32 parsing_entities   = false;
    
    for (i32 line_index = 0; line_index < file.lines.count; line_index++){
        Long_Str line = file.lines.get(line_index);
        
        if (str_equal(line.data, "Setup Data:")){
            parsing_setup_data = true;
            parsing_entities = false;
        }
        
        if (str_equal(line.data, "Entities:")){
            parsing_setup_data = false;
            parsing_entities = true;
            continue;
        }
        
        split_str(line.data, ":{}, ;", &splitted_line);
        
        Entity entity_to_fill = Entity();
        Note note_to_fill = {};
        
        for (i32 i = 0; i < splitted_line.count; i++){
            if (parsing_setup_data){
                if (str_equal(splitted_line.get(i).data, "player_spawn_point")){
                    fill_vector2_from_string(&current_level_context->player_spawn_point, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                    i += 2;
                    continue;
                }
            }
        
            if (!parsing_entities){
                continue;
            }

            if (0){
            } else if (str_equal(splitted_line.get(i).data, "name")){
                str_copy(entity_to_fill.name, splitted_line.get(i+1).data);  
                i++;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "id")){
                fill_i32_from_string(&entity_to_fill.id, splitted_line.get(i+1).data);
                i++;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "pos")){
                fill_vector2_from_string(&entity_to_fill.position, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                i += 2;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "scale")){
                fill_vector2_from_string(&entity_to_fill.scale, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                i += 2;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "pivot")){
                fill_vector2_from_string(&entity_to_fill.pivot, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                i += 2;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "rotation")){
                fill_f32_from_string(&entity_to_fill.rotation, splitted_line.get(i+1).data);
                i += 1;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "color")){
                fill_vector4_from_string(&entity_to_fill.color, splitted_line.get(i+1).data, splitted_line.get(i+2).data, splitted_line.get(i+3).data, splitted_line.get(i+4).data);
                i += 4;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "flags")){
                fill_u64_from_string(&entity_to_fill.flags, splitted_line.get(i+1).data);
                i++;
                
                if (entity_to_fill.flags & LIGHT){
                    // entity_to_fill.light_index = session_context.lights.count;
                    // session_context.lights.add({});
                    // init_entity_light(&entity_to_fill);
                    for (i32 i = 0; i < current_level_context->lights.max_count; i++){                    
                        if (i >= session_context.entity_lights_start_index && !current_level_context->lights.get(i).exists){
                            entity_to_fill.light_index = i;   
                        }
                    }
                }
                continue;
            } else if (str_equal(splitted_line.get(i).data, "vertices")){
                // fill_i32_from_string(&entity_to_fill.rotation);
                fill_vertices_array_from_string(&entity_to_fill.vertices, splitted_line, &i);
                // i--;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "unscaled_vertices")){
                // fill_i32_from_string(&entity_to_fill.rotation);
                fill_vertices_array_from_string(&entity_to_fill.unscaled_vertices, splitted_line, &i);
                //i--;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "texture_name")){
                str_copy(entity_to_fill.texture_name, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "draw_order")){
                fill_i32_from_string(&entity_to_fill.draw_order, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_connected")){
                fill_int_array_from_string(&entity_to_fill.trigger.connected, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "kill_switch_connected")){
                fill_int_array_from_string(&entity_to_fill.enemy.kill_switch.connected, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "trigger_tracking")){
                fill_int_array_from_string(&entity_to_fill.trigger.tracking, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "enemy_big_or_small_killable")){
                fill_b32_from_string(&entity_to_fill.enemy.big_sword_killable, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "blocker_clockwise")){
                fill_b32_from_string(&entity_to_fill.enemy.blocker_clockwise, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "blocker_immortal")){
                fill_b32_from_string(&entity_to_fill.enemy.blocker_immortal, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "propeller_power")){
                fill_f32_from_string(&entity_to_fill.propeller.power, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "propeller_spin_sensitive")){
                fill_b32_from_string(&entity_to_fill.propeller.spin_sensitive, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "sword_kill_speed_modifier")){
                fill_f32_from_string(&entity_to_fill.enemy.sword_kill_speed_modifier, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "shoot_blocker_direction")){
                fill_vector2_from_string(&entity_to_fill.enemy.shoot_blocker_direction, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                i += 2;
            } else if (str_equal(splitted_line.get(i).data, "shoot_blocker_immortal")){
                fill_b32_from_string(&entity_to_fill.enemy.shoot_blocker_immortal, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "enemy_gives_ammo")){
                fill_b32_from_string(&entity_to_fill.enemy.gives_ammo, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "explosive_radius_multiplier")){
                fill_f32_from_string(&entity_to_fill.enemy.explosive_radius_multiplier, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_shadows_size_flag")){
                fill_i32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->shadows_size_flags, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_backshadows_size_flag")){
                fill_i32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->backshadows_size_flags, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_make_shadows")){
                fill_b32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->make_shadows, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_make_backshadows")){
                fill_b32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->make_backshadows, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_bake_shadows")){
                fill_b32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->bake_shadows, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_radius")){
                fill_f32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->radius, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_opacity")){
                fill_f32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->opacity, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_power")){
                fill_f32_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->power, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "light_color")){
                fill_vector4_from_string(&current_level_context->lights.get_ptr(entity_to_fill.light_index)->color, splitted_line.get(i+1).data, splitted_line.get(i+2).data, splitted_line.get(i+3).data, splitted_line.get(i+4).data);
                i += 4;
            } else if (str_equal(splitted_line.get(i).data, "trigger_die_after_trigger")){
                fill_b32_from_string(&entity_to_fill.trigger.die_after_trigger, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_kill_player")){
                fill_b32_from_string(&entity_to_fill.trigger.kill_player, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_kill_enemies")){
                fill_b32_from_string(&entity_to_fill.trigger.kill_enemies, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_open_doors")){
                fill_b32_from_string(&entity_to_fill.trigger.open_doors, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_start_physics_simulation")){
                fill_b32_from_string(&entity_to_fill.trigger.start_physics_simulation, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_track_enemies")){
                fill_b32_from_string(&entity_to_fill.trigger.track_enemies, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_draw_lines_to_tracked")){
                fill_b32_from_string(&entity_to_fill.trigger.draw_lines_to_tracked, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_agro_enemies")){
                fill_b32_from_string(&entity_to_fill.trigger.agro_enemies, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_player_touch")){
                fill_b32_from_string(&entity_to_fill.trigger.player_touch, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_start_cam_rails_horizontal")){
                fill_b32_from_string(&entity_to_fill.trigger.start_cam_rails_horizontal, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_start_cam_rails_vertical")){
                fill_b32_from_string(&entity_to_fill.trigger.start_cam_rails_vertical, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_stop_cam_rails")){
                fill_b32_from_string(&entity_to_fill.trigger.stop_cam_rails, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_cam_rails_points")){
                fill_vector2_array_from_string(&entity_to_fill.trigger.cam_rails_points, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "trigger_lock_camera")){
                fill_b32_from_string(&entity_to_fill.trigger.lock_camera, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_unlock_camera")){
                fill_b32_from_string(&entity_to_fill.trigger.unlock_camera, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_allow_player_shoot")){
                fill_b32_from_string(&entity_to_fill.trigger.allow_player_shoot, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_forbid_player_shoot")){
                fill_b32_from_string(&entity_to_fill.trigger.forbid_player_shoot, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_locked_camera_position")){
                fill_vector2_from_string(&entity_to_fill.trigger.locked_camera_position, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                i += 2;
            } else if (str_equal(splitted_line.get(i).data, "spikes_on_right")){
                fill_b32_from_string(&entity_to_fill.centipede.spikes_on_right, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "spikes_on_left")){
                fill_b32_from_string(&entity_to_fill.centipede.spikes_on_left, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "physics_simulating")){
                fill_b32_from_string(&entity_to_fill.physics_object.simulating, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "on_rope")){
                fill_b32_from_string(&entity_to_fill.physics_object.on_rope, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "physics_rotate_by_velocity")){
                fill_b32_from_string(&entity_to_fill.physics_object.rotate_by_velocity, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "physics_rope_point")){
                fill_vector2_from_string(&entity_to_fill.physics_object.rope_point, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                i += 2;
            } else if (str_equal(splitted_line.get(i).data, "physics_gravity_multiplier")){
                fill_f32_from_string(&entity_to_fill.physics_object.gravity_multiplier, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "physics_mass")){
                fill_f32_from_string(&entity_to_fill.physics_object.mass, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "segments_count")){
                fill_i32_from_string(&entity_to_fill.centipede.segments_count, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "door_open")){
                fill_b32_from_string(&entity_to_fill.door.is_open, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_load_level")){
                fill_b32_from_string(&entity_to_fill.trigger.load_level, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_play_replay")){
                fill_b32_from_string(&entity_to_fill.trigger.play_replay, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_level_name")){
                str_copy(entity_to_fill.trigger.level_name, splitted_line.get(i+1).data);  
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_replay_name")){
                str_copy(entity_to_fill.trigger.replay_name, splitted_line.get(i+1).data);  
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_play_sound")){
                fill_b32_from_string(&entity_to_fill.trigger.play_sound, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_change_zoom")){
                fill_b32_from_string(&entity_to_fill.trigger.change_zoom, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_zoom_value")){
                fill_f32_from_string(&entity_to_fill.trigger.zoom_value, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_sound_name")){
                str_copy(entity_to_fill.trigger.sound_name, splitted_line.get(i+1).data);  
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_shows_entities")){
                fill_b32_from_string(&entity_to_fill.trigger.shows_entities, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_starts_moving_sequence")){
                fill_b32_from_string(&entity_to_fill.trigger.starts_moving_sequence, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_moving")){
                fill_b32_from_string(&entity_to_fill.move_sequence.moving, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_loop")){
                fill_b32_from_string(&entity_to_fill.move_sequence.loop, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_rotate")){
                fill_b32_from_string(&entity_to_fill.move_sequence.rotate, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_speed_related_player_distance")){
                fill_b32_from_string(&entity_to_fill.move_sequence.speed_related_player_distance, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_min_distance")){
                fill_f32_from_string(&entity_to_fill.move_sequence.min_distance, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_max_distance")){
                fill_f32_from_string(&entity_to_fill.move_sequence.max_distance, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_max_distance_speed")){
                fill_f32_from_string(&entity_to_fill.move_sequence.max_distance_speed, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "hidden")){
                fill_b32_from_string(&entity_to_fill.hidden, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "spawn_enemy_when_no_ammo")){
                fill_b32_from_string(&entity_to_fill.spawn_enemy_when_no_ammo, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "jump_shooter_explosive_count")){
                fill_i32_from_string(&entity_to_fill.jump_shooter.explosive_count, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_projectile_flags")){
                fill_u64_from_string(&entity_to_fill.enemy.turret.projectile_settings.enemy_flags, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_shoot_sword_blocker_clockwise")){
                fill_b32_from_string(&entity_to_fill.enemy.turret.projectile_settings.blocker_clockwise, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_homing_projectiles")){
                fill_b32_from_string(&entity_to_fill.enemy.turret.homing, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_shoot_every_tick")){
                fill_i32_from_string(&entity_to_fill.enemy.turret.shoot_every_tick, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_start_tick_delay")){
                fill_i32_from_string(&entity_to_fill.enemy.turret.start_tick_delay, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_projectile_speed")){
                fill_f32_from_string(&entity_to_fill.enemy.turret.projectile_settings.launch_speed, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_projectile_max_lifetime")){
                fill_f32_from_string(&entity_to_fill.enemy.turret.projectile_settings.max_lifetime, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_shoot_width")){
                fill_f32_from_string(&entity_to_fill.enemy.turret.shoot_width, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_shoot_height")){
                fill_f32_from_string(&entity_to_fill.enemy.turret.shoot_height, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "turret_activated")){
                fill_b32_from_string(&entity_to_fill.enemy.turret.activated, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "jump_shooter_shoot_sword_blockers")){
                fill_b32_from_string(&entity_to_fill.jump_shooter.shoot_sword_blockers, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "jump_shooter_shoot_sword_blockers_immortal")){
                fill_b32_from_string(&entity_to_fill.jump_shooter.shoot_sword_blockers_immortal, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "jump_shooter_shoot_bullet_blockers")){
                fill_b32_from_string(&entity_to_fill.jump_shooter.shoot_bullet_blockers, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "jump_shooter_shots_count")){
                fill_i32_from_string(&entity_to_fill.jump_shooter.shots_count, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "jump_shooter_spread")){
                fill_f32_from_string(&entity_to_fill.jump_shooter.spread, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_speed")){
                fill_f32_from_string(&entity_to_fill.move_sequence.speed, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_points")){
                fill_vector2_array_from_string(&entity_to_fill.move_sequence.points, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "note_content")){
                //str_copy(note_to_fill.content, splitted_line.get(i+1).data);
                fill_string(note_to_fill.content, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "note_draw_in_game")){
                fill_b32_from_string(&note_to_fill.draw_in_game, splitted_line.get(i+1).data);
                i++;
            } else{
                //assert(false);
                print("Something unknown during level load");
            }
        }
        
        if (parsing_entities){
            if (entity_to_fill.flags & TEXTURE){
                i64 texture_hash = hash_str(get_substring_before_symbol(entity_to_fill.texture_name, '.'));
                char *trimped_name = get_substring_before_symbol(entity_to_fill.texture_name, '.');
                b32 found = false;
                for (i32 i = 0; i < textures_array.count; i++){
                    if (str_equal(textures_array.get(i).name, trimped_name)){
                        entity_to_fill.texture = textures_array.get(i).texture;
                        found = true;
                        break;
                    }
                }
                if (!found){
                    print(text_format("WARNING: While loading entities could not find texture named %s.", trimped_name));
                }
            }
            
            setup_color_changer(&entity_to_fill);
            Entity *added_entity = add_entity(&entity_to_fill, true);
            
            if (added_entity->flags & NOTE){
                assert(added_entity->note_index != -1);
                Note *added_note = current_level_context->notes.get_ptr(added_entity->note_index);
                str_copy(added_note->content, note_to_fill.content);
                added_note->draw_in_game = note_to_fill.draw_in_game;
            }
            
            // init_entity(added_entity);
            //rotate_to(added_entity, added_entity->rotation);
            
            calculate_bounds(added_entity);
        }
    }
    
    game_state = original_game_state;
    
    b32 is_temp_level = str_start_with_const(name, "temp/TEMP_");
    b32 is_autosave   = str_start_with_const(name, "autosaves/AUTOSAVE_");
    if (!is_temp_level && !is_autosave){
        if (!str_equal(current_level_context->level_name, name)){
            str_copy(session_context.previous_level_name, current_level_context->level_name);
        }
        str_copy(current_level_context->level_name, name);
        print_to_console(text_format("Loaded level: %s", name));
        editor.last_autosave_time = core.time.app_time;
    }
    
    //free_string_array(&splitted_line);
    splitted_line.free_arr();
    unload_file(&file);
        
    loop_entities(init_loaded_entity);
    
    setup_context_cam(current_level_context);
    current_level_context->cam.cam2D.zoom = 0.35f;
    
    // We do that so editor has latest level in it.
    // switch_current_level_context(editor_level-cont)e
    clear_level_context(&game_level_context);
    
    // This shit so that we don't overwrite level that we currently on.
    do {
        last_loaded_editor_level_context_index += 1;    
        last_loaded_editor_level_context_index %= MAX_LOADED_LEVELS;    
    } while (last_loaded_editor_level_context_index == current_editor_level_context_index);
    editor_level_context = &loaded_levels_contexts[last_loaded_editor_level_context_index];
    current_editor_level_context_index = last_loaded_editor_level_context_index;
    clear_level_context(editor_level_context);
    copy_level_context(editor_level_context, &loaded_level_context, true);
    
    if (enter_game_state_on_new_level || game_state == GAME || (initing_game && RELEASE_BUILD)){
        enter_game_state(&loaded_level_context, true);
        
        if (enter_game_state_on_new_level){
            player_data->blood_amount = last_player_data.blood_amount;
            player_data->blood_progress = last_player_data.blood_progress;
            player_data->ammo_count = last_player_data.ammo_count;
        }
        
        enter_game_state_on_new_level = false;
    } else{
        enter_editor_state();
    }
    
    current_level_context->cam.position = current_level_context->player_spawn_point;
    current_level_context->cam.target = current_level_context->player_spawn_point;
    return true;
} // end load level end

inline b32 set_next_collision_stuff(i32 current_index, Collision *col, Entity **other){
    if (current_index >= collisions_buffer.count){
        col->collided = false;
        col->other_entity = NULL;
        return false;
    }
    
    *col = collisions_buffer.get(current_index);
    *other = col->other_entity;
    return true;
}

#define ForCollisions(entity, flags) fill_collisions(entity, &collisions_buffer, flags); Entity *other = NULL; Collision col = {}; for (i32 col_index = 0; set_next_collision_stuff(col_index, &col, &other); col_index++)

global_variable Dynamic_Array<Collision_Grid_Cell*> collision_cells_buffer = Dynamic_Array<Collision_Grid_Cell*>(128);

#define MAX_SPAWN_OBJECTS 128

global_variable Array<Spawn_Object, MAX_SPAWN_OBJECTS> spawn_objects = Array<Spawn_Object, MAX_SPAWN_OBJECTS>();

#define BIRD_ENEMY_COLLISION_FLAGS (GROUND | PLAYER | BIRD_ENEMY | CENTIPEDE_SEGMENT | ENEMY_BARRIER | NO_MOVE_BLOCK)

Entity *spawn_object_by_name(const char* name, Vector2 position){
    for (i32 i = 0; i < spawn_objects.count; i++){
        Spawn_Object *obj = spawn_objects.get_ptr(i);
        if (str_equal(obj->name, name)){
            Entity *e = add_entity(&obj->entity);
            e->position = position;
            return e;
        }
    }
    
    printf("No spawn object named %s\n", name);
    return NULL;
}

void bird_clear_formation(Bird_Enemy *bird){
    if (bird->slot_index != -1){
        current_level_context->bird_slots[bird->slot_index].occupied = false;
        bird->slot_index = -1;
    }
}

inline void free_particle_emitter(i32 index){
    assert(index >= 0 && index < current_level_context->particle_emitters.max_count);
    
    Particle_Emitter *emitter = get_particle_emitter(index);
    if (emitter){
        for (i32 i = emitter->particles_start_index; i < emitter->particles_max_index; i++){
            current_level_context->particles.get_ptr(i)->enabled = false;
        }
    } else{
        printf("L_WARNING: No emitter existing on free_particle_emitter. Index is %d\n", index);
    }
    
    *current_level_context->particle_emitters.get_ptr(index) = {};
}

inline void free_particle_emitters(i32 *start_ptr, i32 count){
    for (i32 i = 0; i < count; i++){
        free_particle_emitter(*(start_ptr + i));       
    }
}

inline void free_entity_particle_emitters(Entity *entity){
    Array<i32, MAX_ENTITY_EMITTERS> *emitters_indexes = &entity->particle_emitters_indexes;
    // free_particle_emitters(emitters_indexes->data, emitters_indexes->count);
    for (i32 i = 0; i < emitters_indexes->count; i++){    
        Particle_Emitter *emitter = get_particle_emitter(emitters_indexes->get(i));
        if (emitter){
            emitter->should_extinct = true;
        }
    }
    emitters_indexes->clear();
}

void init_bird_emitters(Entity *entity){
    Array<i32, MAX_ENTITY_EMITTERS> *emitters_indexes = &entity->particle_emitters_indexes;
    free_entity_particle_emitters(entity);
    entity->bird_enemy.trail_emitter_index = add_entity_particle_emitter(entity, entity->flags & EXPLOSIVE ? &little_fire_emitter : &air_dust_emitter);
    enable_emitter(entity->bird_enemy.trail_emitter_index, entity->position);
    entity->bird_enemy.attack_emitter_index = add_entity_particle_emitter(entity, &small_air_dust_trail_emitter_copy);
    entity->bird_enemy.alarm_emitter_index  = add_entity_particle_emitter(entity, &alarm_smoke_emitter_copy);
    entity->bird_enemy.fire_emitter_index = add_entity_particle_emitter(entity, &fire_emitter);
    entity->bird_enemy.smoke_fire_emitter_index = add_entity_particle_emitter(entity, &smoke_fire_emitter_copy);
    entity->bird_enemy.collision_emitter_index = add_entity_particle_emitter(entity, &rifle_bullet_emitter);
}

void init_bird_entity(Entity *entity){
    //entity->flags = ENEMY | BIRD_ENEMY | PARTICLE_EMITTER;
    assert(entity->flags > 0);
    entity->collision_flags = BIRD_ENEMY_COLLISION_FLAGS;//GROUND | PLAYER | BIRD_ENEMY;
    change_color(entity, entity->flags & EXPLOSIVE ? ORANGE * 0.9f : YELLOW * 0.9f);
    
    change_scale(entity, {6, 10});

    entity->enemy.sword_kill_speed_modifier = 4;
    
    init_bird_emitters(entity);
        
    //entity->emitter = entity->emitters.last_ptr();
    str_copy(entity->name, "enemy_bird"); 
    setup_color_changer(entity);
}

void init_spawn_objects(){
    Entity block_base_entity = Entity({0, 0}, {50, 10}, {0.5f, 0.5f}, 0, GROUND);
    block_base_entity.color = BROWN;
    str_copy(block_base_entity.name, "block_base"); 
    setup_color_changer(&block_base_entity);
    
    Spawn_Object block_base_object;
    copy_entity(&block_base_object.entity, &block_base_entity);
    str_copy(block_base_object.name, block_base_entity.name);
    spawn_objects.add(block_base_object);
    
    Entity no_move_block_entity = Entity({0, 0}, {50, 10}, {0.5f, 0.5f}, 0, GROUND | NO_MOVE_BLOCK | LIGHT);
    no_move_block_entity.color = PURPLE;
    str_copy(no_move_block_entity.name, "no_move_block"); 
    setup_color_changer(&no_move_block_entity);
    
    Spawn_Object no_move_block_object;
    copy_entity(&no_move_block_object.entity, &no_move_block_entity);
    str_copy(no_move_block_object.name, no_move_block_entity.name);
    spawn_objects.add(no_move_block_object);
    
    Entity note_entity = Entity({0, 0}, {20, 15}, {0.5f, 0.5f}, 0, NOTE | TEXTURE);
    note_entity.color = Fade(WHITE, 0.7f);
    str_copy(note_entity.name, "note"); 
    str_copy(note_entity.texture_name, "editor_note.png");
    note_entity.texture = get_texture(note_entity.texture_name);
    setup_color_changer(&note_entity);
    
    Spawn_Object note_object;
    copy_entity(&note_object.entity, &note_entity);
    str_copy(note_object.name, note_entity.name);
    spawn_objects.add(note_object);
    
    Entity dummy_entity = Entity({0, 0}, {10, 5}, {0.5f, 0.5f}, 0, DUMMY);
    dummy_entity.color  = Fade(GREEN, 0.5f);
    // dummy_entity.hidden = true;
    str_copy(dummy_entity.name, "dummy_entity"); 
    setup_color_changer(&dummy_entity);
    
    Spawn_Object dummy_object;
    copy_entity(&dummy_object.entity, &dummy_entity);
    str_copy(dummy_object.name, dummy_entity.name);
    spawn_objects.add(dummy_object);
    
    Entity platform_entity = Entity({0, 0}, {50, 5}, {0.5f, 0.5f}, 0, PLATFORM);
    platform_entity.color = Fade(ColorBrightness(BROWN, -0.1f), 0.1f);
    str_copy(platform_entity.name, "platform"); 
    setup_color_changer(&platform_entity);
    
    Spawn_Object platform_object;
    copy_entity(&platform_object.entity, &platform_entity);
    str_copy(platform_object.name, platform_entity.name);
    spawn_objects.add(platform_object);
    
    Entity enemy_ammo_pack_entity = Entity({0, 0}, {5, 5}, {0.5f, 0.5f}, 0, ENEMY | AMMO_PACK);
    enemy_ammo_pack_entity.color = ColorBrightness(RED, -0.1f);
    str_copy(enemy_ammo_pack_entity.name, "ammo_pack"); 
    setup_color_changer(&enemy_ammo_pack_entity);
    
    Spawn_Object enemy_ammo_pack_object;
    copy_entity(&enemy_ammo_pack_object.entity, &enemy_ammo_pack_entity);
    str_copy(enemy_ammo_pack_object.name, enemy_ammo_pack_entity.name);
    spawn_objects.add(enemy_ammo_pack_object);
    
    Entity big_sword_charge_giver_entity = Entity({0, 0}, {10, 10}, {0.5f, 0.5f}, 0, ENEMY | GIVES_BIG_SWORD_CHARGE);
    big_sword_charge_giver_entity.color = ColorBrightness(GREEN, 0.5f);
    str_copy(big_sword_charge_giver_entity.name, "big_sword_charge_giver"); 
    setup_color_changer(&big_sword_charge_giver_entity);
    
    Spawn_Object big_sword_charge_giver_object;
    copy_entity(&big_sword_charge_giver_object.entity, &big_sword_charge_giver_entity);
    str_copy(big_sword_charge_giver_object.name, big_sword_charge_giver_entity.name);
    spawn_objects.add(big_sword_charge_giver_object);
    
    Entity turret_direct_entity = Entity({0, 0}, {5, 15}, {0.5f, 1.0f}, 0, ENEMY | TURRET);
    turret_direct_entity.enemy.unkillable = true;
    turret_direct_entity.color = ColorBrightness(PURPLE, 0.5f);
    str_copy(turret_direct_entity.name, "turret_direct"); 
    setup_color_changer(&turret_direct_entity);
    
    Spawn_Object turret_direct_object;
    copy_entity(&turret_direct_object.entity, &turret_direct_entity);
    str_copy(turret_direct_object.name, turret_direct_entity.name);
    spawn_objects.add(turret_direct_object);
    
    Entity turret_homing_entity = Entity({0, 0}, {5, 15}, {0.5f, 1.0f}, 0, ENEMY | TURRET);
    turret_homing_entity.enemy.unkillable = true;
    {
        Turret *turret = &turret_homing_entity.enemy.turret;
        turret->homing = true;
        turret->projectile_settings.launch_speed = 200;
        turret->projectile_settings.max_lifetime = 5;
        turret->shoot_every_tick = 8;
    }
    turret_homing_entity.color = ColorBrightness(PURPLE, 0.1f);
    str_copy(turret_homing_entity.name, "turret_homing"); 
    setup_color_changer(&turret_homing_entity);
    
    Spawn_Object turret_homing_object;
    copy_entity(&turret_homing_object.entity, &turret_homing_entity);
    str_copy(turret_homing_object.name, turret_homing_entity.name);
    spawn_objects.add(turret_homing_object);
    
    Entity bird_entity = Entity({0, 0}, {6, 10}, {0.5f, 0.5f}, 0, ENEMY | BIRD_ENEMY | PARTICLE_EMITTER);
    init_bird_entity(&bird_entity);
    
    Spawn_Object enemy_bird_object;
    copy_entity(&enemy_bird_object.entity, &bird_entity);
    str_copy(enemy_bird_object.name, bird_entity.name);
    spawn_objects.add(enemy_bird_object);
    
    Entity win_block_entity = Entity({0, 0}, {3, 3}, {0.5f, 0.5f}, 0, WIN_BLOCK | ENEMY | SHOOT_BLOCKER);
    win_block_entity.color = GREEN * 0.9f;
    win_block_entity.color_changer.start_color = win_block_entity.color;
    win_block_entity.color_changer.target_color = win_block_entity.color * 1.5f;
    str_copy(win_block_entity.name, "win_block"); 
    setup_color_changer(&win_block_entity);
    
    Spawn_Object win_block_object;
    copy_entity(&win_block_object.entity, &win_block_entity);
    str_copy(win_block_object.name, win_block_entity.name);
    spawn_objects.add(win_block_object);
    
    Entity agro_area_entity = Entity({0, 0}, {20, 20}, {0.5f, 0.5f}, 0, TRIGGER);
    agro_area_entity.color = Fade(VIOLET, 0.6f);
    agro_area_entity.color_changer.start_color = agro_area_entity.color;
    agro_area_entity.color_changer.target_color = agro_area_entity.color * 1.5f;
    str_copy(agro_area_entity.name, "agro_area"); 
    setup_color_changer(&agro_area_entity);
    
    Spawn_Object argo_area_object;
    copy_entity(&argo_area_object.entity, &agro_area_entity);
    str_copy(argo_area_object.name, agro_area_entity.name);
    spawn_objects.add(argo_area_object);
    
    Entity trigger_entity = Entity({0, 0}, {20, 20}, {0.5f, 0.5f}, 0, TRIGGER);
    trigger_entity.color = Fade(GREEN, 0.6f);
    str_copy(trigger_entity.name, "trigger"); 
    setup_color_changer(&trigger_entity);
    
    Spawn_Object trigger_object;
    copy_entity(&trigger_object.entity, &trigger_entity);
    str_copy(trigger_object.name, trigger_entity.name);
    spawn_objects.add(trigger_object);
    
    Entity kill_trigger_entity = Entity({0, 0}, {20, 20}, {0.5f, 0.5f}, 0, TRIGGER);
    kill_trigger_entity.trigger.kill_player = true;
    kill_trigger_entity.color = Fade(RED, 0.6f);
    str_copy(kill_trigger_entity.name, "kill_trigger"); 
    setup_color_changer(&kill_trigger_entity);
    
    Spawn_Object kill_trigger_object;
    copy_entity(&kill_trigger_object.entity, &kill_trigger_entity);
    str_copy(kill_trigger_object.name, kill_trigger_entity.name);
    spawn_objects.add(kill_trigger_object);
    
    Entity kill_switch_entity = Entity({0, 0}, {20, 10}, {0.5f, 0.5f}, 0, ENEMY | KILL_SWITCH);
    kill_switch_entity.color = ColorBrightness(RED, 0.3f);
    str_copy(kill_switch_entity.name, "kill_switch"); 
    setup_color_changer(&kill_switch_entity);
    
    Spawn_Object kill_switch_object;
    copy_entity(&kill_switch_object.entity, &kill_switch_entity);
    str_copy(kill_switch_object.name, kill_switch_entity.name);
    spawn_objects.add(kill_switch_object);
    
    Entity enemy_barrier_entity = Entity({0, 0}, {20, 80}, {0.5f, 0.5f}, 0, ENEMY | ENEMY_BARRIER | MULTIPLE_HITS);
    enemy_barrier_entity.color = ColorBrightness(GRAY, 0.2f);
    str_copy(enemy_barrier_entity.name, "enemy_barrier"); 
    setup_color_changer(&enemy_barrier_entity);
    
    Spawn_Object enemy_barrier_object;
    copy_entity(&enemy_barrier_object.entity, &enemy_barrier_entity);
    str_copy(enemy_barrier_object.name, enemy_barrier_entity.name);
    spawn_objects.add(enemy_barrier_object);
    
    Entity spikes_entity = Entity({0, 0}, {20, 5}, {0.5f, 0.5f}, 0, TRIGGER | SPIKES);
    spikes_entity.trigger.kill_player = true;
    spikes_entity.color = Fade(RED, 0.9f);
    str_copy(spikes_entity.name, "spikes"); 
    setup_color_changer(&spikes_entity);
    
    Spawn_Object spikes_object;
    copy_entity(&spikes_object.entity, &spikes_entity);
    str_copy(spikes_object.name, spikes_entity.name);
    spawn_objects.add(spikes_object);
    
    Entity propeller_entity = Entity({0, 0}, {20, 120}, {0.5f, 1.0f}, 0, PROPELLER);
    propeller_entity.color = Fade(BLUE, 0.4f);
    propeller_entity.color_changer.start_color = propeller_entity.color;
    propeller_entity.color_changer.target_color = propeller_entity.color * 1.5f;
    str_copy(propeller_entity.name, "propeller"); 
    setup_color_changer(&propeller_entity);
    
    Spawn_Object propeller_object;
    copy_entity(&propeller_object.entity, &propeller_entity);
    str_copy(propeller_object.name, propeller_entity.name);
    spawn_objects.add(propeller_object);
    
    Entity door_entity = Entity({0, 0}, {5, 80}, {0.5f, 0.5f}, 0, DOOR | GROUND | TRIGGER);
    door_entity.trigger.player_touch = false;
    door_entity.color = ColorBrightness(PURPLE, 0.6f);
    str_copy(door_entity.name, "door"); 
    setup_color_changer(&door_entity);
    
    Spawn_Object door_object;
    copy_entity(&door_object.entity, &door_entity);
    str_copy(door_object.name, door_entity.name);
    spawn_objects.add(door_object);
    
    Entity enemy_trigger_entity = Entity({0, 0}, {10, 75}, {0.5f, 0.5f}, 0, ENEMY | TRIGGER);
    enemy_trigger_entity.trigger.player_touch = false;
    enemy_trigger_entity.color = ColorBrightness(BLUE, 0.6f);
    str_copy(enemy_trigger_entity.name, "enemy_trigger"); 
    setup_color_changer(&enemy_trigger_entity);
    
    Spawn_Object enemy_trigger_object;
    copy_entity(&enemy_trigger_object.entity, &enemy_trigger_entity);
    str_copy(enemy_trigger_object.name, enemy_trigger_entity.name);
    spawn_objects.add(enemy_trigger_object);
    
    Entity centipede_entity = Entity({0, 0}, {7, 5}, {0.5f, 0.5f}, 0, CENTIPEDE | MOVE_SEQUENCE | ENEMY);
    centipede_entity.move_sequence.moving = true;
    centipede_entity.move_sequence.loop = true;
    centipede_entity.move_sequence.rotate = true;
    centipede_entity.color = ColorBrightness(RED, 0.6f);
    str_copy(centipede_entity.name, "centipede"); 
    setup_color_changer(&centipede_entity);
    
    Spawn_Object centipede_object;
    copy_entity(&centipede_object.entity, &centipede_entity);
    str_copy(centipede_object.name, centipede_entity.name);
    spawn_objects.add(centipede_object);
    
    Entity centipede_segment_entity = Entity({0, 0}, {4, 6}, {0.5f, 0.5f}, 0, ENEMY | CENTIPEDE_SEGMENT | MOVE_SEQUENCE);
    centipede_segment_entity.need_to_save = false;
    centipede_segment_entity.color = ColorBrightness(ORANGE, 0.3f);
    str_copy(centipede_segment_entity.name, "centipede_segment"); 
    setup_color_changer(&centipede_segment_entity);
    
    Spawn_Object centipede_segment_object;
    copy_entity(&centipede_segment_object.entity, &centipede_segment_entity);
    str_copy(centipede_segment_object.name, centipede_segment_entity.name);
    spawn_objects.add(centipede_segment_object);
    
    Entity shoot_stoper_entity = Entity({0, 0}, {8, 14}, {0.5f, 0.5f}, 0, ENEMY | SHOOT_STOPER);
    shoot_stoper_entity.color = ColorBrightness(BLACK, 0.3f);
    str_copy(shoot_stoper_entity.name, "shoot_stoper"); 
    setup_color_changer(&shoot_stoper_entity);
    
    Spawn_Object shoot_stoper_object;
    copy_entity(&shoot_stoper_object.entity, &shoot_stoper_entity);
    str_copy(shoot_stoper_object.name, shoot_stoper_entity.name);
    spawn_objects.add(shoot_stoper_object);
    
    Entity hit_booster_entity = Entity({0, 0}, {8, 12}, {0.5f, 0.5f}, 0, ENEMY | HIT_BOOSTER);
    hit_booster_entity.color = ColorBrightness(YELLOW, 0.3f);
    hit_booster_entity.enemy.max_hits_taken = -1;
    str_copy(hit_booster_entity.name, "hit_booster"); 
    setup_color_changer(&hit_booster_entity);
    
    Spawn_Object hit_booster_object;
    copy_entity(&hit_booster_object.entity, &hit_booster_entity);
    str_copy(hit_booster_object.name, hit_booster_entity.name);
    spawn_objects.add(hit_booster_object);
    
    // we use move sequence on jump shooter only to set jump points
    Entity jump_shooter_entity = Entity({0, 0}, {10, 14}, {0.5f, 0.5f}, 0, ENEMY | JUMP_SHOOTER | MOVE_SEQUENCE | PARTICLE_EMITTER);
    jump_shooter_entity.move_sequence.moving = true;
    jump_shooter_entity.move_sequence.loop = true;
    jump_shooter_entity.enemy.max_hits_taken = 6;
    jump_shooter_entity.color = ColorBrightness(BLACK, 0.3f);
    str_copy(jump_shooter_entity.name, "jump_shooter"); 
    setup_color_changer(&jump_shooter_entity);
    
    Spawn_Object jump_shooter_object;
    copy_entity(&jump_shooter_object.entity, &jump_shooter_entity);
    str_copy(jump_shooter_object.name, jump_shooter_entity.name);
    spawn_objects.add(jump_shooter_object);
}

void add_spawn_object_from_texture(Texture texture, char *name){
    Entity texture_entity = Entity({0, 0}, {texture.width / 10.0f, texture.height / 10.0f}, {0.5f, 0.5f}, 0, texture, TEXTURE);
    texture_entity.color = WHITE;
    texture_entity.color_changer.start_color = texture_entity.color;
    texture_entity.color_changer.target_color = texture_entity.color * 1.5f;
    str_copy(texture_entity.name, name); 
    
    texture_entity.texture = texture;
    str_copy(texture_entity.texture_name, name);
    // assign_texture(&texture_entity, texture, name);
    
    Spawn_Object texture_object;
    copy_entity(&texture_object.entity, &texture_entity);
    str_copy(texture_object.name, texture_entity.name);
    
    spawn_objects.add(texture_object);
}

Texture spiral_clockwise_texture;
Texture spiral_counterclockwise_texture;
Texture hitmark_small_texture;
Texture jump_shooter_bullet_hint_texture;
Texture big_sword_killable_texture;
Texture small_sword_killable_texture;
Texture perlin_texture;
Texture missing_texture;

Sound_Handler *missing_sound = NULL;

Texture get_texture(const char *name){
    Texture found_texture;
    
    char *trimped_name = get_substring_before_symbol(name, '.');
    
    b32 found = false;
    for (i32 i = 0; i < textures_array.count; i++){
        if (str_equal(textures_array.get(i).name, trimped_name)){
            found_texture = textures_array.get(i).texture;
            found = true;
        }
    }
    if (!found){
        print(text_format("WARNING: Texture named %s cannot be found", trimped_name));
        found_texture = missing_texture;
    }
    
    return found_texture;
}

void load_textures(){
    FilePathList textures = LoadDirectoryFiles("resources\\textures");
    for (i32 i = 0; i < textures.count; i++){
        char *name = textures.paths[i];
        
        // if (!str_end_with(name, ".png")){
        //     continue;
        // }
        
        Texture texture = LoadTexture(name);
        
        substring_after_line(name, "resources\\textures\\");
        name = get_substring_before_symbol(name, '.');
        
        // i64 hash = hash_str(name);
        Texture_Data data = {};
        str_copy(data.name, name);
        data.texture = texture;
        
        textures_array.add(data);
        
        add_spawn_object_from_texture(texture, name);
    }
    UnloadDirectoryFiles(textures);
    
    missing_texture                 = get_texture("MissingTexture");
    spiral_clockwise_texture        = get_texture("vpravo");
    spiral_counterclockwise_texture = get_texture("levo");
    hitmark_small_texture           = get_texture("hitmark_small");
}

inline void loop_entities(void (func)(Entity*)){
    for (i32 i = 0; i < current_level_context->entities.max_count; i++){
        if (!current_level_context->entities.has_index(i)){
            continue;
        }
    
        Entity *e = current_level_context->entities.get_ptr(i);
        if (!e->enabled){
            continue;
        }
        
        func(e);
    }
}

void print_to_console(const char *text){
    console.str += text_format("\t>%s\n", text);
}

inline void init_loaded_entity(Entity *entity){
    // if (entity->flags & BIRD_ENEMY){
    //     init_bird_entity(entity);
    // }
}


inline i32 table_next_avaliable(Hash_Table_Int<Entity> table, i32 index, FLAGS flags){
    for (i32 i = index; i < table.max_count; i++){
        if (table.has_index(i) && (flags == 0 || table.get_ptr(i)->flags & flags)){
            return i;
        }
    }
    
    return table.max_count;
}

inline i32 next_entity_avaliable(Level_Context *level_context, i32 index, Entity **entity, FLAGS flags){
    for (i32 i = index; i < level_context->entities.max_count; i++){
        if (level_context->entities.has_index(i) && (flags == 0 || level_context->entities.get_ptr(i)->flags & flags)){
            *entity = level_context->entities.get_ptr(i);
            return i;
        }
    }
    
    *entity = NULL;
    return level_context->entities.max_count;
}

// inline void assign_texture(Entity *entity, Texture texture, const char *texture_name){
//     entity->texture = texture;
//     str_copy(entity->texture_name, texture_name);
// }

void init_light(Light *light){
    // if (light->exists){
    //     free_light(light);        
    // }

    if (light->shadows_size_flags        & ULTRA_SMALL_LIGHT){
        light->shadows_size = 64;
    } else if (light->shadows_size_flags & SMALL_LIGHT){
        light->shadows_size = 128;
    } else if (light->shadows_size_flags & MEDIUM_LIGHT){
        light->shadows_size = 256;
    } else if (light->shadows_size_flags & BIG_LIGHT){
        light->shadows_size = 512;
    } else if (light->shadows_size_flags & HUGE_LIGHT){
        light->shadows_size = 1024;
    } else if (light->shadows_size_flags & GIANT_LIGHT){
        light->shadows_size = 2048;
    }
    
    if (light->backshadows_size_flags        & ULTRA_SMALL_LIGHT){
        light->backshadows_size = 64;
    } else if (light->backshadows_size_flags & SMALL_LIGHT){
        light->backshadows_size = 128;
    } else if (light->backshadows_size_flags & MEDIUM_LIGHT){
        light->backshadows_size = 256;
    } else if (light->backshadows_size_flags & BIG_LIGHT){
        light->backshadows_size = 512;
    } else if (light->backshadows_size_flags & HUGE_LIGHT){
        light->backshadows_size = 1024;
    } else if (light->backshadows_size_flags & GIANT_LIGHT){
        light->backshadows_size = 2048;
    }
    
    if (light->bake_shadows){
        // light->geometry_size = fminf(light->shadows_size * 4, 2048);
    }
    
    if (light->make_shadows){
        light->shadowmask_rt  = LoadRenderTexture(light->shadows_size, light->shadows_size);
    } else{
        light->shadowmask_rt = {};
    }
    
    if (light->make_backshadows){
        light->backshadows_rt = LoadRenderTexture(light->backshadows_size, light->backshadows_size);
    } else{
        light->backshadows_rt = {};
    }
            
    if (light->make_shadows || light->make_backshadows){
        // light->geometry_rt = LoadRenderTexture(light->geometry_size, light->geometry_size);
    }
}

void copy_light(Light *dest, Light *src){
    Light original_dest = *dest;
    *dest = *src;
    dest->shadowmask_rt = original_dest.shadowmask_rt;
    dest->backshadows_rt = original_dest.backshadows_rt;
}

void init_propeller_emitter_settings(Entity *e, Particle_Emitter *air_emitter){
    enable_emitter(air_emitter);
    air_emitter->position            = e->position;
    air_emitter->over_distance       = 0;
    air_emitter->speed_multiplier    = e->propeller.power / 5.0f;
    air_emitter->count_multiplier    = e->propeller.power / 5.0f;
    air_emitter->lifetime_multiplier = (1.8f * (e->scale.y / 120.0f)) / air_emitter->speed_multiplier;
    air_emitter->spawn_offset        = e->up * e->scale.x * 0.5f;
    air_emitter->spawn_area          = {e->scale.x, e->scale.x};
    air_emitter->direction           = e->up;
}

Light *init_entity_light(Entity *entity, Light *light_copy, b32 free_light){
    Light *new_light = NULL;
    
    //Means we will copy ourselves, maybe someone changed size or any other shit
    if (!light_copy && entity->light_index > -1){
        light_copy = current_level_context->lights.get_ptr(entity->light_index);
    } else if (!light_copy){
    }
    
    if (free_light){
        free_entity_light(entity);
    }
    
    for (i32 i = 0; i < current_level_context->lights.max_count; i++){
        if (!current_level_context->lights.get_ptr(i)->exists && i >= session_context.entity_lights_start_index){
            new_light = current_level_context->lights.get_ptr(i);
            entity->light_index = i;
            break;
        } else{
        }
    }
    if (new_light){
        if (light_copy){
            copy_light(new_light, light_copy);
        } else{
            *new_light = {};
            new_light->color = WHITE;
        }
        
        new_light->connected_entity_id = entity->id;

        init_light(new_light);
        
        new_light->exists = true;
    } else{
        print("WARNING: Could not found light to init new, everyting was consumed");
    }
    
    return new_light;
}

void init_entity(Entity *entity){
    if (entity->flags & ENEMY){
        entity->enemy.original_scale = entity->scale;
    }
    
    if (entity->flags & BIRD_ENEMY){
        entity->enemy.max_hits_taken = 3;
        init_bird_entity(entity);
    }
    
    // init kill switch
    if (entity->flags & KILL_SWITCH){
        entity->enemy.max_hits_taken = 5;
    }
    
    if (entity->flags & ENEMY_BARRIER){
        entity->enemy.max_hits_taken = 5;
    }
    
    // init hit booster
    if (entity->flags & HIT_BOOSTER){
        entity->enemy.max_hits_taken = -1;
    }
    
    // init turret
    if (entity->flags & TURRET){
        entity->enemy.unkillable = true;
    }
    
    // init explosive
    if (entity->flags & EXPLOSIVE){
        entity->color_changer.change_time = 5.0f;
        Light explosive_light = {};
        if (entity->light_index != -1){
            explosive_light = current_level_context->lights.get(entity->light_index);
        } 
        // explosive_light.make_backshadows = false; @WTF screen goes black in game mode with this shit. Should change the way lights stored and way we get access to them so don't bother, but wtf ebat (also render doc don't loading with this shit)
        if (entity->enemy.explosive_radius_multiplier >= 3){
            explosive_light.shadows_size_flags = BIG_LIGHT;
            explosive_light.backshadows_size_flags = BIG_LIGHT;
            explosive_light.make_backshadows = true;
            explosive_light.make_shadows     = true;
        } else if (entity->enemy.explosive_radius_multiplier > 1.5f){
            explosive_light.shadows_size_flags = MEDIUM_LIGHT;
            explosive_light.backshadows_size_flags = MEDIUM_LIGHT;
            explosive_light.make_backshadows = true;
            explosive_light.make_shadows     = true;
        } else{
            explosive_light.make_shadows     = false;
            explosive_light.make_backshadows = false;
        }
        
        Light *new_light = init_entity_light(entity, &explosive_light, true);
        if (new_light){
            new_light->radius = 120;
            new_light->color = Fade(ColorBrightness(ORANGE, 0.2f), 1.0f);
            new_light->power = 1.0f;
        }
    }
    
    // init no move block
    if (entity->flags & NO_MOVE_BLOCK){
        Light *new_light = init_entity_light(entity);
        if (new_light){
            new_light->color = entity->color;
            new_light->bake_shadows = true;
            new_light->opacity = 0.5f;
        }
    }
    
    if (entity->flags & BLOCKER && game_state == GAME){
        // init blocker
        if (entity->enemy.blocker_sticky_id != -1 && current_level_context->entities.has_key(entity->enemy.blocker_sticky_id)){
            current_level_context->entities.get_by_key_ptr(entity->enemy.blocker_sticky_id)->destroyed = true;
        }
        
        if (!entity->enemy.blocker_immortal){
            Texture texture = entity->enemy.blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
            Entity *sticky_entity = add_entity(entity->position, {10, 10}, {0.5f, 0.5f}, 0, texture, TEXTURE | STICKY_TEXTURE);
            init_entity(sticky_entity);
            str_copy(sticky_entity->name, "blocker_attack_mark");
            sticky_entity->need_to_save = false;
            //sticky_entity->texture = texture;
            sticky_entity->draw_order = 1;
            sticky_entity->sticky_texture.max_lifetime   = 0;
            // sticky_entity->sticky_texture.line_color  = Fade(ORANGE, 0.3f);
            sticky_entity->sticky_texture.need_to_follow = true;
            sticky_entity->sticky_texture.follow_id      = entity->id;
            sticky_entity->sticky_texture.birth_time     = core.time.game_time;
            
            sticky_entity->sticky_texture.alpha = 0.8f;
            
            entity->enemy.blocker_sticky_id = sticky_entity->id;
        }
    }
    
    if (entity->flags & SWORD_SIZE_REQUIRED && game_state == GAME){
        // init sword size required
        if (entity->enemy.sword_required_sticky_id != -1 && current_level_context->entities.has_key(entity->enemy.sword_required_sticky_id)){
            current_level_context->entities.get_by_key_ptr(entity->enemy.sword_required_sticky_id)->destroyed = true;
        }
        
        Texture texture = entity->enemy.big_sword_killable ? big_sword_killable_texture : small_sword_killable_texture;
        Entity *sticky_entity = add_entity(entity->position, {15, 30}, {0.5f, 0.5f}, 0, texture, TEXTURE | STICKY_TEXTURE);
        
        sticky_entity->sticky_texture.base_size = {4, 8};
        if (!entity->enemy.big_sword_killable){
            sticky_entity->sticky_texture.base_size = {6, 12};
            sticky_entity->sticky_texture.alpha = 0.8f;
        } else{
            sticky_entity->sticky_texture.alpha = 0.4f;
        }
        
        init_entity(sticky_entity);
        str_copy(sticky_entity->name, "sword_size_attack_mark");
        sticky_entity->need_to_save = false;
        //sticky_entity->texture = texture;
        sticky_entity->draw_order = 1;
        sticky_entity->sticky_texture.max_lifetime   = 0;
        // sticky_entity->sticky_texture.line_color     = Fade(BLUE, 0.3f);
        sticky_entity->sticky_texture.need_to_follow = true;
        // sticky_entity->sticky_texture.draw_line      = true;
        sticky_entity->sticky_texture.follow_id      = entity->id;
        sticky_entity->sticky_texture.birth_time     = core.time.game_time;
        
        entity->enemy.sword_required_sticky_id = sticky_entity->id;
    }
    
    // init propeller
    if (entity->flags & PROPELLER){
        // entity->emitters.clear();
        free_entity_particle_emitters(entity);
        entity->propeller.air_emitter_index = add_entity_particle_emitter(entity, &air_emitter_copy);
        
        Particle_Emitter *air_emitter = get_particle_emitter(entity->propeller.air_emitter_index);
        
        if (air_emitter){
            init_propeller_emitter_settings(entity, air_emitter);
        }
    }        
    
    if (entity->flags & DOOR){
        entity->flags |= TRIGGER;
        entity->trigger.player_touch = false;
        //entity->door.open_sound = sounds_array.get_by_key_ptr(hash_str("OpenDoor"));
        //entity->door.is_open = false;
    }
    
    if (entity->flags & CENTIPEDE && game_state == GAME){
        // init centipede
        // free_entity(entity);
        
        Centipede *centipede = &entity->centipede;
        centipede->segments_ids.clear();
        // centipede->segments_ids.add(entity->id);
        // centipede->segments.clear();
        for (i32 i = 0; i < centipede->segments_count; i++){
            Entity* segment = spawn_object_by_name("centipede_segment", entity->position);
            segment->centipede_head = entity;
            change_up(segment, entity->up);
            segment->draw_order = entity->draw_order + 1;
            centipede->segments_ids.add(segment->id);
            Entity *previous;
            if (i > 0){
                previous = current_level_context->entities.get_by_key_ptr(centipede->segments_ids.get(i-1));
            } else{
                previous = entity;
            }

            segment->position = previous->position - previous->up * previous->scale.y * 1.0f;
            segment->move_sequence = entity->move_sequence;
            
            segment->hidden = entity->hidden;
            
            segment->flags = (entity->flags ^ CENTIPEDE) | CENTIPEDE_SEGMENT;
            segment->enemy = entity->enemy;
            init_entity(segment);
        }
        
        entity->flags ^= ENEMY;
    }
    
    if (entity->flags & PHYSICS_OBJECT && game_state == GAME){
        if (entity->physics_object.on_rope){
            Entity *rope_entity = NULL;
            if (entity->physics_object.rope_id == -1){
                rope_entity = add_entity(entity->position, {1, 10}, {0.5f, 1.0f}, 0, BLACK, BLOCK_ROPE);
                rope_entity->need_to_save = false;
                entity->physics_object.rope_id = rope_entity->id;
            } else{
                rope_entity = get_entity_by_id(entity->physics_object.rope_id);
            }
            
            // spawn rope point and check
            Entity *up_rope_point_entity = NULL;
            if (entity->physics_object.up_rope_point_id == -1){
                up_rope_point_entity = add_entity(entity->physics_object.rope_point, {5, 5}, {0.5f, 0.5f}, 0, GREEN, ROPE_POINT);
                up_rope_point_entity->draw_order = entity->draw_order - 1;
                up_rope_point_entity->need_to_save = false;
                entity->physics_object.up_rope_point_id = up_rope_point_entity->id;
            } else{
                up_rope_point_entity = get_entity_by_id(entity->physics_object.up_rope_point_id);
            }
            
            Entity *down_rope_point_entity = NULL;
            if (entity->physics_object.down_rope_point_id == -1){
                down_rope_point_entity = add_entity(entity->physics_object.rope_point, {5, 5}, {0.5f, 0.5f}, 0, GREEN, ROPE_POINT);
                down_rope_point_entity->draw_order = entity->draw_order - 1;
                down_rope_point_entity->need_to_save = false;
                entity->physics_object.down_rope_point_id = down_rope_point_entity->id;
            } else{
                down_rope_point_entity =  get_entity_by_id(entity->physics_object.down_rope_point_id);
            }
        }
    }
    
    if (entity->flags & JUMP_SHOOTER){
        // init jump shooter
        entity->enemy.max_hits_taken = 6;
        free_entity_particle_emitters(entity);
        entity->jump_shooter.trail_emitter_index  = add_entity_particle_emitter(entity, &air_dust_emitter);
        
        Particle_Emitter *trail_emitter = get_particle_emitter(entity->jump_shooter.trail_emitter_index);
        if (trail_emitter){
            trail_emitter->follow_entity = false;
            enable_emitter(trail_emitter, entity->position);
        }
        
        entity->jump_shooter.flying_emitter_index = add_entity_particle_emitter(entity, &small_air_dust_trail_emitter_copy);
        Particle_Emitter *flying_emitter = get_particle_emitter(entity->jump_shooter.flying_emitter_index);
        if (flying_emitter){
            flying_emitter->follow_entity = false;
        }
        
        entity->enemy.sword_kill_speed_modifier = 10;
    }
    
    if (entity->flags & PHYSICS_OBJECT || entity->collision_flags == 0){
        entity->collision_flags |= GROUND | ENEMY | PLAYER;
    }
    
    //init light
    if (entity->flags & LIGHT){
        // Light old_light = {};
        // if (entity->light_index != -1){
        //     Light *current_light = current_level_context->lights.get_ptr(entity->light_index);           
        //     UnloadRenderTexture(current_light->shadowmask_rt);
        //     UnloadRenderTexture(current_light->geometry_rt);
        //     UnloadRenderTexture(current_light->backshadows_rt);
        //     current_light->exists = false;
        //     old_light = *current_light;
        //     *current_light = {};
        //     entity->light_index = -1;
        // }
        // init_entity_light(entity, NULL, true);
    }
    
    // init trigger 
    if (entity->flags & TRIGGER){
        if (entity->trigger.cam_rails_points.max_count > 0 && !entity->trigger.start_cam_rails_horizontal && !entity->trigger.start_cam_rails_vertical){
            entity->trigger.cam_rails_points.free_arr();            
        }
    }
    
    setup_color_changer(entity);
} // end init entity

inline void save_current_level(){
    save_level(current_level_context->level_name);
}

inline void autosave_level(){
    i32 max_autosaves = 5;
    i32 autosave_index = -1;    
    for (i32 i = 0; i < max_autosaves; i++){
        const char *path = text_format("levels/autosaves/AUTOSAVE_%d_%s.level", i, current_level_context->level_name);        
        if (!FileExists(path)){
            autosave_index = i;
            break;
        }
    }
    
    // Means we did not found vacant number so we'll see for oldest
    if (autosave_index == -1){
        i64 oldest_time = -1;
        
        for (i32 i = 0; i < max_autosaves; i++){
            const char *path = text_format("levels/autosaves/AUTOSAVE_%d_%s.level", i, current_level_context->level_name);        
            u64 modification_time = GetFileModTime(path);
            if (oldest_time == -1 || modification_time < oldest_time){
                oldest_time = modification_time;
                autosave_index = i;
            }
        }
    }
    
    assert(autosave_index != -1);
    
    save_level(text_format("autosaves/AUTOSAVE_%d_%s", autosave_index, current_level_context->level_name));
}

void load_level_by_name(const char *name){
    editor.last_autosave_time = core.time.app_time;
    load_level(name);
    
    // enter_editor_state();
}

void try_load_next_level(){
    b32 found = false;
    ForEntities(entity, TRIGGER){
        if (entity->trigger.load_level){
            found = true;
            if (load_level(entity->trigger.level_name)){            
                print_to_console("Next level loaded successfuly");
                enter_editor_state();
            } else{
                print_to_console("Next level FAILED TO LOAD");
            }
            break;
        }
    }
    
    if (!found){
        print_to_console("Could not find trigger that will load next level");
    }
}

void try_load_previous_level(){
    if (session_context.previous_level_name[0]){
        if (load_level(session_context.previous_level_name)){
            print_to_console("Previous level loaded successfuly");
            // enter_editor_state();
        } else{
            print_to_console("Previous level FAILED TO LOAD");
        }
    } else{
        print_to_console("No previous level saved in buffer");
    }
}

void reload_level(){
    load_level(current_level_context->level_name);       
}

Console_Command make_console_command(const char *name, void (func)() = NULL, void (func_arg)(const char*) = NULL){
    Console_Command command;
    str_copy(command.name, name);
    command.func = func;
    command.func_arg = func_arg;
    return command;
}

void print_current_level(){
    console.str += "\t>";
    console.str += current_level_context->level_name;
    console.str += "\n";
}

void create_level(const char *level_name){
    char *name;
    name = get_substring_before_symbol(level_name, '.');
    const char *path = text_format("levels/%s.level", name);
    
    FILE *fptr = fopen(path, "r");
    
    if (fptr != NULL){
         console.str += "this level already exists\n";
    } else{
        clean_up_scene();
        // switch_current_level_context(&loaded_level_context);
        // clear_level_context(&loaded_level_context);
        clear_level_context(editor_level_context);
        
        save_level(level_name);
        enter_editor_state();
        console.str += "Level successfuly created";
    }
    
    if (fptr){
        fclose(fptr);
    }
}

void print_create_level_hint(){
    console.str += "\t>Provide level name";
}

void reload_level_files(){
    console.level_files.clear();
    FilePathList levels = LoadDirectoryFiles("levels");
    for (i32 i = 0; i < levels.count; i++){
        char *name = levels.paths[i];
        
        if (!str_end_with(name, ".level") || str_contains(name, "TEMP_") || str_contains(name, "AUTOSAVE_")){
            continue;
        }
        
        Medium_Str level_name;
        str_copy(level_name.data, name);
        substring_after_line(level_name.data, "levels\\");
        substring_before_symbol(level_name.data, '.');
        console.level_files.add(level_name);
    }
    UnloadDirectoryFiles(levels);
}

void print_hotkeys_to_console(){
    console.str += "\t>Ctrl+LeftMouse - Multiselect\n";
    console.str += "\t>Ctrl+Shift+Mouse - Move multiselected\n";
    console.str += "\t>Ctrl+RightMouse - Exclude multiselect\n";
    console.str += "\t>Ctrl+Shift+Space - Toggle Game/Editor\n";
    console.str += "\t>Ctrl+S - Save current level.\n";
    console.str += "\t>Alt - See and move vertices with mouse.\n";
    console.str += "\t>Alt+V - While moving vertex for snap it to closest.\n";
    console.str += "\t>Alt+1-5 - Fast entities creation.\n";
    console.str += "\t>Space - Create menu\n";
    console.str += "\t>P - Move player spawn point\n";
    console.str += "\t>TAB - Pause in game\n";
    console.str += "\t>Ctrl+Shift+H - Freecam in game\n";
    console.str += "\t>Right_Ctrl+L - Unlock camera\n";
    console.str += "\t>K - Teleport player to mouse in game\n";
    console.str += "\t>WASD - Scaling selected entity. Hold Alt for big.\n";
    console.str += "\t>QE - Rotating selected entity. Hold Alt for big.\n";
    
    console.str += "Commands:\n\t>debug - debug commands info\n";
    console.str += "\t>save <level> - save current level or specify level where to save\n";
    console.str += "\t>level <level> - get current level name or load level if provided\n";
    console.str += "\t>load <level> - load level\n";
    console.str += "\t>create / new_level <level> - create empty level\n";
    console.str += "\t>next / previous / reload / restart_game / first\n";
    console.str += "\t>level_speedrun - Speedrun for levels\n";
    console.str += "\t>game_speedrun - Whole game speedrun. Death puts in game begining.\n";
    console.str += "\t>speedrun_disable\n";
}

void debug_unlock_camera(){
    state_context.cam_state.locked = false;
    state_context.cam_state.on_rails_horizontal = false;
    state_context.cam_state.on_rails_vertical   = false;
}

void print_debug_commands_to_console(){
    console.str += "\t\t>Debug Functions:\n";
    console.str += "\t>infinite_ammo\n";
    console.str += "\t>die\n";
    console.str += "\t>enemy_ai\n";
    console.str += "\t>god_mode\n";
    console.str += "\t>add_ammo\n";
    console.str += "\t>unlock_camera\n";
    console.str += "\t>full_light\n";
    console.str += "\t>collision_grid\n";
    console.str += "\t>timescale <scale>\n";
    console.str += "\t>draw_triggers\n";
    
    console.str += "\t>play_replay - Plays current recorded game state replay. (Single level)\n";
    console.str += "\t>save_replay - Saves current replay to file\n";
    console.str += "\t>replay_load - Loads replay from file\n";
    
    console.str += "\t\t>Debug Info:\n";
    console.str += "\t>player_speed\n";
    console.str += "\t>entities_count\n";
}

void debug_toggle_player_speed(){
    debug.info_player_speed = !debug.info_player_speed;
}

void debug_toggle_view_only_lightmaps(){
    debug.view_only_lightmaps = !debug.view_only_lightmaps;
}

void debug_print_entities_count(){
    i32 count = 0;
    ForEntities(entity, 0){
        count++;
    }
    console.str += text_format("\t>Entities count: %d\n", count);
}

void debug_infinite_ammo(){
    debug.infinite_ammo = !debug.infinite_ammo;
    console.str += text_format("\t>Infinite ammo %s\n", debug.infinite_ammo ? "enabled" : "disabled");
}

void debug_enemy_ai(){
    debug.enemy_ai = !debug.enemy_ai;
    console.str += text_format("\t>Enemy ai %s\n", debug.enemy_ai ? "enabled" : "disabled");
}

void debug_god_mode(){
    debug.god_mode = !debug.god_mode;
    console.str += text_format("\t>God mode %s\n", debug.god_mode ? "enabled" : "disabled");
}

void debug_toggle_full_light(){
    debug.full_light = !debug.full_light;
    console.str += text_format("\t>Full light is %s\n", debug.full_light ? "enabled" : "disabled");
}

void debug_toggle_lightmap_view(){
    if (debug.full_light){
        debug.full_light = false;
        debug.view_only_lightmaps = true;
    } else if (debug.view_only_lightmaps){
        debug.view_only_lightmaps = false;
    } else{
        debug.full_light = true;
    }
}

void debug_toggle_collision_grid(){
    debug.draw_collision_grid = !debug.draw_collision_grid;
    console.str += text_format("\t>Collision grid is %s\n", debug.draw_collision_grid ? "enabled" : "disabled");
}

void debug_set_default_time_scale(){
    core.time.debug_target_time_scale = 1;    
}

void debug_set_time_scale(const char *text){
    core.time.debug_target_time_scale = to_f32(text);    
}

void debug_set_time_scale(f32 scale){
    core.time.debug_target_time_scale = scale;    
}

void save_replay(const char *replay_name){
    const char *name = text_format("replays/%s.replay", get_substring_before_symbol(replay_name, '.'));
    
    FILE *fptr;
    fptr = fopen(name, "wb");
    
    size_t write_result = fwrite(level_replay.input_record.data, sizeof(Replay_Frame_Data), level_replay.input_record.count, fptr);
    
    if (write_result != -1){
        console.str += text_format("\t>Temp replay named %s is saved\n", name);
    }
}

void save_temp_replay(){
    save_replay(text_format("TEMP_%s", get_substring_before_symbol(current_level_context->level_name, '.')));
}

void play_loaded_replay(){
    session_context.playing_replay = true;
    Entity *replay_player_entity = add_player_entity(&replay_player_data);
    replay_player_entity->flags |= REPLAY_PLAYER;
}

void load_replay(const char *replay_name){
    const char *name = text_format("replays/%s.replay", get_substring_before_symbol(replay_name, '.'));
    FILE *fptr;
    fptr = fopen(name, "rb");

    if (!fptr){
        print_to_console(text_format("No such replay %s", name));
        return;
    }

    size_t read_result = fread(level_replay.input_record.data, sizeof(Replay_Frame_Data), level_replay.input_record.max_count, fptr);
    level_replay.input_record.count = read_result;    
    level_replay.start_frame = session_context.game_frame_count;
    
    if (read_result != -1){
        // session_context.playing_replay = true;
        // enter_editor_state();
        // enter_game_state(current_level_context, true);
        play_loaded_replay();
    
        console.str += text_format("\t>Replay named %s is loaded\n", name);
    }
}

void load_temp_replay(){
    load_replay(text_format("TEMP_%s", get_substring_before_symbol(current_level_context->level_name, '.')));
}


void debug_toggle_play_replay(){
    session_context.playing_replay = !session_context.playing_replay;
        
    if (session_context.playing_replay){
        enter_game_state(editor_level_context, true);
        play_loaded_replay();
    }
    
    console.str += text_format("\t>Replay mode is %s\n", session_context.playing_replay ? "enabled" : "disabled");
}

void restart_game(){
    load_level(first_level_name);
    // enter_editor_state();
    
    // We could already enter game state if we was in it while loading.
    if (game_state != GAME){
        enter_game_state(current_level_context, true);
    }
    
    player_data->ammo_count = 0;
    session_context.speedrun_timer.time = 0;        
}

void begin_level_speedrun(){
    if (!session_context.speedrun_timer.level_timer_active){
        reload_level();
        enter_editor_state();
        enter_game_state(current_level_context, true);
        
        session_context.speedrun_timer.level_timer_active = true;        
        session_context.speedrun_timer.game_timer_active  = false;        
        session_context.speedrun_timer.time = 0;        
    } else{
        disable_speedrun();
    }
}

void disable_speedrun(){
    session_context.speedrun_timer.level_timer_active = false;
    session_context.speedrun_timer.game_timer_active = false;
    session_context.speedrun_timer.time = 0;
}

void begin_game_speedrun(){
    if (!session_context.speedrun_timer.game_timer_active){
        restart_game();
        enter_editor_state();
        enter_game_state(current_level_context, true);
        
        session_context.speedrun_timer.level_timer_active = false;        
        session_context.speedrun_timer.game_timer_active  = true;        
        session_context.speedrun_timer.time = 0;        
    } else{
        disable_speedrun();
    }
}

void debug_add_100_ammo(){
    if (player_entity){
        add_player_ammo(100);
    }    
}

void debug_stop_game(){
    core.time.target_time_scale = 0;
    core.time.time_scale = 0;
    debug.drawing_stopped = true;
}

void debug_toggle_draw_triggers(){
    debug.draw_areas_in_game = !debug.draw_areas_in_game;
}

void save_lightmaps_to_file(){
    for (i32 i = 0; i < lightmaps.max_count; i++){
        Image lightmap_image = LoadImageFromTexture(lightmaps.get(i).global_illumination_rt.texture);
        ExportImage(lightmap_image, text_format("resources/lightmaps/%s_%d_lightmap.png", current_level_context->level_name, i));
        UnloadImage(lightmap_image);
    }
}

void init_console(){
    reload_level_files();    

    console.str = init_string();

    console.commands.add(make_console_command("hotkeys", print_hotkeys_to_console));
    console.commands.add(make_console_command("hotkey",  print_hotkeys_to_console));
    console.commands.add(make_console_command("key",     print_hotkeys_to_console));
    console.commands.add(make_console_command("keys",    print_hotkeys_to_console));
    console.commands.add(make_console_command("help",    print_hotkeys_to_console));
    
    console.commands.add(make_console_command("debug",          print_debug_commands_to_console));
    console.commands.add(make_console_command("player_speed",   debug_toggle_player_speed));
    console.commands.add(make_console_command("entities_count", debug_print_entities_count));
    console.commands.add(make_console_command("infinite_ammo",  debug_infinite_ammo));
    console.commands.add(make_console_command("die",  kill_player));
    console.commands.add(make_console_command("enemy_ai",       debug_enemy_ai));
    console.commands.add(make_console_command("god_mode",       debug_god_mode));
    console.commands.add(make_console_command("add_ammo",       debug_add_100_ammo));
    console.commands.add(make_console_command("unlock_camera",  debug_unlock_camera));
    console.commands.add(make_console_command("full_light",     debug_toggle_full_light));
    console.commands.add(make_console_command("collision_grid", debug_toggle_collision_grid));
    console.commands.add(make_console_command("draw_triggers", debug_toggle_draw_triggers));
    
    console.commands.add(make_console_command("save",     save_current_level, save_level_by_name));
    console.commands.add(make_console_command("load",     NULL, load_level_by_name));
    console.commands.add(make_console_command("level",    print_current_level, load_level_by_name));
    console.commands.add(make_console_command("next",     try_load_next_level, NULL));
    console.commands.add(make_console_command("previous", try_load_previous_level, NULL));
    console.commands.add(make_console_command("reload",   reload_level, NULL));
    
    console.commands.add(make_console_command("save_lightmap", save_lightmaps_to_file, NULL));
    
    console.commands.add(make_console_command("restart_game",      restart_game, NULL));
    console.commands.add(make_console_command("first",             restart_game, NULL));
    console.commands.add(make_console_command("level_speedrun",    begin_level_speedrun, NULL));
    console.commands.add(make_console_command("game_speedrun",     begin_game_speedrun, NULL));
    console.commands.add(make_console_command("speedrun_disable",  disable_speedrun, NULL));
    
    console.commands.add(make_console_command("timescale", debug_set_default_time_scale, debug_set_time_scale));
    
    console.commands.add(make_console_command("create",    print_create_level_hint, create_level));
    console.commands.add(make_console_command("new_level", print_create_level_hint, create_level));
    
    console.commands.add(make_console_command("play_replay", debug_toggle_play_replay, NULL));
    console.commands.add(make_console_command("save_replay", save_temp_replay, save_replay));
    console.commands.add(make_console_command("replay_load", load_temp_replay, load_replay));
}

Music ambient_theme;
Music wind_theme;
Music tires_theme;
Music relas_music;
f32 tires_volume = 0.0f;

void load_sounds(){
    ambient_theme = LoadMusicStream("resources/audio/music/AmbientChurch.wav");
    ambient_theme.looping = true;
    SetMusicVolume(ambient_theme, 0.14f);
    PlayMusicStream(ambient_theme);
    
    wind_theme = LoadMusicStream("resources/audio/music/wind.ogg");
    wind_theme.looping = true;
    SetMusicVolume(wind_theme, 0.0f);
    PlayMusicStream(wind_theme);
    
    tires_theme = LoadMusicStream("resources/audio/music/TiresStopping.wav");
    tires_theme.looping = true;
    SetMusicVolume(tires_theme, tires_volume);
    PlayMusicStream(tires_theme);
    
    relas_music = LoadMusicStream("resources/audio/music/Beethoven - Fur Elise.ogg");
    relas_music.looping = true;
    SetMusicVolume(relas_music, 1);
    PlayMusicStream(relas_music);
    
    FilePathList sounds = LoadDirectoryFiles("resources\\audio");
    for (i32 i = 0; i < sounds.count; i++){
        char *name = sounds.paths[i];
        
        if (!str_end_with(name, ".ogg") && !str_end_with(name, ".wav")){
            continue;
        }
        
        Sound sound = LoadSound(name);
        substring_after_line(name, "resources\\audio\\");
        name = get_substring_before_symbol(name, '.');
        
        Sound_Handler handler = {};
        str_copy(handler.name, name);
        
        for (i32 s = 0; s < handler.buffer.max_count; s++){
            handler.buffer.add(LoadSoundAlias(sound));
        }
        
        // i64 hash = hash_str(name);
        //UnloadSound(sound);
        
        sounds_array.add(handler);
        
        if (str_contains(name, "MissingSound")){
            missing_sound = sounds_array.last_ptr();
        }
    }
    
    UnloadDirectoryFiles(sounds);
}

void play_sound(Sound_Handler *handler){
    assert(handler->buffer.count > 0);
    
    Sound sound = handler->buffer.get(handler->current_index);
    handler->current_index = (handler->current_index + 1) % handler->buffer.max_count;
    
    SetSoundVolume(sound, rnd(handler->base_volume - handler->volume_variation, handler->base_volume + handler->volume_variation));
    SetSoundPitch (sound, rnd(handler->base_pitch - handler->pitch_variation, handler->base_pitch + handler->pitch_variation));
    
    PlaySound(sound);
}

void play_sound(Sound_Handler *handler, Vector2 position, f32 volume_multiplier = 1, f32 base_pitch = 1.0f, f32 pitch_variation = 0.3f){
    assert(handler->buffer.count > 0);
    
    Sound sound = handler->buffer.get(handler->current_index);
    handler->current_index = (handler->current_index + 1) % handler->buffer.max_count;
    
    //check vector to camera for volume and pan
    Vector2 to_position = position - current_level_context->cam.position;
    f32 len = magnitude(to_position);
    f32 max_len = 250;
    
    // Because we could be really not at the center of the screen with locked cam. We'll see how it's gonna be with horizontal rails.
    if (state_context.cam_state.locked || state_context.cam_state.on_rails_vertical){
        max_len = 700;
    }
    
    f32 distance_t = clamp01(len / max_len);
    
    f32 volume = lerp(handler->base_volume, 0.2f, distance_t * distance_t * distance_t);
    f32 pitch  = lerp(base_pitch, 0.6f, distance_t * distance_t * distance_t);
    
    f32 on_right = normalized(to_position.x);
    f32 side_t = clamp01((to_position.x * on_right) / max_len);
    f32 pan_add = lerp(0.0f, 0.4f * on_right * -1, side_t * side_t);
    
    SetSoundVolume(sound, rnd(volume - handler->volume_variation, volume + handler->volume_variation) * volume_multiplier);
    SetSoundPitch (sound, rnd(pitch - pitch_variation, pitch + pitch_variation));
    SetSoundPan   (sound, 0.5f + pan_add);
    //pan    
    
    PlaySound(sound);
}

void play_sound(const char* name, Vector2 position, f32 volume_multiplier = 1, f32 base_pitch = 1.0f, f32 pitch_variation = 0.3f){
    char *trimped_name = get_substring_before_symbol(name, '.');    
    Sound_Handler *found_handler = NULL;
    
    for (i32 i = 0; i < sounds_array.count; i++){
        if (str_equal(sounds_array.get_ptr(i)->name, trimped_name)){
            found_handler = sounds_array.get_ptr(i);
        }
    }
    if (!found_handler){
        printf("NO SOUND found %s\n", name);
        found_handler = missing_sound;
        // return;
    }
    
    play_sound(found_handler, position, volume_multiplier, base_pitch, pitch_variation);
}

inline void play_sound(const char* name, f32 volume_multiplier = 1, f32 base_pitch = 1.0f, f32 pitch_variation = 0.3f){
    play_sound(name, current_level_context->cam.position, volume_multiplier, base_pitch, pitch_variation);
}

// RenderTexture emitters_occluders_rt;
// Shader emitters_occluders_shader;
// RenderTexture voronoi_seed_rt;
// Shader voronoi_seed_shader;
// RenderTexture jump_flood_rt;
// Shader jump_flood_shader;
// RenderTexture distance_field_rt;
// Shader distance_field_shader;
RenderTexture global_illumination_rt;
// Shader global_illumination_shader;

RenderTexture light_geometry_rt;

RenderTexture final_light_rt;
Shader env_light_shader;

Shader gaussian_blur_shader;

global_variable Image white_pixel_image;
global_variable Texture white_pixel_texture;
global_variable Image white_transparent_pixel_image;
global_variable Texture white_transparent_pixel_texture;
global_variable Image black_pixel_image;
global_variable Texture black_pixel_texture;

// #define LIGHT_TEXTURE_SCALING_FACTOR 0.25f
// #define LIGHT_TEXTURE_SIZE_MULTIPLIER 2.0f

void init_level_context(Level_Context *level_context){
    assert(!level_context->inited);

    //init context
    for (i32 i = 0; i < level_context->lights.max_count; i++){
        Light *light = level_context->lights.get_ptr(i);
        *(light) = {};
        
        if (i < session_context.temp_lights_count){
            light->make_shadows             = true;
            light->make_backshadows         = true;
            light->additional_shadows_flags = 0;
            light->grow_time                = 0;
            light->shrink_time              = 0;
            light->birth_time = -12;
            
            i32 size = ULTRA_SMALL_LIGHT;
            if (i < session_context.big_temp_lights_count){
                size = BIG_LIGHT;
            } else if (i < session_context.big_temp_lights_count + session_context.huge_temp_lights_count){
                size = HUGE_LIGHT;
            } else{ // So it's usual temp lights
                light->make_shadows = false;
                light->make_backshadows = false;
            }
            
            light->shadows_size_flags       = size;
            light->backshadows_size_flags   = size;

            init_light(light);
        }
    }
    
    level_context->inited = true;
    
    for (i32 i = 0; i < level_context->particle_emitters.max_count; i++){
        *level_context->particle_emitters.get_ptr(i) = {};
    }
    for (i32 i = 0; i < level_context->line_trails.max_count; i++){
        *level_context->line_trails.get_ptr(i) = {};
    }
}

// global_variable RenderTexture emitters_occluders_rt;
global_variable RenderTexture voronoi_seed_rt;
global_variable RenderTexture jump_flood_rt;
// global_variable RenderTexture distance_field_rt;
// global_variable RenderTexture raytracing_global_illumination_rt;

// #define MAX_LIGHTMAPS 3

global_variable Shader voronoi_seed_shader;
global_variable Shader jump_flood_shader;
global_variable Shader distance_field_shader;
global_variable Shader global_illumination_shader;

i32 light_texture_width  = (i32)(2048 * 1.0f);
i32 light_texture_height = (i32)(2048 * 1.0f);

Shader load_shader(const char *vertex, const char *fragment){
    Shader loaded = LoadShader(vertex, fragment);
    if (!IsShaderReady(loaded)){
        print("WARNIGNG: Shader could not load");
    }
    
    return loaded;
}

void load_render(){
    // emitters_occluders_rt = LoadRenderTexture(light_texture_width, light_texture_height);
    voronoi_seed_rt = LoadRenderTexture(light_texture_width, light_texture_height);
    jump_flood_rt = LoadRenderTexture(light_texture_width, light_texture_height);
    // distance_field_rt = LoadRenderTexture(light_texture_width, light_texture_height);
    // raytracing_global_illumination_rt = LoadRenderTexture(light_texture_width, light_texture_height);
    for (i32 i = 0; i < lightmaps.max_count; i++){
        lightmaps.get_ptr(i)->global_illumination_rt = LoadRenderTexture(light_texture_width, light_texture_height);
        lightmaps.get_ptr(i)->emitters_occluders_rt  = LoadRenderTexture(light_texture_width, light_texture_height);
        lightmaps.get_ptr(i)->distance_field_rt      = LoadRenderTexture(light_texture_width, light_texture_height);
    }
    
    voronoi_seed_shader = LoadShader(0, "./resources/shaders/voronoi_seed.fs");
    jump_flood_shader = LoadShader(0, "./resources/shaders/jump_flood.fs");
    distance_field_shader = LoadShader(0, "./resources/shaders/distance_field.fs");
    global_illumination_shader = load_shader(0, "./resources/shaders/global_illumination1.fs");
}

void init_game(){
    initing_game = true;
    str_copy(loaded_level_context.name, "loaded_level_context");
    // str_copy(editor_level_context.name, "editor_level_context");
    str_copy(game_level_context.name, "game_level_context");
    str_copy(checkpoint_level_context.name, "checkpoint_level_context");
    
    // Now we need to init all level contexts once 
    init_level_context(&loaded_level_context);
    init_level_context(&game_level_context);
    init_level_context(&checkpoint_level_context);
    init_level_context(&undo_level_context);
    init_level_context(&copied_entities_level_context);
    
    for (i32 i = 0; i < MAX_LOADED_LEVELS; i++){
        init_level_context(&loaded_levels_contexts[i]);
        str_copy(loaded_levels_contexts[i].name, text_format("editor_level_context_%d", i));
    }
    editor_level_context = &loaded_levels_contexts[0];
    
    player_data = &real_player_data;
    
    switch_current_level_context(&loaded_level_context);

    game_state = EDITOR;

    session_context.entity_lights_start_index = session_context.temp_lights_count; 
    
    render = {};
    
    i32 cells_columns = (i32)(session_context.collision_grid.size.x / session_context.collision_grid.cell_size.x);
    i32 cells_rows    = (i32)(session_context.collision_grid.size.y / session_context.collision_grid.cell_size.y);
    session_context.collision_grid.cells = (Collision_Grid_Cell*)malloc(cells_columns * cells_rows * sizeof(Collision_Grid_Cell));
    session_context.collision_grid_cells_count = cells_columns * cells_rows;
    
    for (i32 i = 0; i < cells_columns * cells_rows; i++){
        session_context.collision_grid.cells[i] = {};
    }
    
    white_pixel_image = GenImageColor(1, 1, WHITE);
    white_pixel_texture = LoadTextureFromImage(white_pixel_image);
    
    white_transparent_pixel_image = GenImageColor(1, 1, Fade(WHITE, 0));
    white_transparent_pixel_texture = LoadTextureFromImage(white_transparent_pixel_image);
    
    black_pixel_image = GenImageColor(1, 1, BLACK);
    black_pixel_texture = LoadTextureFromImage(black_pixel_image);
    
    render.test_shader = LoadShader(0, "./resources/shaders/test_shader.fs");
    
    // global_illumination_shader = LoadShader(0, "./resources/shaders/global_illumination1.fs");
    
    env_light_shader = LoadShader(0, "./resources/shaders/env_light.fs");
    
    gaussian_blur_shader = LoadShader(0, "./resources/shaders/gaussian_blur.fs");

    input = {};
    init_console();
    // current_level = {};
    load_textures();
    init_spawn_objects();
    
    jump_shooter_bullet_hint_texture = get_texture("JumpShooterHintBullet.png");
    big_sword_killable_texture        = get_texture("BigSwordSticky.png");
    small_sword_killable_texture       = get_texture("SmallSwordSticky.png");
    perlin_texture = get_texture("PerlinNoise1.png");
    
    load_sounds();
    
    load_render();
    
    mouse_entity = Entity(input.mouse_position, {1, 1}, {0.5f, 0.5f}, 0, 0);
    
    char level_name_to_load[256] = "\0";
    if (RELEASE_BUILD){
        str_copy(level_name_to_load, first_level_name);
    } else{
        str_copy(level_name_to_load, "test_level");
    }
    
    load_level(level_name_to_load);
    
    initing_game = false;
} // end init game end

void destroy_player(){
    assert(player_entity);

    player_entity->destroyed = true;
    player_entity->enabled   = false;
    
    assert(current_level_context->entities.has_key(player_data->connected_entities_ids.ground_checker_id));
    get_entity_by_id(player_data->connected_entities_ids.ground_checker_id)->destroyed = true;
    assert(current_level_context->entities.has_key(player_data->connected_entities_ids.sword_entity_id));
    get_entity_by_id(player_data->connected_entities_ids.sword_entity_id)->destroyed = true;
    
    player_entity = NULL;
}

void clean_up_scene(){
    if (current_level_context){
        ForEntities(entity, 0){
            // entity->color = entity->color_changer.start_color;
        }
        
        for (i32 i = 0; i < MAX_BIRD_POSITIONS; i++){
            current_level_context->bird_slots[i].occupied = false;
        }
    }

    state_context = {};
    checkpoint_trigger_id = -1;
    
    session_context.speedrun_timer.paused = false;
    if (!session_context.speedrun_timer.game_timer_active){
        session_context.speedrun_timer.time = 0;        
    }
    
    assign_selected_entity(NULL);
    editor.in_editor_time = 0;
    close_create_box();
}

Entity *add_player_entity(Player *data){
    Entity *new_player_entity = add_entity(current_level_context->player_spawn_point, {1.0f, 2.0f}, {0.5f, 0.5f}, 0, RED, PLAYER | PARTICLE_EMITTER);
    new_player_entity->collision_flags = GROUND | ENEMY;
    new_player_entity->draw_order = 30;
    
    Entity *ground_checker = add_entity(new_player_entity->position - new_player_entity->up * new_player_entity->scale.y * 0.5f, {new_player_entity->scale.x * 0.9f, new_player_entity->scale.y * 1.5f}, {0.5f, 0.5f}, 0, 0); 
    ground_checker->collision_flags = GROUND;
    ground_checker->color = Fade(PURPLE, 0.8f);
    ground_checker->draw_order = 31;
    
    Entity *left_wall_checker = add_entity(new_player_entity->position - new_player_entity->right * new_player_entity->scale.x * 0.5f + Vector2_up * new_player_entity->scale.y * 0.65f, {new_player_entity->scale.x * 0.6f, new_player_entity->scale.y * 0.1f}, {0.5f, 0.5f}, 0, 0); 
    left_wall_checker->collision_flags = GROUND;
    left_wall_checker->color = Fade(PURPLE, 0.8f);
    left_wall_checker->draw_order = 31;
    
    Entity *right_wall_checker = add_entity(new_player_entity->position + new_player_entity->right * new_player_entity->scale.x * 0.5f + Vector2_up * new_player_entity->scale.y * 0.65f, {new_player_entity->scale.x * 0.6f, new_player_entity->scale.y * 0.1f}, {0.5f, 0.5f}, 0, 0); 
    right_wall_checker->collision_flags = GROUND;
    right_wall_checker->color = Fade(PURPLE, 0.8f);
    right_wall_checker->draw_order = 31;
    
    Entity *sword_entity = add_entity(current_level_context->player_spawn_point, data->sword_start_scale, {0.5f, 1.0f}, 0, GRAY + RED * 0.1f, SWORD);
    sword_entity->collision_flags = ENEMY;
    sword_entity->color   = GRAY + RED * 0.1f;
    sword_entity->draw_order = 25;
    str_copy(sword_entity->name, "Player_Sword");
    
    data->connected_entities_ids.ground_checker_id = ground_checker->id;
    data->connected_entities_ids.left_wall_checker_id = left_wall_checker->id;
    data->connected_entities_ids.right_wall_checker_id = right_wall_checker->id;
    data->connected_entities_ids.sword_entity_id = sword_entity->id;
    data->dead_man = false;
    
    data->timers = {};
    
    free_entity_particle_emitters(new_player_entity);
    data->stun_emitter_index        = add_entity_particle_emitter(new_player_entity, &air_dust_emitter);
    data->rifle_trail_emitter_index = add_entity_particle_emitter(new_player_entity, &gunpowder_emitter);
    data->tires_emitter_index       = add_entity_particle_emitter(new_player_entity, &tires_emitter_copy);
    
    Particle_Emitter *tires_emitter = get_particle_emitter(data->tires_emitter_index);
    if (tires_emitter){
        tires_emitter->follow_entity = false;
    }
    
    Particle_Emitter *rifle_trail_emitter = get_particle_emitter(data->rifle_trail_emitter_index);
    if (rifle_trail_emitter){
        rifle_trail_emitter->follow_entity = false;
    }
    
    data->ammo_count = last_player_data.ammo_count;
    
    return new_player_entity;
}

void enter_game_state(Level_Context *level_context, b32 should_init_entities){
    // player_data = {};
    real_player_data = {};
    player_data = &real_player_data;

    clean_up_scene();
    clear_level_context(&game_level_context);
    
    state_context = {};
    session_context.just_entered_game_state = true;
    core.time.game_time = 0;
    core.time.hitstop = 0;
    core.time.previous_dt = 0;
    
    HideCursor();
    DisableCursor();
    
    // clear_level_context(&game_level_context);
    switch_current_level_context(&game_level_context);
    copy_level_context(&game_level_context, level_context, should_init_entities);
    
    Vector2 grid_target_pos = current_level_context->player_spawn_point;
    session_context.collision_grid.origin = {(f32)((i32)grid_target_pos.x - ((i32)grid_target_pos.x % (i32)session_context.collision_grid.cell_size.x)), (f32)((i32)grid_target_pos.y - ((i32)grid_target_pos.y % (i32)session_context.collision_grid.cell_size.y))};

    state_context.timers.last_collision_cells_clear_time = core.time.app_time;
    for (i32 i = 0; i < session_context.collision_grid_cells_count; i++){        
        session_context.collision_grid.cells[i].entities_ids.clear();
    }
    
    game_state = GAME;
    
    current_level_context->cam.cam2D.zoom = 0.35f;
    current_level_context->cam.target_zoom = 0.35f;
    current_level_context->cam.position = current_level_context->player_spawn_point;
    
    session_context.game_frame_count = 0;
    if (!session_context.playing_replay){
        level_replay.input_record.clear();
    }
    
    if (should_init_entities){
        player_entity = add_player_entity(player_data);
        
        ForEntities(entity, 0){
            update_editor_entity(entity);
            init_entity(entity);
            update_entity_collision_cells(entity);
        }
    }
}

void kill_player(){
    if (debug.god_mode && !state_context.we_got_a_winner || player_data->dead_man || debug.dragging_player){ 
        return;
    }
    
    death_player_data = *player_data;

    emit_particles(&big_blood_emitter_copy, player_entity->position, player_entity->up, 1, 1);
    player_data->dead_man = true;
    player_data->timers.died_time = core.time.game_time;
    play_sound("PlayerTakeDamage", player_entity->position);
    
    // @VISUAL: It's better to separate default flash and player death flash.
    state_context.timers.background_flash_time = core.time.app_time;
}

void enter_editor_state(){
    game_state = EDITOR;
    state_context = {};
    
    session_context.playing_replay = false;
    
    player_data->dead_man = false; 
    
    // We want to enable cursor when user hits escape key.
    HideCursor();
    DisableCursor();
    
    clean_up_scene();
    
    switch_current_level_context(editor_level_context);
    core.time.game_time = 0;
    core.time.hitstop = 0;
    core.time.previous_dt = 0;
    
    SetMusicVolume(tires_theme, 0);
    SetMusicVolume(wind_theme, 0);
}

Vector2 screen_to_world(Vector2 pos){
    f32 zoom = current_level_context->cam.cam2D.zoom;

    f32 width = current_level_context->cam.width   ;
    f32 height = current_level_context->cam.height ;

    Vector2 screen_pos = pos;
    Vector2 world_pos = {(screen_pos.x - width * 0.5f) / current_level_context->cam.unit_size, (height * 0.5f - screen_pos.y) / current_level_context->cam.unit_size};
    world_pos /= zoom;
    world_pos = world_pos + current_level_context->cam.position;
    
    return world_pos;
}

inline Vector2 game_mouse_pos(){
    return screen_to_world(input.screen_mouse_position);
}

void fixed_game_update(f32 dt){
    frame_rnd = perlin_noise3(core.time.game_time, core.time.app_time, 5) * 2 - 1.0f;
    frame_on_circle_rnd = get_perlin_in_circle(1.0f);

    if (game_state == GAME && !state_context.in_pause_editor){
        if (!session_context.playing_replay){
            //record input
            if (level_replay.input_record.count >= MAX_INPUT_RECORDS - 1){
                // level_replay.input_record.remove_first_half();
            } else{
                input.rnd_state = rnd_state;
                input.player_position = player_entity->position;
                level_replay.input_record.add({input});
            }
        } else{
            i32 frame = session_context.game_frame_count - level_replay.start_frame;
            if (frame >= level_replay.input_record.count){
                // debug_set_time_scale(0);
                session_context.playing_replay = false;
            } else{
                replay_input = level_replay.input_record.get(frame).frame_input;
                // core.time = level_replay.input_record.get(session_context.game_frame_count).frame_time_data;
                rnd_state = replay_input.rnd_state;
            }
        }
    }

    debug.dragging_player = false;
    if (game_state == GAME && player_entity){
        if (IsKeyDown(KEY_K) && !console.is_open){
            player_entity->position = input.mouse_position;
            debug.dragging_player = true;
            player_data->velocity = Vector2_zero;
        } 
    } 

    update_entities(dt);
    update_particle_emitters(dt);
    // update_particles(dt);
    
    // update camera
    if (game_state == GAME && player_entity && !state_context.free_cam && !state_context.in_pause_editor && (!is_in_death_instinct() || !is_death_instinct_threat_active())){
        f32 time_since_death_instinct_stop = core.time.app_time - state_context.death_instinct.stop_time;
        
        f32 locked_speed_t = clamp01(time_since_death_instinct_stop);
        f32 locked_speed_multiplier = lerp(0.001f, 1.0f, locked_speed_t * locked_speed_t * locked_speed_t);
    
        if (!state_context.cam_state.locked){
            Vector2 player_velocity = player_data->velocity;
            f32 target_speed_multiplier = 1;
        
            f32 time_since_heavy_collision = core.time.game_time - player_data->heavy_collision_time;
            if (magnitude(player_data->velocity) < 80 && core.time.game_time > 5 && time_since_heavy_collision <= 1.0f){
                player_velocity = player_data->heavy_collision_velocity;
                target_speed_multiplier = 0.05f;
            }
            
            f32 player_speed = magnitude(player_velocity);
        
            Vector2 target_position_velocity_addition = player_velocity * 0.25f;
            Vector2 target_position = player_entity->position + Vector2_up * 20 + target_position_velocity_addition;
            
            if (state_context.cam_state.on_rails_horizontal || state_context.cam_state.on_rails_vertical){
                Entity *rails_trigger_entity = get_entity_by_id(state_context.cam_state.rails_trigger_id);
                Dynamic_Array<Vector2> *rails_points = &rails_trigger_entity->trigger.cam_rails_points;
                assert(rails_trigger_entity);
                
                b32 on_rails = rails_points->count >= 2;
                if (on_rails){
                    Vector2 rails_player_position = player_entity->position + target_position_velocity_addition;
                    Vector2 point1 = rails_points->get(0);
                    Vector2 point2 = rails_points->get(1);
                    #define SECTION_POS(point) (state_context.cam_state.on_rails_horizontal ? point.x : point.y)
                    f32 player_section_pos = state_context.cam_state.on_rails_horizontal ? rails_player_position.x : rails_player_position.y;
                    f32 last_section_pos = state_context.cam_state.on_rails_horizontal ? rails_points->last().x : rails_points->last().y;
                    b32 is_going_right_or_up = SECTION_POS(point2) > SECTION_POS(point1);
                    
                    if (0) {
                    } else if (is_going_right_or_up && player_section_pos < SECTION_POS(point1)
                     || !is_going_right_or_up && player_section_pos > SECTION_POS(point1)){
                        target_position = point1;    
                    } else if(is_going_right_or_up && player_section_pos > last_section_pos
                     || !is_going_right_or_up && player_section_pos < last_section_pos){
                        target_position = rails_points->last();
                    } else{
                        for (i32 i = 0; i < rails_points->count - 1; i++){                
                            point1 = rails_points->get(is_going_right_or_up ? i : i+1);
                            point2 = rails_points->get(is_going_right_or_up ? i+1 : i); 
                            if (player_section_pos >= SECTION_POS(point1) && player_section_pos <= SECTION_POS(point2)){
                                break;
                            }
                        }
                        
                        // f32 section_len = magnitude(point2 - point1);
                        f32 section_len = (SECTION_POS(point2) - SECTION_POS(point1));
                        f32 section_t = clamp01((section_len - (SECTION_POS(point2) - player_section_pos)) / section_len);
                    
                        target_position = lerp(point1, point2, section_t);
                    }
                }
            } else{
                // Camera is unlocked completely
                // This section tries to keep enemies in sight. 
                // Currently we count enemies in 4 diagonal directions and each enemy in direction reduces amount of displacement.
                Vector2 additional_position = Vector2_zero;
                
                local_persist i32 direction_enemies_count[4];
                memset(direction_enemies_count, 0, sizeof(direction_enemies_count));
                // @OPTIMIZATION: Of course here we want just to through enemies, not through all entities.
                ForEntities(entity, ENEMY){
                    if (entity->flags == ENEMY || entity->flags & AMMO_PACK || entity->flags & HIT_BOOSTER || entity->flags & PROJECTILE || !entity->enemy.in_agro || entity->enemy.dead_man || entity->flags & PROJECTILE){
                        continue;
                    }
                    
                    Vector2 vec_to_enemy = entity->position - player_entity->position;
                    Vector2 dir_to_enemy = normalized(vec_to_enemy);
                    f32 distance_to_enemy = magnitude(vec_to_enemy);
                    
                    if (distance_to_enemy > 1000){
                        continue;
                    }
                    
                    // {-1, -1}
                    i32 direction_index = 0;
                    // {-1, 1}
                    if (dir_to_enemy.x < 0 && dir_to_enemy.y > 0) direction_index = 1;
                    // {1, 1}
                    if (dir_to_enemy.x > 0 && dir_to_enemy.y > 0) direction_index = 2;
                    // {1, -1}
                    if (dir_to_enemy.x > 0 && dir_to_enemy.y < 0) direction_index = 3;
                    
                    direction_enemies_count[direction_index] += 1;
                    i32 enemies_count = direction_enemies_count[direction_index];
                    
                    if (enemies_count > 20){
                        continue;
                    }
                    
                    Vector2 addition = dir_to_enemy * (((60.0f * 0.35f) / current_level_context->cam.cam2D.zoom) / enemies_count);
                    // If enemy really close or to far we want to reduce amount of displacement
                    f32 min_threshold = (SCREEN_WORLD_SIZE / current_level_context->cam.cam2D.zoom);
                    f32 max_threshold = (min_threshold * 2);
                    if (distance_to_enemy <= min_threshold){ 
                        addition *= (distance_to_enemy / min_threshold);
                    } else if (distance_to_enemy >= max_threshold){
                        f32 t = (max_threshold / distance_to_enemy);
                        addition *= t * t;
                    }
                    
                    if (entity->flags & CENTIPEDE_SEGMENT){
                        addition *= 0.1f;
                    }
                    
                    additional_position += addition;
                    
                    f32 max_x = (SCREEN_WORLD_SIZE / current_level_context->cam.cam2D.zoom) * 0.25f;
                    f32 max_y = (SCREEN_WORLD_SIZE / aspect_ratio / current_level_context->cam.cam2D.zoom) * 0.2f;
                    clamp(&additional_position.x, -max_x, max_x);
                    clamp(&additional_position.y, -max_y, max_y);
                }
                
                target_position += additional_position;
                
                if (additional_position != Vector2_zero){
                    if (dot(player_data->velocity, additional_position) < 0){
                        target_position += target_position_velocity_addition * 0.3f;
                    } else{
                        target_position -= target_position_velocity_addition * 0.5f;
                    }
                }
            } 
            
            Vector2 vec_to_target = target_position - current_level_context->cam.target;
            Vector2 vec_to_player = player_entity->position - current_level_context->cam.target;
            
            f32 target_dot = dot(vec_to_target, vec_to_player);
            
            f32 speed_t = clamp01(player_speed / 200.0f);
            
            f32 target_speed = lerp(3, 10, speed_t * speed_t);
            target_speed *= target_speed_multiplier;
            
            current_level_context->cam.target = lerp(current_level_context->cam.target, target_position, clamp01(dt * target_speed));
            
            f32 cam_speed = lerp(10.0f, 100.0f, speed_t * speed_t);
            
            current_level_context->cam.position = lerp(current_level_context->cam.position, current_level_context->cam.target, clamp01(dt * cam_speed * locked_speed_multiplier));
            
        // Locked camera
        } else if ((!is_in_death_instinct() || !is_death_instinct_threat_active()) || state_context.free_cam){
            current_level_context->cam.position = lerp(current_level_context->cam.position, current_level_context->cam.target, clamp01(dt * 4 * locked_speed_multiplier));
            if (magnitude(current_level_context->cam.target - current_level_context->cam.position) <= EPSILON){
                current_level_context->cam.position = current_level_context->cam.target;
            }
        }
    }
    
    state_context.cam_state.trauma -= dt * state_context.cam_state.trauma_decrease_rate;
    state_context.cam_state.trauma = clamp01(state_context.cam_state.trauma);
    
    state_context.explosion_trauma = clamp01(state_context.explosion_trauma - dt * 20);

    if (!is_in_death_instinct() || !is_death_instinct_threat_active()){
        f32 zoom_speed = game_state == GAME ? 3 : 10;
        Cam *cam = &current_level_context->cam;
        
        f32 target_zoom = cam->target_zoom;
        if (game_state == GAME && player_data->in_slowmo){
            target_zoom *= 1.2f;
        }
        cam->cam2D.zoom = lerp(cam->cam2D.zoom, target_zoom, dt * zoom_speed);
        
        if (core.time.real_dt >= 0.1){
            cam->cam2D.zoom = cam->target_zoom;
        }
    }
    
    if (abs(current_level_context->cam.cam2D.zoom - current_level_context->cam.target_zoom) <= EPSILON){
        current_level_context->cam.cam2D.zoom = current_level_context->cam.target_zoom;
    }

    input.press_flags = 0;
    input.sum_mouse_delta = Vector2_zero;
    input.sum_mouse_wheel = 0;
    
    session_context.game_frame_count += 1;
} // end fixed game update

void update_console(){
    b32 can_control_console = !editor.create_box_active;
    if (can_control_console && (IsKeyPressed(KEY_SLASH) || (console.is_open && IsKeyPressed(KEY_ESCAPE)))){
        if (console.is_open){
            console.is_open = false;
            console.closed_time = core.time.app_time;
            
            if (str_equal(focus_input_field.tag, "console_input_field")){
                focus_input_field.in_focus = false;
            }
        } else{
            console.is_open = true;
            console.opened_time = core.time.app_time;
            make_next_input_field_in_focus("console");
        }
    }
            
    if (console.is_open && can_control_console){
        f32 time_since_open = core.time.app_time - console.opened_time;
        console.open_progress = clamp01(time_since_open / 0.3f);
        
        Color color = lerp(WHITE * 0, GRAY, console.open_progress * console.open_progress);
        
        console.args.clear();
        split_str(focus_input_field.content, " ", &console.args);
        
        b32 content_changed = false;
        for (i32 i = 0; i < console.commands.count && console.args.count == 1; i++){
            if (str_start_with(console.commands.get(i).name, console.args.get(0).data)){
                make_ui_text(console.commands.get(i).name, {3.0f, (f32)screen_height * 0.5f + focus_input_field.font_size}, focus_input_field.font_size, color * 0.7f, "console_hint_text");
                
                if (IsKeyPressed(KEY_TAB) && console.args.count == 1){
                    set_focus_input_field(console.commands.get(i).name);
                    content_changed = true;
                }
                break;
            }
        }
        
        if (console.args.count == 2 && (str_equal(console.args.get(0).data, "level") || str_equal(console.args.get(0).data, "load"))){
            for (i32 i = 0; i < console.level_files.count; i++){
                if (str_contains(console.level_files.get(i).data, console.args.get(1).data)){
                    const char *new_console_content = text_format("%s %s", console.args.get(0).data, console.level_files.get(i).data);
                    
                    make_ui_text(new_console_content, {3.0f, (f32)screen_height * 0.5f + focus_input_field.font_size}, focus_input_field.font_size, color * 0.7f, "console_hint_text");
                    
                    if (IsKeyPressed(KEY_TAB)){
                        set_focus_input_field(new_console_content);
                        content_changed = true;
                    }
                    break;
                }
            }
        }
        
        if (!content_changed){
            if (IsKeyPressed(KEY_UP) && console.history_max > 0 && console.history.count > 0){
                set_focus_input_field(console.history.get(console.history.count - 1).data);
                console.history.count--;
                content_changed = true;
            }
            if (IsKeyPressed(KEY_DOWN) && console.history_max >= console.history.count){
                if (console.history_max == console.history.count){
                    clear_focus_input_field();
                } else{
                    set_focus_input_field(console.history.get(console.history.count).data);
                    console.history.count++;
                }
                content_changed = true;
            }
        }
        
        if (make_input_field("", {0.0f, (f32)screen_height * 0.5f}, {(f32)screen_width, focus_input_field.font_size}, "console_input_field", color, false)){
            console.str += focus_input_field.content;
            console.str += "\n";
            
            //if we used hint while input, for example
            if (content_changed){
                console.args.clear();
                split_str(focus_input_field.content, " ", &console.args);
            }
            
            b32 found_command = false;
            
            for (i32 i = 0; i < console.commands.count && console.args.count > 0; i++){
                Console_Command command = console.commands.get(i);
                if (str_equal(command.name, console.args.get(0).data)){
                    if (command.func_arg && console.args.count > 1){
                        command.func_arg(console.args.get(1).data);
                    } else if (command.func){
                        command.func();   
                    }
                    found_command = true;
                    break;
                }
            }
            
            if (!found_command && console.args.count > 0){
                console.str += "\t>Command was not found :D\n";
            }
            
            Medium_Str history_str;            
            str_copy(history_str.data, focus_input_field.content);
            console.history.add(history_str);
            console.history_max = console.history.count;
            
            clear_focus_input_field();
        }
    }
}

Cam get_cam_for_resolution(i32 width, i32 height){
    Cam cam = current_level_context->cam;
    cam.unit_size = width / SCREEN_WORLD_SIZE; 
    cam.cam2D.target = cast(Vector2){ width/2.0f, height/2.0f };
    cam.cam2D.offset = cast(Vector2){ width/2.0f, height/2.0f };
    cam.width = width;
    cam.height = height;
    
    return cam;
}

void update_game(){
    frame_rnd = rnd01();
    frame_on_circle_rnd = rnd_on_circle();
    
    //update input
    // if (!session_context.playing_replay){    
        input.rnd_state = rnd_state;
        input.mouse_delta = GetMouseDelta();
        input.screen_mouse_position += input.mouse_delta;
        input.sum_mouse_delta += input.mouse_delta;
        clamp(&input.screen_mouse_position.x, 0, screen_width);
        clamp(&input.screen_mouse_position.y, 0, screen_height);
        
        input.mouse_position = game_mouse_pos();
        input.mouse_wheel = GetMouseWheelMove();
        input.sum_mouse_wheel += input.mouse_wheel;
        
        input.direction.x = 0;
        input.direction.y = 0;
        
        b32 can_player_input = !console.is_open;
        
        if (can_player_input){
            if (IsKeyDown(KEY_RIGHT)){
                input.direction.x = 1;
                input.hold_flags |= RIGHT;
            } else if (IsKeyDown(KEY_LEFT)){
                input.direction.x = -1;
                input.hold_flags |= LEFT;
            }
            if (IsKeyDown(KEY_UP)){
                input.direction.y = 1;
                input.hold_flags |= UP;
            } else if (IsKeyDown(KEY_DOWN)){
                input.direction.y = -1;
                input.hold_flags |= DOWN;
            }
            if (IsKeyDown(KEY_D)){
                input.direction.x = 1;
                input.hold_flags |= RIGHT;
            } else if (IsKeyDown(KEY_A)){
                input.direction.x = -1;
                input.hold_flags |= LEFT;
            }
            if (IsKeyDown(KEY_W)){
                input.direction.y = 1;
                input.hold_flags |= UP;
            } else if (IsKeyDown(KEY_S)){
                input.direction.y = -1;
                input.hold_flags |= DOWN;
            }
            
            if (IsKeyDown(KEY_F)){
                input.hold_flags |= SWORD_BIG_DOWN;
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
                input.hold_flags |= SHOOT_DOWN;
            }
            
            if (input.direction.x != 0 || input.direction.y != 0){
                normalize(&input.direction);
            }
            
            if (input.tap_direction.x == 0 && IsKeyPressed(KEY_RIGHT)){
                input.tap_direction.x = 1;
            } else if (input.tap_direction.x == 0 && IsKeyPressed(KEY_LEFT)){
                input.tap_direction.x = -1;
            } else{
                input.tap_direction.x = 0;
            }
            if (input.tap_direction.y == 0 && IsKeyPressed(KEY_UP)){
                input.tap_direction.y = 1;
            } else if (input.tap_direction.y == 0 && IsKeyPressed(KEY_DOWN)){
                input.tap_direction.y = -1;
            } else{
                input.tap_direction.y = 0;
            }
        
            if (input.hold_flags & RIGHT){
                input.sum_direction.x = 1;
            } else if (input.hold_flags & LEFT){
                input.sum_direction.x = -1;
            }
            
            if (input.hold_flags & UP){
                input.sum_direction.y = 1;
            } else if (input.hold_flags & DOWN){
                input.sum_direction.y = -1;
            }
            
            if (IsKeyPressed(KEY_SPACE)){
                input.press_flags |= JUMP;
            }
            if (IsKeyPressed(KEY_F)){
                input.press_flags |= SWORD_BIG;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                input.press_flags |= SHOOT;
            }
            
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
                input.hold_flags |= SPIN_DOWN;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
                input.press_flags |= SPIN;
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
                input.press_flags |= SPIN_RELEASED;
            }
            
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
                input.press_flags |= SHOOT_RELEASED;
            }
        }
    // }
    //end update input
    
    if (screen_size_changed){
        global_cam_data.width = screen_width;
        global_cam_data.height = screen_height;
        global_cam_data.unit_size = screen_width / SCREEN_WORLD_SIZE; 
        global_cam_data.cam2D.target = cast(Vector2){ screen_width/2.0f, screen_height/2.0f };
        global_cam_data.cam2D.offset = cast(Vector2){ screen_width/2.0f, screen_height/2.0f };
        
        if (global_cam_data.width != 0 && global_cam_data.height != 0 && !window_minimized){
            aspect_ratio = (f32)global_cam_data.width / (f32)global_cam_data.height;
            UnloadRenderTexture(render.main_render_texture);
            UnloadRenderTexture(global_illumination_rt);
            UnloadRenderTexture(light_geometry_rt);
            
            render.main_render_texture = LoadRenderTexture(screen_width, screen_height);
            global_illumination_rt = LoadRenderTexture(screen_width, screen_height);
            light_geometry_rt = LoadRenderTexture(screen_width, screen_height);
        }
        
        setup_context_cam(current_level_context);
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_SPACE)){
        if (game_state == EDITOR){
            enter_game_state(editor_level_context, true);
        } else if (game_state == GAME){
            enter_editor_state();
        }
    } 
    
    // editor game pause
    if (IsKeyPressed(KEY_TAB) && !console.is_open && game_state == GAME){
        assign_selected_entity(NULL);
        state_context.in_pause_editor = !state_context.in_pause_editor;
        if (state_context.in_pause_editor){
            editor.in_editor_time = 0;
        }
    }
    
    if (game_state == GAME && !console.is_open){
        if (IsKeyPressed(KEY_T)){
            if (session_context.speedrun_timer.game_timer_active && player_data->dead_man){
                restart_game();
                session_context.speedrun_timer.time = 0;
            } else if (session_context.speedrun_timer.level_timer_active){
                // enter_editor_state();
                enter_game_state(editor_level_context, true);
                session_context.speedrun_timer.time = 0;
            } else{
                b32 is_have_checkpoint = checkpoint_trigger_id != -1;
                session_context.playing_replay = false;            
                // enter_editor_state();
                if (is_have_checkpoint){
                    enter_game_state(&checkpoint_level_context, false);
                    player_entity = checkpoint_player_entity;
                    real_player_data = checkpoint_player_data;
                    core.time = checkpoint_time;
                    state_context = checkpoint_state_context;
                    
                    player_data->velocity = Vector2_zero;
                    session_context.speedrun_timer.time = 0;
                } else{
                    enter_game_state(editor_level_context, true);
                }
            }
        }
    }
    
    core.time.app_time += GetFrameTime();
    core.time.real_dt = GetFrameTime();
    
    // update death instinct
    if (is_in_death_instinct() && is_death_instinct_threat_active() && game_state == GAME){
        f32 time_since_death_instinct = core.time.app_time - state_context.death_instinct.start_time;
        
        Entity *threat_entity = get_entity_by_id(state_context.death_instinct.threat_entity_id);
        Vector2 cam_position = player_entity->position + (threat_entity->position - player_entity->position) * 0.5f;
        
        if (state_context.death_instinct.last_reason == ENEMY_ATTACKING){
            f32 distance_t = (1.0f - clamp01(magnitude(threat_entity->position - player_entity->position) / get_death_instinct_radius(threat_entity->scale)));
            
            f32 t = EaseOutQuint(distance_t);
            core.time.target_time_scale = lerp(0.6f, 0.02f, t * t);
            current_level_context->cam.position        = lerp(current_level_context->cam.position, lerp(current_level_context->cam.target, cam_position, t * t), core.time.real_dt * t * 5);
            current_level_context->cam.cam2D.zoom      = lerp(current_level_context->cam.cam2D.zoom, lerp(current_level_context->cam.target_zoom, 0.55f, t * t), core.time.real_dt * t * 5);
        } else if (state_context.death_instinct.last_reason == SWORD_WILL_EXPLODE){
            f32 distance_t = (1.0f - clamp01(state_context.death_instinct.angle_till_explode / 150.0f));
            f32 t = EaseOutQuint(distance_t);
            core.time.target_time_scale = lerp(0.6f, 0.015f, t);
            current_level_context->cam.position        = lerp(current_level_context->cam.position, lerp(current_level_context->cam.target, cam_position, t), core.time.real_dt * t * 5);
            current_level_context->cam.cam2D.zoom      = lerp(current_level_context->cam.cam2D.zoom, lerp(current_level_context->cam.target_zoom, 0.55f, t), core.time.real_dt * t * 5);
        } else{
            current_level_context->cam.position        = lerp(current_level_context->cam.position, cam_position, clamp01(core.time.real_dt * 5));
            current_level_context->cam.cam2D.zoom      = lerp(current_level_context->cam.cam2D.zoom, 0.55f, clamp01(core.time.real_dt * 5));
            core.time.target_time_scale = lerp(core.time.target_time_scale, 0.03f, clamp01(core.time.real_dt * 10));
        }
        
        f32 instinct_t = time_since_death_instinct / state_context.death_instinct.duration;
        make_line(player_entity->position, threat_entity->position, Fade(RED, instinct_t * instinct_t));
        f32 radius_multiplier = lerp(80.0f, 10.0f, sqrtf(instinct_t));
        Color ring_color = Fade(ColorBrightness(RED, abs(sinf(core.time.app_time * lerp(1.0f, 10.0f, instinct_t)) * 0.8f - 0.5f)), instinct_t * 0.4f);
        make_ring_lines(threat_entity->position, 1.0f * radius_multiplier, 2.0f * radius_multiplier, 14, ring_color);
        
        if (time_since_death_instinct >= 0.2f && !state_context.death_instinct.played_effects){
            play_sound("DeathInstinct", 2);
            state_context.death_instinct.played_effects = true;
        }
        
        state_context.death_instinct.was_in_death_instinct = true;
    } else if (state_context.death_instinct.was_in_death_instinct){
        stop_death_instinct();
        // core.time.target_time_scale = 1;
        state_context.death_instinct.was_in_death_instinct = false;
    } else if (game_state == GAME){
        core.time.target_time_scale = lerp(core.time.target_time_scale, 1.0f, clamp01(core.time.real_dt * 2));        
        if (1.0f - core.time.target_time_scale <= 0.01f){
            core.time.target_time_scale = 1;
        }
    }
    
    if (game_state == GAME && !state_context.in_pause_editor){
        core.time.unscaled_dt = GetFrameTime();
        if (core.time.hitstop > 0){
            core.time.time_scale = fminf(core.time.time_scale, 0.1f);
            core.time.hitstop -= core.time.real_dt;
        } 
        
        if (core.time.hitstop <= 0){
            if (IsKeyDown(KEY_LEFT_SHIFT) && core.time.target_time_scale > 0.4f){
                core.time.time_scale = 0.25f;
                player_data->timers.slowmo_timer = fminf(player_data->timers.slowmo_timer + core.time.real_dt, 10.0f);
                player_data->in_slowmo = true;
            } else{
                core.time.time_scale = core.time.target_time_scale;
                player_data->timers.slowmo_timer = fmaxf(player_data->timers.slowmo_timer - core.time.real_dt, 0);
                player_data->in_slowmo = false;
            }
        }
        
        if (core.time.debug_target_time_scale != 1 && (core.time.debug_target_time_scale < core.time.target_time_scale || core.time.hitstop <= 0)){
           core.time.time_scale = core.time.debug_target_time_scale; 
        }
        
        core.time.dt = GetFrameTime() * core.time.time_scale;
    } else if (game_state == EDITOR || state_context.in_pause_editor){
        core.time.unscaled_dt = 0;
        core.time.dt          = 0;
    }

    
    if (game_state == EDITOR || state_context.in_pause_editor){
        update_editor_ui();
        update_editor();
    }
    
    update_console();
    
    if (game_state == GAME && !state_context.in_pause_editor){
        f32 full_delta = core.time.dt + core.time.previous_dt;
        core.time.previous_dt = 0;
        
        full_delta = Clamp(full_delta, 0, 0.5f * core.time.time_scale);
        session_context.updated_today = false;
        while (full_delta >= TARGET_FRAME_TIME){
            core.time.fixed_dt = TARGET_FRAME_TIME;
            core.time.game_time += core.time.fixed_dt;
            fixed_game_update(core.time.fixed_dt);
            session_context.updated_today = true;
            full_delta -= TARGET_FRAME_TIME;
        }
        
        if (session_context.updated_today){
            input.hold_flags = 0;
            core.time.not_updated_accumulated_dt = 0;
            input.sum_direction = Vector2_zero;
        } else{ 
            core.time.not_updated_accumulated_dt = full_delta;
        }
        
        core.time.previous_dt = full_delta;
    } else{
        fixed_game_update(core.time.real_dt);
    }
    
    // update speedrun timer
    if (game_state == GAME && (session_context.speedrun_timer.level_timer_active || session_context.speedrun_timer.game_timer_active)){
        Color color = WHITE;
        if (state_context.we_got_a_winner){
            session_context.speedrun_timer.paused = true;
            color = GREEN;
        } else if (player_data->dead_man){
            color = RED;
            session_context.speedrun_timer.paused = true;
        }
        
        if (!session_context.speedrun_timer.paused){
            session_context.speedrun_timer.time += core.time.dt;
        }
        
        const char *title_and_time = text_format("%s\n%.4f", session_context.speedrun_timer.level_timer_active ? current_level_context->level_name : "Game speedrun", session_context.speedrun_timer.time);
        make_ui_text(title_and_time, {screen_width * 0.46f, 5}, "speedrun_timer", color, 22);
    }
    
    // update_entities();
    // if (!session_context.updated_today && game_state == GAME){
    //     update_particle_emitters(core.time.dt);
    // }
    // // update_particles();
    
    if (IsKeyPressed(KEY_PAGE_UP)){
        state_context.free_cam = !state_context.free_cam;
        if (!state_context.free_cam){
            current_level_context->cam.target_zoom = debug.last_zoom;
        } else{
            debug.last_zoom = current_level_context->cam.target_zoom;
        }
    }
    
    if (IsKeyPressed(KEY_L) && IsKeyDown(KEY_RIGHT_ALT)){
        debug_unlock_camera();
    }
    
    if (game_state == GAME && player_entity && !state_context.free_cam && !state_context.in_pause_editor){
    } else{
        f32 zoom = current_level_context->cam.target_zoom;

        // update editor camera
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            current_level_context->cam.position += (cast(Vector2){-input.mouse_delta.x / zoom, input.mouse_delta.y / zoom}) / (current_level_context->cam.unit_size);
        }
        if (input.mouse_wheel != 0 && !console.is_open && !editor.create_box_active){
            f32 max_zoom = 5;
            f32 min_zoom = 0.03f;
            if (input.mouse_wheel > 0 || input.mouse_wheel < 0){
                current_level_context->cam.target_zoom += input.mouse_wheel * 0.05f;
                clamp(&current_level_context->cam.target_zoom, min_zoom, max_zoom);
            }
        }
    }
    
    if (editor.update_cam_view_position){
        current_level_context->cam.view_position = current_level_context->cam.position;
    }
    
    draw_game();
    
    #if RELEASE_BUILD
    UpdateMusicStream(ambient_theme);
    #endif
    UpdateMusicStream(wind_theme);
    UpdateMusicStream(tires_theme);
    
    if (state_context.playing_relax || state_context.we_got_a_winner){
        UpdateMusicStream(relas_music);
    }
    
    session_context.just_entered_game_state = false;
    
    
    // We do this so lights don't bake all at one frame 
    session_context.baked_shadows_this_frame = false;
    
    session_context.app_frame_count += 1;
} // update game end

void update_color_changer(Entity *entity, f32 dt){
    Color_Changer *changer = &entity->color_changer;
    
    if (changer->changing || changer->frame_changing){
        f32 t = abs(sinf(core.time.app_time * changer->change_time));
        entity->color = lerp(changer->start_color, changer->target_color, t);
    } else if (entity->flags & EXPLOSIVE){
        Color target_color = ColorBrightness(changer->start_color, 2);
        f32 t = abs(sinf(core.time.game_time * changer->change_time));
        entity->color = lerp(changer->start_color, target_color, t);
        
        if (entity->light_index > -1){
            current_level_context->lights.get_ptr(entity->light_index)->color = lerp(Fade(target_color, 1), Fade(ColorBrightness(ORANGE, 0.3f), 0.9f), t);
            current_level_context->lights.get_ptr(entity->light_index)->radius = get_explosion_radius(entity) * 2 * lerp(0.9f, 1.3f, t);
        }
    } else if (changer->interpolating) {
        entity->color = lerp(changer->start_color, changer->target_color, changer->progress);
    } else{
        if (game_state == EDITOR || state_context.in_pause_editor){
            entity->color = changer->start_color;
        }
    }
    
    if (entity->flags & BLOCKER){
        entity->color = ColorTint(entity->color, RAYWHITE);
    }
    
    changer->frame_changing = false;
}

b32 check_col_point_rec(Vector2 point, Entity *e){
    Vector2 l_u = get_left_up_no_rot(e);
    Vector2 r_d = get_right_down_no_rot(e);

    return ((point.x >= l_u.x) && (point.x <= r_d.x) && (point.y >= r_d.y) && (point.y <= l_u.y));
}

inline b32 check_col_circles(Circle a, Circle b){
    f32 distance = sqr_magnitude(a.position - b.position);
    
    return distance < a.radius * a.radius + b.radius * b.radius;
}

inline Vector2 get_rotated_vector_90(Vector2 v, f32 clockwise){
    return {-v.y * clockwise, v.x * clockwise};
}

inline void fill_arr_with_normals(Array<Vector2, MAX_VERTICES> *normals, Array<Vector2, MAX_VERTICES> vertices){
    //@INCOMPLETE now only for rects and triangles, need to find proper algorithm for calculating edge normals from vertices because 
    //we add vertices in triangle shape
    // Update 03.03.2025: Graham scan algorithm should do the job if we will really need it.
    
    if (vertices.count == 4){
        //up
        Vector2 edge1 = vertices.get(0) - vertices.get(1);
        normals->add(normalized(get_rotated_vector_90(edge1, 1)));
        //left
        Vector2 edge2 = vertices.get(1) - vertices.get(3);
        normals->add(normalized(get_rotated_vector_90(edge2, 1)));
        //bottom
        Vector2 edge3 = vertices.get(3) - vertices.get(2);
        normals->add(normalized(get_rotated_vector_90(edge3, 1)));
        //right
        Vector2 edge4 = vertices.get(2) - vertices.get(0);
        normals->add(normalized(get_rotated_vector_90(edge4, 1)));
    } else if (vertices.count == 3){
        Vector2 edge1 = vertices.get(0) - vertices.get(1);
        normals->add(normalized(get_rotated_vector_90(edge1, 1)));
        
        Vector2 edge2 = vertices.get(1) - vertices.get(2);
        normals->add(normalized(get_rotated_vector_90(edge2, 1)));
        
        Vector2 edge3 = vertices.get(2) - vertices.get(0);
        normals->add(normalized(get_rotated_vector_90(edge3, 1)));
    } else{
        assert(false);
    }
}

inline b32 check_rectangles_collision(Vector2 pos1, Vector2 scale1, Vector2 pos2, Vector2 scale2){
    b32 solution = pos1.x + scale1.x * 0.5f > pos2.x - scale2.x * 0.5f &&
                   pos1.x - scale1.x * 0.5f < pos2.x + scale2.x * 0.5f &&
                   pos1.y + scale1.y * 0.5f > pos2.y - scale2.y * 0.5f &&
                   pos1.y - scale1.y * 0.5f < pos2.y + scale2.y * 0.5f;
                   
    return solution;
}

inline b32 check_bounds_collision(Vector2 pos1, Vector2 pos2, Bounds bounds1, Bounds bounds2, Vector2 pivot1 = {0.5f, 0.5f}, Vector2 pivot2 = {0.5f, 0.5f}){
    Vector2 pivot_add1 = multiply(pivot1, bounds1.size);
    Vector2 position1 = pos1 + bounds1.offset;
    //firstly for left up
    Vector2 with_pivot_pos1 = {position1.x - pivot_add1.x, position1.y + pivot_add1.y};
    Vector2 final1 = {with_pivot_pos1.x + bounds1.size.x * 0.5f, with_pivot_pos1.y - bounds1.size.y * 0.5f};
    
    Vector2 pivot_add2 = multiply(pivot2, bounds2.size);
    Vector2 position2 = pos2 + bounds2.offset;
    //firstly for left up
    Vector2 with_pivot_pos2 = {position2.x - pivot_add2.x, position2.y + pivot_add2.y};
    Vector2 final2 = {with_pivot_pos2.x + bounds2.size.x * 0.5f, with_pivot_pos2.y - bounds2.size.y * 0.5f};
    
    
    return check_rectangles_collision(final1, bounds1.size, final2, bounds2.size);
}

inline b32 check_bounds_collision(Entity *entity1, Entity *entity2){
    return check_bounds_collision(entity1->position, entity2->position, entity1->bounds, entity2->bounds, entity1->pivot, entity2->pivot);
}

inline b32 check_bounds_collision(Vector2 position1, Bounds bounds1, Entity *entity2){
    Vector2 pivot_add2 = multiply(entity2->pivot, entity2->bounds.size);
    Vector2 position2 = entity2->position + entity2->bounds.offset;
    //firstly for left up
    Vector2 with_pivot_pos2 = {position2.x - pivot_add2.x, position2.y + pivot_add2.y};
    Vector2 final2 = {with_pivot_pos2.x + entity2->bounds.size.x * 0.5f, with_pivot_pos2.y - entity2->bounds.size.y * 0.5f};
    
    return check_rectangles_collision(position1 + bounds1.offset, bounds1.size, final2, entity2->bounds.size);
}

Collision check_collision(Vector2 position1, Vector2 position2, Array<Vector2, MAX_VERTICES> vertices1, Array<Vector2, MAX_VERTICES> vertices2, Vector2 pivot1 = {0.5f, 0.5f}, Vector2 pivot2 = {0.5f, 0.5f}){
    Collision result = {};
    
    Bounds bounds1 = get_bounds(vertices1, pivot1);
    Bounds bounds2 = get_bounds(vertices2, pivot2);
    if (!check_bounds_collision(position1, position2, bounds1, bounds2, pivot1, pivot2)){
        return result;
    }

    global_normals.count = 0;
    fill_arr_with_normals(&global_normals, vertices1);
    fill_arr_with_normals(&global_normals, vertices2);
    
    f32 overlap = INFINITY;
    Vector2 min_overlap_axis = Vector2_zero;
    
    Vector2 min_overlap_projection = {};

    for (i32 i = 0; i < global_normals.count; i++){
        Vector2 projections[2];
        //x - min, y - max
        projections[0].x =  INFINITY;
        projections[1].x =  INFINITY;
        projections[0].y = -INFINITY;
        projections[1].y = -INFINITY;
        
        Vector2 axis = global_normals.get(i);

        for (i32 shape = 0; shape < 2; shape++){
            Array<Vector2, MAX_VERTICES> vertices;
            Vector2 position;
            if (shape == 0) {
                vertices = vertices1;
                position = position1;
            } else{
                vertices = vertices2;
                position = position2;
            }
            
            for (i32 j = 0; j < vertices.count; j++){            
                f32 p = dot(global(position, vertices.get(j)), axis);
                
                f32 min = fmin(projections[shape].x, p);
                f32 max = fmax(projections[shape].y, p);
                
                projections[shape].x = min;
                projections[shape].y = max;
            }
        }
        
        f32 new_overlap = fmin(fmin(projections[0].y, projections[1].y) - fmax(projections[0].x, projections[1].x), overlap);
        if (new_overlap != overlap){
            overlap = new_overlap;
            min_overlap_axis = axis;
            min_overlap_projection.x = projections[0].x;
            min_overlap_projection.y = projections[0].y;
        }
        
        if (!(projections[1].y >= projections[0].x && projections[0].y >= projections[1].x)){
            return result;
        }
    }
    
    Vector2 vec_to_first = position1 - position2;
    
    result.collided = true;
    result.overlap = overlap;
    result.dir_to_first = normalized(vec_to_first);
    result.normal = dot(result.dir_to_first, min_overlap_axis) > 0 ? min_overlap_axis : min_overlap_axis * -1.0f;
    result.point = position1 - result.normal * ((min_overlap_projection.y - min_overlap_projection.x) * 0.5f);
    
    return result;
}

Collision check_entities_collision(Entity *entity1, Entity *entity2){
    Collision result = check_collision(entity1->position, entity2->position, entity1->vertices, entity2->vertices, entity1->pivot, entity2->pivot);
    result.other_entity = entity2;
    
    return result;
}

void resolve_collision(Entity *entity, Collision col){
    if (col.collided){
        entity->position += col.normal * col.overlap;
    }
}

Collision_Grid_Cell *get_collision_cell_from_position(Vector2 position){
    Collision_Grid grid = session_context.collision_grid;    
    
    Vector2 origin_to_pos = position - grid.origin;
    
    i32 max_columns = (i32)(grid.size.x / grid.cell_size.x);
    
    i32 column = floor(((origin_to_pos.x + grid.size.x * 0.5f) / grid.cell_size.x));
    i32 row    = floor(((origin_to_pos.y + grid.size.y * 0.5f) / grid.cell_size.y));
    
    if (column < 0 || column >= max_columns || row < 0 || row >= (i32)(grid.size.y / grid.cell_size.y)){
        return NULL;
    }
    
    Collision_Grid_Cell *cell = &grid.cells[column + row * max_columns];
    return cell;
}

void fill_collision_cells(Vector2 position, Array<Vector2, MAX_VERTICES> vertices, Bounds bounds, Vector2 pivot, Dynamic_Array<Collision_Grid_Cell*> *out_cells){
    out_cells->clear();
    Collision_Grid grid = session_context.collision_grid;
    Vector2 center = position + bounds.offset;
    center += {(0.5f - pivot.x) * bounds.size.x, (pivot.y - 0.5f) * bounds.size.y};
    
    // In this for loops we go left to right | bottom to top and it doesn't cover right side, so in loop after we cover fully right side.
    for (f32 h_pos = center.x - bounds.size.x * 0.5f; h_pos < center.x + bounds.size.x * 0.5f; h_pos += grid.cell_size.x){
        for (f32 v_pos = center.y - bounds.size.y * 0.5f; v_pos < center.y + bounds.size.y * 0.5f; v_pos += grid.cell_size.y){    
            Collision_Grid_Cell *cell = get_collision_cell_from_position({h_pos, v_pos});
            if (cell){
                out_cells->add(cell);
            }
        }
        
        Collision_Grid_Cell *cell = get_collision_cell_from_position({h_pos, center.y + bounds.size.y * 0.5f});
        if (cell){
            out_cells->add(cell);
        }
    }
    
    for (f32 v_pos = center.y - bounds.size.y * 0.5f; v_pos < center.y + bounds.size.y * 0.5f; v_pos += grid.cell_size.y){
        Collision_Grid_Cell *cell = get_collision_cell_from_position({center.x + bounds.size.x * 0.5f, v_pos});
        if (cell){
            out_cells->add(cell);
        }
    }
    
    Collision_Grid_Cell *cell = get_collision_cell_from_position({center.x + bounds.size.x * 0.5f, center.y + bounds.size.y * 0.5f});
    if (cell){
        out_cells->add(cell);
    }
}

void update_entity_collision_cells(Entity *entity){
    fill_collision_cells(entity->position, entity->vertices, entity->bounds, entity->pivot, &collision_cells_buffer);    
    
    for (i32 i = 0; i < collision_cells_buffer.count; i++){
        Collision_Grid_Cell *cell = collision_cells_buffer.get(i);
        
        if (cell && !cell->entities_ids.contains(entity->id)){
            cell->entities_ids.add(entity->id);
        }
    }
}

global_variable Dynamic_Array<i32> added_collision_ids = Dynamic_Array<i32>();

void fill_collisions(Vector2 position, Array<Vector2, MAX_VERTICES> vertices, Bounds bounds, Vector2 pivot, Dynamic_Array<Collision> *result, FLAGS include_flags, i32 my_id){
    result->clear();
    
    fill_collision_cells(position, vertices, bounds, pivot, &collision_cells_buffer);
    added_collision_ids.clear();
    
    for (i32 i = 0; i < collision_cells_buffer.count; i++){
        Collision_Grid_Cell *cell = collision_cells_buffer.get(i);
        
        for (i32 c = 0; c < cell->entities_ids.count; c++){
            Entity *other = get_entity_by_id(cell->entities_ids.get(c));
            
            if (!other || other->destroyed || !other->enabled || other->flags <= 0 || ((other->flags & include_flags) <= 0 && include_flags > 0) || (other->hidden && game_state == GAME && !state_context.in_pause_editor) || other->id == my_id || added_collision_ids.contains(other->id)){
                continue;
            }
            
            Collision col = check_collision(position, other->position, vertices, other->vertices, pivot, other->pivot);
            
            if (col.collided){
                added_collision_ids.add(other->id);
                col.other_entity = other;
                result->add(col);
            }
        }
    }
}

void fill_collisions(Entity *entity, Dynamic_Array<Collision> *result, FLAGS include_flags){
    if (entity->destroyed || !entity->enabled){
        return;
    }
    
    fill_collisions(entity->position, entity->vertices, entity->bounds, entity->pivot, result, include_flags, entity->id);
}

void fill_collisions_rect(Vector2 position, Vector2 scale, Vector2 pivot, Dynamic_Array<Collision> *result, FLAGS include_flags){
    Array<Vector2, MAX_VERTICES> vertices = Array<Vector2, MAX_VERTICES>();
    add_rect_vertices(&vertices, pivot);    
    for (i32 i = 0; i < vertices.count; i++){
        vertices.get_ptr(i)->x *= scale.x;
        vertices.get_ptr(i)->y *= scale.y;
    }
    Bounds bounds = get_bounds(vertices, pivot);
    bounds.offset = Vector2_zero;
    make_rect_lines(position, bounds.size, pivot, 5, RED);
    
    
    fill_collisions(position, vertices, bounds, pivot, result, include_flags);   
}

Entity *get_entity_by_index(i32 index){
    if (!current_level_context->entities.has_index(index)){
        //log error
        print("Attempt to get empty entity by index");
        return NULL;
    }
    
    return current_level_context->entities.get_ptr(index);
}

inline Entity *get_entity_by_id(i32 id, Level_Context *level_context){
    if (!level_context){
        level_context = current_level_context;
    }
    if (id == -1 || !level_context->entities.has_key(id)){
        return NULL;
    }
    
    return level_context->entities.get_by_key_ptr(id);
}

i32 get_index_of_entity_id(i32 *ids_array, i32 count, i32 id_to_find){
    for (i32 i = 0; i < count; i++){
        if (ids_array[i] == id_to_find){
            return i;
        }
    }
    
    return -1;
}

inline b32 entity_array_contains_id(Entity *arr, i32 count, i32 id){
    for (i32 i = 0; i < count; i++){
        if (arr[i].id == id){
            return true;
        }
    }
    
    return false;
}

Collision raycast(Vector2 start_position, Vector2 direction, f32 len, FLAGS include_flags, f32 step = 4, i32 my_id = -1){
    f32 current_len = 0;
    Array<Vector2, MAX_VERTICES> ray_vertices = Array<Vector2, MAX_VERTICES>();
    
    b32 found = false;
    Collision result = {};
    while (current_len < len){
        if (current_len + step > len){
            current_len = len;
        } else{
            current_len += step;
        }
        Vector2 east_direction = get_rotated_vector_90(direction, -1);
        ray_vertices.clear();
        ray_vertices.add(direction * current_len + east_direction * 0.5f);
        ray_vertices.add(direction * current_len - east_direction * 0.5f);
        ray_vertices.add(east_direction * 0.5f);
        ray_vertices.add(east_direction * -0.5f);
        
        Bounds ray_bounds = get_bounds(ray_vertices, {0.5f, 1.0f});
    
        fill_collisions(start_position, ray_vertices, ray_bounds, {0.5f, 1.0f}, &collisions_buffer, include_flags, my_id);
    
        for (i32 i = 0; i < collisions_buffer.count; i++){
            result = collisions_buffer.get(i);
            found = true;
            result.point = start_position + direction * current_len - direction * result.overlap;
            break;
        }
        
        if (found){
            break;
        }
    }
    return result;
}

void assign_moving_vertex_entity(Entity *e, i32 vertex_index){
    Vector2 *vertex = e->vertices.get_ptr(vertex_index);

    assign_selected_entity(e);
    editor.moving_vertex = vertex;
    editor.moving_vertex_index = vertex_index;
    editor.moving_vertex_entity = e;
    editor.moving_vertex_entity_id = e->id;
    
    editor.dragging_entity = NULL;
}

void move_vertex(Entity *entity, Vector2 target_position, i32 vertex_index){
    Vector2 *vertex = entity->vertices.get_ptr(vertex_index);
    Vector2 *unscaled_vertex = entity->unscaled_vertices.get_ptr(vertex_index);
    
    Vector2 local_target = local(entity, target_position);

    *vertex = local_target;
    *unscaled_vertex = {vertex->x / entity->scale.x, vertex->y / entity->scale.y};
    
    calculate_bounds(entity);
}

void copy_entity(Entity *dest, Entity *src){
    *dest = *src;
}

void add_undo_action(Undo_Action undo_action){
    // Because we could go back and forth on current undo index with undo/redo. 
    // Hard to wrap mind about that without remembering how undos work, but that's should work.
    Undo_Action *undo_slot = current_level_context->undo_actions.get_ptr(current_level_context->undo_actions.count);
    undo_slot->changed_entities.free_arr();
    Level_Context *original_level_context = current_level_context;
    switch_current_level_context(&undo_level_context);
    for (i32 i = 0; i < undo_slot->deleted_entities.count; i++){
        free_entity(undo_slot->deleted_entities.get_ptr(i));
    }
    undo_slot->deleted_entities.free_arr();
    
    original_level_context->undo_actions.add(undo_action);
    
    if (original_level_context->undo_actions.count >= MAX_UNDOS){
        for (i32 i = 0; i < (i32)(MAX_UNDOS * 0.5f); i++){
            original_level_context->undo_actions.get_ptr(i)->changed_entities.free_arr();
            for (i32 d = 0; d < undo_slot->deleted_entities.count; d++){
                free_entity(undo_slot->deleted_entities.get_ptr(d));
            }
            original_level_context->undo_actions.get_ptr(i)->deleted_entities.free_arr();
        }
        original_level_context->undo_actions.remove_first_half();
    }
    
    switch_current_level_context(original_level_context);
    
    editor.max_undos_added = original_level_context->undo_actions.count;
}

void undo_add_position(Entity *entity, Vector2 position_change){
    Undo_Action undo_action;
    undo_action.position_change = position_change;
    undo_action.moved_entity_points = editor.move_entity_points;
    undo_action.entity_id = entity->id;
    add_undo_action(undo_action);
}

void undo_add_multiselect_position_change(Vector2 change){
    Undo_Action undo_action;
    undo_action.position_change = change;
    undo_action.moved_entity_points = editor.multiselected_entities.count > 1 ? true : editor.move_entity_points;

    for (i32 i = 0; i < editor.multiselected_entities.count; i++){
        undo_action.changed_entities.add(editor.multiselected_entities.get(i));
    }
    
    add_undo_action(undo_action);
}

void undo_add_draw_order(Entity *entity, i32 draw_order_change){
    Undo_Action undo_action;
    undo_action.draw_order_change = draw_order_change;
    undo_action.entity_id = entity->id;
    add_undo_action(undo_action);
}

void undo_add_scaling(Entity *entity, Vector2 scale_change){
    Undo_Action undo_action;
    undo_action.entity_id = entity->id;
    undo_action.scale_change = scale_change;
    
    //SHOULD REMEMBER THEM BEFORE
    undo_apply_vertices_change(editor.selected_entity, &undo_action);
    
    add_undo_action(undo_action);
}

void undo_add_rotation(Entity *entity, f32 rotation_change){
    Undo_Action undo_action;
    undo_action.entity_id = entity->id;
    undo_action.rotation_change = rotation_change;
    
    //SHOULD REMEMBER THEM BEFORE
    undo_apply_vertices_change(editor.selected_entity, &undo_action);
    
    add_undo_action(undo_action);
}

void editor_delete_entity(Entity *entity, b32 add_undo){
    if (add_undo){
        Undo_Action undo_action = {};
        undo_action.entity_was_deleted = true;
        Level_Context *original_level_context = current_level_context;
        switch_current_level_context(&undo_level_context);
        undo_action.deleted_entities.add(Entity(editor.selected_entity, true, original_level_context));
        undo_action.changed_entities.add(editor.selected_entity->id);
        switch_current_level_context(original_level_context);
        // copy_entity(&undo_action.deleted_entity, editor.selected_entity);
        // undo_action.entity_id = undo_action.deleted_entity.id;
        add_undo_action(undo_action);
    }
    entity->destroyed = true;
    editor.selected_entity = NULL;
    editor.dragging_entity = NULL;
    editor.cursor_entity   = NULL;
}

void editor_delete_multiselected_entities(b32 add_undo_to_list, Undo_Action *undo_action){    
    if (undo_action){
        undo_action->changed_entities.clear();
        undo_action->deleted_entities.clear();
    }

    if (undo_action && add_undo_to_list){
        undo_action->entity_was_deleted = true;
    }
    for (i32 i = 0; i < editor.multiselected_entities.count; i++){
        Entity *entity = get_entity_by_id(editor.multiselected_entities.get(i));
        if (!entity){
            continue;    
        }
        
        Level_Context *original_level_context = current_level_context;
        switch_current_level_context(&undo_level_context);
        
        if (undo_action){
            undo_action->deleted_entities.add(Entity(entity, true, original_level_context));
            undo_action->changed_entities.add(entity->id);
        }
        switch_current_level_context(original_level_context);
        editor_delete_entity(entity, false);
    }
    
    if (add_undo_to_list){
        add_undo_action(*undo_action);
    }
        
    // Do not add multiselection selection type undo here, because on undo/redo we selecting them if we deleted them.
    editor.multiselected_entities.clear();
}

inline void editor_delete_multiselected_entities(){
    Undo_Action undo_action = {};
    editor_delete_multiselected_entities(true, &undo_action);
}

void editor_delete_entity(i32 entity_id, b32 add_undo){
    assert(current_level_context->entities.has_key(entity_id));
    editor_delete_entity(current_level_context->entities.get_by_key_ptr(entity_id), add_undo);
}

void undo_apply_vertices_change(Entity *entity, Undo_Action *undo_action){
    for (i32 i = 0; i < entity->vertices.count; i++){
        *undo_action->vertices_change.get_ptr(i) = entity->vertices.get(i) - editor.vertices_start.get(i);
        *undo_action->unscaled_vertices_change.get_ptr(i) = entity->unscaled_vertices.get(i) - editor.unscaled_vertices_start.get(i);
    }
    undo_action->vertices_change.count = entity->vertices.count;
    undo_action->unscaled_vertices_change.count = entity->unscaled_vertices.count;
    undo_action->entity_id = entity->id;
}

void undo_add_vertices_change(Entity *entity){
    Undo_Action undo_action = {};   
    undo_apply_vertices_change(entity, &undo_action);
    add_undo_action(undo_action);
}

void undo_remember_vertices_start(Entity *entity){
    editor.vertices_start.clear();
    editor.unscaled_vertices_start.clear();
    for (i32 i = 0; i < entity->vertices.count; i++){
        *editor.vertices_start.get_ptr(i) = entity->vertices.get(i);
        *editor.unscaled_vertices_start.get_ptr(i) = entity->unscaled_vertices.get(i);
    }
    editor.vertices_start.count = entity->vertices.count; 
    editor.unscaled_vertices_start.count = entity->unscaled_vertices.count; 
}

//new selected could be NULL
void assign_selected_entity(Entity *new_selected){
    if (editor.selected_entity){
        editor.selected_entity->color_changer.changing = 0;
        editor.selected_entity->color = editor.selected_entity->color_changer.start_color;
    }
    
    if (new_selected){
        new_selected->color_changer.changing = 1;
        editor.selected_entity_id = new_selected->id;
    }
    
    editor.selected_entity = new_selected;
    
    focus_input_field.in_focus = false;
}

void start_closing_create_box(){
    editor.create_box_closing = true;
    editor.create_box_lifetime = editor.create_box_slide_time;
}

void close_create_box(){
    editor.create_box_active = false;
    editor.create_box_closing = false;
    if (str_equal(focus_input_field.tag, "create_box")){
        focus_input_field.in_focus = false;
    }

    editor.create_box_lifetime = 0;
}

void make_color_picker(Vector2 inspector_position, Vector2 inspector_size, f32 v_pos, Color *color_ptr){
    make_ui_text("Color: ", {inspector_position.x + 25, v_pos}, "light_bake_shadows");
    f32 color_h_pos_mult = 0.2f;
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, WHITE, *color_ptr == WHITE, "light_color_picker_white")){
        *color_ptr = WHITE;
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(RED, 0.3f), *color_ptr == ColorBrightness(RED, 0.3f), "light_color_picker_RED")){
        *color_ptr = ColorBrightness(RED, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(ORANGE, 0.3f), *color_ptr == ColorBrightness(ORANGE, 0.3f), "light_color_picker_ORANGE")){
        *color_ptr = ColorBrightness(ORANGE, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(SKYBLUE, 0.3f), *color_ptr == ColorBrightness(SKYBLUE, 0.3f), "light_color_picker_SKYBLUE")){
        *color_ptr = ColorBrightness(SKYBLUE, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(BLUE, 0.3f), *color_ptr == ColorBrightness(BLUE, 0.3f), "light_color_picker_BLUE")){
        *color_ptr = ColorBrightness(BLUE, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(GREEN, 0.3f), *color_ptr == ColorBrightness(GREEN, 0.3f), "light_color_picker_GREEN")){
        *color_ptr = ColorBrightness(GREEN, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(LIME, 0.3f), *color_ptr == ColorBrightness(LIME, 0.3f), "light_color_picker_LIME")){
        *color_ptr = ColorBrightness(LIME, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(PINK, 0.3f), *color_ptr == ColorBrightness(PINK, 0.3f), "light_color_picker_PINK")){
        *color_ptr = ColorBrightness(PINK, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(VIOLET, 0.3f), *color_ptr == ColorBrightness(VIOLET, 0.3f), "light_color_picker_VIOLET")){
        *color_ptr = ColorBrightness(VIOLET, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(MAGENTA, 0.3f), *color_ptr == ColorBrightness(MAGENTA, 0.3f), "light_color_picker_MAGENTA")){
        *color_ptr = ColorBrightness(MAGENTA, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(BROWN, 0.3f), *color_ptr == ColorBrightness(BROWN, 0.3f), "light_color_picker_MAGENTA")){
        *color_ptr = ColorBrightness(BROWN, 0.3f);
    }
    color_h_pos_mult += 0.05f;
}

void make_light_size_picker(Vector2 inspector_position, Vector2 inspector_size, f32 v_pos, f32 height_add, i32 *size_flags, Entity *selected){
    assert(selected->light_index != -1);
    f32 h_pos_mult = 0.05f;
    if (make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & ULTRA_SMALL_LIGHT, "ultra_small_size_flag")){
        *size_flags = ULTRA_SMALL_LIGHT;
        init_entity_light(selected, current_level_context->lights.get_ptr(selected->light_index), true);
    }
    make_ui_text("(64): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "ultra_small_size_flag");
    h_pos_mult += 0.15f;
    
    if (make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & SMALL_LIGHT, "small_size_flag")){
        *size_flags = SMALL_LIGHT;
        init_entity_light(selected, current_level_context->lights.get_ptr(selected->light_index), true);
    }
    make_ui_text("(128): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "small_size_flag");
    h_pos_mult += 0.15f;
    
    if (make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & MEDIUM_LIGHT, "medium_light_flag")){
        *size_flags = MEDIUM_LIGHT;
        init_entity_light(selected, current_level_context->lights.get_ptr(selected->light_index), true);
    }
    make_ui_text("(256): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "medium_light_flag");
    h_pos_mult += 0.15f;

    if (make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & BIG_LIGHT, "big_light_flag")){
        *size_flags = BIG_LIGHT;
        init_entity_light(selected, current_level_context->lights.get_ptr(selected->light_index), true);
    }
    make_ui_text("(512): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "big_light_flag");
    h_pos_mult += 0.15f;

    if (make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & HUGE_LIGHT, "huge_light_flag")){
        *size_flags = HUGE_LIGHT;
        init_entity_light(selected, current_level_context->lights.get_ptr(selected->light_index), true);
    }
    make_ui_text("(1024): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "huge_light_flag");
    h_pos_mult += 0.15f;

    if (make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & GIANT_LIGHT, "giant_light_flag")){
        *size_flags = GIANT_LIGHT;
        init_entity_light(selected, current_level_context->lights.get_ptr(selected->light_index), true);
    }
    make_ui_text("(2048): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "giant_light_flag");
    h_pos_mult += 0.15f;
}

#ifndef INSPECTOR_MACRO
#define INSPECTOR_MACRO
    #define INSPECTOR_UI_TOGGLE_COLOR(text, tag, bool_to_change, color, additional_action) { \
        make_ui_text(text, {inspector_position.x + h_pos, v_pos}, tag, color); \
        if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, bool_to_change, tag)){ \
            bool_to_change = !bool_to_change; \
            additional_action; \
        } \
        v_pos += height_add; \
    }   
    #define INSPECTOR_UI_TOGGLE(text, tag, bool_to_change, additional_action) { \
        INSPECTOR_UI_TOGGLE_COLOR(text, tag, bool_to_change, WHITE, additional_action); \
    }   
    #define INSPECTOR_UI_TOGGLE_FLAGS(text, tag, flags, flag, additional_action) { \
        make_ui_text(text, {inspector_position.x + h_pos, v_pos}, tag); \
        if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, flags & flag, tag)){ \
            flags ^= flag; \
            additional_action; \
        } \
        v_pos += height_add; \
    }   
    
    #define INSPECTOR_UI_INPUT_FIELD_COLOR(text, tag, format, value_to_change, convert_function, color, additional_action) { \
        make_ui_text(text, {inspector_position.x + h_pos, v_pos}, tag, color); \
        if (make_input_field(text_format(format, value_to_change), {inspector_position.x + inspector_size.x * 0.4f, v_pos}, 100, tag)){ \
            value_to_change = convert_function(focus_input_field.content); \
            additional_action; \
        } \
        v_pos += height_add;\
    }
    #define INSPECTOR_UI_INPUT_FIELD(text, tag, format, value_to_change, convert_function, additional_action) { \
        INSPECTOR_UI_INPUT_FIELD_COLOR(text, tag, format, value_to_change, convert_function, WHITE, additional_action); \
    }
#endif

void update_editor_ui(){
    //inspector logic
    
    // move entity points hint
    if (editor.selected_entity || editor.multiselected_entities.count > 0){
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F3)){
            editor.move_entity_points = !editor.move_entity_points;
        }
        make_ui_text(text_format("Ctrl+F3:\nMove entity points: %s", editor.move_entity_points ? "YES" : "NO"), {10, screen_height * 0.5f}, 30, Fade(GREEN, 0.6f), "move_entity_points_hint");
    }
    
    Entity *selected = editor.selected_entity;
    if (selected){
        Vector2 inspector_size = {screen_width * 0.2f, screen_height * 0.6f};
        Vector2 inspector_position = {screen_width - inspector_size.x - inspector_size.x * 0.1f, 0 + inspector_size.y * 0.05f};
        make_ui_image(inspector_position, inspector_size, {0, 0}, SKYBLUE * 0.7f, "inspector_window");
        f32 height_add = 30 * UI_SCALING;
        f32 v_pos = inspector_position.y + height_add + 40;
        f32 h_pos = 5;
        
        make_ui_text(text_format("ID: %d", selected->id), {inspector_position.x + inspector_size.x * 0.4f, inspector_position.y - 10}, 18, WHITE, "inspector_id"); 
        
        make_ui_text(text_format("Name: ", selected->id), {inspector_position.x, inspector_position.y + 10}, 24, BLACK, "inspector_id"); 
        if (make_input_field(text_format("%s", selected->name), {inspector_position.x + 65, inspector_position.y + 10}, {200, 25}, "inspector_name") ){
            str_copy(selected->name, focus_input_field.content);
        }
        
        make_ui_text("POSITION", {inspector_position.x + inspector_size.x * 0.4f, inspector_position.y + 40}, 24, WHITE * 0.9f, "inspector_pos");
        make_ui_text("X:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_pos_x");
        make_ui_text("Y:", {inspector_position.x + 5 + 35 + 100, v_pos}, 22, BLACK * 0.9f, "inspector_pos_y");
        if (make_input_field(text_format("%.3f", selected->position.x), {inspector_position.x + 30, v_pos}, {100, 25}, "inspector_pos_x")
            || make_input_field(text_format("%.3f", selected->position.y), {inspector_position.x + 30 + 100 + 35, v_pos}, {100, 25}, "inspector_pos_y")
            ){
            Vector2 old_position = selected->position;
            if (str_equal(focus_input_field.tag, "inspector_pos_x")){
                selected->position.x = to_f32(focus_input_field.content);
            } else if (str_equal(focus_input_field.tag, "inspector_pos_y")){
                selected->position.y = to_f32(focus_input_field.content);
            } else{
                assert(false);
            }
            undo_add_position(selected, selected->position - old_position);
        }
        v_pos += height_add;
        
        make_ui_text("SCALE", {inspector_position.x + inspector_size.x * 0.4f, inspector_position.y + 20 + v_pos - height_add}, 24, WHITE * 0.9f, "inspector_scale");
        v_pos += height_add;
        make_ui_text("X:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_scale_x");
        make_ui_text("Y:", {inspector_position.x + 5 + 35 + 100, v_pos}, 22, BLACK * 0.9f, "inspector_scale_y");
        if (make_input_field(text_format("%.3f", editor.selected_entity->scale.x), {inspector_position.x + 30, v_pos}, {100, 25}, "inspector_scale_x")
            || make_input_field(text_format("%.3f", editor.selected_entity->scale.y), {inspector_position.x + 30 + 100 + 35, v_pos}, {100, 25}, "inspector_scale_y")
            ){
            Vector2 old_scale = editor.selected_entity->scale;
            Vector2 new_scale = old_scale;
            undo_remember_vertices_start(editor.selected_entity);
            
            if (str_equal(focus_input_field.tag, "inspector_scale_x")){
                new_scale.x = to_f32(focus_input_field.content);
            } else if (str_equal(focus_input_field.tag, "inspector_scale_y")){
                new_scale.y = to_f32(focus_input_field.content);
            } else{
                assert(false);
            }
            
            Vector2 scale_add = new_scale - old_scale;
            if (scale_add != Vector2_zero){
                add_scale(editor.selected_entity, scale_add);
            }
            
            undo_add_scaling(editor.selected_entity, scale_add);
        }
        v_pos += height_add;
        
        make_ui_text("Rotation:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_rotation");
        if (make_input_field(text_format("%.2f", editor.selected_entity->rotation), {inspector_position.x + 150, v_pos}, {75, 25}, "inspector_rotation")
            ){
            f32 old_rotation = editor.selected_entity->rotation;
            f32 new_rotation = old_rotation;
            
            undo_remember_vertices_start(editor.selected_entity);
            
            if (str_equal(focus_input_field.tag, "inspector_rotation")){
                new_rotation = to_f32(focus_input_field.content);
            } else{
                assert(false);
            }
            
            f32 rotation_add = new_rotation - old_rotation;
            if (rotation_add != 0){
                rotate(editor.selected_entity, rotation_add);
            }
            
            undo_add_rotation(editor.selected_entity, rotation_add);
        }
        v_pos += height_add;
        
        make_ui_text("Draw Order:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_rotation");
        if (make_input_field(text_format("%d", editor.selected_entity->draw_order), {inspector_position.x + 150, v_pos}, {75, 25}, "inspector_draw_order")
            ){
            i32 old_draw_order = editor.selected_entity->draw_order;
            i32 new_draw_order = old_draw_order;
            
            if (str_equal(focus_input_field.tag, "inspector_draw_order")){
                new_draw_order = to_i32(focus_input_field.content);
            } else{
                assert(false);
            }
            
            i32 draw_order_add = new_draw_order - old_draw_order;
            if (draw_order_add != 0){
                editor.selected_entity->draw_order += draw_order_add;
            }
            
            undo_add_draw_order(editor.selected_entity, draw_order_add);
        }
        v_pos += height_add;
        
        height_add = 16;// * fmax(UI_SCALING, 0.5f);
        f32 type_font_size = 24;
        f32 type_info_v_pos = type_font_size;
        
        for (f32 line_pos = v_pos; line_pos < inspector_size.y; line_pos += height_add){
            make_ui_image({inspector_position.x + 5, line_pos}, {inspector_size.x - 10, 1}, {0, 0}, Fade(ColorBrightness(SKYBLUE, 0.4f), 0.5f), "ui_line");
        }

        //entity settings overall
        if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Entity settings", "entity_settings")){
            editor.draw_entity_settings = !editor.draw_entity_settings;
        }
        v_pos += height_add;
        
        if (editor.draw_entity_settings){
            INSPECTOR_UI_TOGGLE("Hidden: ", "entity_hidden", selected->hidden, );
            
            INSPECTOR_UI_TOGGLE("Spawn enemy when no ammo: ", "spawn_no_ammo", selected->spawn_enemy_when_no_ammo, );
            
            INSPECTOR_UI_TOGGLE_FLAGS("Physics object: ", "physics_object", selected->flags, PHYSICS_OBJECT, );
            // physics object inspector ui
            if (selected->flags & PHYSICS_OBJECT){
                h_pos = 25;
                INSPECTOR_UI_TOGGLE("Simulating: ", "physics_simulating", selected->physics_object.simulating, );
                INSPECTOR_UI_TOGGLE("On rope: ", "on_rope", selected->physics_object.on_rope, );
                INSPECTOR_UI_TOGGLE("Rotate by velocity: ", "physics_rotate_by_velocity", selected->physics_object.rotate_by_velocity, );
                INSPECTOR_UI_INPUT_FIELD("Gravity multiplier: ", "physics_gravity_multiplier", "%.1f", selected->physics_object.gravity_multiplier, to_f32, );
                INSPECTOR_UI_INPUT_FIELD("Mass: ", "physics_mass", "%.1f", selected->physics_object.mass, to_f32, clamp(&selected->physics_object.mass, 0.1f, 100000.0f));
                
                h_pos = 5;
            }
            
            INSPECTOR_UI_TOGGLE_FLAGS("No move block: ", "no_move_block", selected->flags, NO_MOVE_BLOCK, ); 
            INSPECTOR_UI_TOGGLE_FLAGS("Move sequence: ", "entity_move_sequence", selected->flags, MOVE_SEQUENCE, );
            
            // move sequence inspector ui
            if (selected->flags & MOVE_SEQUENCE){
                f32 h_pos = 15;
                INSPECTOR_UI_TOGGLE("Moving: ", "move_sequence_moving", selected->move_sequence.moving, );
                INSPECTOR_UI_TOGGLE("Loop: ", "move_sequence_loop", selected->move_sequence.loop, );
                INSPECTOR_UI_TOGGLE("Rotate: ", "move_sequence_rotate", selected->move_sequence.rotate, );
                
                INSPECTOR_UI_INPUT_FIELD("Speed: ", "move_sequence_speed", "%.1f", selected->move_sequence.speed, to_f32, );  
                
                INSPECTOR_UI_TOGGLE("Speed related player distance: : ", "move_sequence_speed_related_player_distance", selected->move_sequence.speed_related_player_distance, );
                if (selected->move_sequence.speed_related_player_distance){
                    f32 h_pos = 25;
                    INSPECTOR_UI_INPUT_FIELD("Min distance: ", "move_sequence_min_distance", "%.1f", selected->move_sequence.min_distance, to_f32, );  
                    INSPECTOR_UI_INPUT_FIELD("Max distance: ", "move_sequence_max_distance", "%.1f", selected->move_sequence.max_distance, to_f32, );  
                    INSPECTOR_UI_INPUT_FIELD("Max distance speed: ", "move_sequence_max_distance_speed", "%.1f", selected->move_sequence.max_distance_speed, to_f32, );  
                }
                
                make_ui_text(text_format("Points count: %d", selected->move_sequence.points.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_count");
                type_info_v_pos += type_font_size;
                make_ui_text("Ctrl+L clear points", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_clear");
                type_info_v_pos += type_font_size;
                make_ui_text("Ctrl+M Remove point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_remove");
                type_info_v_pos += type_font_size;
                make_ui_text("Ctrl+N Add point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_add_point");
                type_info_v_pos += type_font_size;
                
                make_ui_text("Move sequence settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "move_sequence_settings");
                type_info_v_pos += type_font_size;
            }
            
            if (selected->flags & PROPELLER){            
                INSPECTOR_UI_INPUT_FIELD("Propeller power: ", "propeller_power", "%.0f", selected->propeller.power, to_f32, );
                INSPECTOR_UI_TOGGLE("Spin sensitive :", "propeller_spin_sensitive", selected->propeller.spin_sensitive, );
            }
        } // entity inspector end
        
        if (selected->flags & NOTE){
            f32 h_pos = 15;
            Note *note = current_level_context->notes.get_ptr(selected->note_index);
            INSPECTOR_UI_TOGGLE("NOTE draw in game: ", "note_draw_in_game", note->draw_in_game, );
        }
        
        // inspector light inspector
        if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Light settings", "light_settings")){
            editor.draw_light_settings = !editor.draw_light_settings;
        }
        v_pos += height_add;
        
        if (editor.draw_light_settings){
            INSPECTOR_UI_TOGGLE_FLAGS("Make light: ", "make_light", selected->flags, LIGHT, 
                if (selected->flags & LIGHT){
                    init_entity_light(selected);                    
                } else{
                    free_entity_light(selected);
                }
            );
            if (selected->flags & LIGHT && selected->light_index >= 0){
                Light *light = current_level_context->lights.get_ptr(selected->light_index);
                
                make_color_picker(inspector_position, inspector_size, v_pos, &light->color);
                v_pos += height_add;
                
                INSPECTOR_UI_INPUT_FIELD("Light radius: ", "light_radius", "%.2f", light->radius, to_f32, );
                INSPECTOR_UI_INPUT_FIELD("Light opacity: ", "light_opacity", "%.2f", light->opacity, to_f32, );
                INSPECTOR_UI_INPUT_FIELD("Light power: ", "light_power", "%.2f", light->power, to_f32, );
                
                // Leave this without macro for colors.
                make_ui_text("Bake shadows: ", {inspector_position.x + 5, v_pos}, "light_bake_shadows", 17, ColorBrightness(light->bake_shadows ? GREEN : RED, 0.5f));
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, light->bake_shadows, "light_bake_shadows")){
                    light->bake_shadows = !light->bake_shadows;
                    init_entity_light(selected, light, true);
                }
                v_pos += height_add;
                
                INSPECTOR_UI_TOGGLE("Make shadows: ", "light_make_shadows", light->make_shadows, init_entity_light(selected, light, true));

                if (light->make_shadows){
                    make_ui_text("Shadows Size flags: ", {inspector_position.x + 5, v_pos}, "shadows_size_flags");
                    v_pos += height_add;
                    make_light_size_picker(inspector_position, inspector_size, v_pos, height_add, &light->shadows_size_flags, selected);
                    v_pos += height_add * 2;
                }
                
                INSPECTOR_UI_TOGGLE("Make backshadows: ", "light_make_backshadows", light->make_backshadows, init_entity_light(selected, light, true));

                if (light->make_backshadows){
                    make_ui_text("Backshadows Size flags: ", {inspector_position.x + 5, v_pos}, "backshadows_size_flags");
                    v_pos += height_add;
                    make_light_size_picker(inspector_position, inspector_size, v_pos, height_add, &light->backshadows_size_flags, selected);
                    v_pos += height_add * 2;
                }
            }
        }
        
        // trigger inspector
        if (selected->flags & TRIGGER){
            if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Trigger settings", "trigger_settings")){
                editor.draw_trigger_settings = !editor.draw_trigger_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_trigger_settings){
                if (make_button({inspector_position.x + inspector_size.x * 0.2f, v_pos + 3}, {inspector_size.x * 0.6f, height_add - 4}, "Trigger (in game)", "trigger_now_button", SKYBLUE, ColorBrightness(BROWN, -0.3f)) && game_state == GAME){
                    selected->trigger.debug_should_trigger_now = true;
                }
                v_pos += height_add;
            
                INSPECTOR_UI_TOGGLE("Activate on player: ", "trigger_player_touch", selected->trigger.player_touch, );
                INSPECTOR_UI_TOGGLE("Die after trigger: ", "trigger_die_after_trigger", selected->trigger.die_after_trigger, );
                INSPECTOR_UI_TOGGLE("Kill player: ", "trigger_kill_player", selected->trigger.kill_player, );
                INSPECTOR_UI_TOGGLE("Kill enemies: ", "trigger_kill_enemies", selected->trigger.kill_enemies, );
                INSPECTOR_UI_TOGGLE("Doors Open(1) Close(0): ", "trigger_open_doors", selected->trigger.open_doors, );
                INSPECTOR_UI_TOGGLE("Start physics: ", "trigger_start_physics_simulation", selected->trigger.start_physics_simulation, );
                INSPECTOR_UI_TOGGLE("Lines to tracked: ", "trigger_draw_lines_to_tracked", selected->trigger.draw_lines_to_tracked, );
                INSPECTOR_UI_TOGGLE("Agro enemies: ", "trigger_agro_enemies", selected->trigger.agro_enemies, );
                INSPECTOR_UI_TOGGLE("Show(1) Hide(0) entities: ", "trigger_shows_entities", selected->trigger.shows_entities, );
                INSPECTOR_UI_TOGGLE("Starts moving sequence: ", "trigger_starts_moving_sequence", selected->trigger.starts_moving_sequence, );
                
                Color cam_section_color = ColorBrightness(PINK, 0.4f);
                INSPECTOR_UI_TOGGLE_COLOR("Change zoom: ", "trigger_change_zoom", selected->trigger.change_zoom, cam_section_color, );
                if (selected->trigger.change_zoom){
                    f32 h_pos = 10;
                    INSPECTOR_UI_INPUT_FIELD_COLOR("Zoom value: ", "trigger_zoom_value", "%.2f", selected->trigger.zoom_value, to_f32, ColorBrightness(cam_section_color, -0.1f), );
                }
                
                INSPECTOR_UI_TOGGLE_COLOR("Cam rails horizontal: ", "trigger_start_cam_rails_horizontal", selected->trigger.start_cam_rails_horizontal, cam_section_color, init_entity(selected));
                INSPECTOR_UI_TOGGLE_COLOR("Cam rails vertical: ", "trigger_start_cam_rails_vertical", selected->trigger.start_cam_rails_vertical, cam_section_color, init_entity(selected));
                INSPECTOR_UI_TOGGLE_COLOR("Stop cam rails: ", "trigger_stop_cam_rails", selected->trigger.stop_cam_rails, cam_section_color, init_entity(selected));
                
                INSPECTOR_UI_TOGGLE_COLOR("Lock camera: ", "trigger_lock_camera", selected->trigger.lock_camera, cam_section_color, 
                    if (selected->trigger.lock_camera && selected->trigger.locked_camera_position == Vector2_zero){
                        selected->trigger.locked_camera_position = selected->position;
                    }
                );
                
                INSPECTOR_UI_TOGGLE_COLOR("Unlock camera: ", "trigger_unlock_camera", selected->trigger.unlock_camera, cam_section_color, );
                
                INSPECTOR_UI_TOGGLE("Allow player shoot: ", "trigger_allow_player_shoot", selected->trigger.allow_player_shoot, );
                INSPECTOR_UI_TOGGLE("Forbid player shoot: ", "trigger_forbid_player_shoot", selected->trigger.forbid_player_shoot, );
                
                INSPECTOR_UI_TOGGLE_COLOR("Play sound: ", "trigger_play_sound", selected->trigger.play_sound, cam_section_color, );
                if (selected->trigger.play_sound){
                    make_ui_text("Sound name: ", {inspector_position.x + 5, v_pos}, "trigger_play_sound_name_text");
                    if (make_input_field(selected->trigger.sound_name, {inspector_position.x + inspector_size.x * 0.4f, v_pos}, {inspector_size.x * 0.25f, 20}, "trigger_sound_name") ){
                        str_copy(selected->trigger.sound_name, focus_input_field.content);
                    }
                    v_pos += height_add;
                }
                
                INSPECTOR_UI_TOGGLE("Load level: ", "trigger_load_level", selected->trigger.load_level, );
                if (selected->trigger.load_level){
                    make_ui_text("Level name: ", {inspector_position.x + 5, v_pos}, "trigger_load_level_name_text");
                    if (make_input_field(selected->trigger.level_name, {inspector_position.x + inspector_size.x * 0.4f, v_pos}, {inspector_size.x * 0.6f, 20}, "trigger_load_level_name") ){
                        str_copy(selected->trigger.level_name, focus_input_field.content);
                    }
                    v_pos += height_add;
                }
                INSPECTOR_UI_TOGGLE("Play replay: ", "trigger_play_replay", selected->trigger.play_replay, );
                if (selected->trigger.play_replay){
                    make_ui_text("Replay name: ", {inspector_position.x + 5, v_pos}, "trigger_replay_name");
                    if (make_input_field(selected->trigger.replay_name, {inspector_position.x + inspector_size.x * 0.4f, v_pos}, {inspector_size.x * 0.6f, 20}, "trigger_replay_name") ){
                        str_copy(selected->trigger.replay_name, focus_input_field.content);
                    }
                    v_pos += height_add;
                }
            }
        
            if (selected->trigger.start_cam_rails_horizontal || selected->trigger.start_cam_rails_vertical){
                make_ui_text("Ctrl+L rails clear points", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "cam_rails_clear");
                type_info_v_pos += type_font_size;
                make_ui_text("Ctrl+M Rails Remove point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "cam_rails_remove");
                type_info_v_pos += type_font_size;
                make_ui_text("Ctrl+N Rails Add point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "cam_rails_add_point");
                type_info_v_pos += type_font_size;
                make_ui_text(text_format("Rails points count: %d", selected->trigger.cam_rails_points.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "trigger_rails_points_count");
            type_info_v_pos += type_font_size;

            }
            if (selected->trigger.change_zoom){
                make_ui_text("Ctrl+R: Camera position", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "locked_cam_position");
                type_info_v_pos += type_font_size;
            }
            make_ui_text("Clear ALL Connected: Ctrl+L", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_clear");
            type_info_v_pos += type_font_size;
            make_ui_text("Remove selected: Ctrl+D", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_remove");
            type_info_v_pos += type_font_size;
            make_ui_text("Assign New: Ctrl+A", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_assign");
            type_info_v_pos += type_font_size;
            make_ui_text("Assign tracking enemy: Ctrl+Q", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_assign");
            type_info_v_pos += type_font_size;
            make_ui_text(text_format("Connected count: %d", selected->trigger.connected.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "trigger_connected_count");
            type_info_v_pos += type_font_size;
            make_ui_text(text_format("Tracking count: %d", selected->trigger.tracking.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "trigger_tracking_count");
            type_info_v_pos += type_font_size;
            make_ui_text("Trigger settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "trigger_settings");
            type_info_v_pos += type_font_size;
        }
        
        if (selected->flags & KILL_SWITCH){
            make_ui_text("Clear ALL Connected: Ctrl+L", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "kill_switch_clear");
            type_info_v_pos += type_font_size;
            make_ui_text("Remove selected: Ctrl+D", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "kill_switch_remove");
            type_info_v_pos += type_font_size;
            make_ui_text("Assign New: Ctrl+A", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "kill_switch_assign");
            type_info_v_pos += type_font_size;
            make_ui_text(text_format("Connected count: %d", selected->trigger.connected.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "kill_switch_connected_count");
            type_info_v_pos += type_font_size;
            make_ui_text("Kill switch settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "kill_switch_settings");
            type_info_v_pos += type_font_size;
        }
        
        // enemy inspector
        if (selected->flags & ENEMY){
            if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Enemy settings", "enemy_settings")){
                editor.draw_enemy_settings = !editor.draw_enemy_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_enemy_settings){
                INSPECTOR_UI_TOGGLE("Gives ammo: ", "enemy_gives_ammo", selected->enemy.gives_ammo, );
                
                INSPECTOR_UI_TOGGLE_FLAGS("Explosive: ", "enemy_explosive", selected->flags, EXPLOSIVE, 
                    if (!(selected->flags & EXPLOSIVE)){
                        free_entity_light(selected);
                    }
                    init_entity(selected);
                );
                
                if (selected->flags & EXPLOSIVE){
                    h_pos = 25;
                    INSPECTOR_UI_INPUT_FIELD("Explosion radius: ", "explosive_radius_multiplier", "%.1f", selected->enemy.explosive_radius_multiplier, to_f32, 
                        selected->enemy.explosive_radius_multiplier = fmaxf(selected->enemy.explosive_radius_multiplier, 0);
                        init_entity(selected);
                    );
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Blocker: ", "enemy_blocker", selected->flags, BLOCKER, init_entity(selected)); 
                
                if (selected->flags & BLOCKER){
                    h_pos = 25;
                    INSPECTOR_UI_TOGGLE("Blocker immortal: ", "blocker_immortal", selected->enemy.blocker_immortal, init_entity(selected));
                    if (!selected->enemy.blocker_immortal){
                        INSPECTOR_UI_TOGGLE("Blocker clockwise: ", "blocker_clockwise", selected->enemy.blocker_clockwise, init_entity(selected));
                    }
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Shoot blocker: ", "enemy_shoot_blocker", selected->flags, SHOOT_BLOCKER, init_entity(selected)); 
                if (selected->flags & SHOOT_BLOCKER){
                    h_pos = 25;
                    INSPECTOR_UI_TOGGLE("Shoot blocker immortal: ", "shoot_blocker_immortal", selected->enemy.shoot_blocker_immortal, init_entity(selected));
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Sword size required: ", "enemy_sword_size_required", selected->flags, SWORD_SIZE_REQUIRED, init_entity(selected)); 
                if (selected->flags & SWORD_SIZE_REQUIRED){
                    h_pos = 25;
                    INSPECTOR_UI_TOGGLE("Big (1) or small (0) killable: ", "enemy_big_or_small_killable", selected->enemy.big_sword_killable, init_entity(selected));
                
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Multiple hits: ", "enemy_multiple_hits", selected->flags, MULTIPLE_HITS, init_entity(selected)); 
            }
        
            make_ui_text(text_format("Ctrl+O/P Sword kill speed: %.1f", selected->enemy.sword_kill_speed_modifier), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "sword_kill_speed_modifier_change");
            type_info_v_pos += type_font_size;
            
            if (selected->flags & SHOOT_BLOCKER){
                if (!selected->enemy.shoot_blocker_immortal){
                    make_ui_text(text_format("Ctrl+F/G Shoot Block Vector: {%.2f, %.2f}", selected->enemy.shoot_blocker_direction.x, selected->enemy.shoot_blocker_direction.y), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "shoot_blocker_direction");
                    type_info_v_pos += type_font_size;
                }
            }

            make_ui_text("Enemy settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "enemy_settings");
            type_info_v_pos += type_font_size;
        } // enemy inspector end
        
        if (selected->flags & PROPELLER){
            make_ui_text(text_format("Ctrl+Q/E Power change: %.0f", selected->propeller.power), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "propeller_power");
            type_info_v_pos += type_font_size;
            
            make_ui_text("Propeller settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "propeller_settings");
            type_info_v_pos += type_font_size;

        }
        
        if (selected->flags & CENTIPEDE){
            if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Centipede settings", "centipede_settings")){
                editor.draw_centipede_settings = !editor.draw_centipede_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_centipede_settings){
                INSPECTOR_UI_TOGGLE("Spikes on right: ", "spikes_on_right", selected->centipede.spikes_on_right, );
                INSPECTOR_UI_TOGGLE("Spikes on left: ", "spikes_on_left", selected->centipede.spikes_on_left, );

                INSPECTOR_UI_INPUT_FIELD("Segments count:", "segments_count", "%d", selected->centipede.segments_count, to_i32,
                    selected->centipede.segments_count = fminf(selected->centipede.segments_count, MAX_CENTIPEDE_SEGMENTS);
                );
            }
        }
        
        // jumps shooter inspector
        if (selected->flags & JUMP_SHOOTER){
            if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Jump shooter settings", "jump_shooter_settings")){
                editor.draw_jump_shooter_settings = !editor.draw_jump_shooter_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_jump_shooter_settings){
                INSPECTOR_UI_INPUT_FIELD("Shots count:", "jump_shooter_shots_count", "%d", selected->jump_shooter.shots_count, to_i32, );
                INSPECTOR_UI_INPUT_FIELD("Spread:", "jump_shooter_spread", "%.1f", selected->jump_shooter.spread, to_f32,
                    selected->jump_shooter.spread = clamp(selected->jump_shooter.spread, 0.0f, 180.0f);
                );   
                
                INSPECTOR_UI_INPUT_FIELD("Explosive count:", "jump_shooter_explosive_count", "%d", selected->jump_shooter.explosive_count, to_i32, 
                    selected->jump_shooter.explosive_count = fmin(fmin(selected->jump_shooter.explosive_count, 64), selected->jump_shooter.shots_count);
                );

                INSPECTOR_UI_TOGGLE("Shoot sword blockers: ", "shoot_sword_blockers", selected->jump_shooter.shoot_sword_blockers, );
                
                if (selected->jump_shooter.shoot_sword_blockers){
                    h_pos = 15;
                    INSPECTOR_UI_TOGGLE("Sword blockers immortal: ", "shoot_sword_blockers_immortal", selected->jump_shooter.shoot_sword_blockers_immortal, );
                    h_pos = 5;
                }
                
                make_ui_text("Shoot bullet blockers: ", {inspector_position.x + 5, v_pos}, "shoot_bullet_blockers");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->jump_shooter.shoot_bullet_blockers, "shoot_bullet_blockers")){
                    selected->jump_shooter.shoot_bullet_blockers = !selected->jump_shooter.shoot_bullet_blockers;
                }
                v_pos += height_add;
            }
        }
        
        // inspector turret inspector
        if (selected->flags & TURRET){
            if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Turret settings", "turret_settings")){
                editor.draw_turret_settings = !editor.draw_turret_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_turret_settings){
                Turret *turret = &selected->enemy.turret;
                
                INSPECTOR_UI_TOGGLE_FLAGS("Shoot blockers: ", "turret_shoot_blockers", turret->projectile_settings.enemy_flags, BLOCKER, );
                
                if (turret->projectile_settings.enemy_flags & BLOCKER){
                    h_pos = 15;
                    INSPECTOR_UI_TOGGLE("Sword blockers clockwise: ", "turret_shoot_sword_blocker_clockwise", turret->projectile_settings.blocker_clockwise, );
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Shoot explosive: ", "turret_shoot_explosive", turret->projectile_settings.enemy_flags, EXPLOSIVE, );
                
                // I think it's better to just have separate turret entitites for homing ones so we could change visuals without problems.
                // INSPECTOR_UI_TOGGLE("Homing projectiels: ", "turret_homing_projectiles", turret->projectile_settings.homing, );
                INSPECTOR_UI_TOGGLE("Activated: ", "turret_activated", turret->activated, );
                INSPECTOR_UI_INPUT_FIELD("Shoot every x tick: ", "turret_shoot_every_tick", "%d", turret->shoot_every_tick, to_i32, );
                INSPECTOR_UI_INPUT_FIELD("Start delay: ", "turret_start_tick_delay", "%d", turret->start_tick_delay, to_i32, );
                INSPECTOR_UI_INPUT_FIELD("Projectile speed: ", "turret_projectile_speed", "%.0f", turret->projectile_settings.launch_speed, to_f32, );
                INSPECTOR_UI_INPUT_FIELD("Max lifetime: ", "turret_projectile_max_lifetime", "%.0f", turret->projectile_settings.max_lifetime, to_f32, );
                if (turret->homing){
                    INSPECTOR_UI_INPUT_FIELD("Shoot width: ", "turret_shoot_width", "%.0f", turret->shoot_width, to_f32, );
                    INSPECTOR_UI_INPUT_FIELD("Shoot height: ", "turret_shoot_height", "%.0f", turret->shoot_height, to_f32, );
                }
            }
        }

        if (selected->flags & DOOR){
            if (make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Door settings", "door_settings")){
                editor.draw_door_settings = !editor.draw_door_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_door_settings){
                INSPECTOR_UI_TOGGLE("Open: ", "door_open_closed", selected->door.is_open, );
            }
        
            make_ui_text(text_format("Ctrl+T Trigger: %s", selected->door.is_open ? "Open" : "Close"), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "door_trigger");
            type_info_v_pos += type_font_size;
            
            make_ui_text("Door settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "door_settings");
            type_info_v_pos += type_font_size;
        }
        
        //type info background
        make_ui_image({inspector_position.x - 160, (f32)screen_height - type_info_v_pos}, {(f32)screen_width * 0.5f, type_info_v_pos}, {0, 0}, SKYBLUE * 0.7f, "inspector_type_info_background");
    }
    
    //create box
    b32 writing_other_input_field = focus_input_field.in_focus && !editor.create_box_active;
    b32 can_control_create_box = !console.is_open && !writing_other_input_field;
    b32 need_close_create_box = false;
    
    if (can_control_create_box && IsKeyPressed(KEY_SPACE) && editor.in_editor_time > 0.05f){
        if (editor.create_box_active && !editor.create_box_closing){
            need_close_create_box = true;
        } else{ //open create box
            editor.create_box_open_mouse_position = input.mouse_position;
            
            editor.create_box_scrolled = 0;
            editor.create_box_active = true;
            editor.create_box_closing = false;
            editor.create_box_lifetime = 0;
            make_next_input_field_in_focus("create_box");
            assign_selected_entity(NULL);
        }
    }
    
    if (can_control_create_box && IsKeyPressed(KEY_ESCAPE)){
        if (editor.create_box_active){
            need_close_create_box = true;
        } else if (editor.selected_entity){
            assign_selected_entity(NULL);
        }
    }
    
    if (can_control_create_box && editor.create_box_active){
        if (IsKeyPressed(KEY_DOWN)){
            editor.create_box_selected_index++;
            if (editor.create_box_selected_index < 0){
                editor.create_box_selected_index = 0;
            }
        }
        if (IsKeyPressed(KEY_UP)){
            editor.create_box_selected_index--;
            if (editor.create_box_selected_index < 0){
                editor.create_box_selected_index = 0;
            }
        }
    
        Vector2 field_size = {600, 50};
        Vector2 field_target_position = {screen_width * 0.5f - field_size.x * 0.5f, 100};
        Vector2 field_start_position = field_target_position - Vector2_up * field_size.y * 6;
        
        if (editor.create_box_closing){
            editor.create_box_lifetime -= core.time.real_dt;
            if (editor.create_box_lifetime <= 0){
                need_close_create_box = true;
            }
        } else{
            editor.create_box_lifetime += core.time.real_dt;
        }
        
        f32 create_t = clamp01(editor.create_box_lifetime / editor.create_box_slide_time);
        
        Vector2 field_position = lerp(field_start_position, field_target_position, EaseOutBack(create_t));
        
        i32 input_len = str_len(focus_input_field.content);
        i32 fitting_count = 0;
        f32 alpha_multiplier = lerp(0.0f, 1.0f, clamp01(create_t * 2));
        
        for (i32 i = 0; i < spawn_objects.count; i++){
            Spawn_Object obj = spawn_objects.get(i);
            if (input_len > 0 && !str_contains(obj.name, focus_input_field.content)){
                continue;
            }
            
            editor.create_box_scrolled += GetMouseWheelMove();
            Vector2 obj_position = field_position + Vector2_up * field_size.y * (fitting_count + 1) + Vector2_up * editor.create_box_scrolled + Vector2_right * field_size.x * 0.2f;
            Vector2 obj_size = {field_size.x * 0.6f, field_size.y};
            
            b32 this_object_selected = editor.create_box_selected_index == fitting_count;
            
            Color button_color = lerp(BLACK * 0, BLACK * 0.9f, clamp01(create_t * 2));
            Color text_color   = lerp(WHITE * 0, WHITE * 0.9f, clamp01(create_t * 2));
            
            if (make_button(obj_position, obj_size, {0, 0}, obj.name, 24, "create_box", button_color, text_color) || (this_object_selected && IsKeyPressed(KEY_ENTER))){
                Entity *entity = add_entity(&obj.entity);
                entity->position = editor.create_box_open_mouse_position;
                need_close_create_box = true;
                
                Undo_Action undo_action;
                undo_action.spawned_entity = *entity;
                undo_action.entity_id = entity->id;
                undo_action.entity_was_spawned = true;
                add_undo_action(undo_action);
            }
            
            if (obj.entity.flags & TEXTURE){
                Vector2 texture_position = obj_position - Vector2_right * field_size.y;
                make_ui_image(obj.entity.texture, texture_position, {field_size.y, field_size.y}, {0, 0}, Fade(WHITE, alpha_multiplier), "create_box_obj_texture");
            }
            
            if (this_object_selected){
                f32 color_multiplier = lerp(0.7f, 0.9f, (sinf(core.time.app_time * 3) + 1) * 0.5f);
                Color selected_color = lerp(WHITE * 0, WHITE * color_multiplier, clamp01(create_t * 2));
                make_ui_image(obj_position, {obj_size.x * 0.2f, obj_size.y}, {1, 0}, selected_color, "create_box");
            }
            
            fitting_count++;
        }
        
        if (fitting_count > 0 && editor.create_box_selected_index > fitting_count - 1){
            editor.create_box_selected_index = fitting_count - 1;   
        }
    
        if (make_input_field("", field_position, field_size, "create_box", GRAY, false)){
            need_close_create_box = true;
        }
    }
    
    if (need_close_create_box){
        if (editor.create_box_closing){
            close_create_box();
        } else{
            start_closing_create_box();
        }
    }
} //editor ui end

Entity *get_cursor_entity(){
    Entity *cursor_entity_candidate = NULL;
    
    b32 mouse_on_selected_entity = editor.selected_entity && check_entities_collision(&mouse_entity, editor.selected_entity).collided;
    
    fill_collisions(&mouse_entity, &collisions_buffer, 0);
    
    for (i32 i = 0; i < collisions_buffer.count; i++){
        Entity *e = collisions_buffer.get(i).other_entity;
        if (editor.last_click_position == input.mouse_position){
            //If we long enough on one entity we assume that we want to pick it up and not to cycle
            f32 time_since_last_click = core.time.app_time - editor.last_click_time;
            if (time_since_last_click >= 0.5f && mouse_on_selected_entity){
                if (e->id == editor.selected_entity->id){
                    cursor_entity_candidate = e;
                }
            } else if (!cursor_entity_candidate && editor.selected_entity && e->id != editor.selected_entity->id){
                cursor_entity_candidate = e;
            } else if (!editor.place_cursor_entities.contains(e)){
                cursor_entity_candidate = e;
            }
        } else{ //Mouse moved from last click
            //If mouse moved we always want cursor entity to be a selected entity
            if (mouse_on_selected_entity){
                if (e->id == editor.selected_entity->id){
                    cursor_entity_candidate = e;
                }
            } else{
                b32 other_entity_preferable = (e->flags & GROUND || e->flags & PLATFORM || e->flags & ENEMY || e->flags & PROPELLER || e->flags & TRIGGER || e->flags & DUMMY || e->flags & (CENTIPEDE | CENTIPEDE_SEGMENT));
                //We want to pick most valuable entity for selection and not background if there's something more around
                if (!cursor_entity_candidate || other_entity_preferable){
                    cursor_entity_candidate = e;
                }
            }
            editor.place_cursor_entities.clear();
        }
    }        
    
    return cursor_entity_candidate;
}

Entity *editor_spawn_entity(const char *name, Vector2 position){
    Entity *entity = spawn_object_by_name(name, input.mouse_position);
    
    if (entity){
        Undo_Action undo_action;
        undo_action.spawned_entity = *entity;
        undo_action.entity_id = entity->id;
        undo_action.entity_was_spawned = true;
        add_undo_action(undo_action);
    }
    
    return entity;
}

b32 snap_vertex_to_closest(Entity *entity, Vector2 *entity_vertex, i32 vertex_index){ 
    if (!editor.selected_entity){
        return false;
    }

    Vector2 closest_vertex_global = Vector2_zero;
    f32 distance_to_closest_vertex = INFINITY;

    for (i32 i = 0; i < current_level_context->entities.max_count; i++){        
        Entity *e = current_level_context->entities.get_ptr(i);
        
        if (!e->enabled){
            continue;
        }

        for (i32 v = 0; v < e->vertices.count; v++){
            Vector2 *vertex = e->vertices.get_ptr(v);
            
            Vector2 vertex_global = global(e, *vertex);
                
            if (e->id != editor.selected_entity->id){
                f32 sqr_distance = sqr_magnitude(global(editor.selected_entity, *entity_vertex) - vertex_global);
                if (sqr_distance < distance_to_closest_vertex){
                    distance_to_closest_vertex = sqr_distance;
                    closest_vertex_global = vertex_global;
                }
            }
        }
    }
    
    Vector2 new_position = closest_vertex_global - *entity_vertex;
    Vector2 position_change = new_position - entity->position;
    entity->position = new_position;     
    undo_add_position(entity, position_change);
    
    // // Because when we start moving vertex we remembering these vertices already. 
    // // So if we do that here aswell - on undo vertices will go on place where we pressed button.
    // // Really need to change undo system though.
    // if (!editor.moving_vertex_entity){
    //     undo_remember_vertices_start(editor.selected_entity);
    // }
    // move_vertex(editor.selected_entity, closest_vertex_global, vertex_index);

    // undo_add_vertices_change(editor.selected_entity);
    
    return true;
}

inline b32 is_vertex_on_mouse(Vector2 vertex_global){
    return check_col_circles({input.mouse_position, 1}, {vertex_global, 0.5f * (0.4f / current_level_context->cam.cam2D.zoom)});
}

void editor_move_entity_points(Entity *entity, Vector2 displacement){
    if (entity->flags & MOVE_SEQUENCE){
        for (i32 i = 0; i < entity->move_sequence.points.count; i++){
            *entity->move_sequence.points.get_ptr(i) += displacement;
        }
    }
    if (entity->flags & TRIGGER){
        for (i32 i = 0; i < entity->trigger.cam_rails_points.count; i++){
            *entity->trigger.cam_rails_points.get_ptr(i) += displacement;
        }
    }
}

inline Vector2 get_editor_mouse_move(){
    f32 zoom = current_level_context->cam.cam2D.zoom;
    return cast(Vector2){input.mouse_delta.x / zoom, -input.mouse_delta.y / zoom} / (current_level_context->cam.unit_size);
}

inline f32 round_to_factor(f32 number, f32 quantization_factor){
    return roundf(number / quantization_factor) * quantization_factor;
}
inline Vector2 round_to_factor(Vector2 vec, f32 quantization_factor){
    return {round_to_factor(vec.x, quantization_factor), round_to_factor(vec.y, quantization_factor)};
}

void editor_mouse_move_entity(Entity *entity){
    Vector2 move_delta = get_editor_mouse_move();
    
    f32 cell_size = 5;
    
    b32 moving_without_cell_bound = IsKeyDown(KEY_LEFT_ALT);
    if (!moving_without_cell_bound){
        move_delta = input.mouse_position - (entity->position + editor.dragging_start_mouse_offset);
    }
    
    if (moving_without_cell_bound){
        entity->position += move_delta;
    } else if (sqr_magnitude(move_delta) >= (cell_size * 0.5f * cell_size * 0.5f)){
        Vector2 next_position = entity->position + move_delta;
        Vector2 cell_position = {round_to_factor(next_position.x, cell_size), round_to_factor(next_position.y, cell_size)};
        move_delta = cell_position - entity->position;
        entity->position += move_delta;
    }
    
    if (editor.move_entity_points){
        editor_move_entity_points(entity, move_delta);
    }
}

void restore_deleted_entities(Undo_Action *action){
    assert(action->deleted_entities.count > 0);
    assert(action->deleted_entities.count == action->changed_entities.count);
    editor.multiselected_entities.clear();
    for (i32 i = 0; i < action->deleted_entities.count; i++){            
        i32 deleted_entity_id = action->changed_entities.get(i);
        // We should now have deleted entity id present on scene anyhow, because even if we spawned someone and 
        // he's taked that id - on undo we should remove him.
        assert(get_entity_by_id(deleted_entity_id) == NULL);
        Entity *restored_entity = add_entity(action->deleted_entities.get_ptr(i), true, &undo_level_context);
        restored_entity->id = deleted_entity_id;
        
        if (action->changed_entities.count > 1){
            editor.multiselected_entities.add(deleted_entity_id);
        } else{
            editor.selected_entity = restored_entity;
        }
    }
}

inline i32 get_index_of_id(Dynamic_Array<i32> *arr, i32 id){
    return get_index_of_entity_id(arr->data, arr->count, id);
}

void add_to_multiselection(i32 id, b32 add_to_undo){
    i32 index = get_index_of_id(&editor.multiselected_entities, id);
    
    if (index == -1){
        editor.multiselected_entities.add(id);
        
        if (add_to_undo){
            Undo_Action undo_action = {};
            undo_action.added_to_multiselection = true;
            undo_action.changed_entities.add(id);
            add_undo_action(undo_action);
        }
    }
}

void add_to_multiselection(Dynamic_Array<i32> *ids, b32 add_to_undo){
    Undo_Action undo_action = {};
    undo_action.added_to_multiselection = true;
    
    for (i32 i = 0 ; i < ids->count; i++){
        i32 id = ids->get(i);
        i32 index = get_index_of_id(&editor.multiselected_entities, id);
        
        if (index == -1){
            editor.multiselected_entities.add(id);
            
            if (add_to_undo){
                undo_action.changed_entities.add(id);
            }
        }
    }
    
    if (add_to_undo && undo_action.changed_entities.count > 0){
        add_undo_action(undo_action);
    }
}

void remove_from_multiselection(Dynamic_Array<i32> *ids, b32 add_to_undo){
    Undo_Action undo_action = {};
    undo_action.removed_from_multiselection = true;

    for (i32 i = 0; i < ids->count; i++){
        i32 id = ids->get(i);
        i32 index_to_remove_from_multiselected = get_index_of_id(&editor.multiselected_entities, id);
        if (index_to_remove_from_multiselected != -1){
            editor.multiselected_entities.remove(index_to_remove_from_multiselected);
            
            if (add_to_undo){
                undo_action.changed_entities.add(id);
            }
        }
    }
    
    if (add_to_undo && undo_action.changed_entities.count > 0){
        add_undo_action(undo_action);
    }
}

void remove_from_multiselection(i32 id, b32 add_to_undo){
    i32 index = get_index_of_id(&editor.multiselected_entities, id);
    if (index != -1){
        editor.multiselected_entities.remove(index);
        
        if (add_to_undo){
            Undo_Action undo_action = {};
            undo_action.removed_from_multiselection = true;
            undo_action.changed_entities.add(id);
            add_undo_action(undo_action);
        }
    }
}

void clear_multiselected_entities(b32 add_to_undo){
    if (add_to_undo && editor.multiselected_entities.count > 0){
        Undo_Action undo_action = {};
        undo_action.removed_from_multiselection = true;
        for (i32 i = 0; i < editor.multiselected_entities.count; i++){
            undo_action.changed_entities.add(editor.multiselected_entities.get(i));
        }
        add_undo_action(undo_action);
    }
    
    editor.multiselected_entities.clear();
}

b32 clicked_on_entity_edge(f32 rotation, Vector2 edge_center, b32 is_horizontal, f32 orthogonal_size){
    Array<Vector2, MAX_VERTICES> edge_vertices = Array<Vector2, MAX_VERTICES>();
    add_rect_vertices(&edge_vertices, {0.5f, 0.5f});
    f32 selection_radius = 2.5f;
    for (i32 i = 0; i < edge_vertices.count; i++){
        if (is_horizontal){
            edge_vertices.get_ptr(i)->x *= selection_radius;
            edge_vertices.get_ptr(i)->y *= orthogonal_size;
        } else{
            edge_vertices.get_ptr(i)->y *= selection_radius;
            edge_vertices.get_ptr(i)->x *= orthogonal_size;
        }
        rotate_around_point(edge_vertices.get_ptr(i), Vector2_zero, rotation);
    }
    
    b32 clicked_edge = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) 
                        && check_collision(edge_center, mouse_entity.position, edge_vertices, mouse_entity.vertices, {0.5f, 0.5f}, mouse_entity.pivot).collided;
    return clicked_edge;
}

void try_move_entity_edges(Entity *e){
    if (editor.moving_entity_edge_type == NONE && !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        return;
    }
    
    if (editor.moving_entity_edge_type != NONE && editor.moving_entity_edge_id != e->id){
        return;
    }
    
    // These settings for left edge just for example. We will set it again on switch anyway.
    Vector2 scale_side = Vector2_right * -1;
    Vector2 position_change_direction = e->right * -1;
    Vector2 edge_center = e->position + position_change_direction * e->scale.x * e->pivot.x;
    f32 scale_modifier = 1;
    
    if (editor.moving_entity_edge_type == NONE){
        if (0){
        } else if (clicked_on_entity_edge(e->rotation, e->position + e->right * e->scale.x * e->pivot.x, true, e->scale.y)){
            editor.moving_entity_edge_type = RIGHT_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position - e->right * e->scale.x * e->pivot.x, true, e->scale.y)){
            editor.moving_entity_edge_type = LEFT_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position + e->up * e->scale.y * e->pivot.y, false, e->scale.x)){
            editor.moving_entity_edge_type = TOP_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position - e->up * e->scale.y * e->pivot.y, false, e->scale.x)){
            editor.moving_entity_edge_type = BOTTOM_EDGE;
        } else{
            return;
        }
        
        if (editor.moving_entity_edge_type != NONE){
            // So we really clicked on edge this frame and we should remember vertices because I am just writed retarded 
            // undo system long ago and this shit needs full rewrite.
            undo_remember_vertices_start(e);
            editor.moving_edge_start_entity_position = e->position;
            editor.moving_edge_start_entity_scale = e->scale;
        }
    }
    
    switch (editor.moving_entity_edge_type){
        case LEFT_EDGE:{
            edge_center = e->position + e->right * -1 * e->scale.x * e->pivot.x;
            position_change_direction = e->right * -1;
            scale_side = Vector2_right * -1;
            scale_modifier = -1;
        } break;
        case RIGHT_EDGE:{
            edge_center = e->position + e->right * e->scale.x * e->pivot.x;
            position_change_direction = e->right;
            scale_side = Vector2_right;
            scale_modifier = 1;
        } break;
        case TOP_EDGE:{
            edge_center = e->position + e->up * e->scale.y * e->pivot.y;
            position_change_direction = e->up;
            scale_side = Vector2_up;
            scale_modifier = 1;
        } break;
        case BOTTOM_EDGE:{
            edge_center = e->position + e->up * -1 * e->scale.y * e->pivot.y;
            position_change_direction = e->up * -1;
            scale_side = Vector2_up * -1;
            scale_modifier = -1;
        } break;
    }
    
    // If we currently not moving edge we will not end up here because of "return" in switch.
    
    editor.moving_entity_edge_id = e->id;
    
    Vector2 to_mouse = input.mouse_position - edge_center;
    f32 edge_mouse_dot = dot(to_mouse, position_change_direction);
    
    f32 scale_amount = 5;
    while (abs(edge_mouse_dot) >= scale_amount * 0.75f){
    
        Vector2 next_scale = round_to_factor(e->scale + scale_side * scale_modifier * normalized(edge_mouse_dot) * scale_amount, scale_amount);
        
        if (next_scale.x >= scale_amount && next_scale.y >= scale_amount){
            Vector2 position_change = position_change_direction * normalized(edge_mouse_dot) * scale_amount * 0.5f;
            change_scale(e, next_scale);
            e->position += position_change;
        }
        
        edge_mouse_dot -= scale_amount * normalized(edge_mouse_dot);
    }
}

// This can be called not only when game_state is EDITOR, but even when we're in pause for example.
void update_editor(){
    if (IsKeyPressed(KEY_ESCAPE)){
        EnableCursor();
        ShowCursor();
        SetMousePosition(input.screen_mouse_position.x, input.screen_mouse_position.y);
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        HideCursor();
        DisableCursor();
    }

    // levels switching context stitch
    if (game_state == EDITOR){
        if (IsKeyPressed(KEY_ONE) && IsKeyDown(KEY_LEFT_CONTROL) && !console.is_open){
            current_editor_level_context_index += 1;    
            current_editor_level_context_index %= MAX_LOADED_LEVELS;    
            
            Level_Context *next_context = &loaded_levels_contexts[current_editor_level_context_index];
            i32 cycled = 0;
            while (cycled <= MAX_LOADED_LEVELS && !(*next_context->level_name)){
                cycled += 1;
                current_editor_level_context_index += 1;    
                current_editor_level_context_index %= MAX_LOADED_LEVELS;    
                next_context = &loaded_levels_contexts[current_editor_level_context_index];
            }
            
            if (*next_context->level_name){
                editor_level_context = next_context;
                switch_current_level_context(editor_level_context, true);
            }
            
            // setup_context_cam(current_level_context);
        }
    
        // We need grid to be at camera center because levels could be quite big and even our mouse collision detection does not work
        // without grid at that place. 
        // BUT i've recently (08.03.2025 currently) made that origin is on player spawn point in editor. Don't remember why. 
        // There's could be other scary reason.
        // Vector2 grid_target_pos = current_level_context->player_spawn_point;
        Vector2 grid_target_pos = current_level_context->cam.position;
        session_context.collision_grid.origin = {(f32)((i32)grid_target_pos.x - ((i32)grid_target_pos.x % (i32)session_context.collision_grid.cell_size.x)), (f32)((i32)grid_target_pos.y - ((i32)grid_target_pos.y % (i32)session_context.collision_grid.cell_size.y))};
    }
    Undo_Action undo_action;
    b32 something_in_undo = false;
    b32 can_control_with_single_button = !focus_input_field.in_focus && !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT);
    b32 can_select = !clicked_ui;
    
    f32 dt = core.time.real_dt;
    
    editor.in_editor_time += dt;

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_L)){
        editor.update_cam_view_position = !editor.update_cam_view_position;
    }
    
    b32 moving_editor_cam = false;
    
    f32 zoom = current_level_context->cam.target_zoom;

    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        moving_editor_cam = true;
    }
    
    // b32 need_move_vertices = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select;
    // b32 need_snap_vertex = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_V);
    
    i32 selected_vertex_index;
    
    mouse_entity.position = input.mouse_position;
    
    i32 cursor_entities_count = 0;
    
    Entity *moving_vertex_entity_candidate = NULL;
    i32 moving_vertex_candidate = -1;
    
    // Spawn shortcuts
    if (IsKeyDown(KEY_LEFT_ALT)){
        if (IsKeyPressed(KEY_ONE)){
            editor_spawn_entity("block_base", input.mouse_position);
        }
        if (IsKeyPressed(KEY_TWO)){
            editor_spawn_entity("dummy_entity", input.mouse_position);
        }
        if (IsKeyPressed(KEY_THREE)){
            editor_spawn_entity("ammo_pack", input.mouse_position);
        }
        if (IsKeyPressed(KEY_FOUR)){
            editor_spawn_entity("enemy_bird", input.mouse_position);
        }
        if (IsKeyPressed(KEY_FIVE)){
            assign_selected_entity(editor_spawn_entity("note", input.mouse_position));
            make_next_input_field_in_focus("note");
        }
    }
    
    //editor entities loop
    for (i32 i = 0; i < current_level_context->entities.max_count; i++){        
        Entity *e = current_level_context->entities.get_ptr(i);
        
        if (!e->enabled){
            continue;
        }
        
        if ((check_entities_collision(&mouse_entity, e)).collided){
            cursor_entities_count++;
        }
        
        // //editor vertices
        // for (i32 v = 0; v < e->vertices.count && need_move_vertices; v++){
        //     Vector2 *vertex = e->vertices.get_ptr(v);
            
        //     Vector2 vertex_global = global(e, *vertex);
            
        //     if (need_move_vertices && (!moving_vertex_entity_candidate || (editor.selected_entity && e->id == editor.selected_entity->id))){
        //         if (is_vertex_on_mouse(vertex_global)){
        //             moving_vertex_entity_candidate = e;
        //             moving_vertex_candidate = v;
        //         }
        //     }
        // }
        
        b32 maybe_want_to_move_edges = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if (maybe_want_to_move_edges && !editor.is_scaling_entity){
            try_move_entity_edges(e);
        } else{
            if (editor.moving_entity_edge_type != NONE && e->id == editor.moving_entity_edge_id){
                if (e->scale != editor.moving_edge_start_entity_scale){
                    Vector2 position_change = e->position - editor.moving_edge_start_entity_position;
                    Vector2 scale_change    = e->scale - editor.moving_edge_start_entity_scale;
                    Undo_Action undo_action = {};
                    undo_action.position_change = position_change;
                    undo_action.moved_entity_points = false;
                    undo_action.scale_change = scale_change;
                    
                    // We remembered vertices when just clicked on edge in try_move_entity_edges.
                    undo_apply_vertices_change(e, &undo_action);
                    
                    add_undo_action(undo_action);
                }
                editor.moving_entity_edge_type = NONE;
                editor.moving_entity_edge_id = -1;
            }
        }
        
        //editor move sequence points        
        // We don't want to move points if selected entity already is move sequence or if selected is trigger with cam rails.
        b32 cannot_move_points = editor.selected_entity && ((editor.selected_entity->flags & MOVE_SEQUENCE || (editor.selected_entity->flags & TRIGGER && editor.selected_entity->trigger.cam_rails_points.count > 0)) && editor.selected_entity->id != e->id);
        for (i32 p = 0; e->flags & MOVE_SEQUENCE && IsKeyDown(KEY_LEFT_ALT) && p < e->move_sequence.points.count && !cannot_move_points; p++){
            Vector2 *point = e->move_sequence.points.get_ptr(p);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && check_col_circles({input.mouse_position, 1}, {*point, 0.5f / current_level_context->cam.cam2D.zoom})){
                *point = input.mouse_position;
            }
        }
        
        //editor move cam rails points        
        for (i32 p = 0; e->flags & TRIGGER && (e->trigger.start_cam_rails_horizontal || e->trigger.start_cam_rails_vertical) && IsKeyDown(KEY_LEFT_ALT) && p < e->trigger.cam_rails_points.count && !cannot_move_points; p++){
            Vector2 *point = e->trigger.cam_rails_points.get_ptr(p);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && check_col_circles({input.mouse_position, 1}, {*point, 0.5f / current_level_context->cam.cam2D.zoom})){
                *point = input.mouse_position;
            }
        }
    }
    
    //assign move vertex
    // if (need_move_vertices && moving_vertex_entity_candidate){
    //     assign_moving_vertex_entity (moving_vertex_entity_candidate, moving_vertex_candidate);
    //     undo_remember_vertices_start(moving_vertex_entity_candidate);
    // }
    
    // if (need_snap_vertex && editor.moving_vertex && editor.moving_vertex_entity){
    //     snap_vertex_to_closest(editor.moving_vertex_entity, editor.moving_vertex, editor.moving_vertex_index);
        
    //     editor.moving_vertex = NULL;
    //     editor.moving_vertex_entity = NULL;
    // }
    
    if (editor.selected_entity && IsKeyDown(KEY_LEFT_ALT)){
        i32 vertex_snap_index = -1;
        if (IsKeyPressed(KEY_T))   vertex_snap_index = 0;
        if (IsKeyPressed(KEY_Y))   vertex_snap_index = 1;
        if (IsKeyPressed(KEY_F)) vertex_snap_index = 2;
        if (IsKeyPressed(KEY_G))  vertex_snap_index = 3;
        
        if (vertex_snap_index != -1 && vertex_snap_index < editor.selected_entity->vertices.count){
            Vector2 *vertex = editor.selected_entity->vertices.get_ptr(vertex_snap_index);
            snap_vertex_to_closest(editor.selected_entity, vertex, vertex_snap_index);
        }
    }
    
    //This means we clicked all entities in one mouse position, so we want to cycle
    if (cursor_entities_count <= editor.place_cursor_entities.count){
        editor.place_cursor_entities.clear();
    }
    
    editor.cursor_entity = get_cursor_entity();
    
    if (editor.cursor_entity){
        editor.cursor_entity->color_changer.frame_changing = true;    
    }
    
    b32 need_start_dragging = false;
    
    // mouse select editor
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select){
        if (editor.cursor_entity){ //select entity
            b32 is_same_selected_entity = editor.selected_entity != NULL && editor.selected_entity->id == editor.cursor_entity->id;
            need_start_dragging = is_same_selected_entity;
            if (!is_same_selected_entity){
                // multiselect exclude multiselect remove
                b32 removed = false;
                if (IsKeyDown(KEY_LEFT_CONTROL)){
                    i32 contains_index = get_index_of_id(&editor.multiselected_entities, editor.cursor_entity->id);
                    if (contains_index != -1){
                        // editor.multiselected_entities.remove(contains_index);
                        remove_from_multiselection(editor.cursor_entity->id, true);
                        removed = true;
                    } else{
                        add_to_multiselection(editor.cursor_entity->id, true);
                    }
                }
                
                if (!removed){
                    if (!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_SHIFT)){
                        clear_multiselected_entities(true);
                    }
                    assign_selected_entity(editor.cursor_entity);
                    editor.place_cursor_entities.add(editor.selected_entity);
                    
                    editor.selected_this_click = true;
                }
            }
        }
    } 
    
    // multiselect
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        editor.excluding_multiselection = true;     
        editor.selection_multiselected_entities.clear();
        editor.multiselect_start_point = input.mouse_position;
    } else if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        editor.selection_multiselected_entities.clear();
        
        if (!editor.multiselecting){
            clear_multiselected_entities(true);
        }
        editor.multiselecting = false;
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && editor.excluding_multiselection){
        editor.excluding_multiselection = false;
        
        remove_from_multiselection(&editor.selection_multiselected_entities, true);
        
        editor.selection_multiselected_entities.clear();
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        editor.multiselecting = true;       
        editor.multiselect_start_point = input.mouse_position;
        editor.selection_multiselected_entities.clear();
    }
    
    if ((editor.multiselecting || editor.excluding_multiselection) && sqr_magnitude(input.mouse_position - editor.multiselect_start_point) > 1){
        editor.selection_multiselected_entities.clear();
    
        Vector2 pivot = Vector2_zero;    
        if (input.mouse_position.x >= editor.multiselect_start_point.x) pivot.x = 0;
        else pivot.x = 1;
        if (input.mouse_position.y >= editor.multiselect_start_point.y) pivot.y = 1;
        else pivot.y = 0;
        
        Vector2 scale = {abs(input.mouse_position.x - editor.multiselect_start_point.x), abs(input.mouse_position.y - editor.multiselect_start_point.y)};
        fill_collisions_rect(editor.multiselect_start_point, scale, pivot, &collisions_buffer, 0);
        
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Entity *other = collisions_buffer.get_ptr(i)->other_entity;
            if (get_index_of_entity_id(editor.selection_multiselected_entities.data, editor.selection_multiselected_entities.count, other->id) != -1){
                continue;
            }
            
            editor.selection_multiselected_entities.add(other->id);
            
            if (!editor.excluding_multiselection){
                other->color_changer.frame_changing = true;
                make_rect_lines(other->position + other->bounds.offset, other->bounds.size, other->pivot, 2.0f / current_level_context->cam.cam2D.zoom, BLUE); 
            }
        }
        
        Color color = editor.excluding_multiselection ? RED : BLUE;
        make_rect_lines(editor.multiselect_start_point, scale, pivot, 2.0f / (current_level_context->cam.cam2D.zoom), color);
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && editor.multiselecting){
        editor.multiselecting = false;
        
        add_to_multiselection(&editor.selection_multiselected_entities, true);
    }
    
    // update multiselected
    if (editor.multiselected_entities.count > 0){
        local_persist b32 was_moving_multiselected = false;
        b32 should_move_multiselected = IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL);
        
        if (!was_moving_multiselected && should_move_multiselected){
            editor.multiselect_moving_displacement = Vector2_zero;   
            editor.dragging_start = input.mouse_position;
        }
        if (should_move_multiselected){
            // This thing exist only for undo.
            editor.multiselect_moving_displacement += get_editor_mouse_move();
        }
        if (was_moving_multiselected && !should_move_multiselected){
            undo_add_multiselect_position_change(editor.multiselect_moving_displacement);
        }
        
        was_moving_multiselected = should_move_multiselected;
    
        Vector2 most_right_entity_position;
        Vector2 most_left_entity_position;
        Vector2 most_top_entity_position;
        Vector2 most_bottom_entity_position;
        
        // Detecting required movement for multiselected;
        Vector2 moving_displacement = Vector2_zero;
        if (IsKeyDown(KEY_LEFT_ALT)){
            moving_displacement = get_editor_mouse_move();
        } else{
            f32 cell_size = 5;
            
            if (sqr_magnitude(input.mouse_position - editor.dragging_start) >= (cell_size * 0.5f * cell_size * 0.5f)){
                // While moving multiselected entities we canot directly set position like we do in editor_mouse_move_entity,
                // so here we calculate current quantized displacement from last moving.
                Vector2 cell_mouse_position = round_to_factor(input.mouse_position, cell_size);
                moving_displacement = cell_mouse_position - editor.dragging_start;
                editor.dragging_start += moving_displacement;
            }
        }


        for (i32 entity_index = 0; entity_index < editor.multiselected_entities.count; entity_index++){
            Entity *entity = get_entity_by_id(editor.multiselected_entities.get(entity_index));
            if (!entity){
                continue;
            }
            
            b32 excluding_this_entity = editor.excluding_multiselection && editor.selection_multiselected_entities.contains(entity->id);
            if (excluding_this_entity){
                continue;
            }
            
            if (should_move_multiselected){
                // We want to move entity points on multiselect moving.
                b32 was_moving_entity_points = editor.move_entity_points;
                if (editor.multiselected_entities.count > 1){
                    editor.move_entity_points = true;
                }
                assign_selected_entity(NULL);
                
                
                // editor_mouse_move_mulentity(entity);
            
                if (moving_displacement != Vector2_zero){
                    entity->position += moving_displacement;
                    
                    if (editor.move_entity_points){
                        editor_move_entity_points(entity, moving_displacement);
                    }
                }
                
                editor.move_entity_points = was_moving_entity_points;
            }
            
            entity->color_changer.frame_changing = true;
            make_rect_lines(entity->position + entity->bounds.offset, entity->bounds.size, entity->pivot, 2.0f / current_level_context->cam.cam2D.zoom, BLUE); 
            
            if (entity_index == 0){
                most_right_entity_position = entity->position;
                most_left_entity_position = entity->position;
                most_top_entity_position = entity->position;
                most_bottom_entity_position = entity->position;
            } else{
                if (entity->position.x > most_right_entity_position.x) most_right_entity_position = entity->position;
                if (entity->position.x < most_left_entity_position.x) most_left_entity_position = entity->position;
                if (entity->position.y > most_top_entity_position.y) most_top_entity_position = entity->position;
                if (entity->position.y < most_bottom_entity_position.y) most_bottom_entity_position = entity->position;
            }
        }
        
        editor.multiselected_entities_center = {most_left_entity_position.x + (most_right_entity_position.x - most_left_entity_position.x) * 0.5f, most_bottom_entity_position.y + (most_top_entity_position.y - most_bottom_entity_position.y) * 0.5f};
        
        if (IsKeyPressed(KEY_X)){
            editor_delete_multiselected_entities();
        }
    }
    
    if (editor.dragging_entity == NULL && !editor.selected_this_click && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL) && editor.selected_entity != NULL && need_start_dragging && can_select){ // assign dragging entity
        if (editor.cursor_entity != NULL){
            if (editor.moving_vertex == NULL && editor.selected_entity->id == editor.cursor_entity->id && editor.moving_entity_edge_type == NONE){
                editor.dragging_entity = editor.selected_entity;
                editor.dragging_entity_id = editor.selected_entity->id;
                editor.dragging_start = editor.dragging_entity->position;
                editor.dragging_start_mouse_offset = input.mouse_position - editor.dragging_start;
            }
        }
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && can_select){ //stop dragging entity
        if (editor.selected_entity && !editor.selected_this_click && editor.cursor_entity){
            if (editor.dragging_time <= 0.1f && editor.cursor_entity->id == editor.selected_entity->id){
                assign_selected_entity(NULL);
            }
        }
        
        editor.dragging_time = 0;
        editor.selected_this_click = false;
        
        if (editor.dragging_entity){
            undo_add_position(editor.dragging_entity, editor.dragging_entity->position - editor.dragging_start);
        }
        
        editor.dragging_entity = NULL;
        
        if (editor.moving_vertex_entity){
            something_in_undo = true;
            undo_action.entity_id = editor.moving_vertex_entity->id;
            undo_apply_vertices_change(editor.moving_vertex_entity, &undo_action);
        }
        
        editor.moving_vertex = NULL;
        editor.moving_vertex_entity = NULL;
    }
    
    //entity tap moving
    if (editor.selected_entity){
        f32 arrows_move_amount = 0.1f;
        Vector2 move = input.tap_direction * 0.1f;
        if (move.x != 0 || move.y != 0){
            editor.selected_entity->position += move;
            undo_action.position_change = move;
            undo_action.entity_id = editor.selected_entity->id;
            something_in_undo = true;
        }
    }
    
    if (editor.moving_vertex != NULL){
        move_vertex(editor.moving_vertex_entity, input.mouse_position, editor.moving_vertex_index);
    }
    
    //editor copy
    if ((editor.selected_entity || editor.multiselected_entities.count > 0) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)){
        Level_Context *original_level_context = current_level_context;
        switch_current_level_context(&copied_entities_level_context);
        for (i32 i = 0; i < editor.copied_entities.count; i++){
            free_entity(editor.copied_entities.get_ptr(i));
        }
        editor.copied_entities.clear();
        if (editor.multiselected_entities.count > 0){
            for (i32 i = 0; i < editor.multiselected_entities.count; i++){
                Entity *entity_to_copy = get_entity_by_id(editor.multiselected_entities.get(i), original_level_context);   
                // We keep id here so later we could verify different connected entities by ids. 
                editor.copied_entities.add(Entity(entity_to_copy, true, original_level_context));
            }
            editor.copied_entities_center = editor.multiselected_entities_center;
        } else{
            Entity *entity_to_copy = get_entity_by_id(editor.selected_entity->id, original_level_context);   
            editor.copied_entities.add(Entity(entity_to_copy, true, original_level_context));
            // copy_entity(&editor.copied_entity, editor.selected_entity);
            editor.copied_entities_center = entity_to_copy->position;
        }
        
        switch_current_level_context(original_level_context);
        editor.is_copied = true;
    }
    
    // editor paste
    if (editor.is_copied && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)){
        if (editor.copied_entities.count > 0){
            Vector2 paste_position = {round_to_factor(input.mouse_position.x, 5), round_to_factor(input.mouse_position.y, 5)};
            
            assign_selected_entity(NULL);
            
            Undo_Action undo_action = {};
            undo_action.entity_was_spawned = true;
        
            local_persist Dynamic_Array<i32> spawned_entities = Dynamic_Array<i32>(128);
            spawned_entities.clear();
            clear_multiselected_entities(true);
            for (i32 i = 0; i < editor.copied_entities.count; i++){
                Entity *to_spawn = editor.copied_entities.get_ptr(i);
                Entity *spawned = add_entity(to_spawn, false, &copied_entities_level_context);
                spawned_entities.add(spawned->id);
                spawned->position += paste_position - editor.copied_entities_center;
                editor_move_entity_points(spawned, paste_position - editor.copied_entities_center);
                
                if (editor.copied_entities.count == 1){
                    assign_selected_entity(spawned);
                } else{
                    editor.multiselected_entities.add(spawned->id);
                }
                
                undo_action.changed_entities.add(spawned->id);
            }
            assert(spawned_entities.count == editor.copied_entities.count);
            
            add_undo_action(undo_action);
            
            // Right now we want to verify connected entities only to triggers.
            // Again - that's because when we copy trigger and in multiselected was his connected guys - they will have different
            // ids. We know original ids (in copied_entiies we keep ids because they in different level context)
            // and we will know which ids they have now, because we track spawned entities and they 
            // have the same indexes as copied entities. If that was a bad explanation I've explained it also in do-list 
            // in 'Loading multiple levels' task.
            for (i32 i = 0; i < spawned_entities.count; i++){
                Entity *spawned =  get_entity_by_id(spawned_entities.get(i));
                if (spawned->flags & TRIGGER){
                    // We have original trigger connected and tracking in copied_entities.
                    spawned->trigger.connected.clear();                                      
                    spawned->trigger.tracking.clear();
                    // @CLEANUP: Will have to change here when we'll remove all of types from entity. Nothing scary.
                    Entity *copied_trigger_entity = editor.copied_entities.get_ptr(i);
                    // Here we want to go through all copied entities and find entities with ids from copied trigger.
                    // Then we want to add entity from spawned with the same index to connected and tracked of new trigger.
                    // That's confusing because it's just is. Not sure if it's even possible to make simpler.
                    // But on the bright side - that's not so much code.
                    //
                    // UPDATE after ~3 months - completely understandable. Making same thing for KILL_SWITCH now.
                    for (i32 x = 0; x < spawned_entities.count; x++){
                        Entity *other_copied_entity = editor.copied_entities.get_ptr(x);
                        if (copied_trigger_entity->trigger.connected.contains(other_copied_entity->id)){
                            spawned->trigger.connected.add(spawned_entities.get(x));
                        }
                        if (copied_trigger_entity->trigger.tracking.contains(other_copied_entity->id)){
                            spawned->trigger.tracking.add(spawned_entities.get(x));
                        }
                    }
                }
                
                // This thing is trying to catch that moment where we paste entity that was connected to some trigger or kill switch
                // and we want to assign pasted entity to this trigger or kill switch.
                // (Actually entity itself don't know it is connected to something, so we're going through all triggers/killswitches
                // and look for original copied entity id - that means our original was connected and we connecting newly created one).
                //
                // We check for spawned entities count because that's actually could be frustrating
                // when we copy big chunks of level    
                // without trigger and trigger connecting to new level parts that could be not even relevant to him.
                if (spawned_entities.count < 10){
                    i32 originally_copied_id = editor.copied_entities.get_ptr(i)->id;
                    i32 spawned_id = spawned->id;
                    ForEntities(entity, TRIGGER | KILL_SWITCH){
                        // If this trigger or kill switch happened to be in copied - we do not assign anything new to him, 
                        // because he will want to have his own connected.
                        // That's somewhat hard to understand for some reason, but that's just works and *prevents* situations
                        // when in pasted [trigger, enemy] enemy connects to old trigger aswell.
                        if (entity_array_contains_id(editor.copied_entities.data, editor.copied_entities.count, entity->id)){
                            continue;
                        }
                        if (entity->flags & TRIGGER && entity->trigger.connected.contains(originally_copied_id)){
                            entity->trigger.connected.add(spawned_id);
                        }
                        
                        if (entity->flags & KILL_SWITCH && entity->enemy.kill_switch.connected.contains(originally_copied_id)){
                            entity->enemy.kill_switch.connected.add(spawned_id);
                        }
                    }
                }
            }
        } else{
        }
    }
    
    //editor ruler
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && IsKeyDown(KEY_LEFT_ALT)){
        editor.ruler_active = true;
        editor.ruler_start_position = input.mouse_position;
    } else if (editor.ruler_active && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
        editor.ruler_active = false;
    }
    
    //editor Delete entity
    if (can_control_with_single_button && IsKeyPressed(KEY_X) && editor.selected_entity){
        editor_delete_entity(editor.selected_entity, true);
    }
    
    if (editor.dragging_entity != NULL){
        editor.dragging_time += dt;
    }
    
    if (editor.dragging_entity != NULL && !moving_editor_cam){
        editor_mouse_move_entity(editor.dragging_entity);
    }
    
    //editor Entity to mouse or go to entity
    if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.dragging_entity){
        editor.dragging_entity->position = input.mouse_position;
    } else if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.selected_entity){
        current_level_context->cam.position = editor.selected_entity->position;
    }
    
    //editor free entity rotation
    if (editor.selected_entity && IsKeyDown(KEY_LEFT_ALT)){
        f32 rotation = 0;
        f32 speed = 50;
        if (!editor.is_rotating_entity && (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_Q))){
            editor.rotating_start = editor.selected_entity->rotation;
            undo_remember_vertices_start(editor.selected_entity);
            editor.is_rotating_entity = true;
        } 
        
        if (IsKeyDown(KEY_E)){
            rotation = dt * speed;
        } else if (IsKeyDown(KEY_Q)){
            rotation = -dt * speed;
        }
        
        if (rotation != 0 && editor.is_rotating_entity){
            rotate(editor.selected_entity, rotation);
        }
        
        if (editor.is_rotating_entity && (IsKeyUp(KEY_E) && IsKeyUp(KEY_Q))){
            undo_add_rotation(editor.selected_entity, editor.selected_entity->rotation - editor.rotating_start);
            editor.is_rotating_entity = false;
        } 
    } else if (editor.selected_entity && can_control_with_single_button){
        // editor snap entity rotation.
        local_persist f32 holding_time = 0;
        f32 to_rotate = 0;
        if (IsKeyPressed(KEY_E)){
            to_rotate = 15;
        }
        if (IsKeyPressed(KEY_Q)){
            to_rotate = -15;
        }
        
        if (to_rotate != 0){
            f32 next_rotation = round_to_factor(editor.selected_entity->rotation + to_rotate, 15);
            to_rotate = next_rotation - editor.selected_entity->rotation;
            undo_remember_vertices_start(editor.selected_entity);
            rotate(editor.selected_entity, to_rotate);
            undo_add_rotation(editor.selected_entity, (to_rotate));
        }
        
        if (IsKeyReleased(KEY_E) || IsKeyReleased(KEY_Q)){
            holding_time = 0;
        }
        
        if (IsKeyDown(KEY_E) || IsKeyDown(KEY_Q)){
            holding_time += dt;
            if (holding_time >= 0.2f){
                f32 direction = IsKeyDown(KEY_E) ? 15 : -15;
                undo_remember_vertices_start(editor.selected_entity);
                rotate(editor.selected_entity, direction);
                undo_add_rotation(editor.selected_entity, (direction));
                holding_time = 0;
            }
        }
    }
    
    //editor free entity scaling
    if (editor.selected_entity && IsKeyDown(KEY_LEFT_ALT) && editor.moving_entity_edge_type == NONE){
        Vector2 scaling = {};
        f32 speed = 80;
        
        if (!editor.is_scaling_entity && (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_A))){
            editor.scaling_start = editor.selected_entity->scale;
            undo_remember_vertices_start(editor.selected_entity);
            editor.is_scaling_entity = true;
        }
        if      (IsKeyDown(KEY_W)) scaling.y += speed * dt;
        else if (IsKeyDown(KEY_S)) scaling.y -= speed * dt;
        if      (IsKeyDown(KEY_D)) scaling.x += speed * dt;
        else if (IsKeyDown(KEY_A)) scaling.x -= speed * dt;

        if (scaling != Vector2_zero && editor.is_scaling_entity){
            add_scale(editor.selected_entity, scaling);
        }
        
        if (editor.is_scaling_entity && (IsKeyUp(KEY_W) && IsKeyUp(KEY_S) && IsKeyUp(KEY_A) && IsKeyUp(KEY_D))){
            Vector2 scale_change = editor.selected_entity->scale - editor.scaling_start;
            
            undo_add_scaling(editor.selected_entity, scale_change);
            editor.is_scaling_entity = false;
        } 
    } else if (editor.selected_entity && can_control_with_single_button && editor.moving_entity_edge_type == NONE){
        local_persist f32 holding_time = 0;
        Vector2 scaling = Vector2_zero;
        f32 scale_amount = 5;
        
        if      (IsKeyPressed(KEY_W)) scaling.y += scale_amount;
        else if (IsKeyPressed(KEY_S)) scaling.y -= scale_amount;
        if      (IsKeyPressed(KEY_D)) scaling.x += scale_amount;
        else if (IsKeyPressed(KEY_A)) scaling.x -= scale_amount;


        if (scaling != Vector2_zero){
            Vector2 next_scale = editor.selected_entity->scale + scaling;
            next_scale = {round_to_factor(next_scale.x, scale_amount), round_to_factor(next_scale.y, scale_amount)};
            scaling = next_scale - editor.selected_entity->scale;
        
            undo_remember_vertices_start(editor.selected_entity);
            add_scale(editor.selected_entity, scaling);
            undo_add_scaling(editor.selected_entity, scaling);
        }
        
        if (IsKeyReleased(KEY_W) || IsKeyReleased(KEY_S) || IsKeyReleased(KEY_A) || IsKeyReleased(KEY_D)){
            holding_time = 0;
        }
        
        local_persist i32 hold_scale_times = 0;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D)){
            f32 delay = 0.2f;
            if (hold_scale_times >= 1){
                delay = 0.05f;
            }
            holding_time += dt;
            if (holding_time >= delay){
                hold_scale_times += 1;
                if (hold_scale_times > 10){
                    scale_amount = 20;
                }
                if      (IsKeyDown(KEY_W)) scaling.y += scale_amount;
                else if (IsKeyDown(KEY_S)) scaling.y -= scale_amount;
                if      (IsKeyDown(KEY_D)) scaling.x += scale_amount;
                else if (IsKeyDown(KEY_A)) scaling.x -= scale_amount;
                
                undo_remember_vertices_start(editor.selected_entity);
                add_scale(editor.selected_entity, scaling);
                undo_add_scaling(editor.selected_entity, scaling);
                holding_time = 0;
            }
        } else{
            hold_scale_times = 0;
        }
    }
    
    //editor components management
    if (editor.selected_entity){
        Entity *selected = editor.selected_entity;
        if (selected->flags & TRIGGER){
            b32 wanna_assign = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A);
            b32 wanna_assign_tracking_enemy = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Q);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D);
            b32 wanna_change_locked_camera_position = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_R);
            
            b32 wanna_add_cam_rails_point    = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N);
            b32 wanna_remove_cam_rails_point = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_M);
            b32 wanna_clear_cam_rails_points = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L);
            //trigger assign or remove
            if (wanna_assign || wanna_remove){
                fill_collisions(&mouse_entity, &collisions_buffer, DOOR | ENEMY | SPIKES | GROUND | PLATFORM | MOVE_SEQUENCE | TRIGGER | DUMMY | TEXTURE);
                
                for (i32 i = 0; i < collisions_buffer.count; i++){
                    Collision col = collisions_buffer.get(i);
                    
                    if (wanna_assign && !wanna_remove && !selected->trigger.connected.contains(col.other_entity->id)){
                        selected->trigger.connected.add(col.other_entity->id);
                        break;
                    } else if (wanna_remove && !wanna_assign){
                        if (selected->trigger.connected.contains(col.other_entity->id)){
                            selected->trigger.connected.remove(selected->trigger.connected.find(col.other_entity->id));
                        } else if (selected->trigger.tracking.contains(col.other_entity->id)){
                            selected->trigger.tracking.remove(selected->trigger.tracking.find(col.other_entity->id));
                        }
                        break;
                    }
                }
            }
            
            if (wanna_change_locked_camera_position){
                selected->trigger.locked_camera_position = input.mouse_position;
            }
            
            if (wanna_assign_tracking_enemy){
                fill_collisions(&mouse_entity, &collisions_buffer, ENEMY | CENTIPEDE);
                for (i32 i = 0; i < collisions_buffer.count; i++){
                    Collision col = collisions_buffer.get(i);
                    
                    if (!selected->trigger.tracking.contains(col.other_entity->id)){
                        selected->trigger.tracking.add(col.other_entity->id);
                    }
                }
            }
            
            //trigger clear
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)){
                selected->trigger.connected.clear();
            }
            
            if (wanna_remove_cam_rails_point){
                for (i32 i = 0; i < selected->trigger.cam_rails_points.count; i++){
                    Vector2 point = selected->trigger.cam_rails_points.get(i);   
                    
                    if (check_col_circles({input.mouse_position, 1}, {point, 0.5f  * (0.4f / current_level_context->cam.cam2D.zoom)})){       
                        selected->trigger.cam_rails_points.remove(i);
                        break;
                    }
                }
            }
            if (wanna_add_cam_rails_point){
                selected->trigger.cam_rails_points.add(input.mouse_position);
            }
            if (wanna_clear_cam_rails_points){
                selected->trigger.cam_rails_points.clear();
            }
        }
        
        if (selected->flags & KILL_SWITCH){
            b32 wanna_assign = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D);
            
            Kill_Switch *kill_switch = &selected->enemy.kill_switch;
            //kill switch assign or remove
            if (wanna_assign || wanna_remove){
                fill_collisions(&mouse_entity, &collisions_buffer, ENEMY);
                for (i32 i = 0; i < collisions_buffer.count; i++){
                    Collision col = collisions_buffer.get(i);
                    
                    if (wanna_assign && !wanna_remove && !kill_switch->connected.contains(col.other_entity->id)){
                        kill_switch->connected.add(col.other_entity->id);
                        break;
                    } else if (wanna_remove && !wanna_assign){
                        if (kill_switch->connected.contains(col.other_entity->id)){
                            kill_switch->connected.remove(kill_switch->connected.find(col.other_entity->id));
                        }
                        break;
                    }
                }
            }
            
            //kill switch clear
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)){
                kill_switch->connected.clear();
            }
        }
        
        //enemy components
        if (selected->flags & ENEMY){
            b32 wanna_increase_sword_kill_speed  = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P);
            b32 wanna_decrease_sword_kill_speed  = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O);
            
            b32 wanna_rotate_shoot_blocker_direction   = IsKeyDown(KEY_LEFT_CONTROL) && (IsKeyDown(KEY_F) || IsKeyDown(KEY_G));
            
            if (wanna_increase_sword_kill_speed || wanna_decrease_sword_kill_speed){   
                f32 speed_change = wanna_increase_sword_kill_speed ? 0.5f : -0.5f;
                selected->enemy.sword_kill_speed_modifier += speed_change;
            }
            
            if (wanna_rotate_shoot_blocker_direction && selected->flags & SHOOT_BLOCKER){
                f32 speed = IsKeyDown(KEY_F) ? -40 : 40;
                selected->enemy.shoot_blocker_direction = get_rotated_vector(selected->enemy.shoot_blocker_direction, speed * core.time.real_dt);
            }
        }
        
        // propeller change
        if (selected->flags & PROPELLER){
            b32 wanna_increase_power = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E);
            b32 wanna_decrease_power = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Q);
            
            if (wanna_increase_power || wanna_decrease_power){
                f32 power_change = wanna_increase_power ? 100 : -100;
                selected->propeller.power += power_change;
            }
        }
        
        // door settings
        if (selected->flags & DOOR){
            b32 wanna_trigger = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_T);
            
            if (wanna_trigger){
                activate_door(selected, !selected->door.is_open);
            }
        }
        
        // move sequence settings
        if (selected->flags & MOVE_SEQUENCE){
            b32 wanna_clear    = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L);
            b32 wanna_add    = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_M);
            
            if (wanna_remove){
                for (i32 i = 0; i < selected->move_sequence.points.count; i++){
                    Vector2 point = selected->move_sequence.points.get(i);   
                    
                    if (check_col_circles({input.mouse_position, 1}, {point, 0.5f  * (0.4f / current_level_context->cam.cam2D.zoom)})){       
                        selected->move_sequence.points.remove(i);
                        break;
                    }
                }
            }
            if (wanna_add){
                selected->move_sequence.points.add(input.mouse_position);
            }
            if (wanna_clear){
                selected->move_sequence.points.clear();
            }
        }
    }
    
    //undo logic
    if (something_in_undo){
        add_undo_action(undo_action);
    }
    
    if (current_level_context->undo_actions.count > 0 && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z) && !IsKeyDown(KEY_LEFT_SHIFT)){
        Undo_Action *action = current_level_context->undo_actions.pop_ptr();
        
        focus_input_field.in_focus = false;
        
        // So we removing changed entities from multiselection
        if (action->added_to_multiselection){
            for (i32 i = 0; i < action->changed_entities.count; i++){
                i32 id = action->changed_entities.get(i);
                i32 index_in_multiselected = get_index_of_id(&editor.multiselected_entities, id);
                if (index_in_multiselected != -1){
                    editor.multiselected_entities.remove(index_in_multiselected);
                }
            }
        }
        
        // So we adding it again
        if (action->removed_from_multiselection){
            for (i32 i = 0; i < action->changed_entities.count; i++){
                i32 id = action->changed_entities.get(i);
                if (!editor.multiselected_entities.contains(id)){
                    editor.multiselected_entities.add(id);
                }
            }
        }        
        
        if (action->entity_was_deleted){
            restore_deleted_entities(action);            
        } else if (action->entity_was_spawned){
            if (action->entity_id != -1){
                editor_delete_entity(action->entity_id, false);
            }
            
            if (action->changed_entities.count > 0){
                editor.multiselected_entities.clear();
                for (i32 i = 0; i < action->changed_entities.count; i++){
                    editor.multiselected_entities.add(action->changed_entities.get(i));
                }
                editor_delete_multiselected_entities(false, action);
            }
        } else{
            for (i32 i = 0; i < action->changed_entities.count; i++){
                Entity *changed_entity = get_entity_by_id(action->changed_entities.get(i));
                // It should be there anyway i think, because even if we delete them - we should restore them firstly.
                assert(changed_entity);
                
                changed_entity->position -= action->position_change;
                if (action->moved_entity_points){
                    editor_move_entity_points(changed_entity, action->position_change * -1.0f);
                }
            }
            
            if (action->entity_id != -1){
                assert(current_level_context->entities.has_key(action->entity_id));
                Entity *undo_entity = current_level_context->entities.get_by_key_ptr(action->entity_id);
    
                undo_entity->position   -= action->position_change;
                if (action->moved_entity_points){
                    editor_move_entity_points(undo_entity, action->position_change * -1.0f);
                }
                
                
                undo_entity->scale      -= action->scale_change;
                undo_entity->rotation   -= action->rotation_change;
                undo_entity->draw_order -= action->draw_order_change;
                
                for (i32 i = 0; i < action->vertices_change.count; i++){
                    *undo_entity->vertices.get_ptr(i)          -= action->vertices_change.get(i);
                    *undo_entity->unscaled_vertices.get_ptr(i) -= action->unscaled_vertices_change.get(i);
                }
                rotate(undo_entity, 0);
                
                calculate_bounds(undo_entity);
            }
        }
    }
    
    // redo logic
    b32 need_make_redo = editor.max_undos_added > current_level_context->undo_actions.count && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_Z);
    if (need_make_redo){
        current_level_context->undo_actions.count++;        
        Undo_Action *action = current_level_context->undo_actions.last_ptr();
        
        // So we adding it again.
        if (action->added_to_multiselection){
            for (i32 i = 0; i < action->changed_entities.count; i++){
                i32 id = action->changed_entities.get(i);
                if (!editor.multiselected_entities.contains(id)){
                    editor.multiselected_entities.add(id);
                }
            }
        }
        
        // So we removing it again
        if (action->removed_from_multiselection){
            for (i32 i = 0; i < action->changed_entities.count; i++){
                i32 id = action->changed_entities.get(i);
                i32 index_in_multiselected = get_index_of_id(&editor.multiselected_entities, id);
                if (index_in_multiselected != -1){
                    editor.multiselected_entities.remove(index_in_multiselected);
                }
            }
        }        
        
        if (action->entity_was_deleted){ //so we need delete this again
            for (i32 i = 0; i < action->changed_entities.count; i++){
                i32 entity_id_to_delete = action->changed_entities.get(i);
                assert(get_entity_by_id(entity_id_to_delete));
                editor_delete_entity(get_entity_by_id(entity_id_to_delete), false);
            }
        } else if (action->entity_was_spawned){ //so we need spawn this again
            if (action->entity_id != -1){
                Entity *restored_entity = add_entity(&action->spawned_entity, true);
                restored_entity->id = action->spawned_entity.id;
                action->entity_id = restored_entity->id;
            }
            
            if (action->deleted_entities.count > 0){
                restore_deleted_entities(action);
            }
        } else{
            for (i32 i = 0; i < action->changed_entities.count; i++){
                Entity *changed_entity = get_entity_by_id(action->changed_entities.get(i));
                // It should be there anyway i think, because even if we delete them - we should restore them firstly.
                assert(changed_entity);
                
                changed_entity->position += action->position_change;
                if (action->moved_entity_points){
                    editor_move_entity_points(changed_entity, action->position_change);
                }
            }
            
            if (action->entity_id != -1){
                assert(current_level_context->entities.has_key(action->entity_id));
                Entity *undo_entity = current_level_context->entities.get_by_key_ptr(action->entity_id);
                
                undo_entity->position   += action->position_change;
                if (action->moved_entity_points){
                    editor_move_entity_points(undo_entity, action->position_change);
                }
                
                
                undo_entity->scale      += action->scale_change;
                undo_entity->rotation   += action->rotation_change;
                undo_entity->draw_order += action->draw_order_change;
                
                for (i32 i = 0; i < action->vertices_change.count; i++){
                    *undo_entity->vertices.get_ptr(i)          += action->vertices_change.get(i);
                    *undo_entity->unscaled_vertices.get_ptr(i) += action->unscaled_vertices_change.get(i);
                }
                rotate(undo_entity, 0);
                
                calculate_bounds(undo_entity);
            }
        }
    }
    
    //editor Save level
    if (IsKeyPressed(KEY_J) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL)){
        save_current_level();
    }
    if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)){
        save_current_level();
        state_context.timers.background_flash_time = core.time.app_time;
    }
    
    f32 time_since_autosave = core.time.app_time - editor.last_autosave_time;
    if (time_since_autosave > 40 && game_state == EDITOR){
        autosave_level();
        editor.last_autosave_time = core.time.app_time;
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        editor.last_click_position = input.mouse_position;
        editor.last_click_time = core.time.game_time;
    }
    
    if (can_control_with_single_button && IsKeyPressed(KEY_P) && !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL)){
        current_level_context->player_spawn_point = input.mouse_position;
    }

    clicked_ui = false;
} // update editor end

void change_color(Entity *entity, Color new_color){
    entity->color = new_color;
    setup_color_changer(entity);
}

Bounds get_bounds(Array<Vector2, MAX_VERTICES> vertices, Vector2 pivot){
    f32 top_vertex    = -INFINITY;
    f32 bottom_vertex =  INFINITY;
    f32 right_vertex  = -INFINITY;
    f32 left_vertex   =  INFINITY;
    
    Vector2 middle_position;
    
    for (i32 i = 0; i < vertices.count; i++){
        Vector2 *vertex = vertices.get_ptr(i);
        
        if (vertex->y > top_vertex){
            top_vertex = vertex->y;
        }
        if (vertex->y < bottom_vertex){
            bottom_vertex = vertex->y;
        }
        if (vertex->x > right_vertex){
            right_vertex = vertex->x;
        }
        if (vertex->x < left_vertex){
            left_vertex = vertex->x;
        }
    }    
    
    middle_position = {(1.0f - pivot.x) * left_vertex + pivot.x * right_vertex,
                       pivot.y * bottom_vertex + (1.0f - pivot.y) * top_vertex};
    
    return {{right_vertex - left_vertex, top_vertex - bottom_vertex}, middle_position};
}

inline void calculate_bounds(Entity *entity){
    entity->bounds = get_bounds(entity->vertices, entity->pivot);
}

void calculate_vertices(Entity *entity){
    for (i32 i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        Vector2 unscaled_vertex = entity->unscaled_vertices.get(i);
        
        f32 up_dot    = dot(entity->up,    unscaled_vertex);
        f32 right_dot = dot(entity->right, unscaled_vertex);

        *vertex = normalized(unscaled_vertex) + (entity->up * up_dot * entity->scale.y) + (entity->right * right_dot * entity->scale.x);
    }
}

void change_scale(Entity *entity, Vector2 new_scale){
    Vector2 old_scale = entity->scale;
    
    entity->scale = new_scale;
    
    clamp(&entity->scale.x, 0.01f, 10000);
    clamp(&entity->scale.y, 0.01f, 10000);

    calculate_vertices(entity);    
    calculate_bounds(entity);
}

void add_scale(Entity *entity, Vector2 added){
    change_scale(entity, entity->scale + added);
}

void change_up(Entity *entity, Vector2 new_up){
    rotate_to(entity, (atan2f(new_up.x, new_up.y) * RAD2DEG));
}

void change_right(Entity *entity, Vector2 new_right){
    rotate_to(entity, atan2f(-new_right.y, new_right.x) * RAD2DEG);
}

void rotate_around_point(Vector2 *target, Vector2 origin, f32 rotation){
    f32 s = -sinf(rotation * DEG2RAD);
    f32 c =  cosf(rotation * DEG2RAD);
    
    target->x -= origin.x;
    target->y -= origin.y;
    
    // rotate point
    f32 xnew = target->x * c - target->y * s;
    f32 ynew = target->x * s + target->y * c;
    
    // translate point back:
    target->x = xnew + origin.x;
    target->y = ynew + origin.y;
}

void rotate_to(Entity *entity, f32 new_rotation){
    while (new_rotation >= 360){
        new_rotation -= 360;
    }
    while (new_rotation < 0){
        new_rotation += 360;
    }

    f32 old_rotation = entity->rotation;

    entity->rotation = new_rotation;
    
    entity->up    = {sinf(new_rotation * DEG2RAD),  cosf(new_rotation * DEG2RAD)};
    normalize(&entity->up);
    entity->right = {cosf(new_rotation * DEG2RAD), -sinf(new_rotation * DEG2RAD)};
    normalize(&entity->right);
    
    for (i32 i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        rotate_around_point(vertex, {0, 0}, entity->rotation - old_rotation);
        rotate_around_point(entity->unscaled_vertices.get_ptr(i), {0, 0}, entity->rotation - old_rotation);
    }
    
    calculate_bounds(entity);
}

void rotate(Entity *entity, f32 rotation){
    rotate_to(entity, entity->rotation + rotation);
}

void player_accelerate(Entity *entity, Vector2 dir, f32 wish_speed, f32 acceleration, f32 dt){
    f32 speed_in_wish_direction = dot(player_data->velocity, dir);
    
    f32 speed_difference = wish_speed - speed_in_wish_direction;        
    
    //means we above max speed
    if (speed_difference <= 0){
        return;
    }
    
    f32 acceleration_speed = acceleration * speed_difference * dt;
    if (acceleration_speed > speed_difference){
        acceleration_speed = speed_difference;
    }
    
    player_data->velocity.x += dir.x * acceleration_speed;
}

void player_ground_move(Entity *entity, f32 dt){
    f32 walk_speed = player_data->in_big_sword ? player_data->big_sword_ground_walk_speed : player_data->ground_walk_speed;
    
    Vector2 input_direction = input.sum_direction;
    
    b32 wanna_stop = input_direction.x == 0 || player_data->on_no_move_block;
    
    Vector2 wish_walking_plane = get_rotated_vector_90(player_data->ground_normal, -input_direction.x);
    
    if (wanna_stop){
        f32 stopping_deceleration = player_data->ground_deceleration;
        
        Vector2 deceleration_plane = get_rotated_vector_90(player_data->ground_normal, normalized(player_data->velocity.x));
        
        f32 speed_in_wish_plane = dot(deceleration_plane, player_data->velocity);
        f32 speed_change        = fminf(stopping_deceleration * dt, -speed_in_wish_plane);
        player_data->velocity  += deceleration_plane * speed_change;
    } else{
        f32 walking_acceleration = player_data->ground_acceleration;
        
        b32 walking_same_direction = input_direction.x != 0 && dot(wish_walking_plane, player_data->velocity_plane) > 0;
        if (!walking_same_direction){
            walking_acceleration *= 3;
        }
        
        f32 speed_in_wish_plane = dot(wish_walking_plane, player_data->velocity);
        f32 max_allowed_speed_difference = walk_speed - speed_in_wish_plane;
        
        if (speed_in_wish_plane >= walk_speed && input_direction.x * player_data->velocity.x > 0){
            max_allowed_speed_difference = speed_in_wish_plane - walk_speed;
            
            wish_walking_plane *= -1;
            
            walking_acceleration *= 0.1f;
        }
        
        f32 speed_change = fminf(walking_acceleration * dt, max_allowed_speed_difference);
        
        player_data->velocity += wish_walking_plane * speed_change;
    }
}

void player_air_move(Entity *entity, f32 dt){
    f32 walk_speed = player_data->in_big_sword ? player_data->big_sword_air_walk_speed : player_data->air_walk_speed;
    
    Vector2 input_direction = input.sum_direction;
    
    b32 wanna_stop = input_direction.x == 0;
    
    f32 wish_direction = input_direction.x;
    
    if (wanna_stop){
        f32 stopping_deceleration = player_data->air_deceleration;
        
        f32 deceleration_direction = -normalized(player_data->velocity.x);
        
        f32 speed_in_wish_direction = deceleration_direction * player_data->velocity.x;
        f32 speed_change = fminf(stopping_deceleration * dt, -speed_in_wish_direction);
        player_data->velocity.x += deceleration_direction * speed_change;
    } else{
        f32 walking_acceleration = player_data->air_acceleration;
        
        b32 walking_same_direction = input_direction.x != 0 && (wish_direction * player_data->velocity.x) > 0;
        if (!walking_same_direction){
            walking_acceleration *= 3;
        }
        
        f32 speed_in_wish_direction = wish_direction * player_data->velocity.x;
        f32 max_allowed_speed_difference = walk_speed - speed_in_wish_direction;
        
        if (speed_in_wish_direction >= walk_speed && input_direction.x * player_data->velocity.x > 0){
            max_allowed_speed_difference = speed_in_wish_direction - walk_speed;
            wish_direction *= -1;
            
            f32 overspeed = speed_in_wish_direction - walk_speed;
            f32 overspeed_t = clamp01(overspeed / 100.0f);
            walking_acceleration = lerp(walking_acceleration * 0.01f, walking_acceleration, overspeed_t);
        }
        
        f32 speed_change = fminf(walking_acceleration * dt, max_allowed_speed_difference);
        
        player_data->velocity.x += wish_direction * speed_change;
    }
}

void add_hitstop(f32 added, b32 can_go_over_limit){
    b32 was_over_limit = core.time.hitstop > 0.1f;
    if (core.time.hitstop < 0){
        core.time.hitstop = 0;
    }

    core.time.hitstop += added;
    
    if (can_go_over_limit && core.time.hitstop > 0.5f){
        clamp(&core.time.hitstop, 0, 0.5f);
    } else if (core.time.hitstop > 0.1f && !was_over_limit){
        clamp(&core.time.hitstop, 0,  0.1f);
    }
}

void shake_camera(f32 trauma){
    state_context.cam_state.trauma += trauma;
    state_context.cam_state.trauma = clamp01(state_context.cam_state.trauma);
}

void add_blood_amount(Player *player, f32 added){
    player->blood_amount += added;
    clamp(&player->blood_amount, 0, player->max_blood_amount);
    player->blood_progress = player->blood_amount / player->max_blood_amount;
}

void win_level(){
    if (!state_context.we_got_a_winner){    
        state_context.we_got_a_winner = true;
        kill_player();
    }
}

void set_sword_velocity(f32 value){
    player_data->sword_angular_velocity = value;
    player_data->sword_spin_direction = normalized(player_data->sword_angular_velocity);
    f32 sword_max_spin_speed = 5000;
    player_data->sword_spin_progress = clamp01(abs(player_data->sword_angular_velocity) / sword_max_spin_speed);
}

inline void add_player_ammo(i32 amount){
    player_data->ammo_count += amount;
    
    player_data->ammo_count = clamp(player_data->ammo_count, 0, 3333);
    
    if (player_data->ammo_count == 0 && amount < 0){
        player_data->timers.last_bullet_shot_time = core.time.game_time;
    }
}

inline b32 is_sword_can_damage(){
    f32 threshold = player_data->is_sword_accelerating ? 0.01f : 0.5f;
    return !is_player_in_stun() && player_data->sword_spin_progress >= threshold;
}

inline b32 can_damage_blocker(Entity *blocker_entity){
    return is_sword_can_damage() && !blocker_entity->enemy.blocker_immortal && (blocker_entity->enemy.blocker_clockwise ? player_data->sword_spin_direction > 0 : player_data->sword_spin_direction < 0);
}

inline b32 can_damage_sword_size_required_enemy(Entity *enemy_entity){
    return is_sword_can_damage() && player_data->in_big_sword == enemy_entity->enemy.big_sword_killable;
}

inline b32 can_sword_damage_enemy(Entity *enemy_entity){
    b32 sword_can_damage = is_sword_can_damage();
    b32 is_blocker_damageble = true;
    if (enemy_entity->flags & BLOCKER){
        is_blocker_damageble = can_damage_blocker(enemy_entity);
    }
    b32 is_sword_size_required_damageble = true;
    if (enemy_entity->flags & SWORD_SIZE_REQUIRED){
        is_sword_size_required_damageble = can_damage_sword_size_required_enemy(enemy_entity);
    }
    
    return sword_can_damage && ((is_blocker_damageble && is_sword_size_required_damageble) || enemy_entity->enemy.dead_man);
}

void sword_kill_enemy(Entity *enemy_entity, Vector2 *enemy_velocity){
    Entity *sword = get_entity_by_id(player_data->connected_entities_ids.sword_entity_id);
    enemy_velocity->y = fmaxf(100.0f, 100.0f + enemy_velocity->y);
    enemy_velocity->x = player_data->sword_spin_direction * 50 + enemy_velocity->x;
    
    if (!enemy_entity->enemy.dead_man){
        add_hitstop(0.1f);
    }
    
    Vector2 particles_direction = enemy_entity->up;
    if (sword_tip_ground_emitter_index != -1){
        particles_direction = get_particle_emitter(sword_tip_ground_emitter_index)->direction;
    }
    // Should just do a enemy flag for serious enemies instead of picking everyone individualy.
    if (enemy_entity->flags & JUMP_SHOOTER){
        kill_enemy(enemy_entity, sword->position + sword->up * sword->scale.y * sword->pivot.y, particles_direction, 1.0f);
        add_explosion_light(enemy_entity->position, 75, 0.03f, 0.1f, ColorBrightness(RED, 0.4f));
    } else{
        // Vector2 kill_position = sword->position + sword->up * sword->scale.y * sword->pivot.y;
        stun_enemy(enemy_entity, enemy_entity->position, particles_direction, true);
    }
}

b32 is_type(Entity *entity, FLAGS flags){
    return entity->flags & flags;
}

b32 try_sword_damage_enemy(Entity *enemy_entity, Vector2 hit_position){
    if (!can_sword_damage_enemy(enemy_entity)){
        return false;
    }

    b32 killed_enemy = false;
    if (is_sword_can_damage() && !is_player_in_stun() && is_enemy_can_take_damage(enemy_entity)){
        b32 is_it_utility_enemy = enemy_entity->flags & (HIT_BOOSTER | TRIGGER);
                
        Enemy *enemy = &enemy_entity->enemy;
                
        if (enemy_entity->flags & AMMO_PACK){
            add_player_ammo(1);
        }
        add_blood_amount(player_data, 10);
    
        b32 was_alive_before_hit = !enemy->dead_man;
        f32 hitstop_add = 0;
        
        Vector2 particles_direction = enemy_entity->up;
        if (sword_tip_ground_emitter_index != -1){
            particles_direction = get_particle_emitter(sword_tip_ground_emitter_index)->direction;
        }
        
        if (enemy_entity->flags & HIT_BOOSTER){
            player_data->velocity = enemy_entity->up * enemy->hit_booster.boost;
            player_data->timers.hit_booster_time = core.time.game_time;
        }
        
        // We also set this last hit variable in kill_enemy and stun_enemy, but as we see now we don't want to kill or stun 
        // every enemy that take hit, so it have sense to set it where actual hit is delivered.
        enemy->last_hit_time = core.time.game_time;
        
        b32 can_kill = true;
        
        if (enemy_entity->flags & MULTIPLE_HITS){
            Multiple_Hits *mod = &enemy->multiple_hits;
            mod->made_hits += 1;
            // So he do not regen immediately after hit.
            mod->timer = 0;
            
            if (mod->made_hits < mod->required_hits){
                can_kill = false;
            }
        }
        if (enemy->max_hits_taken <= -1){
            can_kill = false;
        }
        
        if (can_kill){
            if (enemy_entity->flags & GIVES_BIG_SWORD_CHARGE){
                player_data->current_big_sword_charges += 1;                
                clamp(&player_data->current_big_sword_charges, 0, player_data->max_big_sword_charges);
            }
            
            if (enemy_entity->flags & BIRD_ENEMY){
                sword_kill_enemy(enemy_entity, &enemy_entity->bird_enemy.velocity);
            } else if (enemy_entity->flags & JUMP_SHOOTER){
                sword_kill_enemy(enemy_entity, &enemy_entity->jump_shooter.velocity);
            } else{
                kill_enemy(enemy_entity, hit_position, particles_direction, false, lerp(1.0f, 1.5f, sqrtf(player_data->sword_spin_progress)));
            }
            
            killed_enemy = true;
        }
        // player_data->sword_angular_velocity += player_data->sword_spin_direction * 1400;
        
        f32 max_speed_boost = 6 * player_data->sword_spin_direction * enemy->sword_kill_speed_modifier;
        f32 max_vertical_speed_boost = player_data->grounded ? 0 : 20;
        if (player_data->velocity.y > 0){
            max_vertical_speed_boost *= 0.3f;   
        }
        
        if (!player_data->grounded){
            player_data->velocity += Vector2_up * max_vertical_speed_boost + Vector2_right * max_speed_boost; 
        }
                         
        if (was_alive_before_hit){
            add_hitstop(0.01f + hitstop_add);
            shake_camera(0.1f);
        }
        
        if (enemy_entity->flags & AMMO_PACK){
            play_sound("AmmoCollect", hit_position, 0.5f, 1.1f, 0.1f);  
        } else if (enemy_entity->flags & BIRD_ENEMY && was_alive_before_hit){
            play_sound("SwordHit33", hit_position, 0.5f, 1.1f, 0.1f);
        } else if (enemy_entity->flags & JUMP_SHOOTER && was_alive_before_hit){
            play_sound("SwordHit2222", hit_position, 0.5f, 0.7f, 0.1f);
        } else{
            play_sound("SwordKill", hit_position, 0.5f);
        }
        
        // sword hitmarks
        {
            Color hitmark_color = WHITE;
            f32 hitmark_scale = 1;
            b32 is_hitmark_follow = false;
            
            if (enemy_entity->flags & (CENTIPEDE_SEGMENT | TRIGGER | BIRD_ENEMY)){
                is_hitmark_follow = true;
            }
            
            if (enemy_entity->flags & EXPLOSIVE){
                hitmark_scale += 2;
                hitmark_color = Fade(ColorBrightness(ORANGE, 0.3f), 0.8f);
            }
            
            if (enemy_entity->flags & HIT_BOOSTER){
                hitmark_scale += 1.5f;
                hitmark_color = ColorBrightness(RED, 0.3f);
            }
            
            add_hitmark(enemy_entity, is_hitmark_follow, hitmark_scale, hitmark_color); 
        }
    }
    
    return killed_enemy;
}

inline void cut_rope(Entity *entity, Vector2 point = Vector2_zero){
    if (point == Vector2_zero){
        point = entity->position;
    }
    entity->destroyed = true;
    emit_particles(&rifle_bullet_emitter, point, entity->up, 6, 50);
    play_sound("RopeCut", point);
}

void calculate_sword_collisions(Entity *sword, Entity *player_entity){
    fill_collisions(sword, &collisions_buffer, GROUND | ENEMY | WIN_BLOCK | CENTIPEDE_SEGMENT | PLATFORM | BLOCK_ROPE);
    
    for (i32 i = 0; i < collisions_buffer.count; i++){
        Collision col = collisions_buffer.get(i);
        Entity *other = col.other_entity;
        
        // blocker block
        if ((other->flags & BLOCKER || other->flags & SWORD_SIZE_REQUIRED) && !is_player_in_stun()){
            if (is_sword_can_damage() && !can_sword_damage_enemy(other)){
                player_data->velocity = player_data->velocity * -0.5f;
                emit_particles(&rifle_bullet_emitter, col.point, col.normal, 3, 5);
                set_sword_velocity(normalized(-player_data->sword_angular_velocity) * 150);
                player_data->weak_recoil_stun_start_time = core.time.game_time;
                add_hitstop(0.1f);
                shake_camera(0.7f);
                // changed pitch from 0.5f and changed sound from 0.4f
                play_sound("SwordBlock", col.point, 0.3f, 0.75f, 0.1f);
                emit_particles(&sparks_emitter_copy, sword->position + sword->up * sword->scale.y * sword->pivot.y, col.normal, 1.5f, 1.0f);
                emit_particles(&shockwave_emitter_copy, sword->position + sword->up * sword->scale.y * sword->pivot.y, col.normal, 1.0f, 1.0f);
                continue;
            }
        }
        
        if (other->flags & ENEMY){
            try_sword_damage_enemy(other, sword->position + sword->up * sword->scale.y * sword->pivot.y);
        }
        
        if (other->flags & WIN_BLOCK && !is_player_in_stun()){
            win_level();
        }
        
        if (other->flags & BLOCK_ROPE && player_data->sword_spin_progress >= 0.7f){
            // cut rope
            cut_rope(other, col.point);
        }
        
        if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT || other->flags & PLATFORM){
            player_data->sword_hit_ground = true;
        }
    }
}

void push_player_up(f32 power){
    if (player_data->velocity.y < 0){
        player_data->velocity.y = 0;
    }
    
    player_data->velocity.y += power;
    player_data->timers.since_jump_timer = 0;
    player_data->grounded = false;
}

void push_or_set_player_up(f32 power){
    if (player_data->velocity.y > power){
        power *= 0.25f;
    }

    player_data->velocity.y += power;
    
    player_data->timers.since_jump_timer = 0;
    player_data->grounded = false;
}

f32 apply_physics_force(Vector2 velocity, f32 mass, Physics_Object *to_whom, Vector2 normal){
    Vector2 velocity_direction = normalized(velocity);
    if (normal == Vector2_zero){
        normal = velocity_direction * -1.0f;
    }
    f32 collision_force_multiplier = 1;

    f32 direction_normal_dot = dot(velocity_direction, normal);
    to_whom->velocity += ((velocity * mass * direction_normal_dot * -1) / to_whom->mass);
    collision_force_multiplier = to_whom->mass / mass;
    
    return collision_force_multiplier;
}

void resolve_physics_collision(Vector2 *my_velocity, f32 my_mass, Vector2 *their_velocity, f32 their_mass, Vector2 normal){
    Vector2 velocity_direction = normalized(*my_velocity);
    if (normal == Vector2_zero){
        normal = velocity_direction * -1.0f;
    }
    f32 collision_force_multiplier = 1;

    if (dot(*my_velocity, *their_velocity) > magnitude(*their_velocity)){
        Vector2 velocity_difference = *my_velocity - *their_velocity;
        f32 target_magnitude = magnitude(velocity_difference);
        velocity_difference = lerp(Vector2_zero, velocity_difference, clamp01((my_mass / their_mass) * 0.1f));
        f32 direction_normal_dot = dot(velocity_direction, normal);
        *their_velocity += ((velocity_difference));
        collision_force_multiplier = clamp(their_mass / my_mass, 0.0f, 1.0f);
        
        *my_velocity -= normal * dot(velocity_difference, normal) * collision_force_multiplier;
    }
}

inline void update_player_connected_entities_positions(Entity *player_entity){
    Entity *ground_checker     = get_entity_by_id(player_data->connected_entities_ids.ground_checker_id);
    Entity *left_wall_checker  = get_entity_by_id(player_data->connected_entities_ids.left_wall_checker_id);
    Entity *right_wall_checker = get_entity_by_id(player_data->connected_entities_ids.right_wall_checker_id);
    Entity *sword              = get_entity_by_id(player_data->connected_entities_ids.sword_entity_id);
    
    ground_checker->position     = player_entity->position - player_entity->up * player_entity->scale.y * 0.5f;
    left_wall_checker->position  = player_entity->position - player_entity->right * player_entity->scale.x * 1.0f + Vector2_up * player_entity->scale.y * 0.3f;
    right_wall_checker->position = player_entity->position + player_entity->right * player_entity->scale.x * 1.0f + Vector2_up * player_entity->scale.y * 0.3f;
    sword->position = player_entity->position;
    // rifle->position = sword->position + sword->up * sword->scale.y;
}

inline Vector2 get_move_plane(Vector2 normal, f32 move_dir){
    return get_rotated_vector_90(normal, -normalized(move_dir));
}

inline void player_snap_to_plane(Vector2 normal){
    player_data->ground_normal = normal;
    player_data->velocity_plane = get_move_plane(player_data->ground_normal, player_data->velocity.x);
    player_data->velocity = player_data->velocity_plane * magnitude(player_data->velocity);
}

inline b32 is_player_in_stun(){
    f32 max_weak_stun_time = 0.3f;
    f32 in_weak_stun_time   = core.time.game_time - player_data->weak_recoil_stun_start_time;
    return (in_weak_stun_time <= max_weak_stun_time);
}

void update_player(Entity *player_entity, f32 dt, Input input){
    assert(player_entity->flags & PLAYER);

    if (player_data->dead_man){
        return;
    }
    
    Entity *ground_checker     = get_entity_by_id(player_data->connected_entities_ids.ground_checker_id);
    Entity *left_wall_checker  = get_entity_by_id(player_data->connected_entities_ids.left_wall_checker_id);
    Entity *right_wall_checker = get_entity_by_id(player_data->connected_entities_ids.right_wall_checker_id);
    Entity *sword              = get_entity_by_id(player_data->connected_entities_ids.sword_entity_id);
    
    update_player_connected_entities_positions(player_entity);    
    
    f32 in_big_sword_time = core.time.game_time - player_data->big_sword_start_time;
    
    f32 big_sword_max_time = 1.4f;
    
    // big sword
    if (in_big_sword_time > big_sword_max_time){
        if (input.press_flags & SPIN && player_data->current_big_sword_charges > 0){
            player_data->big_sword_start_time = core.time.game_time;    
            // player_data->max_speed_multiplier = 2.0f;
            player_data->in_big_sword = true;
            play_sound("SwordSwingBig", 0.9f, 1.0f, 0.05f);
            
            assert(player_data->current_big_sword_charges > 0 && player_data->current_big_sword_charges <= player_data->max_big_sword_charges);
            player_data->current_big_sword_charges -= 1;
        } else{
            // player_data->max_speed_multiplier = 1.0f;
            if (player_data->in_big_sword){
                play_sound("SwordSwing", 0.9f, 1.5f, 0.05f);
            }
            player_data->in_big_sword = false;
        }
                
    }
    
    Vector2 sword_target_size = player_data->in_big_sword ? player_data->big_sword_scale : player_data->sword_start_scale;
    
    change_scale(sword, lerp(sword->scale, sword_target_size, dt * 5));
    
    Vector2 sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
    
    Particle_Emitter *chainsaw_emitter = get_particle_emitter(chainsaw_emitter_index);
    
    if (input.press_flags & SPIN){
        chainsaw_emitter->position = input.mouse_position;
        chainsaw_emitter->last_emitted_position = input.mouse_position;
        chainsaw_emitter->enabled = true;
    }
    if (input.press_flags & SPIN_RELEASED){
        chainsaw_emitter->enabled = false;
    }
    
    f32 max_big_sword_speed = 4500;
    f32 max_small_sword_speed = 2200;
    
    Vector2 input_direction = input.sum_direction;
    
    f32 sword_max_spin_speed = player_data->in_big_sword ? max_big_sword_speed : max_small_sword_speed;
    
    b32 can_sword_spin = !is_player_in_stun();
    if (can_sword_spin){
        f32 sword_spin_sense = player_data->in_big_sword ? 40 : 10; 
        
        f32 wish_angular_velocity = input_direction.x * sword_max_spin_speed;
        
        if (core.time.time_scale < 1){
            sword_spin_sense /= core.time.time_scale;
            sword_spin_sense = fminf(sword_spin_sense, 60);
        }
        player_data->sword_angular_velocity = lerp(player_data->sword_angular_velocity, wish_angular_velocity, dt * sword_spin_sense);
    }
    player_data->is_sword_accelerating = input_direction.x != 0;
    
    player_data->sword_spin_progress = clamp01(abs(player_data->sword_angular_velocity) / sword_max_spin_speed);
    
    b32 rifle_failed_hard = false;
    
    Particle_Emitter *rifle_trail_emitter = get_particle_emitter(player_data->rifle_trail_emitter_index);
    if (rifle_trail_emitter){
        rifle_trail_emitter->position = sword_tip;
        rifle_trail_emitter->direction = sword->up;
    }
    
    // @DO REdo this machinegun shit when we'll know for sure how we think this should work.
    i32 shoots_queued = 0;
    local_persist f32 shoot_press_time = -12;
    local_persist f32 rifle_in_machinegun_mode = false;
    if (input.press_flags & SHOOT){
        shoot_press_time = core.time.game_time;
        shoots_queued += 1;
    }
    
    if (input.hold_flags & SHOOT_DOWN && shoot_press_time > 0){
        f32 hold_time = core.time.game_time - shoot_press_time;
        
        if (!rifle_in_machinegun_mode && hold_time >= 0.2f){
            rifle_in_machinegun_mode = true;
            shoots_queued += 1;
            shoot_press_time = core.time.game_time;
            hold_time -= 0.2f;
        }
        
        f32 machinegun_shoots_delay = 0.03f;
        if (rifle_in_machinegun_mode){
            while (hold_time >= machinegun_shoots_delay){
                hold_time -= machinegun_shoots_delay;
                shoots_queued += 1;
                shoot_press_time = core.time.game_time;
            }
        }
    }
    
    if (input.press_flags & SHOOT_RELEASED){
        shoot_press_time = -12;
        rifle_in_machinegun_mode = false;
    }
    
    if (player_data->ammo_count == 0 && core.time.game_time - player_data->timers.last_bullet_shot_time >= 3.0f){
        add_player_ammo(1);
    }
    
    if (input.press_flags & SHOOT){
        if ((player_data->ammo_count <= 0 && !debug.infinite_ammo)){
            play_sound("FailedRifleActivation", 0.4f);
        }
        
        player_data->timers.rifle_shake_start_time = core.time.game_time;
        emit_particles(&gunpowder_emitter, sword_tip, sword->up);
    }
    
    // player shoot
    b32 can_shoot_rifle = (player_data->ammo_count > 0 || debug.infinite_ammo) && state_context.shoot_stopers_count == 0;
    
    while (shoots_queued > 0){
        if (can_shoot_rifle){
            Vector2 sword_vec_to_mouse = input.mouse_position - sword->position;
            Vector2 sword_to_mouse = normalized(sword_vec_to_mouse);
            change_up(sword, sword_to_mouse);
            sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
            
            Vector2 sword_tip_vec_to_mouse = input.mouse_position - sword->position;
            Vector2 sword_tip_to_mouse = normalized(sword_tip_vec_to_mouse);
            
            
            Vector2 shoot_direction = sword_tip_to_mouse;
            
            if (rifle_in_machinegun_mode){
                f32 max_spread = 20;
                f32 spread_angle = rnd(-max_spread * 0.5f, max_spread * 0.5f);
                
                shoot_direction = get_rotated_vector(shoot_direction, spread_angle);
            }
            
            f32 rifle_projectile_speed = 1400;
            add_rifle_projectile(sword_tip, shoot_direction * rifle_projectile_speed);
            add_player_ammo(-1);
            
            add_explosion_light(sword_tip, 50, 0.03f, 0.05f, ColorBrightness(ORANGE, 0.3f));
            
            if (!player_data->grounded){
                push_or_set_player_up(rifle_in_machinegun_mode ? 5 : 20);
            }
            
            shake_camera(0.1f);
            play_sound("RifleShot", sword_tip, 0.3f);
            player_data->timers.rifle_shake_start_time = core.time.game_time;
            player_data->timers.rifle_shoot_time = core.time.game_time;
            
            enable_emitter(player_data->rifle_trail_emitter_index);
            
        } else if (input.press_flags & SHOOT){
            player_data->timers.rifle_shake_start_time = core.time.game_time;
            emit_particles(&gunpowder_emitter, sword_tip, sword->up);
            
            // shoot stoper blocked
            if (state_context.shoot_stopers_count > 0){
                local_persist i32 contiguous_failed_shots_count = 0;
                f32 time_since_last_failed_shot = core.time.app_time - state_context.timers.last_shoot_stoper_failed_shot_app_time;
                
                if (time_since_last_failed_shot <= 0.4f){
                    contiguous_failed_shots_count += 1;
                } else{
                    contiguous_failed_shots_count = 0;
                }
                
                state_context.timers.last_shoot_stoper_failed_shot_app_time = core.time.app_time;
                
                if (contiguous_failed_shots_count <= 5){
                    ForEntities(entity, SHOOT_STOPER){
                        if (entity->enemy.in_agro){
                            Entity *sticky_line = add_entity(player_entity->position, {1,1}, {0.5f,0.5f}, 0, STICKY_TEXTURE);
                            sticky_line->sticky_texture.draw_line = true;
                            sticky_line->sticky_texture.line_color = ColorBrightness(VIOLET, 0.1f);
                            sticky_line->sticky_texture.line_width = contiguous_failed_shots_count * 0.5f;
                            sticky_line->sticky_texture.follow_id = entity->id;
                            sticky_line->sticky_texture.need_to_follow = true;
                            sticky_line->position = get_shoot_stoper_cross_position(entity);
                            sticky_line->sticky_texture.birth_time = core.time.game_time;
                            sticky_line->sticky_texture.max_distance = 0;
                            sticky_line->draw_order = 1;
                            shake_camera(0.1f);
                        }
                    }
                }
                
                play_sound("FailedRifleActivation", 0.4f, 0.5f);
            }
        }
        shoots_queued -= 1;
    }
    
    f32 time_since_shoot = core.time.game_time - player_data->timers.rifle_shoot_time;
    
    if (time_since_shoot >= 0.5f && core.time.game_time > 1){
        disable_emitter(player_data->rifle_trail_emitter_index);
    } else{
    }
    
    //rifle activate
    if (input.press_flags & SHOOT){
        // Failed to activate rifle.
    }
    
    // sword->color_changer.progress = can_activate_rifle ? 1 : 0;
    change_color(sword, player_data->in_big_sword ? ColorBrightness(RED, 0.1f) : ColorBrightness(SKYBLUE, 0.3f));
    
    Particle_Emitter *sword_tip_emitter       = get_particle_emitter(blood_trail_emitter_index);
    if (sword_tip_emitter){
        enable_emitter(sword_tip_emitter);
        sword_tip_emitter->position = sword_tip;
    }
    Particle_Emitter *sword_tip_ground_emitter = get_particle_emitter(sword_tip_ground_emitter_index);
    if (sword_tip_ground_emitter){
        sword_tip_ground_emitter->position = sword_tip;
    }
    if (chainsaw_emitter){
        chainsaw_emitter->position = input.mouse_position;
    }
    
    f32 blood_t = player_data->blood_progress;
    f32 spin_t = player_data->sword_spin_progress;
    {
    
        chainsaw_emitter->lifetime_multiplier = 1.0f + spin_t * spin_t * 2; 
        chainsaw_emitter->speed_multiplier    = 1.0f + spin_t * spin_t * 2; 
        
        chainsaw_emitter->count_multiplier = player_data->in_big_sword ? 0.1f : 1;
        chainsaw_emitter->size_multiplier  = player_data->in_big_sword ? 5 : 1;
        chainsaw_emitter->color            = player_data->in_big_sword ? ColorBrightness(ORANGE, 0.2f) : YELLOW;
        
        // sword_tip_emitter->lifetime_multiplier = 1.0f + blood_t * blood_t * 3.0f;
        sword_tip_emitter->speed_multiplier    = 1.0f + blood_t * blood_t * 5.0f;
        sword_tip_emitter->count_multiplier    = blood_t * blood_t * 2.0f;
              
        f32 blood_decease = 25;
              
        add_blood_amount(player_data, -blood_decease * dt);
    }
    
    f32 sword_min_rotation_amount = 5;
    f32 need_to_rotate = player_data->sword_angular_velocity * dt;
    
    if (sword->rotation + need_to_rotate >= 360 || sword->rotation + need_to_rotate < 0){
        // emit_particles(sword_tip_emitter, player_entity->position, sword->up);
        //@SOUND sound on swing could be nice, but we should not depend on actual spin and instead find a proper looping sound.
        // local_persist f32 volume = 0.1f;
        // volume = lerp(volume, 0.1f + player_data->sword_spin_progress * 0.1f + player_data->in_big_sword ? 0.4f : 0.0f, dt * 10);
        // play_sound("SwordSwing", volume, lerp(0.4f, player_data->in_big_sword ? 1.0f : 1.5f, player_data->sword_spin_progress), 0.1f);
    }
    
    player_data->sword_spin_direction = normalized(player_data->sword_angular_velocity);
    
    { // sword rotation
        // Someone could enter sword on previous frame after this update so we'll check for that.
        rotate(sword, -1.0f * 0.5f * sword_min_rotation_amount * player_data->sword_spin_direction);         
        calculate_sword_collisions(sword, player_entity);
        
        rotate(sword, 0.5f * sword_min_rotation_amount * player_data->sword_spin_direction);         
        calculate_sword_collisions(sword, player_entity);
        while(need_to_rotate > sword_min_rotation_amount){
            rotate(sword, sword_min_rotation_amount * player_data->sword_spin_direction);
            calculate_sword_collisions(sword, player_entity);
            need_to_rotate -= sword_min_rotation_amount;
        }
        rotate(sword, need_to_rotate);
        calculate_sword_collisions(sword, player_entity);
    }
    
    player_data->timers.since_jump_timer += dt;
    
    if (!is_player_in_stun()){
        disable_emitter(player_data->stun_emitter_index);
    } else{
        enable_emitter(player_data->stun_emitter_index);
    }
    
    f32 since_hit_booster = core.time.game_time - player_data->timers.hit_booster_time;
    
    //player movement
    if (since_hit_booster <= 0.4f){
        
    } else if (player_data->grounded && !is_player_in_stun() && !player_data->on_propeller){
        player_ground_move(player_entity, dt);
        
        player_snap_to_plane(player_data->ground_normal);
        
        player_entity->position.y -= dt;
        player_data->velocity -= player_data->ground_normal * dt;
        
        player_data->timers.since_airborn_timer = 0;
    } else/* if (!in_climbing_state)*/{
        f32 max_downwards_speed = -75;
    
        if (player_data->velocity.y > 10/* && player_data->timers.since_jump_timer <= 0.3f*/){ //so we make jump gravity
            // f32 max_height_jump_time = 0.2f;
            // f32 jump_t = clamp01(player_data->timers.since_airborn_timer / max_height_jump_time);
            // player_data->gravity_mult = lerp(3.0f, 1.0f, jump_t * jump_t * jump_t);
            f32 t = player_data->velocity.y / 100.0f;
            player_data->gravity_mult = lerp(1.0f, 3.0f, sqrtf(t));
        } else{
            if (input.sum_direction.y < 0 && !is_player_in_stun()){
                player_data->gravity_mult = 6;
                max_downwards_speed = -150;
            } else{
                player_data->gravity_mult = lerp(1.0f, 0.5f, player_data->sword_spin_progress * player_data->sword_spin_progress);
                if (player_data->velocity.y > 0){
                    f32 up_velocity_t = clamp01(player_data->velocity.y / 200.0f);
                    f32 additional_gravity = lerp(0.0f, 2.0f, up_velocity_t * up_velocity_t);
                    player_data->gravity_mult += additional_gravity;
                }
            }
        }
        
        if (!is_player_in_stun()){
            player_air_move(player_entity, dt);
        }
        
        // In air and in big sword mode we keeping velocity mostly horizontal for more control.
        // This on wall check needs so that our wall boost system worked nice.
        if (player_data->in_big_sword && !player_data->on_wall){
            player_data->velocity.y = lerp(player_data->velocity.y, 0.0f, dt * 10);            
        } else{
            player_data->velocity.y -= player_data->gravity * player_data->gravity_mult * dt;
        }
        
        if (player_data->velocity.y < max_downwards_speed){
            player_data->velocity.y = lerp(player_data->velocity.y, max_downwards_speed, 30 * dt);
        }
         
        player_data->timers.since_airborn_timer += dt;
    }
    
    if (input.press_flags & JUMP){
        // This thing tells about button press time, not about real act of jumping.
        player_data->timers.jump_press_time = core.time.game_time;
        
        if (!player_data->grounded){
            player_data->timers.air_jump_press_time = core.time.game_time;
        }
    }
    
    f32 time_since_jump_press = core.time.game_time - player_data->timers.jump_press_time;
    f32 time_since_air_jump_press = core.time.game_time - player_data->timers.air_jump_press_time;
    
    b32 need_jump = (input.press_flags & JUMP && player_data->grounded)
                 || (player_data->grounded && time_since_air_jump_press <= player_data->jump_buffer_time) 
                 || (input.press_flags & JUMP && player_data->timers.since_airborn_timer <= player_data->coyote_time && player_data->timers.since_jump_timer > player_data->coyote_time);
    
    if (need_jump){
        push_player_up(player_data->jump_force);
    }
    
    Vector2 next_pos = {player_entity->position.x + player_data->velocity.x * dt, player_entity->position.y + player_data->velocity.y * dt};
    
    player_entity->position = next_pos;
    
    f32 found_ground = false;
    f32 just_grounded = false;
    
    f32 wall_acceleration = 400;
    
    // player collisions
    f32 time_since_wall_jump = core.time.game_time - player_data->timers.wall_jump_time;
    f32 player_speed = magnitude(player_data->velocity);
    
    f32 time_since_wall_vertical_boost = core.time.game_time - player_data->timers.wall_enter_vertical_boost_time;
    b32 hit_a_wall = false;
    
    f32 wall_vertical_boost = 100;
    
    local_persist f32 timer_since_on_wall = 0;
    f32 allowed_time_on_wall_without_pushing_back = 0.5f;
    
    // We're giving a vertical boost on entering wall collision for nicer movement. 
    // After some time on wall we're starting to push player away from wall because with current movement player could just climb
    // any inclined wall without that.
    
    // player left wall
    fill_collisions(left_wall_checker, &collisions_buffer, GROUND | CENTIPEDE_SEGMENT | PLATFORM | BLOCKER | SHOOT_BLOCKER);
    for (i32 i = 0; i < collisions_buffer.count && !is_player_in_stun(); i++){
        Collision col = collisions_buffer.get(i);
        
        if (time_since_wall_vertical_boost >= 2.0f && player_data->velocity.y < wall_vertical_boost && player_data->velocity.y != 0 && (input_direction.x * col.normal.x < 0)){
            player_data->velocity.y = wall_vertical_boost;
            player_data->timers.wall_enter_vertical_boost_time = core.time.game_time;
        } else if (timer_since_on_wall >= allowed_time_on_wall_without_pushing_back){
            player_data->velocity += col.normal - Vector2_up;
        }
        hit_a_wall = true;
        break;
    }
    
    // player right wall
    fill_collisions(right_wall_checker, &collisions_buffer, GROUND | CENTIPEDE_SEGMENT | PLATFORM | BLOCKER | SHOOT_BLOCKER);
    for (i32 i = 0; i < collisions_buffer.count && !is_player_in_stun(); i++){
        Collision col = collisions_buffer.get(i);
        
        if (time_since_wall_vertical_boost >= 2.0f && player_data->velocity.y < wall_vertical_boost && player_data->velocity.y != 0 && (input_direction.x * col.normal.x < 0)){
            player_data->velocity.y = wall_vertical_boost;
            player_data->timers.wall_enter_vertical_boost_time = core.time.game_time;
        } else if (timer_since_on_wall >= allowed_time_on_wall_without_pushing_back){
            player_data->velocity += col.normal - Vector2_up;
        }
        hit_a_wall = true;
        break;
    }
    
    player_data->on_wall = hit_a_wall;
    
    f32 wall_timer_modifier = hit_a_wall ? 1 : -1;
    timer_since_on_wall = clamp(timer_since_on_wall + dt * wall_timer_modifier, 0.0f, allowed_time_on_wall_without_pushing_back);
    
    Vector2 last_collision_point = Vector2_zero;
    Vector2 last_collision_normal = Vector2_one;
    
    b32 moving_object_detected = false;
    // player ground checker
    FLAGS player_ground_collision_flags = GROUND | ENEMY_BARRIER | PLATFORM | CENTIPEDE_SEGMENT | NO_MOVE_BLOCK;
    fill_collisions(ground_checker, &collisions_buffer, player_ground_collision_flags);
    b32 is_ground_huge_collision_speed = false;
    b32 found_no_move_block = false;
    for (i32 i = 0; i < collisions_buffer.count && !is_player_in_stun(); i++){
        Collision col = collisions_buffer.get(i);
        Entity *other = col.other_entity;
        assert(col.collided);
        
        f32 dot_velocity = dot(col.normal, player_data->velocity);
        if (dot_velocity >= 0){
            continue;
        }
        
        if (other->flags & PLATFORM && dot(player_data->velocity, other->up) > 0){
            continue;
        }
        
        //now we don't want to stand on projectiles
        if ((other->flags & BLOCKER | SHOOT_BLOCKER) && other->flags & PROJECTILE){
            continue;
        }
        
        if ((other->flags & SHOOT_BLOCKER) && !(other->flags & BLOCKER) && !other->enemy.shoot_blocker_immortal){
            continue;
        }
        
        if (other->flags & ENEMY && can_sword_damage_enemy(other) && !(other->flags & CENTIPEDE_SEGMENT)){
            if (try_sword_damage_enemy(other, col.point)){
                continue;
            }
        }
        
        if (other->flags & NO_MOVE_BLOCK){
            found_no_move_block = true;
        }
        
        if (other->flags & CENTIPEDE_SEGMENT){
            if (other->centipede_head->centipede.spikes_on_right && other->centipede_head->centipede.spikes_on_left){
                kill_player();
                return;
            } else if (!other->centipede_head->centipede.spikes_on_right && !other->centipede_head->centipede.spikes_on_left){
                
            } else{
                Vector2 side = other->centipede_head->centipede.spikes_on_right ? other->right : (other->right * -1.0f);
                f32 side_dot = dot(side, player_entity->position - other->position);
                // so we on side of the centipede segments where are SPIKES
                if (side_dot > 0){
                    kill_player();
                    return;
                }
            }
        }
        
        last_collision_point = col.point;
        last_collision_normal = col.normal;
        
        Vector2 velocity_direction = normalized(player_data->velocity);
        f32 before_speed = magnitude(player_data->velocity);
        
        if (before_speed > 200){
            is_ground_huge_collision_speed = true;
        } 
    
        f32 collision_force_multiplier = 1;
        
        if (other->flags & PHYSICS_OBJECT){             // force
            f32 direction_normal_dot = dot(velocity_direction, col.normal);
            other->physics_object.velocity += ((player_data->velocity * PLAYER_MASS) / other->physics_object.mass) * direction_normal_dot * -1;
            collision_force_multiplier = other->physics_object.mass / PLAYER_MASS;
            player_entity->position += other->physics_object.velocity * dt;
            player_data->on_moving_object = true;
            player_data->moving_object_velocity = other->physics_object.velocity;
            moving_object_detected = true;
        }
        
        if (dot((cast(Vector2){0, 1}), col.normal) > 0.5f){
            player_data->velocity -= col.normal * dot(player_data->velocity, col.normal);
        }
        
        if (other->flags & MOVE_SEQUENCE && other->move_sequence.moving){
            player_entity->position += other->move_sequence.moved_last_frame;
        }
        
        f32 angle = fangle(col.normal, player_entity->up);
        
        if (angle <= player_data->max_ground_angle){
            b32 ceiling_too_close = raycast(player_entity->position + Vector2_up * player_entity->scale.y * 0.5f, Vector2_up, 2.0f, GROUND, 2.0f, player_entity->id).collided;
            if (!ceiling_too_close){
                player_entity->position.y += col.overlap;
            } 
            
            found_ground = true;
            player_data->ground_normal = col.normal;
            player_data->ground_point = col.point;
            
            if (!player_data->grounded && !just_grounded){
                player_snap_to_plane(player_data->ground_normal);
                // player_data->velocity_plane = get_rotated_vector_90(player_data->ground_normal, -normalized(player_data->velocity.x));
                // player_data->velocity = player_data->velocity_plane * magnitude(player_data->velocity);
                just_grounded = true;
                
                //heavy landing
                if (before_speed > 200 && magnitude(player_data->velocity) < 100){
                    player_data->heavy_collision_time = core.time.game_time;
                    player_data->heavy_collision_velocity = player_data->velocity;
                    emit_particles(&ground_splash_emitter, col.point, col.normal, 1, 1.5f);
                    shake_camera(0.7f);
                    
                    play_sound("HeavyLanding", col.point, 1.5f);
                }
            }
        }
    } // player ground checker end
    
    player_data->on_no_move_block = found_no_move_block;
    
    if (!moving_object_detected && player_data->on_moving_object){
        if (dot(player_data->moving_object_velocity, player_data->velocity) > magnitude(player_data->velocity)){
        } else if (dot(player_data->moving_object_velocity, player_data->velocity) > 0){
            player_data->velocity += player_data->moving_object_velocity;   
        }
        player_data->on_moving_object = false;
    }
    
    Particle_Emitter *tires_emitter = get_particle_emitter(player_data->tires_emitter_index);
    if (is_ground_huge_collision_speed){
        tires_volume = lerp(tires_volume, 0.5f, core.time.real_dt * 6.0f);
        SetMusicVolume(tires_theme, tires_volume);
    } else{
        tires_volume = lerp(tires_volume, 0.0f, core.time.real_dt * 15.0f);
        SetMusicVolume(tires_theme, tires_volume);
    }
    
    // player body collision
    fill_collisions(player_entity, &collisions_buffer, GROUND | ENEMY_BARRIER | PROPELLER | CENTIPEDE_SEGMENT | PLATFORM | NO_MOVE_BLOCK);
    
    b32 is_body_huge_collision_speed = false;
    b32 on_propeller = false;
    for (i32 i = 0; i < collisions_buffer.count; i++){
        Collision col = collisions_buffer.get(i);
        Entity *other = col.other_entity;
        assert(col.collided);
        
        //now we don't want to stand on projectiles
        if ((other->flags & BLOCKER | SHOOT_BLOCKER) && other->flags & PROJECTILE){
            continue;
        }
        
        if ((other->flags & SHOOT_BLOCKER) && !(other->flags & BLOCKER) && !other->enemy.shoot_blocker_immortal){
            continue;
        }
        
        if (other->flags & PROPELLER){
            // update propeller
            
            // We're keeping propellers to push player and keeping him in claws for now, because that
            // gives us some room for some things to *make* player do and if player can just leave propeller - this thing literally 
            // don't do anything interesting.
            on_propeller = true;
            Vector2 acceleration_dir = other->up;
            Vector2 deceleration_plane = other->right;
            
            Vector2 to_player = player_entity->position - other->position;
            
            f32 deceleration_sign = dot(to_player, deceleration_plane) > 0 ? -1 : 1;
            
            player_data->velocity = lerp(player_data->velocity, (other->up + deceleration_plane * deceleration_sign * 0.1f) * other->propeller.power, dt * 40);
            continue; 
        }

        
        f32 dot_velocity = dot(col.normal, player_data->velocity);
        if (dot_velocity >= 0){
            continue;
        }
        
        if (other->flags & PLATFORM && dot(player_data->velocity, other->up) > 0){
            continue;
        }
        
        if (other->flags & ENEMY && can_sword_damage_enemy(other) && !(other->flags & CENTIPEDE_SEGMENT)){
            if (try_sword_damage_enemy(other, col.point))
            {
                continue;
            }
        }
        
        if (other->flags & CENTIPEDE_SEGMENT){
            if (other->centipede_head->centipede.spikes_on_right && other->centipede_head->centipede.spikes_on_left){
                kill_player();
                return;
            } else if (!other->centipede_head->centipede.spikes_on_right && !other->centipede_head->centipede.spikes_on_left){
                
            } else{
                Vector2 side = other->centipede_head->centipede.spikes_on_right ? other->right : (other->right * -1.0f);
                f32 side_dot = dot(side, player_entity->position - other->position);
                // so we on side of the centipede segments where are SPIKES
                if (side_dot > 0){
                    kill_player();
                    return;
                }
            }
        }
        
        resolve_collision(player_entity, col);
        
        Vector2 velocity_direction = normalized(player_data->velocity);
        
        f32 before_speed = magnitude(player_data->velocity);
        
        if (before_speed > 200){
            is_body_huge_collision_speed = true;
        }
        
        last_collision_point = col.point;
        last_collision_normal = col.normal;
        
        if (is_player_in_stun()){
            player_data->velocity = reflected_vector(player_data->velocity * 0.5f, col.normal);
            shake_camera(0.2f);
            continue;
        }

        f32 collision_force_multiplier = 1;
        
        if (other->flags & PHYSICS_OBJECT){             // force
            f32 direction_normal_dot = dot(velocity_direction, col.normal);
            other->physics_object.velocity += ((player_data->velocity * PLAYER_MASS) / other->physics_object.mass) * direction_normal_dot * -1;
            collision_force_multiplier = other->physics_object.mass / PLAYER_MASS;
        }
        
        clamp(&collision_force_multiplier, 0, 1.0f);
        
        player_data->velocity -= col.normal * dot(player_data->velocity, col.normal) * collision_force_multiplier;
        
        //heavy collision
        if (before_speed > 200 && magnitude(player_data->velocity) < 100){
            player_data->heavy_collision_time = core.time.game_time;
            player_data->heavy_collision_velocity = player_data->velocity;
            emit_particles(&ground_splash_emitter, col.point, col.normal, 1, 1.5f);
            shake_camera(0.7f);
            play_sound("HeavyLanding", col.point, 1.5f);
        }
    } // end player body collisions
    
    player_data->on_propeller = on_propeller;
    
    if (is_body_huge_collision_speed || is_ground_huge_collision_speed){
        if (tires_emitter){
            tires_emitter->position = last_collision_point;
            tires_emitter->direction = last_collision_normal;
            tires_emitter->count_multiplier = 0.2f;
            enable_emitter(tires_emitter);
        }
    } else{
        disable_emitter(tires_emitter);
    }
    
    b32 just_lost_ground_below_my_feet = player_data->grounded && !found_ground && player_data->timers.since_jump_timer >= 0.5f;
    if (just_lost_ground_below_my_feet && !on_propeller){
        Collision col = raycast(player_entity->position, Vector2_up * -1, player_entity->scale.y + ground_checker->scale.y * 2, player_ground_collision_flags, 0.2f, player_entity->id);
        // That situation for snapping to surface while moving on different normal ground.
        if (col.collided){
            Vector2 next_velocity_plane = get_move_plane(col.normal, player_data->velocity.x);
            f32 old_new_velocity_plane_dot = dot(next_velocity_plane, player_data->velocity_plane);
            if (col.normal != player_data->ground_normal && old_new_velocity_plane_dot >= 0.9f){
                found_ground = true;
                player_snap_to_plane(col.normal);
                player_data->velocity -= col.normal;
            }
        } else if (input_direction.x == 0 && player_data->velocity.x < player_data->ground_walk_speed * 0.5f){
            // That for stopping and not falling when on edge and player not holding key forward.
            player_data->velocity = player_data->velocity * -0.8f;;
        }
    }
    player_data->grounded = found_ground;
    
    if (player_data->sword_hit_ground){
        enable_emitter(sword_tip_ground_emitter);
        f32 t = blood_t * spin_t;
        sword_tip_ground_emitter->speed_multiplier = lerp(0.7f, 4.0f, t * t * t);
        sword_tip_ground_emitter->count_multiplier = lerp(0.45f, 1.0f, t * t * t);
    } else{
        sword_tip_ground_emitter->enabled = false;
    }
    
    player_data->sword_hit_ground = false;
    
    f32 wind_t = clamp01(magnitude(player_data->velocity) / 300.0f);
    SetMusicVolume(wind_theme, lerp(0.0f, 1.0f, wind_t * wind_t));
    
    update_player_connected_entities_positions(player_entity);
} // update player end

inline void calculate_collisions(void (respond_func)(Entity*, Collision), Entity *entity){
    fill_collisions(entity, &collisions_buffer, entity->collision_flags);
    
    for (i32 i = 0; i < collisions_buffer.count; i++){
        Collision col = collisions_buffer.get(i);
        respond_func(entity, col);
    }
}

void respond_physics_object_collision(Entity *entity, Collision col){
    Physics_Object *physics_object = &entity->physics_object;
    Entity *other = col.other_entity;
    f32 speed   = magnitude(physics_object->velocity);
    f32 speed_t = clamp01(speed / 300.0f);
    Vector2 direction = normalized(physics_object->velocity);
    
    b32 is_high_velocity = speed > 100;
    
    if (other->flags & GROUND){
        resolve_collision(entity, col);
        
        if (entity->flags & EXPLOSIVE){
            kill_enemy(entity, col.point, direction, lerp(1.0f, 2.0f, speed_t * speed_t));
        } else{
            physics_object->velocity = reflected_vector(physics_object->velocity * 0.5f, col.normal);
        }
        
        if (is_high_velocity){
            emit_particles(&rifle_bullet_emitter, col.point, direction, lerp(0.5f, 2.0f, speed_t * speed_t), 2);
            play_sound("BirdToGround", col.point, 0.5f);
        }
        
        if (physics_object->rotate_by_velocity && col.normal.y >= 0){
            change_up(entity, move_towards(entity->up, col.normal, speed, core.time.fixed_dt));            
        }
    }        
    
    if (other->flags & PLAYER){
        resolve_collision(entity, col);
        f32 force = dot(((physics_object->velocity - player_data->velocity)* physics_object->mass) / PLAYER_MASS, col.normal * -1);   
        // if (physics_object->mass >= PLAYER_MASS && force > 1000){
            // kill_player();
        resolve_physics_collision(&physics_object->velocity, physics_object->mass, &player_data->velocity, PLAYER_MASS, col.normal);
        // player_data->velocity += (physics_object->velocity - player_data->velocity) * 1.1f;
        // } //else{
        // player_data->velocity += physics_object->velocity - player_data->velocity;
        // }
    } else if (other->flags & BIRD_ENEMY){
        f32 force = dot(((physics_object->velocity - other->bird_enemy.velocity)* physics_object->mass) / 5, col.normal * -1);   
        if (physics_object->mass >= 5 && force > 1000){
            kill_enemy(other, col.point, direction, force / 200);
            play_sound("SwordKill", col.point, 0.5f);
        } else{
            resolve_collision(entity, col);
            // other->bird_enemy.velocity += direction * force / 100;
        }
        resolve_physics_collision(&physics_object->velocity, physics_object->mass, &other->bird_enemy.velocity, 5, col.normal);
    } else if (other->flags & ENEMY){
        // Right now we don't give velocity to entities (including player) that standing on some moving thing (physics object 
        // currently). So when jump shooter sticking on this one - he'll just crush it and we done.
        // We check for if jump shooter in jumping state also because if physics block moving fast and jump shooter jump off him - 
        // he can just reach jump shooter and kill him. And I don't want that to happen.
        b32 is_jump_shooter_standing_on_me = other->flags & JUMP_SHOOTER && (other->jump_shooter.states.standing || other->jump_shooter.states.jumping) && other->jump_shooter.standing_on_physics_object_id == entity->id;
        
        b32 should_not_crush_enemy = is_jump_shooter_standing_on_me;
        
        if (!should_not_crush_enemy){
            f32 force = dot(((physics_object->velocity) * physics_object->mass) / 5, col.normal * -1);   
            if (physics_object->mass >= 5 && force > 1000){
                kill_enemy(other, col.point, direction, force / 200);
                play_sound("SwordKill", col.point, 0.5f);
            } 
            Vector2 fictional_velocity = Vector2_zero;
            resolve_physics_collision(&physics_object->velocity, physics_object->mass, &fictional_velocity, 0.1f, col.normal);
        }
    }
}

void respond_jump_shooter_collision(Entity *shooter_entity, Collision col){
    assert(shooter_entity->flags & JUMP_SHOOTER);

    Jump_Shooter *shooter = &shooter_entity->jump_shooter;
    Enemy *enemy = &shooter_entity->enemy;
    Entity *other = col.other_entity;
    f32 speed   = magnitude(shooter->velocity);
    f32 speed_t = clamp01(speed / 300.0f);
    
    b32 is_high_velocity = speed > 100;
    
    b32 should_respond = true;
    
    if (other->flags & PHYSICS_OBJECT){
        apply_physics_force(shooter->velocity, 10, &other->physics_object, col.normal);
    }
    
    if (!enemy->dead_man && other->flags & PLAYER && shooter->states.flying_to_point){
        if (can_sword_damage_enemy(shooter_entity)){
            try_sword_damage_enemy(shooter_entity, shooter_entity->position);
        } else{
            kill_player();
        }
    }
    
    if (other->flags & GROUND){
        resolve_collision(shooter_entity, col);
        
        if (enemy->dead_man){
            emit_particles(&fire_emitter, shooter_entity->position, col.normal, 4, 3);
            play_sound("Explosion", shooter_entity->position, 0.3f);
            add_explosion_light(shooter_entity->position, rnd(100.0f, 250.0f), 0.15f, 0.4f, ColorBrightness(RED, 0.5f));
            shooter_entity->destroyed = true;
            shooter_entity->enabled = false;
            shake_camera(0.6f);
            return;
        }
        
        // jump shooter stop on ground
        if (shooter->states.flying_to_point){
            shooter->current_index = (shooter->current_index + 1) % shooter->move_points.count;
            shooter->states.flying_to_point = false;
            shooter->states.standing = true;
            shooter->states.standing_start_time = core.time.game_time;
            shooter->velocity = Vector2_zero;
            emit_particles(&ground_splash_emitter, col.point, col.normal, 6, 2.5f);
            
            disable_emitter(shooter->flying_emitter_index);
        } else if (!shooter->states.standing){
            shooter->velocity = reflected_vector(shooter->velocity * 0.7f, col.normal);
            emit_particles(&ground_splash_emitter, col.point, col.normal, 1, 0.5f);
            
            if (enemy->was_in_stun){
                enemy->stun_start_time = -234;
            }
        }
    }
}

void respond_bird_collision(Entity *bird_entity, Collision col){
    assert(bird_entity->flags & BIRD_ENEMY);

    Bird_Enemy *bird = &bird_entity->bird_enemy;
    Enemy *enemy = &bird_entity->enemy;
    Entity *other = col.other_entity;
    f32 bird_speed = magnitude(bird->velocity);
    f32 bird_speed_t = clamp01(bird_speed / 300.0f);
    
    b32 is_high_velocity = bird_speed > 100;
    
    b32 should_respond = true;
    if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT || other->flags & ENEMY_BARRIER){
        resolve_collision(bird_entity, col);
        
        if (other->flags & PHYSICS_OBJECT){
            f32 collision_force = apply_physics_force(bird->velocity, 5, &other->physics_object, col.normal);
            
            if (collision_force <= 0.5f){
                should_respond = false;
            }
        }
        
        if (bird->attacking && other->flags & BIRD_ENEMY){
            should_respond = false;            
        }
        
        b32 exploded = false;
        if (bird->attacking && bird_entity->flags & EXPLOSIVE && !(bird_entity->flags & BLOCKER) && !(other->flags & BIRD_ENEMY)){
            kill_enemy(bird_entity, col.point, col.normal);
            exploded = true;
        }
        
        if (should_respond){
            if (enemy->dead_man){
                emit_particles(&fire_emitter, bird_entity->position, col.normal, 2, 3);
                play_sound("Explosion", bird_entity->position, 0.3f);
                
                add_explosion_light(bird_entity->position, rnd(75.0f, 200.0f), 0.1f, 0.3f, ColorBrightness(ORANGE, 0.5f));
                
                bird_entity->destroyed = true;
                bird_entity->enabled = false;
                shake_camera(0.6f);
                return;
            }
            
            bird->velocity = reflected_vector(bird->velocity * 0.9f, col.normal);
            if (bird->attacking){
                bird->attacking = false;
                disable_emitter(bird->attack_emitter_index);
                disable_emitter(bird->alarm_emitter_index);
                bird->roaming = true;
                bird->attacked_time = core.time.game_time;
                bird->roam_start_time = core.time.game_time;
            }
        }
        
        emit_particles(bird->collision_emitter_index, col.point, normalized(bird->velocity), lerp(0.5f, 2.0f, bird_speed_t * bird_speed_t), lerp(5, 20, bird_speed_t * bird_speed_t));
        
        if (is_high_velocity){
            play_sound("BirdToGround", col.point, 0.5f);
        }
    }
    
    if (other->flags & BIRD_ENEMY && !bird->attacking){
        resolve_collision(bird_entity, col);
        
        if (enemy->dead_man){
            emit_particles(&fire_emitter, bird_entity->position, col.normal, 2, 3);
            stun_enemy(other, other->position, col.normal);
        }
        
        bird->velocity              = reflected_vector(bird->velocity * 0.8f, col.normal);
        other->bird_enemy.velocity += reflected_vector(bird->velocity * 0.3f, col.normal * -1);
        
        emit_particles(bird->collision_emitter_index, col.point, normalized(bird->velocity), 0.5f, 1);
        
        if (is_high_velocity){
            play_sound("BirdToBird", col.point, 0.5f);
        }
    }
    
    if (other->flags & PLAYER && !player_data->dead_man && bird->attacking && !enemy->dead_man){
        if (should_kill_player(bird_entity)){
            kill_player();
            if (bird_entity->flags & EXPLOSIVE){
                kill_enemy(bird_entity, col.point, col.normal);
            }
        } else{
            try_sword_damage_enemy(bird_entity, bird_entity->position);
        }
    }
}

void move_by_velocity_with_collisions(Entity *entity, Vector2 velocity, f32 max_frame_move_len, void (respond_collision_func)(Entity*, Collision), f32 dt){
    Vector2 this_frame_move_direction = normalized(velocity);
    f32 this_frame_move_len = magnitude(velocity * dt); 
    
    if (this_frame_move_len > max_frame_move_len * 10){
        print("PHYSICS ERROR: Some objects moves too fast and will require heavy simulation, so it stopped.");
        return;
    }
    
    while(this_frame_move_len > max_frame_move_len){
        entity->position += this_frame_move_direction * max_frame_move_len;
        calculate_collisions(respond_collision_func, entity);
        this_frame_move_len -= max_frame_move_len;
        this_frame_move_direction = normalized(velocity);
    }
    
    entity->position += this_frame_move_direction * this_frame_move_len;
    calculate_collisions(respond_collision_func, entity);
}

void update_bird_enemy(Entity *entity, f32 dt){
    assert(entity->flags & BIRD_ENEMY);
    assert(entity->flags & ENEMY);
    
    Bird_Enemy *bird = &entity->bird_enemy;
    Enemy *enemy = &entity->enemy;
    
    if (!entity->enemy.in_agro && !enemy->dead_man){
        return;
    } else if (entity->enemy.just_awake){
        entity->enemy.just_awake = false;
        bird->roaming = true;
        bird->roam_start_time = core.time.game_time;
        enemy->birth_time = core.time.game_time;
    }
    
    if (entity->flags & MOVE_SEQUENCE){
        entity->move_sequence.moving = false;
    }
    
    Vector2 vec_to_player = player_entity->position - entity->position;
    Vector2 dir_to_player = normalized(vec_to_player);
    f32    distance_to_player = magnitude(vec_to_player);
    
    if (enemy->dead_man){
        bird->velocity.y -= player_data->gravity * dt;
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
        rotate(entity, bird->velocity.x);
        bird_clear_formation(bird);
        
        f32 since_died_time = core.time.game_time - enemy->died_time;
        if (since_died_time >= 15 && sqr_magnitude(entity->position - player_entity->position) >= 50000){
            kill_enemy(entity, entity->position, entity->up);
        }
        return;
    }

    f32 in_stun_time = core.time.game_time - enemy->stun_start_time;
    
    if (core.time.game_time > 3 && in_stun_time <= enemy->max_stun_time){
        rotate(entity, 0.2f * bird->velocity.x);
        bird->velocity = move_towards(bird->velocity, Vector2_zero, magnitude(bird->velocity) * 1.0f, dt);
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
    
        bird->roaming = true;
        bird->roam_start_time = core.time.game_time;
        bird->charging = false;
        bird->attacking = false;
        disable_emitter(bird->attack_emitter_index);
        disable_emitter(bird->alarm_emitter_index);
        bird_clear_formation(bird);
        return;
    }
    
    //update bird states
    if (bird->roaming){
        f32 roam_time = core.time.game_time - bird->roam_start_time;
        f32 max_roam_time = bird->max_roam_time;
        
        if (bird->slot_index != -1){
            max_roam_time *= 0.5f;
        }
        
        if (roam_time >= max_roam_time){
            bird->roaming = false;
            bird->charging = true;
            bird->charging_start_time = core.time.game_time;
        }
    }
    
    if (bird->charging){
        f32 charging_time = core.time.game_time - bird->charging_start_time;
        if (charging_time >= bird->max_charging_time){
            f32 time_since_last_bird_attacked = core.time.game_time - state_context.timers.last_bird_attack_time;
            
            if (time_since_last_bird_attacked >= 0.4f){
                //bird start attack
                state_context.timers.last_bird_attack_time = core.time.game_time;
                change_scale(entity, entity->enemy.original_scale);
            
                change_up(entity, dir_to_player);         
                bird->charging = false;
                bird->attacking = true;
                bird->attack_start_time = core.time.game_time;
                
                f32 bird_attack_speed = 300;
                bird->velocity = dir_to_player * bird_attack_speed;
                
                emit_particles(&attack_sparks_emitter, entity->position, entity->up, 2, 3);
                emit_particles(bird->alarm_emitter_index, entity->position, entity->up);
                
                enable_emitter(bird->attack_emitter_index);
                enable_emitter(bird->alarm_emitter_index);
                
                play_sound("BirdAttack", entity->position);
            }
        } 
    }
    
    if (bird->attacking){
        f32 attacking_time = core.time.game_time - bird->attack_start_time;
        
        if (attacking_time >= bird->max_attack_time){
            bird->attacking = false;
            bird->roaming = true;
            bird->roam_start_time = core.time.game_time;
            disable_emitter(bird->attack_emitter_index);
            disable_emitter(bird->alarm_emitter_index);
            bird->attacked_time = core.time.game_time;
        } 
    }
    
    f32 bird_speed = magnitude(bird->velocity);
    
    f32 time_since_attacked = core.time.game_time - bird->attacked_time;
    
    //update bird
    if (bird->roaming){
        f32 roam_time = core.time.game_time - bird->roam_start_time;
        f32 roam_t = roam_time / bird->max_roam_time;
    
        if (roam_t <= 0.2f && time_since_attacked < 4){
            rotate(entity, bird_speed * 0.2f * normalized(bird->velocity.x));
            bird->velocity = move_towards(bird->velocity, Vector2_zero, bird_speed * 0.8f, dt);
        } else{
            f32 distance_t = clamp01(distance_to_player / 300.0f);
            f32 acceleration = lerp(bird->roam_acceleration * 0.5f, bird->roam_acceleration, distance_t * distance_t);
            f32 max_speed = lerp(bird->max_roam_speed * 0.5f, bird->max_roam_speed, distance_t);
        
            Vector2 target_position = player_entity->position + Vector2_up * 120;        
            if (bird->slot_index == -1){
                for (i32 i = 0; i < MAX_BIRD_POSITIONS; i++){
                    Bird_Slot *slot = &current_level_context->bird_slots[i];
                    
                    if (!slot->occupied){
                        slot->occupied = true;
                        bird->slot_index = i;
                        break;
                    }
                }
            }
            
            if (bird->slot_index != -1){
                target_position = player_entity->position + bird_formation_positions[bird->slot_index];
            } else{
                // If bird could not find slot - it will not attack and wait in roaming until slot is freed
                bird->roam_start_time = core.time.game_time;
                roam_time = 0;
            }
        
            bird->target_position = target_position;
            f32 damping = 1.0f;
            bird->velocity *= 1.0f - (damping * dt);
            bird->velocity += (bird->target_position - entity->position) * acceleration * dt;
            clamp_magnitude(&bird->velocity, max_speed);
            change_up(entity, move_towards(entity->up, bird->velocity, bird_speed, dt));         
        }
        
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
    } else if (bird->charging){
        // bird_clear_formation(bird);
        
        f32 charging_time = core.time.game_time - bird->charging_start_time;
        f32 t = clamp01(charging_time / bird->max_charging_time);
        
        change_scale(entity, lerp(entity->enemy.original_scale, {entity->enemy.original_scale.x * 1.2f, entity->enemy.original_scale.y * 2.0f}, t * t));
        
        bird->velocity = move_towards(bird->velocity, Vector2_zero, bird_speed * 0.8f, dt);
        if (t < 0.4f){
            rotate(entity, bird_speed * 0.2f * normalized(bird->velocity.x));
        } else{
            change_up(entity, move_towards(entity->up, dir_to_player, lerp(0.0f, 30.0f, t * t), dt));         
            f32 charging_back_movement_amount = 3.0f;
            entity->position -= entity->up * charging_back_movement_amount * dt;
        }
        
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
    } else if (bird->attacking){
        f32 attacking_time = core.time.game_time - bird->attack_start_time;
        
        bird_clear_formation(bird);
    
        f32 speed = magnitude(bird->velocity);
        change_up(entity, move_towards(entity->up, dir_to_player, 2, dt));
        bird->velocity = entity->up * speed;
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
        
        if (is_enemy_should_trigger_death_instinct(entity, bird->velocity, dir_to_player, distance_to_player, true)){
            start_death_instinct(entity, ENEMY_ATTACKING);          
        }
        
        f32 attack_line_t = clamp01(attacking_time / 0.5f);
    } else{
        assert(false);
        //what a state
    }
    
    Particle_Emitter *trail_emitter = get_particle_emitter(bird->trail_emitter_index);
    if (trail_emitter){
        trail_emitter->direction = entity->up * -1;
    }
}

inline f32 get_explosion_radius(Entity *entity, f32 base_radius){
    assert(entity->flags & EXPLOSIVE);

    base_radius *= entity->enemy.explosive_radius_multiplier;

    f32 scale_sum = (entity->scale.x * 0.5f + entity->scale.y * 0.5f);
    f32 scale_progress = clamp01(scale_sum / 200.0f);
    
    return lerp(base_radius, base_radius * 6, scale_progress);
}

b32 is_explosion_trauma_active(){
    return core.time.app_time - state_context.timers.background_flash_time <= 0.1f;
}

void add_explosion_trauma(f32 explosion_radius){
    if (explosion_radius < 100 || is_explosion_trauma_active()){
        return;
    }
    
    state_context.explosion_trauma += explosion_radius / 500.0f;
    if (state_context.explosion_trauma >= 1){
        state_context.timers.background_flash_time = core.time.app_time;
        state_context.explosion_trauma = 0;
    }
}

void add_explosion_light(Vector2 position, f32 radius, f32 grow_time, f32 shrink_time, Color color, i32 size, i32 entity_id){
    add_explosion_trauma(radius);
    
    Light *light = NULL;
    
    i32 start_index = session_context.big_temp_lights_count + session_context.huge_temp_lights_count;
    i32 max_count_to_seek = session_context.temp_lights_count;
    if (size >= BIG_LIGHT){ //Means we set huge light
        max_count_to_seek = session_context.big_temp_lights_count + session_context.huge_temp_lights_count;   
        start_index = session_context.big_temp_lights_count;
    } else if (size >= MEDIUM_LIGHT){ //Means we set big light
        max_count_to_seek = session_context.big_temp_lights_count;   
        start_index = 0;
    } else{
    }
    
    for (i32 i = start_index; i < max_count_to_seek; i++){
        if (!current_level_context->lights.get_ptr(i)->exists){
            light = current_level_context->lights.get_ptr(i);
            break;
        }
    }
    
    if (light){
        light->birth_time    = core.time.game_time;
        light->target_radius = radius;
        light->grow_time     = grow_time;
        light->shrink_time   = shrink_time;
        light->color         = color;
        light->opacity       = (f32)color.a / 255.0f;
        light->start_opacity = light->opacity;
        light->exists        = true;
        light->position      = position;
        
        light->additional_shadows_flags = ENEMY | PLAYER | SWORD;
        light->connected_entity_id = entity_id;
    } else{
        print("WARNING: Could not find temp light for explosion");
    }
}

void kill_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, b32 can_wait, f32 particles_speed_modifier){
    assert(enemy_entity->flags & ENEMY);
    
    Enemy *enemy = &enemy_entity->enemy;
    if (1 || !enemy->dead_man){
        if (can_wait){
            if (enemy_entity->flags & EXPLOSIVE && core.time.app_time - state_context.timers.last_explosion_app_time < 0.01f){
                enemy->should_explode = true;
                return;
            }
        }
    
        enemy->stun_start_time = core.time.game_time;
        emit_particles(get_sword_kill_particle_emitter(enemy_entity), kill_position, kill_direction, 1, particles_speed_modifier, 1);
    
        enemy->dead_man = true;
        enemy->died_time = core.time.game_time;
        b32 should_not_be_destroyed = (enemy_entity->flags & (TRIGGER | CENTIPEDE_SEGMENT));
        if (!should_not_be_destroyed){
            enemy_entity->enabled = false;
            enemy_entity->destroyed = true;
    
            if (enemy_entity->flags & SHOOT_STOPER){
                // assert(state_context.shoot_stopers_count >= 0);
                if (state_context.shoot_stopers_count > 0){
                    state_context.shoot_stopers_count--;
                } else{
                    print("WARNING: Shoot stopers count could go below zero. That may be because we skipped trigger and kill it, so no assertion, just warning");            
                }
            }
        }
        
        // kill switch death
        if (enemy_entity->flags & KILL_SWITCH){
            Kill_Switch *kill_switch = &enemy->kill_switch;
            for (i32 i = 0; i < kill_switch->connected.count; i++){
                Entity *connected = get_entity_by_id(kill_switch->connected.get(i));
                if (!connected){
                    continue;
                }
                
                kill_enemy(connected, connected->position, connected->up);
            }
        }
        
        if (enemy_entity->flags & MOVE_SEQUENCE && !(enemy_entity->flags & CENTIPEDE_SEGMENT)){
            enemy_entity->move_sequence.moving = false;
        }
        
        // explosive kill explosive
        if (enemy_entity->flags & EXPLOSIVE){
            state_context.timers.last_explosion_app_time = core.time.app_time;
            f32 explosion_radius = get_explosion_radius(enemy_entity);
            
            Particle_Emitter *explosion_emitter = &explosion_emitter_copy;
            if (enemy->explosive_radius_multiplier > 1.5f){
                explosion_emitter = &big_explosion_emitter_copy;
            }
            
            emit_particles(explosion_emitter, enemy_entity->position, Vector2_up, explosion_radius / 40.0f);
            play_sound("BigExplosion", enemy_entity->position, 0.5f);
            
            i32 light_size_flag = SMALL_LIGHT;
            if (enemy_entity->light_index != -1) light_size_flag = current_level_context->lights.get(enemy_entity->light_index).shadows_size_flags;
            add_explosion_light(enemy_entity->position, explosion_radius * rnd(3.0f, 6.0f), 0.15f, fminf(enemy->explosive_radius_multiplier, 3.0f), ColorBrightness(ORANGE, 0.3f), light_size_flag);
            
            f32 explosion_add_speed = 80;
            i32 spawned_particles_count = 0;
            ForEntities(other_entity, 0){
                Vector2 vec_to_other = other_entity->position - enemy_entity->position;
                f32 distance_to_other = magnitude(vec_to_other);
                
                if (distance_to_other > explosion_radius){
                    continue;
                }
                
                Vector2 dir_to_other = normalized(vec_to_other);
                
                Collision obstacle_collision = raycast(enemy_entity->position, dir_to_other, distance_to_other - 1, GROUND | CENTIPEDE_SEGMENT | CENTIPEDE, distance_to_other - 2, enemy_entity->id);
                if (obstacle_collision.collided){
                    if (spawned_particles_count < 3){
                        emit_particles(&ground_splash_emitter, obstacle_collision.point, obstacle_collision.normal, 4, 5.5f);
                        spawned_particles_count += 1;
                    }
                    continue;
                }
                
                if (other_entity->flags & ENEMY){
                    if (!other_entity->enemy.dead_man){
                        stun_enemy(other_entity, other_entity->position, dir_to_other, true);
                    }
                    
                    if (other_entity->flags & BIRD_ENEMY){
                        other_entity->bird_enemy.velocity += dir_to_other * explosion_add_speed;
                    }
                    if (other_entity->flags & JUMP_SHOOTER){
                        other_entity->jump_shooter.velocity += dir_to_other * explosion_add_speed;
                    }
                } else if (other_entity->flags & PROJECTILE){
                    other_entity->destroyed = true;
                }
                
                if (other_entity->flags & PHYSICS_OBJECT){
                    other_entity->physics_object.velocity += (dir_to_other * explosion_add_speed * (explosion_radius * 0.1f)) / other_entity->physics_object.mass;
                }
                
                if (other_entity->flags & BLOCK_ROPE){
                    cut_rope(other_entity);
                }
                
                if (other_entity->flags & PLAYER && !player_data->dead_man && distance_to_other < explosion_radius * 0.75f){
                    kill_player();
                }
            }
            
            add_hitstop(0.1f * fmaxf(1.0f, enemy->explosive_radius_multiplier * 0.5f), true);
            shake_camera(0.5f * fmaxf(1.0f, enemy->explosive_radius_multiplier * 0.5f));
            
            // centipede explode segments
            if (enemy_entity->flags & CENTIPEDE_SEGMENT){
                // If we don't explode all segments at once then weird things occur when some segments in ground.
                Centipede *head = &enemy_entity->centipede_head->centipede;
                if (!head->all_segments_dead){
                    head->all_segments_dead = true;
                    for (i32 i = 0; i < head->segments_ids.count; i++){
                        Entity *segment = get_entity_by_id(head->segments_ids.get(i));
                        if (segment && segment->id != enemy_entity->id){
                            kill_enemy(segment, segment->position, segment->up);
                        } else if (!segment){
                            print("WARNING: For some reason on exploding all centipede segments some segment was not here AT ALL!!");
                        }
                    }
                }
            }
        } // kill explosive end
    }
}

inline b32 is_enemy_can_take_damage(Entity *enemy_entity, b32 check_for_last_hit_time){
    assert(enemy_entity->flags & ENEMY);

    if (enemy_entity->flags & CENTIPEDE_SEGMENT && enemy_entity->enemy.dead_man){
        return false;
    }
    
    if (enemy_entity->enemy.unkillable){
        return false;
    }
    
    if (enemy_entity->flags & TRIGGER && enemy_entity->enemy.dead_man){
        return false;
    }
    
    if (!check_for_last_hit_time){
        return true;
    }
    
    f32 immune_time = 0.2f;
    if (enemy_entity->flags & MULTIPLE_HITS){
        immune_time = 0.078f;
    }
    
    b32 recently_got_hit = (core.time.game_time - enemy_entity->enemy.last_hit_time) <= immune_time;
    return !recently_got_hit;
}

void agro_enemy(Entity *entity){
    if (entity->enemy.in_agro || entity->enemy.dead_man){
        return;
    }

    entity->enemy.in_agro = true;
    
    add_explosion_light(entity->position, (entity->scale.y + entity->scale.x) * 10, 0.1f, 2.2f, Fade(ColorBrightness(RED, 0.5f), 0.2f), SMALL_LIGHT, entity->id);
    
    if (entity->flags & SHOOT_STOPER){
        state_context.shoot_stopers_count++;
    }
}

void add_fire_light_to_entity(Entity *entity){
    Light *new_fire_light = init_entity_light(entity, NULL, true);
    if (new_fire_light){
        new_fire_light->make_shadows = false;
        new_fire_light->make_backshadows = false;
        new_fire_light->shadows_size_flags = MEDIUM_LIGHT;
        new_fire_light->backshadows_size_flags = MEDIUM_LIGHT;
        new_fire_light->color = ColorBrightness(ORANGE, 0.4f);
        new_fire_light->fire_effect = true;
        // entity->flags |= LIGHT;
    }
}

inline Particle_Emitter *get_sword_kill_particle_emitter(Entity *enemy_entity){
    Particle_Emitter *emitter = &blood_pop_emitter_copy;
    // Should just do a enemy flag for serious enemies instead of picking everyone individualy.
    if (enemy_entity->flags & BIRD_ENEMY){
        emitter = &sword_kill_medium_emitter_copy;
    } else if (enemy_entity->flags & JUMP_SHOOTER){
        emitter = &sword_kill_big_emitter_copy;
    }
    
    return emitter;
}

void stun_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, b32 serious){
    assert(enemy_entity->flags & ENEMY);
    
    Enemy *enemy = &enemy_entity->enemy;
    
    if (enemy_entity->flags & EXPLOSIVE){
        kill_enemy(enemy_entity, kill_position, kill_direction);
        return;
    }
    
    if (1 || is_enemy_can_take_damage(enemy_entity)){
        if (enemy_entity->flags & MOVE_SEQUENCE && !(enemy_entity->flags & CENTIPEDE_SEGMENT)){
            enemy_entity->move_sequence.moving = false;
        }
        agro_enemy(enemy_entity);
    
        enemy->stun_start_time = core.time.game_time;
        enemy->last_hit_time   = core.time.game_time;
        enemy->hits_taken++;
        b32 should_die_in_one_hit = enemy_entity->flags & BIRD_ENEMY && enemy_entity->bird_enemy.attacking;
        
        if ((enemy->hits_taken >= enemy->max_hits_taken || serious || should_die_in_one_hit) && !enemy->dead_man){
            emit_particles(get_sword_kill_particle_emitter(enemy_entity), kill_position, kill_direction, 1, 1, 1);
        
            enemy->dead_man = true;
            enemy->died_time = core.time.game_time;
        
            if (enemy_entity->flags & BIRD_ENEMY){
                // birds handle dead state by themselves
                enable_emitter(enemy_entity->bird_enemy.fire_emitter_index, enemy_entity->position);
                enable_emitter(enemy_entity->bird_enemy.smoke_fire_emitter_index, enemy_entity->position);
                add_fire_light_to_entity(enemy_entity);
            } else if (enemy_entity->flags & JUMP_SHOOTER){
                Particle_Emitter *dead_fire_emitter = get_particle_emitter(add_entity_particle_emitter(enemy_entity, &fire_emitter));
                if (dead_fire_emitter){
                    dead_fire_emitter->position = enemy_entity->position;
                    enable_emitter(dead_fire_emitter);
                }
                add_fire_light_to_entity(enemy_entity);
            } else if (enemy_entity->flags & CENTIPEDE_SEGMENT){
            } else{
                kill_enemy(enemy_entity, kill_position, kill_direction);
            }
        } else{
            enemy->stun_start_time = core.time.game_time;
            enemy->last_hit_time   = core.time.game_time;
        }
        add_hitmark(enemy_entity, true); 
    }
}

void add_rifle_projectile(Vector2 start_position, Vector2 velocity){
    Entity *projectile_entity = add_entity(start_position, {2, 2}, {0.5f, 0.5f}, 0, PINK, PROJECTILE | PARTICLE_EMITTER);
    projectile_entity->projectile.type  = PLAYER_RIFLE_PROJECTILE;
    projectile_entity->projectile.birth_time = core.time.game_time;
    projectile_entity->projectile.velocity = velocity;
    projectile_entity->projectile.max_lifetime = 7;
    
    projectile_entity->projectile.trail_emitter_index = add_and_enable_entity_particle_emitter(projectile_entity, &bullet_trail_emitter_copy, start_position, true);
    add_and_enable_entity_particle_emitter(projectile_entity, &bullet_trail_emitter_copy, start_position, true);
    add_and_enable_entity_particle_emitter(projectile_entity, &magical_sparks_emitter_copy, start_position, true);
    add_and_enable_entity_particle_emitter(projectile_entity, &white_sparks_emitter_copy, start_position, true);
}

inline Vector2 transform_texture_scale(Texture texture, Vector2 wish_scale){
    Vector2 real_scale = {(f32)texture.width / current_level_context->cam.unit_size, (f32)texture.height / current_level_context->cam.unit_size};
    
    return {wish_scale.x / real_scale.x, wish_scale.y / real_scale.y};
}

void add_hitmark(Entity *entity, b32 need_to_follow, f32 scale_multiplier, Color tint){
    Entity *hitmark = add_entity(entity->position, transform_texture_scale(hitmark_small_texture, {45, 45}) * scale_multiplier, {0.5f, 0.5f}, rnd(-90.0f, 90.0f), hitmark_small_texture, TEXTURE | STICKY_TEXTURE);
    hitmark->need_to_save = false;
    init_entity(hitmark);    
    change_color(hitmark, tint);
    hitmark->draw_order = 1;
    str_copy(hitmark->name, "hitmark_small");
    
    hitmark->sticky_texture.need_to_follow   = need_to_follow;
    hitmark->sticky_texture.draw_line        = true;
    hitmark->sticky_texture.follow_id        = entity->id;
    hitmark->sticky_texture.birth_time       = core.time.game_time;
    hitmark->sticky_texture.should_draw_until_expires = true;
    hitmark->sticky_texture.max_distance     = 1000;
    hitmark->sticky_texture.base_size = hitmark->scale;
}

Vector2 get_entity_velocity(Entity *entity){
    if (entity->flags & PLAYER){
        return player_data->velocity;
    }
    if (entity->flags & BIRD_ENEMY){
        return entity->bird_enemy.velocity;
    }
    if (entity->flags & JUMP_SHOOTER){    
        return entity->jump_shooter.velocity;
    }
    if (entity->flags & PHYSICS_OBJECT){
        return entity->physics_object.velocity;
    }
    if (entity->flags & PROJECTILE){
        return entity->projectile.velocity;
    }
    return Vector2_zero;
}

inline b32 compare_difference(f32 first, f32 second, f32 allowed_difference = EPSILON){
    return abs(first - second) <= allowed_difference;
}

inline f32 get_death_instinct_radius(Vector2 threat_scale){
    // return fmaxf(fminf((18.0f / current_level_context->cam.cam2D.zoom), 60), 40) * fmaxf(magnitude(velocity) / 200.0f, 1.0f);
    return 40 + (threat_scale.x + threat_scale.y) - 8.0f;
}

b32 is_death_instinct_threat_active(){
    Entity *threat_entity = get_entity_by_id(state_context.death_instinct.threat_entity_id);
    b32 entity_alive = threat_entity && !threat_entity->destroyed && !threat_entity->enemy.dead_man;
    
    f32 since_death_instinct = core.time.app_time - state_context.death_instinct.start_time;
    b32 is_no_cooldown_on_stop = since_death_instinct <= state_context.death_instinct.allowed_duration_without_cooldown; 
    
    if (entity_alive){
        switch (state_context.death_instinct.last_reason){
            case ENEMY_ATTACKING:{
                Vector2 vec_to_player = player_entity->position - threat_entity->position;
                Vector2 dir_to_player = normalized(vec_to_player);
                f32 distance_to_player = magnitude(vec_to_player);
                
                // We want instinct to stop if player evaded enemy in the beginning, but if we'll do that with cooldown there
                // could be confusions.
                b32 check_for_flying_towards = is_no_cooldown_on_stop; 
                return is_enemy_should_trigger_death_instinct(threat_entity, get_entity_velocity(threat_entity), dir_to_player, distance_to_player, check_for_flying_towards);
            } break;
            case SWORD_WILL_EXPLODE:{
                return player_data->is_sword_will_hit_explosive;     
            } break;
            default: return true;
        }
    } else{
        return false;
    }
}

inline b32 is_in_death_instinct(){
    return core.time.app_time - state_context.death_instinct.start_time <= state_context.death_instinct.duration;
}

inline b32 is_death_instinct_in_cooldown(){
    return core.time.app_time - state_context.death_instinct.cooldown_start_time <= state_context.death_instinct.cooldown;
}

void stop_death_instinct(){
    f32 time_since_death_instinct = core.time.app_time - state_context.death_instinct.start_time;

    Entity *threat_entity = get_entity_by_id(state_context.death_instinct.threat_entity_id);

    b32 is_threat_status_gives_cooldown = true;
    if (state_context.death_instinct.last_reason == ENEMY_ATTACKING){
        // We start cooldown if there was flying guy if he's not here anymore so player really used that instinct and don't just
        // evaded enemy.
        // is_threat_status_gives_cooldown = (!threat_entity || threat_entity->enemy.dead_man || is_enemy_should_trigger_death_instinct(threat_entity, get_entity_velocity(threat_entity), normalized(player_entity->position - threat_entity->position), magnitude(player_entity->position - threat_entity->position), true));
        if (threat_entity){
            is_threat_status_gives_cooldown = !is_death_instinct_threat_active();
        }
    } else if (state_context.death_instinct.last_reason == SWORD_WILL_EXPLODE){
        // If there was explosive we want start cooldown if threat is still alive so player evaded explosion. 
        // That's because if player killed it - he taked risk and succeeded.
        is_threat_status_gives_cooldown = (threat_entity && !threat_entity->enemy.dead_man);
    }
    b32 should_start_cooldown = time_since_death_instinct >= state_context.death_instinct.allowed_duration_without_cooldown && is_threat_status_gives_cooldown;

    if (should_start_cooldown){
        state_context.death_instinct.cooldown_start_time = core.time.app_time;            
    }
    if (time_since_death_instinct >= 0.2f){
        state_context.death_instinct.stop_time = core.time.app_time;
    }
    state_context.death_instinct.start_time = -12;
    state_context.death_instinct.threat_entity_id = -1;
    state_context.death_instinct.played_effects = false;
}

b32 is_enemy_should_trigger_death_instinct(Entity *entity, Vector2 velocity, Vector2 dir_to_player, f32 distance_to_player, b32 check_if_flying_towards){
    return false;

    b32 flying_towards = true;
    // @TODO: We definetely want to better check if enemy is flying towards. For example we can just simulate enemy some steps forward.
    if (check_if_flying_towards){
        flying_towards = distance_to_player < get_death_instinct_radius(entity->scale) && dot(dir_to_player, normalized(velocity)) >= 0.9f;
    }
    
    if (!flying_towards){
        return false;
    }
    
    // That's pretty sloppy when player tries to just kill single enemy with big sword, but without slowmo it's just REALLY hard and 
    // we should left that for late game.
    b32 will_kill_on_hit = (should_kill_player(entity) || (entity->flags & EXPLOSIVE));
    if (!will_kill_on_hit){
        return false;       
    }
    
    Collision ray_collision = raycast(entity->position, dir_to_player, distance_to_player - 2, GROUND | CENTIPEDE_SEGMENT | BLOCKER | SHOOT_BLOCKER, 4, entity->id);
    b32 will_hit_something_before_player = ray_collision.collided;
    // Additional raycast check is because of inprecision of raycast. It makes steps by some amount (2 on writing moment) and if 
    // player will stand too close to wall - it can overshoot and think that we will hit some ground).
    // So this thing checks if ground point that we detecting is farther than player - that could mean that we overshooted.
    // UPDATE: Commented for now because i think i fixed overshooting by changing length calculation on raycast and shrinking 
    // ray width, but we'll see.
    // UPDATE2: Half of change (with shrinking) just breaks stuff because we can't properly detect shkibidi. 
    // So we'll just know about that and for situations like this just take slightly less distance so we don't overshoot.
    return flying_towards && will_kill_on_hit && (!will_hit_something_before_player/* || sqr_magnitude(ray_collision.point - entity->position) >= distance_to_player * distance_to_player*/);
}

b32 start_death_instinct(Entity *threat_entity, Death_Instinct_Reason reason){
    if (is_in_death_instinct() || is_death_instinct_in_cooldown() || player_data->dead_man){
        return false;
    }
    
    state_context.death_instinct.start_time = core.time.app_time;
    state_context.death_instinct.threat_entity_id = threat_entity->id;
    state_context.death_instinct.last_reason = reason;
    return true;
}

inline b32 should_kill_player(Entity *entity){
    return true;// !can_sword_damage_enemy(entity);
}

void calculate_projectile_collisions(Entity *entity){
    Projectile *projectile = &entity->projectile;
    
    if (projectile->type == PLAYER_RIFLE_PROJECTILE){
        fill_collisions(entity, &collisions_buffer, GROUND | ENEMY | WIN_BLOCK | ROPE_POINT);
        // Player *player = player_data;
        
        b32 damaged_enemy = false;
        
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            Entity *other = col.other_entity;
            
            // Dying player rifle projectile is just slow shit and that happens only after bounce. So we destroy it at any collision.
            if (projectile->dying){
                entity->destroyed = true;
                return;
            }
            
            if (projectile->already_hit_ids.count >= projectile->already_hit_ids.max_count || projectile->already_hit_ids.contains(other->id)){
                continue;
            }
            
            b32 need_bounce = false;
            
            Vector2 velocity_dir = normalized(projectile->velocity);
            f32 sparks_speed = 1;
            f32 sparks_count = 1;
            f32 hitstop_add = 0;
            
            if (other->flags & ENEMY && is_enemy_can_take_damage(other, false) && !projectile->dying){
                projectile->already_hit_ids.add(other->id);
                b32 killed = false;
                b32 can_damage = true;
                
                Enemy *enemy = &other->enemy;
                
                if (other->flags & SHOOT_BLOCKER){
                    Vector2 shoot_blocker_direction = get_rotated_vector(enemy->shoot_blocker_direction, other->rotation);
                    f32 velocity_dot_direction = dot(velocity_dir, shoot_blocker_direction);    
                        
                    can_damage = !enemy->shoot_blocker_immortal && (compare_difference(velocity_dot_direction, 1, 0.1f) || compare_difference(velocity_dot_direction, -1, 0.1f));
                    sparks_speed += 2;
                    sparks_count += 2;
                    
                    if (!can_damage){
                        need_bounce = true;
                        enemy->last_hit_time = core.time.game_time;
                        play_sound("ShootBlock", col.point);
                    }
                }
                
                if (other->flags & BIRD_ENEMY && can_damage){
                    other->bird_enemy.velocity += projectile->velocity * 0.05f;
                }
                if (other->flags & JUMP_SHOOTER && can_damage){
                    other->jump_shooter.velocity += projectile->velocity * 0.05f;
                }
                
                if (enemy->max_hits_taken > 1){
                    projectile->velocity = reflected_vector(projectile->velocity * 0.6f, col.normal);
                    projectile->bounced = true;
                    projectile->birth_time = core.time.game_time;
                    stun_enemy(other, entity->position, col.normal);    
                } else if (enemy->max_hits_taken <= -1){
                    
                } else if (can_damage){
                    kill_enemy(other, entity->position, col.normal, false);
                    killed = true;
                }
                
                if (other->flags & TRIGGER && can_damage){
                    sparks_count += 20;
                    hitstop_add = 0.1f;
                }
                
                if (other->flags & BIRD_ENEMY || other->flags & JUMP_SHOOTER){
                    emit_particles(&bullet_strong_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
                } else{
                    emit_particles(&bullet_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
                }
                
                add_hitstop(0.03f + hitstop_add);
                shake_camera(0.1f);
                if (can_damage){
                    f32 time_since_last_hit = core.time.game_time - state_context.timers.last_projectile_hit_time;
                    if (time_since_last_hit <= 0.05f){
                        state_context.contiguous_projectile_hits_count += 1;
                    } else{
                        state_context.contiguous_projectile_hits_count = 0;
                    }
                
                    f32 base_pitch = 1.0f + state_context.contiguous_projectile_hits_count * 0.025f;
                    state_context.timers.last_projectile_hit_time = core.time.game_time;
                    play_sound("RifleHit", col.point, 0.4f + state_context.contiguous_projectile_hits_count * 0.01f, base_pitch);
                }
            
                damaged_enemy = can_damage;
            }
            
            if (other->flags & PHYSICS_OBJECT){
                apply_physics_force(projectile->velocity, 0.5f, &other->physics_object, col.normal);
                emit_particles(&bullet_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
                entity->destroyed = true;
            }
            
            if (other->flags & GROUND){
                entity->destroyed = true;
                emit_particles(&bullet_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
            }
            
            if (other->flags & ROPE_POINT){
                // cut rope point
                other->destroyed = true;
                emit_particles(&rifle_bullet_emitter, col.point, col.normal, 6, 10);
                emit_particles(&bullet_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
                play_sound("RopeCut", col.point);
            }
            
            if (need_bounce){
                projectile->velocity = reflected_vector(projectile->velocity * 0.5f, col.normal);
            }
            
            if (!projectile->dying && core.time.game_time - projectile->last_light_spawn_time >= 0.1f){
                add_explosion_light(col.point, 75, 0.05f, 0.1f, ColorBrightness(damaged_enemy ? RED : SKYBLUE, 0.4f));
                projectile->last_light_spawn_time = core.time.game_time;
            }
        }
    } else if (projectile->type == JUMP_SHOOTER_PROJECTILE){
        fill_collisions(entity, &collisions_buffer, GROUND | PLAYER | CENTIPEDE_SEGMENT | ENEMY_BARRIER | NO_MOVE_BLOCK);
        // @CLEANUP We don't need JUMP_SHOOTER_PROJECTILE anymore because we don't want jump shooter.        
        Enemy *enemy = &entity->enemy;
        
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            Entity *other = col.other_entity;
            
            if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT){
                kill_enemy(entity, col.point, col.normal);
                emit_particles(&bullet_hit_emitter_copy, col.point, col.normal * -1, 1);
            }
            
            if (other->flags & PLAYER && !player_data->dead_man && !enemy->dead_man){
                // It's a good thing that we don't kill player when projectile is blocker or explosive, 
                // but of course we need to better tell player what exactly will kill him on touch. 
                // While projectiles are flying - they're leave particle trail and all flying projectiles 
                // will kill us. So when projectile stopped we should still produce particles for 
                // base projectiles so player can know what he need to know.
                // That also works for enemies - they're kill player on touch when they're producing certain particles.
                b32 can_kill_player = !projectile->dying || !(entity->flags & (BLOCKER | EXPLOSIVE));
                if (can_kill_player){
                    if (can_sword_damage_enemy(entity)){
                        kill_enemy(entity, col.point, col.normal);
                    } else{
                        kill_player();
                    }
                }
            }
        }
    } else if (projectile->type == TURRET_DIRECT_PROJECTILE || projectile->type == TURRET_HOMING_PROJECTILE){
        fill_collisions(entity, &collisions_buffer, GROUND | PLAYER | CENTIPEDE_SEGMENT | ENEMY_BARRIER | NO_MOVE_BLOCK);
        Enemy *enemy = &entity->enemy;
        
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            Entity *other = col.other_entity;
            
            if (other->flags & PLAYER && !player_data->dead_man && !enemy->dead_man){
                kill_player();
            }
            
            kill_enemy(entity, col.point, col.normal);
            emit_particles(&bullet_hit_emitter_copy, col.point, col.normal * -1, 1);
        }
    }
}

void update_projectile(Entity *entity, f32 dt){
    assert(entity->flags & PROJECTILE);
    
    Projectile *projectile = &entity->projectile;
    f32 lifetime = core.time.game_time - projectile->birth_time;
    
    if (projectile->max_lifetime > 0 && lifetime> projectile->max_lifetime){
        if (entity->flags & ENEMY){
            // kill_enemy(entity, entity->position, entity->up);
            entity->destroyed = true;    
        } else{
            entity->destroyed = true;    
        }
        return;
    }
    
    f32 sqr_distance_to_player = sqr_magnitude(entity->position - player_entity->position);
    if (projectile->type == PLAYER_RIFLE_PROJECTILE){
        if (sqr_distance_to_player > 1000 * 1000){
            entity->destroyed = true;
            return;
        }
        
        if (projectile->bounced){
            // Projectile flies for some time after bounce, but then we want it to settle down.
            f32 feel_strong_after_bounce = 0.1f;
            if (lifetime > feel_strong_after_bounce){
                f32 life_overshoot = lifetime - feel_strong_after_bounce;
                projectile->dying = true;
                
                clamp_magnitude(&projectile->velocity, 60);
                projectile->velocity.y -= player_data->gravity * dt;
            }
        }
    }
    
    if (projectile->type == TURRET_HOMING_PROJECTILE){
        Vector2 vec_to_player = player_entity->position - entity->position;
        Vector2 dir = normalized(vec_to_player);
        
        if (dot(dir, entity->up) > 0){
            change_up(entity, move_towards(entity->up, dir, 2, dt));
            f32 projectile_speed = magnitude(projectile->velocity);
            projectile->velocity = entity->up * projectile_speed;
        }
    }
    
    Vector2 move = projectile->velocity * dt;
    Vector2 move_dir = normalized(move);
    f32 move_len = magnitude(move);
    f32 max_move_len = entity->scale.y * 0.5f;
    
    for (i32 i = 0; i < entity->particle_emitters_indexes.count; i++){
        Particle_Emitter *emitter = get_particle_emitter(entity->particle_emitters_indexes.get(i));
        if (emitter){
            if (sqr_distance_to_player >= 500 * 500){
                disable_emitter(emitter);
            } else{
                emitter->direction = move_dir * 1.0f;
                enable_emitter(emitter, entity->position);
            }
        }
    }
    
    if (projectile->type == JUMP_SHOOTER_PROJECTILE){
        if (lifetime >= 0.5f && !projectile->dying){
            Collision ray = raycast(entity->position + entity->up * entity->scale.y * 0.5f, entity->up, 10, GROUND | CENTIPEDE_SEGMENT | BLOCKER | SHOOT_BLOCKER, 5, entity->id);
            if (ray.collided){
                projectile->dying = true;
            }
        }
    
        if (lifetime >= 3 || projectile->dying){
            f32 damping_factor = projectile->dying ? 25 : 4;
            projectile->velocity *= 1.0f - (dt * damping_factor);
            projectile->dying = true;
            if (entity->flags & EXPLOSIVE || entity->flags & BLOCKER){
                disable_emitter(entity->enemy.alarm_emitter_index);
            }
        } else{
        }
    }
    
    while (move_len > max_move_len){
        entity->position += move_dir * max_move_len;
        calculate_projectile_collisions(entity);
        move_len -= max_move_len;
    }
    
    entity->position += move_dir * move_len;
    calculate_projectile_collisions(entity);
    
    change_up(entity, projectile->velocity);
}

void update_sticky_texture(Entity *entity, f32 dt){
    Sticky_Texture *st = &entity->sticky_texture;
    
    Entity *follow_entity = get_entity_by_id(st->follow_id);
    b32 need_to_follow = st->need_to_follow && follow_entity && follow_entity->enabled;
    f32 lifetime = core.time.game_time - st->birth_time;
    f32 lifetime_t = 0;
    if (st->max_lifetime > EPSILON){
        lifetime_t = lifetime / st->max_lifetime;
        if (lifetime >= st->max_lifetime){
            entity->destroyed = true;
        } else{
            entity->color = lerp(entity->color_changer.start_color, Fade(WHITE, 0), EaseOutExpo(lifetime_t));
        }
    }
    
    if (need_to_follow){
        Vector2 target_position = follow_entity->position;
        if (follow_entity->flags & SHOOT_STOPER){
            target_position = get_shoot_stoper_cross_position(follow_entity);
        }
        entity->position = lerp(entity->position, target_position, dt * 40);
    } else if (st->max_lifetime <= EPSILON){
        entity->destroyed = true;
    }
    
    if (follow_entity && follow_entity->flags & ENEMY && follow_entity->enemy.dead_man && !st->should_draw_until_expires){
        st->should_draw_texture = false;
    }
    
    st->need_to_follow = need_to_follow;
}

inline void trigger_editor_verify_connected(Entity *entity){
    for (i32 i = 0; i < entity->trigger.connected.count; i++){   
        // So if entiity was somehow destoyed, annighilated.
        if (!current_level_context->entities.has_key(entity->trigger.connected.get(i))){
            entity->trigger.connected.remove(i);
            i--;
            continue;
        }
    }
    
    for (i32 ii = 0; ii < entity->trigger.tracking.count; ii++){
        i32 id = entity->trigger.tracking.get(ii);
        if (!current_level_context->entities.has_key(id)){
            entity->trigger.tracking.remove(ii);
            ii--;
            continue;
        }
    }
}

void update_editor_entity(Entity *e){
    if (e->flags & CENTIPEDE && game_state == EDITOR){
        e->flags |= ENEMY;
        // e->enemy.dead_man = true;
    }

    if (e->flags & TRIGGER){
        trigger_editor_verify_connected(e);
    }
    
    if (e->flags & PHYSICS_OBJECT){
        if (e->physics_object.on_rope/* && core.time.app_time - e->physics_object.last_pick_rope_point_time > 0.5f*/){
            Collision ray_col = raycast(e->position + e->up * e->scale.y * 0.5f, e->up, 300, GROUND, 4, e->id);
            if (ray_col.collided){
                e->physics_object.rope_point = ray_col.point;
            }
            e->physics_object.last_pick_rope_point_time = core.time.app_time;
        }
    }
    
    if (e->flags & LIGHT){
        if (e->light_index == -1){
            printf("WARNING: Entity with flag LIGHT don't have corresponding light index (name: %s; id: %d)\n", e->name, e->id);
        } else{
            Light *light = current_level_context->lights.get_ptr(e->light_index);
            light->position = e->position;
        }
    }
    
    if (e->flags & DOOR){
        e->door.closed_position = e->door.is_open ? e->position - e->up * e->scale.y : e->position;
        e->door.open_position   = e->door.is_open ? e->position : e->position + e->up * e->scale.y;
    }
    
    // update turret editor
    if (e->flags & TURRET){
        e->enemy.turret.original_up = e->up;
    }
    
    if (e->flags & PROPELLER){
        Particle_Emitter *air_emitter = get_particle_emitter(e->propeller.air_emitter_index);
        if (air_emitter){
            init_propeller_emitter_settings(e, air_emitter);
        }
    }
}

void activate_turret(Entity *entity){
    entity->enemy.turret.activated = true;
    entity->enemy.turret.last_shot_tick = state_context.turret_state.current_tick - entity->enemy.turret.start_tick_delay;
}

void trigger_entity(Entity *trigger_entity, Entity *connected){
    connected->hidden = !trigger_entity->trigger.shows_entities;
    
    b32 should_agro = trigger_entity->trigger.agro_enemies && debug.enemy_ai;
    if (should_agro){
        if (connected->flags & ENEMY){
            agro_enemy(connected);
        }
        
    }
    
    if (connected->flags & CENTIPEDE){
        assert(connected->flags & MOVE_SEQUENCE); // While we move centipede by move sequence we want that to be checked.
        for (i32 i = 0; i < connected->centipede.segments_count; i++){
            Entity *segment = get_entity_by_id(connected->centipede.segments_ids.get(i));
            assert(segment);
            segment->hidden = connected->hidden;
            if (should_agro){
                segment->move_sequence.moving = connected->move_sequence.moving;
                agro_enemy(segment);
            }
        }
    }
    
    if (connected->flags & DOOR && connected->door.is_open != trigger_entity->trigger.open_doors){
        activate_door(connected, trigger_entity->trigger.open_doors);
    }
    
    if (connected->flags & PHYSICS_OBJECT && trigger_entity->trigger.start_physics_simulation){
        connected->physics_object.simulating = true;
    }
    
    if (connected->flags & MOVE_SEQUENCE){
        connected->move_sequence.moving = trigger_entity->trigger.starts_moving_sequence;
    }
    
    if (connected->flags & TURRET){
        activate_turret(connected);
    }
}

i32 update_trigger(Entity *e){
    assert(e->flags & TRIGGER);
    
    b32 trigger_now = false;
    
    if (e->trigger.debug_should_trigger_now){
        e->trigger.debug_should_trigger_now = false;
        trigger_now = true;
    }
    
    if (e->flags & ENEMY && e->enemy.dead_man){
        if (e->trigger.triggered){
            return TRIGGER_SOME_ACTION;
        }
    
        trigger_now = true;
    }
    
    if (e->trigger.kill_enemies){
        fill_collisions(e, &collisions_buffer, ENEMY);
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            kill_enemy(col.other_entity, col.point, col.normal);            
        }
    }
    
    if (/*e->trigger.track_enemies*/ e->trigger.tracking.count > 0 && !e->trigger.triggered){
        b32 found_enemy = false;
        for (i32 i = 0; i < e->trigger.tracking.count; i++){
            i32 id = e->trigger.tracking.get(i);
            if (!current_level_context->entities.has_key(id)){
                e->trigger.tracking.remove(i);
                i--;
                continue;
            }
            
            Entity *tracking_entity = current_level_context->entities.get_by_key_ptr(id);

            if (!tracking_entity->enemy.dead_man){
                found_enemy = true;
                break;
            } 
        }
        
        if (!found_enemy){
            trigger_now = true;            
        }
    }
    
    if (trigger_now || e->trigger.player_touch && check_entities_collision(e, player_entity).collided){
        if (e->trigger.forbid_player_shoot){
            player_data->can_shoot = false;
        }
        if (e->trigger.allow_player_shoot){
            player_data->can_shoot = true;
        }
    
        if (str_contains(e->name, "checkpoint") && checkpoint_trigger_id != e->id){
            clear_level_context(&checkpoint_level_context);
            copy_level_context(&checkpoint_level_context, current_level_context, false);
            checkpoint_player_entity = player_entity;
            checkpoint_player_data = *player_data;
            checkpoint_time = core.time;
            checkpoint_state_context = state_context;
            
            checkpoint_trigger_id = e->id;
        }
        
        if (str_equal(e->name, "relax")){
            state_context.playing_relax = true;
        }
    
        if (e->trigger.load_level){
            b32 we_on_last_level = str_equal(e->trigger.level_name, "LAST_LEVEL_MARK");
            if (we_on_last_level || session_context.speedrun_timer.level_timer_active){
                win_level();
            } else{
                enter_game_state_on_new_level = true;
                last_player_data = *player_data;
                load_level(e->trigger.level_name);
                return TRIGGER_LEVEL_LOAD;
            }
        }
        
        if (e->trigger.play_replay){
            if (!session_context.playing_replay){
                load_replay(e->trigger.replay_name);
            }
        }
        
        if (e->trigger.start_cam_rails_horizontal){
            state_context.cam_state.on_rails_horizontal = true;
            state_context.cam_state.on_rails_vertical = false;
            state_context.cam_state.locked = false;
            state_context.cam_state.rails_trigger_id = e->id;
        }
        if (e->trigger.start_cam_rails_vertical){
            state_context.cam_state.on_rails_vertical = true;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.locked = false;
            state_context.cam_state.rails_trigger_id = e->id;
        }
        if (e->trigger.stop_cam_rails){
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical   = false;
            state_context.cam_state.rails_trigger_id = -1;
        }
        
        if (e->trigger.play_sound && !e->trigger.triggered){
            play_sound(e->trigger.sound_name);
        }
        
        if (e->trigger.change_zoom){
            // With wide monitors happening cut in vertical space so we need to calculate zoom with aspect ratio.
            // 16:9 it's 1.777777 aspect_ratio
            // 21:9 it's 2.333333 aspect_ratio
            f32 target_zoom = e->trigger.zoom_value;
            target_zoom /= (aspect_ratio / 1.77777f);
            current_level_context->cam.target_zoom = target_zoom;
        }
        
        if (e->trigger.unlock_camera){
            state_context.cam_state.locked = false;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical = false;
        } else if (e->trigger.lock_camera){
            state_context.cam_state.locked = true;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical = false;
            current_level_context->cam.target = e->trigger.locked_camera_position;
        }
    
        if (e->flags & DOOR){
            trigger_entity(e, e);
        }
    
        if (e->trigger.kill_player){
            kill_player();
        }
        
        for (i32 i = 0; i < e->trigger.connected.count; i++){
            i32 id = e->trigger.connected.get(i);
            if (!current_level_context->entities.has_key(id)){
                e->trigger.connected.remove(i);
                i--;
                continue;
            }
            
            Entity *connected_entity = current_level_context->entities.get_by_key_ptr(id);
                        
            trigger_entity(e, connected_entity);
        }
        
        e->trigger.triggered = true;
        e->trigger.triggered_time = core.time.game_time;
        
        if (e->trigger.die_after_trigger){
            e->enabled = false;
        }
    }
    
    return TRIGGER_SOME_ACTION;
}

void update_door(Entity *entity){
    Door *door = &entity->door;

    f32 since_triggered = core.time.game_time - door->triggered_time;
    f32 move_time       = door->is_open ? door->time_to_open : door->time_to_close;
    
    if (since_triggered <= move_time){
        Vector2 target_position = door->is_open ? door->open_position   : door->closed_position;
        Vector2 start_position  = door->is_open ? door->closed_position : door->open_position;
        
        f32 t = clamp01(since_triggered / move_time);
        
        entity->position = lerp(start_position, target_position, EaseOutElastic(t));
    }
}

void activate_door(Entity *entity, b32 is_open){
    if (entity->door.is_open != is_open){ 
        play_sound("OpenDoor", entity->position);
        entity->door.is_open = is_open;
        entity->door.triggered_time = core.time.game_time;
    }
}

Collision get_nearest_ground_collision(Vector2 point, f32 radius){
    Array<Vector2, MAX_VERTICES> vertices;
    f32 radius_step = 4;
    f32 current_radius = 0;
    
    add_rect_vertices(&vertices, {0.5f, 0.5f});    
    while (current_radius <= radius){
        current_radius += radius_step        ;
        for (i32 i = 0; i < vertices.count; i++){
            *vertices.get_ptr(i) = normalized(vertices.get(i)) * current_radius;
            rotate_around_point(vertices.get_ptr(i), {0, 0}, 33);
        }
        
        Bounds bounds = get_bounds(vertices, {0.5f, 0.5f});
        
        fill_collisions(point, vertices, bounds, {0.5f, 0.5f}, &collisions_buffer, GROUND);
        
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            if (col.collided){
                return col;
            }
        }
    }
    
    return {};
}

void update_move_sequence(Entity *entity, f32 dt){
    Move_Sequence *sequence = &entity->move_sequence;
    
    if (!sequence->moving || sequence->points.count == 0){
        sequence->moved_last_frame = Vector2_zero;
        return;
    }
    
    if (!sequence->loop && sequence->current_index >= sequence->points.count-1 && sqr_magnitude(entity->position - sequence->points.get(sequence->current_index)) <= EPSILON){
        sequence->moved_last_frame = Vector2_zero;
        return;
    }
    
    Vector2 target  = sequence->points.get((sequence->current_index + 1) % sequence->points.count);
    
    if (sequence->current_index >= sequence->points.count-1 && !sequence->loop){
        target = sequence->points.last();
    }
    
    f32 speed = sequence->speed;
    
    if (sequence->speed_related_player_distance && game_state == GAME){
        f32 distance_to_player = magnitude(player_entity->position - entity->position);
        f32 distance_t = clamp01((distance_to_player + sequence->min_distance) / sequence->max_distance);
        speed = lerp(sequence->speed, sequence->max_distance_speed, distance_t * distance_t);
    }
    
    if (sequence->just_born){
        if (entity->flags & JUMP_SHOOTER){
            Jump_Shooter *shooter = &entity->jump_shooter;
            shooter->move_points.clear();
            for (i32 i = 0; i < sequence->points.count; i++){
                Vector2 point = sequence->points.get(i);
                Collision nearest_ground = get_nearest_ground_collision(point, 20);
                            
                if (nearest_ground.collided){
                    Vector2 point_to_collision = nearest_ground.point - point;
                    Vector2 dir = normalized(point_to_collision);
                    f32 len     = magnitude(point_to_collision);
                    Collision ray_collision = raycast(point, dir, len, GROUND, 1); 
                    
                    if (ray_collision.collided){
                        shooter->move_points.add({ray_collision.point, ray_collision.normal});
                    } else{
                        print("WARNING: Jump shooter, one of it's points can't find good ground to land. Will add bad ground point");
                        shooter->move_points.add({nearest_ground.point, nearest_ground.normal});
                    }
                } else{
                    print("WARNING: Jump shooter, one of it's points can't find any ground to land. Will add air point");
                    shooter->move_points.add({point, Vector2_up});
                }
            }
            
            
            change_up(entity, shooter->move_points.get(0).normal);
            entity->position = shooter->move_points.get(0).position + entity->up * entity->scale.y * (1.0f - entity->pivot.y);
        } else{
            sequence->velocity = normalized(target - entity->position) * speed;
            sequence->wish_position = entity->position;
        }
        
        sequence->just_born = false;
    }
        
    if (entity->flags & JUMP_SHOOTER){
        Jump_Shooter *shooter = &entity->jump_shooter;   
    } else{
        Vector2 previous_position = entity->position;
        sequence->wish_position = move_towards(sequence->wish_position, target, speed, dt);
        
        if (!sequence->loop && sequence->current_index >= sequence->points.count - 2){
            entity->position = move_towards(entity->position, sequence->wish_position, speed, dt);
        } else{
            Vector2 wish_vec = sequence->wish_position - entity->position;
            f32 wish_len = magnitude(wish_vec);
            if (wish_len > 0){
                sequence->wish_velocity = (wish_vec / wish_len) * speed;
                sequence->velocity = move_towards(sequence->velocity, sequence->wish_velocity, speed * 4, dt);
                entity->position += sequence->velocity * dt;
            }
        }
        
        
        if (sequence->rotate){
            change_up(entity, normalized(sequence->velocity));
        }
        
        sequence->moved_last_frame = entity->position - previous_position;
        
        if (magnitude(target - sequence->wish_position) <= EPSILON){
            sequence->current_index = sequence->current_index + 1;
            if (sequence->current_index >= sequence->points.count && sequence->loop){
                sequence->current_index = 0;
            }
        }
    }
}

void update_all_collision_cells(){
    state_context.timers.last_collision_cells_clear_time = core.time.app_time;
    for (i32 i = 0; i < session_context.collision_grid_cells_count; i++){        
        session_context.collision_grid.cells[i].entities_ids.clear();
    }
    
    ForEntities(entity, 0){
        update_entity_collision_cells(entity);
    }
}

void shoot_projectile(Vector2 position, Vector2 direction, Projectile_Settings settings, Projectile_Type type, Color color){
    Vector2 scale = {2, 4};
    
    if (type == TURRET_HOMING_PROJECTILE){
        scale *= 2;
    }
    
    // @CLEANUP: Right now we set additional projectile enemy flags directly to entity, but when we redo entity system we will 
    // want to set that on enemy of spawned projectile.
    Entity *projectile_entity = add_entity(position, scale, {0.5f, 0.5f}, 0, PROJECTILE | ENEMY | PARTICLE_EMITTER | settings.enemy_flags);
    change_color(projectile_entity, color);
    projectile_entity->projectile.birth_time = core.time.game_time;
    projectile_entity->projectile.type = type;
    projectile_entity->projectile.velocity = direction * settings.launch_speed;
    change_up(projectile_entity, direction);
    projectile_entity->projectile.max_lifetime = settings.max_lifetime;
    
    if (projectile_entity->flags & BLOCKER){
        projectile_entity->enemy.blocker_clockwise = settings.blocker_clockwise;
    }
    
    if (type == TURRET_HOMING_PROJECTILE){
        add_and_enable_entity_particle_emitter(projectile_entity, &small_air_dust_trail_emitter_copy, projectile_entity->position, true);
    }
    projectile_entity->enemy.alarm_emitter_index = add_and_enable_entity_particle_emitter(projectile_entity, &alarm_smoke_emitter_copy, projectile_entity->position, true);
    init_entity(projectile_entity);
}

global_variable f32 turret_max_angle_diversion = 70;
inline void update_turret(Entity *entity, f32 dt){
    Turret *turret = &entity->enemy.turret;
    
    if (!turret->activated){
        // if (turret->homing && entity->rotation != turret->original_angle - turret_max_angle_diversion){
        //     rotate_to(entity, turret->original_angle - turret_max_angle_diversion);
        // }
        return;
    }
    
    Projectile_Type projectile_type = TURRET_DIRECT_PROJECTILE;
    Color projectile_color = ColorBrightness(RED, 0.5f);
    
    b32 player_in_range = true;
    b32 player_in_angle_range = true;
    
    turret->see_player = false;
    if (turret->homing){
        projectile_type = TURRET_HOMING_PROJECTILE;
        projectile_color = ColorBrightness(ORANGE, -0.2f);
        
        // f32 sqr_distance = sqr_magnitude(vec_to_player);
            
        if (!check_rectangles_collision(player_entity->position, player_entity->scale, entity->position, {turret->shoot_width, turret->shoot_height})){
            entity->enemy.in_agro = false;
            player_in_range = false;
        } else{
            entity->enemy.in_agro = true;
            Vector2 vec_to_player = player_entity->position - entity->position;
            Vector2 dir = normalized(vec_to_player);
            // This (- 2) because of the shitty raycast where we could hit obstacle *behind* player and count that as we don't 
            // see player, even though we have direct line of sight.
            f32 distance = magnitude(vec_to_player) - entity->scale.y * entity->pivot.y - 2;
            
            Collision ray_collision = raycast(entity->position + entity->up * entity->scale.y * entity->pivot.y, dir, distance, GROUND | ENEMY_BARRIER | NO_MOVE_BLOCK, distance);
            
            if (ray_collision.collided){
                turret->see_player = false;
            } else{
                turret->see_player = true;
            }
            
            f32 dot_original_current = dot(turret->original_up, dir);
            f32 max_allowed_dot = 1.0f - (turret_max_angle_diversion / 90.0f);          
            if (dot_original_current < max_allowed_dot){
                turret->see_player = false;
                player_in_angle_range = false;
            } else{
                f32 rotation_speed = 30;
                // f32 angle_change = normalized(desired_angle - current_angle) * rotation_speed * dt * -1;
                // rotate(entity, angle_change);
                change_up(entity, move_towards(entity->up, dir, 1.0f, dt));
            }
        }
    } else{
        // We currently don't care about non-homing turret seeing player.
        turret->see_player = true;
    }
    
    Turret_State *state = &state_context.turret_state;
    
    // We apply delay only for first shot ever, because next it will just work as intended because of last_shot_tick.
    i32 tick_delay = turret->last_shot_tick == 0 ? turret->start_tick_delay : 0;
    // b32 is_my_tick = state->ticked_this_frame && (state->current_tick - turret->last_shot_tick - tick_delay) >= turret->shoot_every_tick;
    b32 is_my_tick = state->ticked_this_frame && ((state->current_tick - turret->start_tick_delay) % turret->shoot_every_tick) == 0 && turret->last_shot_tick != state->current_tick;
    
    // We don't set last shot tick on real shot because homing turrets will not always see player when tick happens, so 
    // we just tracking ticks for that to work correctly.
    if (is_my_tick){
        turret->last_shot_tick = state->current_tick;
    }
    
    if (is_my_tick && player_in_range && player_in_angle_range /*turret->see_player*/){
        Vector2 start_position = entity->position + entity->up * entity->scale.y * entity->pivot.y;
        shoot_projectile(start_position, entity->up, turret->projectile_settings, projectile_type, projectile_color);
        if (turret->homing){
            play_sound("BirdAttack", entity->position);
        }
    }
}

// We return false if we should stop updating entities this frame. Shoulda make result flag or something, but for now comment will do.
inline b32 update_entity(Entity *e, f32 dt){
    update_color_changer(e, dt);            
    
    if (e->flags & PHYSICS_OBJECT){
        // update rope stuff
         if (e->physics_object.on_rope){
            Entity *rope_entity            = get_entity_by_id(e->physics_object.rope_id);
            Entity *up_rope_point_entity   = get_entity_by_id(e->physics_object.up_rope_point_id);
            Entity *down_rope_point_entity = get_entity_by_id(e->physics_object.down_rope_point_id);
            
            // If any part of rope is missing - we destroy them all.
            if (!rope_entity || !up_rope_point_entity || !down_rope_point_entity){
                e->physics_object.on_rope = false;
                if (rope_entity){
                    rope_entity->destroyed = true;
                }
                if (up_rope_point_entity){
                    up_rope_point_entity->destroyed = true;
                }
                if (down_rope_point_entity){
                    down_rope_point_entity->destroyed = true;
                }
            } else{
                update_entity_collision_cells(rope_entity);
                rope_entity->position = e->position + e->up * e->scale.y * 0.5f;
                Vector2 vec_to_point = e->physics_object.rope_point - (e->position + e->up * e->scale.y * 0.5f);
                f32 len = magnitude(vec_to_point);
                Vector2 dir = normalized(vec_to_point);
                change_up(rope_entity, dir);
                change_scale(rope_entity, {1, len});
                
                up_rope_point_entity->position   = e->physics_object.rope_point;
                down_rope_point_entity->position = e->position + e->up * e->scale.y * 0.5f;
            }
        }
    }
    
    //update light on entity (Lights itself updates in separate place).
    if (e->flags & LIGHT){
        if (e->light_index == -1){
            printf("WARNING: Entity with flag LIGHT don't have corresponding light index. Name: %s, id: %d\n", e->name, e->id);
        }
    }
    
    session_context.collision_grid.origin = {(f32)((i32)player_entity->position.x - ((i32)player_entity->position.x % (i32)session_context.collision_grid.cell_size.x)), (f32)((i32)player_entity->position.y - ((i32)player_entity->position.y % (i32)session_context.collision_grid.cell_size.y))};
    
    // update player
    if (e->flags & PLAYER && !debug.dragging_player){
        if (e->flags & REPLAY_PLAYER){
            if (!session_context.playing_replay){
                e->destroyed = true;
                return true;
            }
            // @HACK if we'll use replay characters more - we should really look into where we use player_data->
            player_data = &replay_player_data;
            e->position = replay_input.player_position;
            update_player(e, dt, replay_input);
            // player_data = &real_player_data;
        } else{
            player_data = &real_player_data;
            update_player(e, dt, input);
        }
    }
      
    // update explosive
    if (e->flags & EXPLOSIVE){
        if (e->enemy.should_explode && !e->enemy.dead_man){
            if (core.time.app_time - state_context.timers.last_explosion_app_time >= 0.01f){    
                kill_enemy(e, e->position, e->up, false);
            }
        }
    }
        
    // update bird enemy
    if (e->flags & BIRD_ENEMY && debug.enemy_ai){
        update_bird_enemy(e, dt);
    }
    
    if (e->flags & TURRET && debug.enemy_ai){
        update_turret(e, dt);
    }
    
    // update multiple hits
    if (e->flags & MULTIPLE_HITS){
        Multiple_Hits *mod = &e->enemy.multiple_hits;
        assert(mod->made_hits <= mod->required_hits && mod->made_hits >= 0);
        if (mod->made_hits > 0 && mod->seconds_to_regen > 0){    
            mod->timer += dt;
            if (mod->timer >= mod->seconds_to_regen){
                mod->timer -= mod->seconds_to_regen;              
                mod->made_hits -= 1;
            }
        } else{
            mod->timer = 0;
        }
    }
    
    // update projectile
    if (e->flags & PROJECTILE){
        update_projectile(e, dt);
    }
    
    for (i32 em = 0; em < e->particle_emitters_indexes.count; em++){
        Particle_Emitter *emitter = get_particle_emitter(e->particle_emitters_indexes.get(em));
        if (emitter && emitter->follow_entity){
            emitter->position = e->position;
        }
        // update_emitter(e->emitters.get_ptr(em), dt);
    }
    
    // update sticky texture
    if (e->flags & STICKY_TEXTURE){
        update_sticky_texture(e, dt);
    }
    
    // update trigger
    if (e->flags & TRIGGER){
        i32 trigger_action_flags = update_trigger(e);
        if (trigger_action_flags & TRIGGER_LEVEL_LOAD){
            return false;
        }
    }
    
    // update move sequence
    if (e->flags & MOVE_SEQUENCE){
        update_move_sequence(e, dt);
    }
    
    if (e->flags & PHYSICS_OBJECT && e->physics_object.simulating){
        //update physics object 
        Vector2 last_position = e->position;
        e->physics_object.velocity.y -= GRAVITY * e->physics_object.gravity_multiplier * dt;
        
        if (e->physics_object.on_rope){
            Vector2 next_velocity_position = e->position + e->physics_object.velocity * dt;
            Vector2 next_swing_position = e->physics_object.rope_point + normalized(next_velocity_position - e->physics_object.rope_point) * magnitude(e->position - e->physics_object.rope_point);
            e->physics_object.velocity = (next_swing_position - e->position) / dt;
        } 
        
        if (e->physics_object.rotate_by_velocity){
            rotate(e, (e->physics_object.velocity.x / e->physics_object.mass) * 10 * dt);
        }
        
        move_by_velocity_with_collisions(e, e->physics_object.velocity, e->scale.x * 0.5f + e->scale.y * 0.5f, &respond_physics_object_collision, dt);
        
        e->physics_object.moved_last_frame = e->position - last_position;
    }
    
    if (e->flags & CENTIPEDE && debug.enemy_ai && !e->enemy.dead_man){
        // update centipede
        Centipede *centipede = &e->centipede;
        
        i32 alive_count = 0;
        for (i32 i = 0; i < centipede->segments_count; i++){
            Entity *segment = current_level_context->entities.get_by_key_ptr(centipede->segments_ids.get(i));
            
            if (!segment->enemy.dead_man){
                alive_count++;
            }
        }
        
        if (alive_count == 0){
            centipede->all_segments_dead = true;
        
            e->enemy.dead_man = true;
            e->enemy.died_time = core.time.game_time;
            e->flags = ENEMY | BIRD_ENEMY | (e->flags & LIGHT); //@WTF?
            Vector2 rnd = rnd_in_circle();// e->move_sequence.moved_last_frame;
            e->bird_enemy.velocity = {e->move_sequence.velocity.x * rnd.x, e->move_sequence.velocity.y * rnd.y};

            e->move_sequence.moving = false;
            e->collision_flags = GROUND;
            init_bird_emitters(e);
            add_fire_light_to_entity(e);
            
            for (i32 i = 0; i < centipede->segments_count; i++){
                Entity *segment = current_level_context->entities.get_by_key_ptr(centipede->segments_ids.get(i));
                
                segment->flags = ENEMY | BIRD_ENEMY;
                segment->move_sequence.moving = false;
                segment->collision_flags = GROUND;
                Vector2 rnd = rnd_in_circle();//*/ segment->move_sequence.moved_last_frame;
                segment->bird_enemy.velocity = {segment->move_sequence.velocity.x * rnd.x, segment->move_sequence.velocity.y * rnd.y};
                init_bird_emitters(segment);
            }
        }
        // end update centipede end
    }

    if (e->flags & JUMP_SHOOTER && debug.enemy_ai){
        // update jump shooter
        Jump_Shooter *shooter = &e->jump_shooter;
        Enemy *enemy = &e->enemy;
        
        if (!enemy->in_agro){
            shooter->states = {};
            shooter->states.standing_start_time = core.time.game_time;
        }
        
        f32 in_stun_time = core.time.game_time - enemy->stun_start_time;
        
        b32 is_stunned = in_stun_time <= enemy->max_stun_time;
        
        if (enemy->dead_man || is_stunned){
            shooter->velocity.y -= GRAVITY * dt;
            rotate(e, shooter->velocity.x * 30 * dt);
            shooter->states = {};
            disable_emitter(shooter->trail_emitter_index);
            shooter->states.standing = false;
            shooter->states.standing_start_time = core.time.game_time;
            
            if (is_stunned){
                enemy->was_in_stun = true;
            }
        } else if (enemy->was_in_stun){
            shooter->states.standing = true;
            shooter->states.standing_start_time = core.time.game_time;
            
            enemy->was_in_stun = false;
        }
        
        f32 block_direction_switch_time = 1.5f;
        
        Particle_Emitter *trail_emitter = get_particle_emitter(shooter->trail_emitter_index);
        Particle_Emitter *flying_emitter = get_particle_emitter(shooter->flying_emitter_index);
        
        Vector2 vec_to_player = player_entity->position - e->position;
        Vector2 dir_to_player = normalized(vec_to_player);
        f32 distance_to_player = magnitude(vec_to_player);
        
        if (shooter->states.standing){
            f32 standing_time = core.time.game_time - shooter->states.standing_start_time;
            f32 max_standing_time = 3.0f;
            
            Collision nearest_ground = get_nearest_ground_collision(e->position, e->scale.x * 0.7f + e->scale.y * 0.7f);
            
            if (nearest_ground.collided){
                shooter->not_found_ground_timer = 0;
                shooter->velocity = Vector2_zero;
                
              //landing animation
                if (standing_time <= 1.0f){
                    f32 landing_t = clamp01(standing_time / 1.0f);
                    
                    Vector2 target_scale = {e->enemy.original_scale.x * 1.5f, e->enemy.original_scale.y * 0.5f};
                    if (landing_t <= 0.20f){
                        f32 t = clamp01(landing_t / 0.25f);
                        change_scale(e, lerp(e->enemy.original_scale, target_scale, EaseOutElastic(t)));
                    } else{
                        f32 t = clamp01((landing_t - 0.25f) / (1.0f - 0.25f));
                        change_scale(e, lerp(target_scale, e->enemy.original_scale, EaseInOutElastic(t)));
                    }
                }
                // squeezing animation
                if (standing_time >= max_standing_time - 1.0f){
                    f32 anim_t = clamp01((standing_time - (max_standing_time - 1.0f)) / 1.0f);
                    
                    Vector2 target_scale = {e->enemy.original_scale.x * 1.4f, e->enemy.original_scale.y * 0.7f};
                    if (anim_t <= 0.85f){
                        f32 t = anim_t / 0.85f;
                        change_scale(e, lerp(e->enemy.original_scale, target_scale, t * t));
                    } else{
                        f32 t = (anim_t - 0.85f) / 0.3f;
                        change_scale(e, lerp(target_scale, e->enemy.original_scale, sqrtf(t)));
                    }
                } 
                //jump shooter jump
                if (standing_time >= max_standing_time){
                    shooter->states.standing = false;
                    shooter->states.jumping = true;
                    shooter->states.jump_start_time = core.time.game_time;
                    shooter->jump_direction = e->up;
                    
                    shooter->velocity = shooter->jump_direction * 250;
                    emit_particles(&ground_splash_emitter, e->position - e->up * e->scale.y * 0.5f, e->up, 4, 1.5f);
                } else{
                    Collision ray_collision = raycast(e->position, normalized(nearest_ground.point - e->position), (e->scale.x + e->scale.y) * 2, GROUND, 1.0f);
                    Vector2 point = Vector2_zero;
                    Vector2 normal = Vector2_up;
                    if (ray_collision.collided){
                        point = ray_collision.point;
                        normal = ray_collision.normal;
                    } else{
                        point = nearest_ground.point;
                        normal = nearest_ground.normal;
                    }
                    
                    Vector2 dir = normalized(e->position - point);
                    e->position = move_towards(e->position, point + dir * e->scale.y * 0.5f, 50, dt);
                    change_up(e, move_towards(e->up, normal, 10, dt));
                    
                    if (nearest_ground.other_entity->flags & PHYSICS_OBJECT){
                        e->position += nearest_ground.other_entity->physics_object.moved_last_frame;
                        shooter->standing_on_physics_object_id = nearest_ground.other_entity->id;
                    } else{
                        shooter->standing_on_physics_object_id = -1;
                    }
                    
                    if (nearest_ground.other_entity->flags & MOVE_SEQUENCE){
                        e->position += nearest_ground.other_entity->move_sequence.moved_last_frame;
                    }
                }
            } else{ // If not found ground
                shooter->not_found_ground_timer += dt;
                shooter->velocity.y -= GRAVITY * dt;
                
                if (shooter->not_found_ground_timer >= 0.4f){
                    shooter->states.standing_start_time = core.time.game_time;
                }
            }
        }
        
        if (shooter->states.jumping){
            f32 jumping_time = core.time.game_time - shooter->states.jump_start_time;
            f32 max_jumping_time = 1.5f;
            f32 jump_t = clamp01((jumping_time / max_jumping_time));
            
            rotate(e, shooter->velocity.x * 20 * dt);
            
            f32 gravity_multiplier = shooter->jump_direction.y > 0 ? lerp(3.0f, 2.0f, jump_t * jump_t) : lerp(-0.5f, 0.0f, jump_t * jump_t);
            shooter->velocity.y -= GRAVITY * gravity_multiplier * dt;
            shooter->velocity.x = lerp(shooter->velocity.x, 0.0f, jump_t * dt * 6);
            
            if (jumping_time >= max_jumping_time || (jumping_time >= max_jumping_time * 0.5f && shooter->velocity.y < 40)){
                shooter->states.jumping = false;
                shooter->states.charging = true;
                shooter->states.charging_start_time = core.time.game_time;
                
                shooter->blocker_clockwise = /*rnd01() >= 0.5f*/ (core.time.game_time - (i32)core.time.game_time) >= 0.5f;
            }
        }
        
        if (shooter->states.charging){
            f32 charging_time = core.time.game_time - shooter->states.charging_start_time;
            f32 charging_t = clamp01(charging_time / shooter->max_charging_time);
            
            // Visual hints above jumper head happening in first half of charging
            if (charging_t <= 0.5f){
                f32 t = charging_t * 2;
                
                block_direction_switch_time = 0.2f;
            }
            
            move_vec_towards(&shooter->velocity, Vector2_zero, lerp(0.0f, 100.0f, sqrtf(charging_t)), dt);
            shooter->velocity.x = lerp(shooter->velocity.x, 0.0f, charging_t * dt * 5);
            
            f32 look_speed = lerp(0.0f, 10.0f, charging_t * charging_t);
            change_right(e, move_towards(e->right, dir_to_player.x > 0 ? dir_to_player : dir_to_player * -1, look_speed, dt));
            
            // jump shooter shoot
            if (charging_time >= shooter->max_charging_time && core.time.game_time - state_context.timers.last_jump_shooter_attack_time >= 0.3f){                    
                f32 angle = -shooter->spread * 0.5f;
                f32 angle_step = shooter->spread / shooter->shots_count;
                
                local_persist Array<i32, 64> explosive_indexes;
                explosive_indexes.clear();
                
                for (i32 i = 0; i < shooter->explosive_count; i++){
                    i32 explosive_index = /*rnd(0, shooter->shots_count)*/ (i32)core.time.game_time * (explosive_indexes.count + 1) % shooter->shots_count;
                    while (explosive_indexes.contains(explosive_index)){
                        explosive_index = (explosive_index+1) % shooter->shots_count;
                    }
                    explosive_indexes.add(explosive_index);
                }
                
                i32 explosive_shots = 0;
                
                for (i32 i = 0; i < shooter->shots_count; i++){
                    // @CLEANUP All of this is now in the shoot_projectile function. We don't use jump shooter anymore so it stays
                    // till the time comes.
                    Vector2 direction = get_rotated_vector(dir_to_player, angle);
                    angle += angle_step;
                    f32 speed = 100;
                    
                    FLAGS additional_flags = 0;
                    if (explosive_indexes.contains(i)){
                        additional_flags |= EXPLOSIVE;        
                    }
                    if (shooter->shoot_sword_blockers){
                        additional_flags |= BLOCKER;
                    }
                    if (shooter->shoot_bullet_blockers){
                        additional_flags |= SHOOT_BLOCKER;
                    }
                    
                    Entity *projectile_entity = add_entity(e->position, {2, 4}, {0.5f, 0.5f}, 0, PROJECTILE | ENEMY | PARTICLE_EMITTER | additional_flags);
                    change_color(projectile_entity, ColorBrightness(RED, 0.4f));
                    projectile_entity->projectile.birth_time = core.time.game_time;
                    projectile_entity->projectile.type = JUMP_SHOOTER_PROJECTILE;
                    projectile_entity->projectile.velocity = direction * speed;
                    projectile_entity->projectile.max_lifetime = 15;
                    projectile_entity->enemy.gives_ammo = false;
                    
                    if (shooter->shoot_sword_blockers){
                        projectile_entity->enemy.blocker_clockwise = shooter->blocker_clockwise;
                        projectile_entity->enemy.blocker_immortal  = shooter->shoot_sword_blockers_immortal;
                    }
                    if (shooter->shoot_bullet_blockers){
                        projectile_entity->enemy.shoot_blocker_immortal = true;
                    }
                    
                    add_and_enable_entity_particle_emitter(projectile_entity, &small_air_dust_trail_emitter_copy, projectile_entity->position, true);
                    projectile_entity->enemy.alarm_emitter_index = add_and_enable_entity_particle_emitter(projectile_entity, &alarm_smoke_emitter_copy, projectile_entity->position, true);
                    init_entity(projectile_entity);
                }
                
                shooter->velocity = dir_to_player * -30 + Vector2_up * 100;
                
                shooter->states.charging = false;
                shooter->states.in_recoil = true;
                shooter->states.recoil_start_time = core.time.game_time;
                
                state_context.timers.last_jump_shooter_attack_time = core.time.game_time;
            }
        }
        
        if (shooter->shoot_sword_blockers){
            // Mostly visual change direction above blocker head
            f32 time_since_block_direction_change = core.time.game_time - shooter->last_visual_blocker_direction_change_time;
            if (time_since_block_direction_change >= block_direction_switch_time){
                shooter->blocker_clockwise = !shooter->blocker_clockwise;
                shooter->last_visual_blocker_direction_change_time = core.time.game_time;
            }
        }
        
        if (shooter->states.in_recoil){
            f32 in_recoil_time = core.time.game_time - shooter->states.recoil_start_time;
            f32 max_recoil_time = 1.0f;
            
            rotate(e, shooter->velocity.x * 20 * dt);
            
            f32 gravity_multiplier = shooter->velocity.y > 0 ? 1.5f : 0.7f;
            shooter->velocity.y -= GRAVITY * gravity_multiplier * dt;
            
            if (in_recoil_time >= max_recoil_time){
                shooter->states.in_recoil = false;
                shooter->states.picking_point = true;
                shooter->states.picking_point_start_time = core.time.game_time;
            }
        }
        
        if (shooter->states.picking_point){
            f32 picking_point_time = core.time.game_time - shooter->states.picking_point_start_time;
            f32 picking_point_t = clamp01(picking_point_time / shooter->max_picking_point_time);
            
            Move_Point next_point = shooter->move_points.get((shooter->current_index + 1) % shooter->move_points.count);
            
            Vector2 vec_to_point = next_point.position - e->position;
            Vector2 dir = normalized(vec_to_point);
            
            move_vec_towards(&shooter->velocity, Vector2_zero, lerp(0.0f, 100.0f, sqrtf(picking_point_t)), dt);
            
            f32 look_speed = lerp(0.0f, 14.0f, picking_point_t * picking_point_t);
            change_up(e, move_towards(e->up, dir, look_speed, dt));
            
            Vector2 target_scale = {e->enemy.original_scale.x * 1.4f, e->enemy.original_scale.y * 1.7f};
            change_scale(e, lerp(e->enemy.original_scale, target_scale, picking_point_t * picking_point_t));
            
            if (trail_emitter){
                trail_emitter->direction = e->up * -1;
                trail_emitter->count_multiplier = lerp(1.0f, 10.0f, sqrtf(picking_point_t));
                trail_emitter->speed_multiplier = lerp(1.0f, 10.0f, sqrtf(picking_point_t));
                trail_emitter->over_time = 1.0f;
            }   
            // jump shooter fly to next
            if (picking_point_time >= shooter->max_picking_point_time){
                shooter->states.picking_point = false;
                shooter->states.flying_to_point = true;
                shooter->states.flying_start_time = core.time.game_time;
                
                shooter->velocity = dir * 400;
                if (flying_emitter){
                    flying_emitter->position = e->position - e->up * e->scale.y * 0.5f;
                    enable_emitter(flying_emitter);
                }
                
                if (trail_emitter){
                    trail_emitter->count_multiplier = 1;
                    trail_emitter->speed_multiplier = 1;
                    trail_emitter->over_time = 2.0f;
                }
            }
        }
        
        if (shooter->states.flying_to_point){
            f32 flying_time = core.time.game_time - shooter->states.flying_start_time;
            f32 max_flying_time = 1.2f;
            f32 flying_t = clamp01(flying_time / max_flying_time);
            // when we fly we just wait for ground collision to change state and if it took too long - we should die
            
            Vector2 target_scale = {e->enemy.original_scale.x * 1.4f, e->enemy.original_scale.y * 1.7f};
            change_scale(e, lerp(target_scale, e->enemy.original_scale, sqrtf(flying_t)));
            
            if (flying_time >= 10.0f){
                kill_enemy(e, e->position, e->up);
            }
            
            Move_Point target_point = shooter->move_points.get((shooter->current_index + 1) % shooter->move_points.count);
            Vector2 vec_to_point = target_point.position - e->position;
            Vector2 dir = normalized(vec_to_point);
            f32 len = magnitude(vec_to_point);
            
            change_up(e, lerp(dir, target_point.normal, clamp01(EaseInOutQuad(flying_t) + lerp(0.0f, 1.0f, 1.0f - clamp01(len / 30.0f)))));
            
            if (is_enemy_should_trigger_death_instinct(e, shooter->velocity, dir_to_player, distance_to_player, true)){
                start_death_instinct(e, ENEMY_ATTACKING);          
            }
        }
        
        move_by_velocity_with_collisions(e, shooter->velocity, e->scale.x * 0.5f + e->scale.y * 0.5f, &respond_jump_shooter_collision, dt);
        
        if (trail_emitter){
            trail_emitter->position = e->position - e->up * e->scale.y * 0.5f;
            if (!shooter->states.picking_point && shooter->velocity != Vector2_zero){
                trail_emitter->direction = normalized(shooter->velocity * -1);
            }
        }
        
        if (shooter->states.flying_to_point){
            if (flying_emitter){
                flying_emitter->position  = e->position - e->up * e->scale.y * 0.5f;
                if (shooter->velocity != Vector2_zero){
                    flying_emitter->direction = normalized(shooter->velocity * -1);
                }
            }
        }                
    } // end update jump shooter
    
    if (e->flags & DOOR){
        update_door(e);
    }
    
    return true;
} //update entity end

void update_entities(f32 dt){
    // update turrets ticks
    state_context.turret_state.ticked_this_frame = false;
    state_context.turret_state.tick_countdown -= dt;
    if (state_context.turret_state.tick_countdown <= 0){
        // 0.2f it's just arbitrary value for turret tick.
        state_context.turret_state.tick_countdown += 0.2f;
        state_context.turret_state.current_tick += 1;
        state_context.turret_state.ticked_this_frame = true;
    }

    Hash_Table_Int<Entity> *entities = &current_level_context->entities;
    
    if (core.time.app_time - state_context.timers.last_collision_cells_clear_time >= 0.2f){
        update_all_collision_cells();        
    }
    
    for (i32 entity_index = 0; entity_index < entities->max_count; entity_index++){
        if (!entities->has_index(entity_index)){
            continue;
        }
    
        Entity *e = entities->get_ptr(entity_index);
        
        if (e->flags & PLAYER){
            if (need_destroy_player){
                destroy_player();   
                need_destroy_player = false;
            }
        }
        
        if (e->destroyed){
            free_entity(e);
            entities->remove_index(entity_index);    
            continue;
        }
        
        if (e->enabled && game_state == GAME && e->spawn_enemy_when_no_ammo && player_data->ammo_count <= 0 && (!current_level_context->entities.has_key(e->spawned_enemy_id) || e->spawned_enemy_id == -1)){
            Entity *spawned = spawn_object_by_name("ammo_pack", e->position);
            e->spawned_enemy_id = spawned->id;
        }
        
        if (!e->enabled || (e->hidden && game_state == GAME && !state_context.in_pause_editor)){
            continue;
        }
        
        // verify kill switch
        if (e->flags & KILL_SWITCH){
            Kill_Switch *kill_switch = &e->enemy.kill_switch;
            for (i32 i = 0; i < kill_switch->connected.count; i++){
                Entity *connected = get_entity_by_id(kill_switch->connected.get(i));
                if (!connected){
                    kill_switch->connected.remove(i);
                    i--;
                    continue;
                }
                // That could happen if entity was destroyed and before that code happens - someone occupies that slot. 
                // I want to know it that will ever happen so i'll leave log here.
                if (!(connected->flags & ENEMY)){
                    kill_switch->connected.remove(i);
                    i--;
                    print("FUNNY situation at verify kill switch");
                    continue;
                }
            }
        }
        
        if (game_state == EDITOR || state_context.in_pause_editor){
            update_color_changer(e, dt);
            if (game_state == EDITOR){
                update_editor_entity(e);
            }
            continue;
        }
    
        update_entity_collision_cells(e);
        if (!update_entity(e, dt)){
            break;
        }
    } // update entities end
}

void move_vec_towards(Vector2 *current, Vector2 target, f32 speed, f32 dt){
    *current = move_towards(*current, target, speed, dt);
}

Vector2 move_towards(Vector2 current, Vector2 target, f32 speed, f32 dt){
    Vector2 vec = (target - current);
    f32 len = magnitude(vec);
    Vector2 dir = normalized(vec);
    
    f32 target_move_len = speed * dt;
    if (target_move_len > len){
        target_move_len = len;
    }
    
    current += dir * target_move_len;
    
    return current;
}

inline void draw_player(Entity *entity){
    assert(entity->flags & PLAYER);
    
    if (player_data->dead_man){
        return;
    }
    
    draw_game_triangle_strip(entity);
    
    if (is_death_instinct_in_cooldown()){
        f32 cooldown_left = state_context.death_instinct.cooldown_start_time + state_context.death_instinct.cooldown - core.time.app_time;
        draw_game_text(entity->position + Vector2_up * 8, text_format("%.1f", cooldown_left), 44, YELLOW);
        state_context.death_instinct.was_in_cooldown = true;
    } else{
        if (state_context.death_instinct.was_in_cooldown){
            state_context.death_instinct.was_in_cooldown = false;
            play_sound("ScifyOne", 1.5f);
        }
        
        draw_game_ring_lines(entity->position, entity->scale.y * 1.05f, entity->scale.y * 2.0f, 5, YELLOW, core.time.game_time * 4, core.time.game_time * 2 + 360);
    }
}

inline Vector2 get_perlin_in_circle(f32 speed){
    return {perlin_noise3_seed(core.time.game_time * speed, 1, 2, rnd(0, 10000)), perlin_noise3_seed(1, core.time.game_time * speed, 3, rnd(0, 10000))};
}

inline Vector2 get_perlin_in_circle(f32 speed, f32 seed1, f32 seed2){
    return {perlin_noise3_seed(core.time.game_time * speed, seed1, seed2, rnd(0, 10000)), perlin_noise3_seed(seed2, core.time.game_time * speed, seed1, rnd(0, 10000))};
}

inline void draw_sword(Entity *entity){
    assert(entity->flags & SWORD);
    
    Entity visual_entity = *entity;
    
    Vector2 handle_end = visual_entity.position + visual_entity.up * visual_entity.scale.y * 0.2f;
    draw_game_line(visual_entity.position, handle_end, visual_entity.scale.x * 0.2f, BLACK);
    
    draw_game_triangle_strip(&visual_entity);
}

inline void draw_rifle(Entity *entity){
    Entity visual_entity = *entity;
    
    f32 time_since_shake = core.time.game_time - player_data->timers.rifle_shake_start_time;
    
    if (time_since_shake <= 0.2f){
        Vector2 perlin_rnd = {perlin_noise3(core.time.game_time * 30, 1, 2), perlin_noise3(1, core.time.game_time * 30, 3)};
        visual_entity.position += perlin_rnd * 1.8f;
    }
    
    visual_entity.color = ColorBrightness(GREEN, 0.3f);
    
    // Vector2 handle_end = visual_entity.position + visual_entity.up * visual_entity.scale.y * 0.2f;
    // draw_game_line(visual_entity.position, handle_end, visual_entity.scale.x * 0.2f, BLACK);
    
    draw_game_triangle_strip(&visual_entity);
    
    draw_game_line_strip(&visual_entity, WHITE);
}

inline Collision get_ray_collision_to_player(Entity *entity, FLAGS collision_flags, f32 reduced_len){
    if (!player_entity){
        print("WARNING: Tried to get ray collision to player, but player is not present");
        return {};     
    }
    
    Vector2 vec_to_player = player_entity->position - entity->position;
    Vector2 dir = normalized(vec_to_player);
    f32 len = magnitude(vec_to_player);
    return raycast(entity->position, dir, len - reduced_len, collision_flags, 6, entity->id);
}

inline void draw_bird_enemy(Entity *entity){
    assert(entity->flags & BIRD_ENEMY);
    
    Entity visual_entity = *entity;
    if (entity->bird_enemy.charging){
        f32 charging_time = core.time.game_time - entity->bird_enemy.charging_start_time;
        f32 t = clamp01(charging_time / entity->bird_enemy.max_charging_time);
        visual_entity.position += get_perlin_in_circle(30) * lerp(0.0f, 1.0f, t * t);
    }
    
    draw_game_triangle_strip(&visual_entity);
    // draw_game_line_strip(&visual_entity, RED);
    make_outline(visual_entity.position, visual_entity.vertices, RED);
}

int compare_entities_draw_order(const void *first, const void *second){
    Entity *entity1 = (Entity*)first;
    Entity *entity2 = (Entity*)second;
    
    if (entity1->draw_order == entity2->draw_order){
        return 0;
    }
    
    return entity1->draw_order < entity2->draw_order ? 1 : -1;
}

Bounds get_cam_bounds(Cam cam, f32 zoom){
    Bounds cam_bounds;
    cam_bounds.size = {(f32)screen_width, (f32)screen_height};
    cam_bounds.size /= zoom;
    cam_bounds.size /= current_level_context->cam.unit_size;
    
    cam_bounds.offset = {0, 0};
    
    return cam_bounds;
}

// inline b32 should_draw_entity_anyway(Entity *e){
//     b32 is_should_draw_anyway = e->flags & TRIGGER || (e->flags & MOVE_SEQUENCE && game_state != GAME);
//     return is_should_draw_anyway;
// }

inline b32 should_not_draw_entity(Entity *e, Cam cam){
    Bounds cam_bounds = get_cam_bounds(cam, cam.cam2D.zoom);
    return /*!should_draw_entity_anyway(e) && */(!check_bounds_collision(cam.view_position, cam_bounds, e) || !e->enabled);
}

inline f32 get_turret_charge_progress(Turret *turret){
    Turret_State *state = &state_context.turret_state;
    f32 between_tick_time       = (f32)(turret->shoot_every_tick) * state->tick_delay;
    f32 from_previous_tick_time = (f32)(state->current_tick - turret->last_shot_tick) * state->tick_delay + (state->tick_delay - state->tick_countdown);
    return clamp01(from_previous_tick_time / between_tick_time);
}

void fill_entities_draw_queue(){
    session_context.entities_draw_queue.clear();
    
    // That also acts entities loop on draw update call. For example we use it for some immediate stuff that should
    // work on occluded entities.
    ForEntities(entity, 0){
        if (!entity->enabled){
            continue;
        }
        
        if (entity->hidden && game_state == GAME && !state_context.in_pause_editor/* && !should_draw_entity_anyway(&e)*/){
            continue;
        }
        
        // always draw bird
        if (entity->flags & BIRD_ENEMY){ 
            Bird_Enemy *bird = &entity->bird_enemy;
            local_persist Color charging_line_color  = Fade(ORANGE, 0.3f);
            local_persist Color attacking_line_color = Fade(RED, 0.6f);
            local_persist f32 charging_line_width = 1.5f;
            local_persist f32 attacking_line_width = 7.0f;
            if (bird->charging && !entity->enemy.dead_man){
                f32 charging_time = core.time.game_time - entity->bird_enemy.charging_start_time;
                f32 t = clamp01(charging_time / entity->bird_enemy.max_charging_time);
                Color attack_line_color = color_fade(charging_line_color, t * t);
                f32 attack_line_width = lerp(0.0f, charging_line_width, t * t);
                Vector2 attack_line_target_position = player_entity->position;
                // @TODO Should make this ray collision check so that line would stop when bird will not fly all the way to player.
                // Will do that when we'll perform collision optimizations.
                // Collision ray_collision = get_ray_collision_to_player(entity, entity->collision_flags, 2);
                // if (ray_collision.collided){
                //     attack_line_target_position = ray_collision.point;
                //     attack_line_color = color_fade(attack_line_color, 0.5f);
                // }
                make_line(entity->position, attack_line_target_position, attack_line_width, attack_line_color);
            }
            
            if (bird->attacking && !entity->enemy.dead_man){
                f32 attacking_time = core.time.game_time - bird->attack_start_time;
                f32 t = clamp01(attacking_time / bird->max_attack_time);
                
                Color attack_line_color = color_fade(attacking_line_color, (1.0f - t) * (1.0f - t));
                if (t <= 0.1f){
                    attack_line_color = lerp(charging_line_color, attack_line_color, t * 10);
                }
                
                f32 attack_line_width = 0;
                if (t <= 0.1f){
                    attack_line_width = lerp(charging_line_width, attacking_line_width, EaseOutElastic(t * 10.0f));
                } else{
                    attack_line_width = lerp(attacking_line_width, 0.5f, EaseOutElastic((t - 0.1f) / 0.9f));
                }
                
                Vector2 target_position = player_entity->position;
                if (dot(target_position - entity->position, entity->up) <= 0){
                    target_position = entity->position + entity->up * 200;
                    attack_line_color = color_fade(attack_line_color, 0.2f);
                }
                
                make_line(entity->position, target_position, attack_line_width, attack_line_color);
            }
        }
        
        // always draw turret
        if (entity->flags & TURRET){
            Turret *turret = &entity->enemy.turret;
            if (turret->homing && turret->see_player){
                f32 t = get_turret_charge_progress(turret);
                
                Color line_color = color_fade(Fade(RED, 0.6f), t);
                f32 line_width = lerp(0.0f, 3.0f, t * t * t);
                
                Vector2 target_position = player_entity->position;
                
                make_line(entity->position + entity->up * entity->scale.y * entity->pivot.y, target_position, line_width, line_color);
            }
            
            if (should_draw_editor_hints() && editor.selected_entity && entity->id == editor.selected_entity->id && turret->homing){
                // draw_game_circle(entity->position, turret->shoot_radius, Fade(RED, 0.2f));
                draw_game_rect(entity->position, {turret->shoot_width, turret->shoot_height}, {0.5f, 0.5f}, 0, Fade(RED, 0.2f));
            }
        }
        
        // always draw move sequence
        if (entity->flags & MOVE_SEQUENCE && should_draw_editor_hints()){
            if (entity->move_sequence.speed_related_player_distance && editor.selected_entity && editor.selected_entity->id == entity->id){
                draw_game_circle(entity->position, entity->move_sequence.max_distance, Fade(RED, 0.05f));
                draw_game_circle(entity->position, entity->move_sequence.min_distance, Fade(BLUE, 0.2f));
            }
                
            for (i32 ii = 0; ii < entity->move_sequence.points.count; ii++){
                Vector2 point = entity->move_sequence.points.get(ii);
                
                Color color = editor.selected_entity && editor.selected_entity->id == entity->id ? ColorBrightness(GREEN, 0.2f) : Fade(BLUE, 0.2f);
                
                if (IsKeyDown(KEY_LEFT_ALT)){
                    draw_game_circle(point, 1  * (0.4f / current_level_context->cam.cam2D.zoom), SKYBLUE);
                    draw_game_text(point - Vector2_up, text_format("%d", ii), 18 / current_level_context->cam.cam2D.zoom, RED);
                    
                    if (entity->flags & JUMP_SHOOTER){
                        Collision nearest_ground = get_nearest_ground_collision(point, 20);
                        if (nearest_ground.collided){
                            Collision ray_collision = raycast(point, normalized(nearest_ground.point - point), magnitude(nearest_ground.point - point), GROUND, 1);
                            if (ray_collision.collided){
                                make_line(ray_collision.point, ray_collision.point + ray_collision.normal * 5, GREEN);
                            }
                        } else{
                            draw_game_circle(point, 1 * (0.4f / current_level_context->cam.cam2D.zoom), RED);
                        }
                    }
                }
                if (ii < entity->move_sequence.points.count - 1){
                    make_line(point, entity->move_sequence.points.get(ii+1), color);
                } else if (entity->move_sequence.loop){
                    make_line(point, entity->move_sequence.points.get(0), color);
                }
            }
        }
        
        // always draw explosive
        if (entity->flags & EXPLOSIVE){
            if (game_state == EDITOR){
                draw_game_circle(entity->position, get_explosion_radius(entity), Fade(ORANGE, 0.1f));
            }
        }
        
        // always draw trigger
        if (entity->flags & TRIGGER){
            Trigger *trigger = &entity->trigger;
            if (should_draw_editor_hints()){
                // draw cam zoom trigger draw trigger zoom draw trigger cam
                if (trigger->change_zoom){
                    Bounds cam_bounds = get_cam_bounds(current_level_context->cam, trigger->zoom_value);
                    Vector2 position = entity->position;
                    if (trigger->lock_camera){
                    }
                    draw_game_circle(trigger->locked_camera_position, 2, PINK);
                    
                    Color cam_border_color = Fade(PINK, 0.15f);
                    if (editor.selected_entity && editor.selected_entity->id == entity->id){
                        cam_border_color = Fade(ColorBrightness(PINK, 0.3f), 0.45f);
                    }
                    position = trigger->locked_camera_position;
                    make_rect_lines(position + cam_bounds.offset, cam_bounds.size, {0.5f, 0.5f}, 2.0f / (current_level_context->cam.cam2D.zoom), cam_border_color);
                    draw_game_text((position + cam_bounds.offset) - cam_bounds.size * 0.5f, text_format("%.2f", trigger->zoom_value), 18.0f / current_level_context->cam.cam2D.zoom, ColorBrightness(color_fade(cam_border_color, 1.5f), 0.5f));
                }
                
                if (trigger->lock_camera){
                }
                
                if (trigger->start_cam_rails_horizontal || trigger->start_cam_rails_vertical){
                    for (i32 ii = 0; ii < trigger->cam_rails_points.count; ii++){
                        Vector2 point = trigger->cam_rails_points.get(ii);
                        
                        Color color = editor.selected_entity && editor.selected_entity->id == entity->id ? ColorBrightness(WHITE, 0.2f) : ColorBrightness(Fade(WHITE, 0.1f), 0.05f);
                        
                        if (IsKeyDown(KEY_LEFT_ALT)){
                            draw_game_circle(point, 1  * (0.4f / current_level_context->cam.cam2D.zoom), SKYBLUE);
                            draw_game_text(point - Vector2_up, text_format("%d", ii), 18 / current_level_context->cam.cam2D.zoom, RED);
                        }
                        if (ii < trigger->cam_rails_points.count - 1){
                            make_line(point, trigger->cam_rails_points.get(ii+1), color);
                        } 
                    }
                }
            }
            
            b32 is_trigger_selected = editor.selected_entity && editor.selected_entity->id == entity->id || (IsKeyDown(KEY_LEFT_ALT) && should_draw_editor_hints());
            f32 since_triggered = core.time.game_time - trigger->triggered_time;
            for (i32 ii = 0; ii < trigger->connected.count; ii++){
                Entity *connected = get_entity_by_id(trigger->connected.get(ii));
                
                if (!connected){
                    continue;
                }
                
                if (connected->flags & DOOR && ((entity->flags ^ TRIGGER) > 0 || game_state != GAME)){
                    Color color = connected->door.is_open == trigger->open_doors ? SKYBLUE : ORANGE;
                    f32 width = connected->door.is_open == trigger->open_doors ? 1.0f : 0.2f;
                    make_line(entity->position, connected->position, width, Fade(ColorBrightness(color, 0.2f), 0.3f));
                } else if (is_trigger_selected && should_draw_editor_hints()){
                    make_line(entity->position, connected->position, RED);
                }
                
                if (trigger->triggered && since_triggered <= 1.5f){
                    f32 full_t = since_triggered / 1.5f;
                    Color start_color = Fade(PURPLE, 0);
                    Color target_color = Fade(ColorBrightness(PURPLE, 0.15f), 0.6f);
                    Color line_color;
                    
                    f32 target_thick = 2.0f;
                    f32 thick;
                    if (full_t <= 0.3f){
                        f32 t = full_t / 0.3f;
                        line_color = lerp(start_color, target_color, t);
                        thick = lerp(0.0f, target_thick, EaseInOutElastic(t));
                    } else{
                        f32 t = (full_t - 0.3f) / 0.7f;
                        line_color = color_fade(target_color, 1.0f - t);
                        thick = lerp(target_thick, 0.0f, t * t);
                    }
                    
                    make_line(entity->position, connected->position, thick, line_color);
                }
            }
            for (i32 ii = 0; ii < trigger->tracking.count; ii++){
                i32 id = trigger->tracking.get(ii);
                Entity *tracked_entity = get_entity_by_id(id);
                if (!tracked_entity){
                    continue;
                }
                                
                if (is_trigger_selected && should_draw_editor_hints()){
                    make_line(entity->position, tracked_entity->position, GREEN);
                } else if (trigger->draw_lines_to_tracked && game_state != EDITOR){
                    if ((tracked_entity->flags & ENEMY | CENTIPEDE) && !tracked_entity->enemy.dead_man){
                        make_line(entity->position, tracked_entity->position, 1.0f, Fade(PINK, 0.3f));
                    }
                }
            }
        }
        
        // always draw kill switch
        if (entity->flags & KILL_SWITCH){
            Kill_Switch *kill_switch = &entity->enemy.kill_switch;
            for (i32 i = 0; i < kill_switch->connected.count; i++){
                Entity *connected = get_entity_by_id(kill_switch->connected.get(i));                
                if (!connected){
                    continue;
                }
                
                f32 width = 3.0f;
                Vector2 first = entity->position;
                Vector2 second = {connected->position.x, entity->position.y};
                Vector2 third = connected->position;
                Color color = Fade(YELLOW, 0.7f);
                draw_game_line(first, second, width, color);
                draw_game_line(second, third, width, color);
            }
        }
        
        // always draw sticky texture
        if (entity->flags & STICKY_TEXTURE){
            Sticky_Texture *st = &entity->sticky_texture;
            Entity *follow_entity = get_entity_by_id(entity->sticky_texture.follow_id);
            
            // We make lights only for immortal sticky textures - like blocker sign.
            if (follow_entity && st->max_lifetime <= 0){
                if (follow_entity->flags & BLOCKER && st->should_draw_texture){
                    make_light(follow_entity->position, 75, 1, 1.0f, WHITE);
                }
                
                if (follow_entity->flags & SWORD_SIZE_REQUIRED && st->should_draw_texture){
                    make_light(follow_entity->position, 75, 1.5, 0.7f, follow_entity->enemy.big_sword_killable ? ColorBrightness(RED, 0.4f) : BLUE);
                }
            }
            
            f32 lifetime = core.time.game_time - st->birth_time;
            f32 lifetime_t = 0.5f;
            if (st->max_lifetime > EPSILON){
                lifetime_t = lifetime / st->max_lifetime;
            }
        
            if (st->draw_line && st->need_to_follow && player_entity){
                Entity *follow_entity = current_level_context->entities.get_by_key_ptr(st->follow_id);
                Color line_color = st->line_color;
                if (follow_entity && follow_entity->flags & ENEMY && st->max_lifetime > 0 && !(follow_entity->flags & SHOOT_STOPER)){
                    line_color = follow_entity->enemy.dead_man ? color_fade(SKYBLUE, 0.3f) : color_fade(RED, 0.3f);
                }
    
                Vector2 vec_to_follow = entity->position - player_entity->position;
                f32 len = magnitude(vec_to_follow);
                if (len <= st->max_distance || st->max_distance <= 0){
                    make_line(player_entity->position, entity->position, st->line_width, lerp(line_color, color_fade(line_color, 0), lifetime_t * lifetime_t));
                }
            }
        }
        
        // This checks for occlusion.
        if (should_not_draw_entity(entity, current_level_context->cam)){
            entity->visible = false;
            continue;
        } else{
            entity->visible = true;
        }
        
        session_context.entities_draw_queue.add(*entity);
    }
    
    qsort(session_context.entities_draw_queue.data, session_context.entities_draw_queue.count, sizeof(Entity), compare_entities_draw_order);
}

#define MAX_LINE_STRIP_POINTS 1024

Array<Vector2, MAX_LINE_STRIP_POINTS> line_strip_points;

void draw_spikes(Entity *e, Vector2 side_direction, Vector2 up_direction, f32 width, f32 height){
    if (drawing_state != CAMERA_DRAWING){
        return;
    }

    line_strip_points.clear();
    f32 frequency = 2;
    Vector2 start_position = e->position - side_direction * width * 0.5f;
    Vector2 end_position   = e->position + side_direction * width * 0.5f;
    
    Vector2 vertical_addition = up_direction * height * 0.8f;
    
    Vector2 vec = end_position - start_position;
    Vector2 dir = normalized(vec);
    f32 len = magnitude(vec);
    
    b32 spike = false;
    for (f32 ii = -frequency; ii <= len + frequency; ii += frequency){
        Vector2 position = start_position + dir * ii + (spike ? vertical_addition : Vector2_zero);
        line_strip_points.add(position);
        spike = !spike;
        
        // Vector2 a = start_position + dir * ii;
        // Vector2 b = a + dir * frequency + vertical_addition;
        // Vector2 c = b + dir * frequency - vertical_addition;
        // draw_game_triangle(c, b, a, e->hidden ? Fade(RED, 0.3f) : RED);
    }
    
    Color color = Fade(e->color, 0.1f);
    if (e->hidden){
        e->color = color_fade(color, 0.2f);
    }
    
    draw_game_triangle_strip(e, color);
    draw_game_line_strip(line_strip_points.data, line_strip_points.count, e->hidden ? Fade(RED, 0.3f) : RED);
}

inline Vector2 get_shoot_stoper_cross_position(Entity *entity){
    return entity->position + entity->up * entity->scale.y * 0.85f;
}

inline b32 should_draw_editor_hints(){
    return (game_state == EDITOR || state_context.in_pause_editor || debug.draw_areas_in_game);
}

void draw_entity(Entity *e){
    if (!e || !current_level_context->entities.has_key(e->id)){
        return;
    }

    if (!e->enabled){
        return;
    }
    
    if (e->flags & TEXTURE){
        // draw texture
        i32 exclude_flags = NOTE;
        
        if (!(e->flags & exclude_flags)){
            Vector2 position = e->position;
            // draw sticky texture texture
            if (e->flags & STICKY_TEXTURE){
                if (e->sticky_texture.should_draw_texture){
                    change_scale(e, (e->sticky_texture.base_size) / fminf(current_level_context->cam.cam2D.zoom, 0.35f)); 
                    make_texture(e->texture, position, e->scale, e->pivot, e->rotation, Fade(e->color, ((f32)e->color.a / 255.0f) * e->sticky_texture.alpha));
                }
            } else{
                draw_game_texture(e->texture, position, e->scale, e->pivot, e->rotation, e->color);
            }
        }
    }
    
    // draw note
    if (e->flags & NOTE){
        assert(e->note_index != -1);
        Note *note = current_level_context->notes.get_ptr(e->note_index);
        if (game_state == EDITOR || state_context.in_pause_editor){
            make_texture(e->texture, e->position, e->scale, e->pivot, e->rotation, e->color);
            // draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
            if (editor.selected_entity && editor.selected_entity->id == e->id || (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_ALT)) || focus_input_field.in_focus && str_contains(focus_input_field.tag, text_format("%d", e->id))){
                Vector2 note_size = {screen_width * 0.2f, screen_height * 0.2f};
                i32 content_count = str_len(note->content);
                f32 chars_scaling_treshold = 200 * UI_SCALING;
                if (content_count > chars_scaling_treshold){
                    note_size *= lerp(1.0f, 2.5f, clamp01(((f32)content_count - chars_scaling_treshold) / (chars_scaling_treshold * 4)));
                }
                if (make_input_field(note->content, world_to_screen_with_zoom(e->position + (cast(Vector2){e->scale.x * 0.5f, e->scale.y * -0.5f})), note_size, text_format("note%d", e->id))){
                    str_copy(note->content, focus_input_field.content);
                }
            }
        } else if (note->draw_in_game && game_state == GAME){
            draw_game_text(e->position, note->content, 50, note->in_game_color);
        }
    }
    
    if (e->flags & GROUND || e->flags & PLATFORM || e->flags == 0 || e->flags & PROJECTILE){
        // draw ground
        if (e->vertices.count > 0){
            draw_game_triangle_strip(e);
        } else{
            draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
        }
    }
    
    if (e->flags & NO_MOVE_BLOCK){
        make_outline(e->position, e->vertices, PURPLE);
    }
    
    if (e->flags & DOOR){
        if (editor.selected_entity && editor.selected_entity->id == e->id){
            Vector2 previous_position = e->position;
            Vector2 target_position = e->door.is_open ? e->door.closed_position : e->door.open_position;
            // make_line(e->position, target_position, GREEN);
            
            e->position = target_position;
            f32 color_blink = abs(sinf(core.time.app_time * 2) * 0.5f);
            draw_game_triangle_strip(e, Fade(LIME, color_blink * 0.5f + 0.2f));
            draw_game_line_strip(e, ColorBrightness(LIME, color_blink));
            e->position = previous_position;
        }
    }
    
    if (e->flags & PHYSICS_OBJECT){
        // draw physics object
        if (e->physics_object.on_rope && game_state == EDITOR){
            Vector2 start_point = e->position + e->up * e->scale.y * 0.5f;
            make_line(start_point, e->physics_object.rope_point, 1, BLACK);
        }
    }
    
    if (e->flags & BLOCK_ROPE){
        draw_game_triangle_strip(e);
    }
    
    if (e->flags & ROPE_POINT){
        draw_game_circle(e->position, e->scale.x * 0.8f, e->color);
    }
    
    if (e->flags & DUMMY){
        // draw dummy
        if (game_state == EDITOR || state_context.in_pause_editor){
            draw_game_triangle_strip(e);
            draw_game_line_strip(e, SKYBLUE);
        }
    }
    
    if (e->flags & PLAYER){
        // draw player
        draw_player(e);
    }
    
    if (e->flags & SWORD){
        // draw sword
        draw_sword(e);
    }
    
    if (e->flags & SHOOT_STOPER){
        // draw shoot stoper
        f32 line_width = e->scale.x * 0.1f;
        Vector2 top = e->position + e->up * e->scale.y * 0.5f;
        Vector2 cross_position = get_shoot_stoper_cross_position(e);
        
        draw_game_line(top, top + e->up * e->scale.y * 0.5f, line_width, BLACK);
        draw_game_line(cross_position - e->right * e->scale.x * 0.6f, cross_position + e->right * e->scale.x * 0.6f, line_width, BLACK);
    }
    
    // draw enemies
    if (e->flags & BIRD_ENEMY){
        draw_bird_enemy(e);
    } else if (e->flags & CENTIPEDE_SEGMENT){
        Color color = e->color;
        if (e->enemy.dead_man){
            //color = Fade(color, 0.3f);
            color = Fade(BLACK, 0.3f);
        }
        draw_game_triangle_strip(e, color);
        if (e->centipede_head->centipede.spikes_on_right){
            draw_spikes(e, e->up, e->right, e->scale.y, e->scale.x);
        } else{
            if (!e->enemy.dead_man){
                draw_game_circle(e->position + e->right * e->scale.x * 0.5f, 2.0f, GREEN);
            }
        }
        if (e->centipede_head->centipede.spikes_on_left){
            draw_spikes(e, e->up, e->right * -1.0f, e->scale.y, e->scale.x);
        } else{
            if (!e->enemy.dead_man){
                draw_game_circle(e->position - e->right * e->scale.x * 0.5f, 2.0f, GREEN);
            }
        }
    } else if (e->flags & CENTIPEDE){
        // draw centipede
        draw_game_triangle_strip(e);
    } else if (e->flags & JUMP_SHOOTER){
        // draw jump shooter
        
        if (e->jump_shooter.states.charging){
            f32 charging_time = core.time.game_time - e->jump_shooter.states.charging_start_time;
            f32 charging_t = charging_time / e->jump_shooter.max_charging_time;
            e->position += get_perlin_in_circle(50) * lerp(0.0f, 1.0f, charging_t * charging_t);
        }
        if (e->jump_shooter.states.picking_point){
            f32 picking_point_time = core.time.game_time - e->jump_shooter.states.picking_point_start_time;
            f32 picking_point_t = picking_point_time / e->jump_shooter.max_picking_point_time;
            e->position += get_perlin_in_circle(50) * lerp(0.0f, 1.0f, picking_point_t * picking_point_t);
        }
        if (e->jump_shooter.states.flying_to_point){
            e->position += get_perlin_in_circle(25);
        }

        draw_game_triangle_strip(e);
        
        Color hint_color = Fade(ColorBrightness(WHITE, -0.2f), 0.8f);
        Vector2 bullet_hint_position = e->position + e->up * e->scale.y * 0.6f;
        Vector2 bullet_hint_scale = {e->scale.x * 0.7f, e->scale.y * 1.1f};
        
        if (e->jump_shooter.explosive_count > 0){
            Color target_color = ColorBrightness(WHITE, 4);
            f32 color_t = abs(sinf(core.time.game_time * 3));
            hint_color = lerp(hint_color, target_color, color_t);
            
            if (e->jump_shooter.states.charging){
                f32 charging_time = core.time.game_time - e->jump_shooter.states.charging_start_time;
                f32 charging_t = charging_time / e->jump_shooter.max_charging_time;
                
                f32 radius = lerp(0.0f, 40.0f, charging_t * charging_t);
                draw_game_circle(bullet_hint_position + e->up * bullet_hint_scale.y * 0.5f, radius, Fade(ORANGE, 0.1f));
            }
        }
        
        draw_game_texture(jump_shooter_bullet_hint_texture, bullet_hint_position, bullet_hint_scale, {0.5f, 1.0f}, e->rotation, hint_color);
        
        if (e->jump_shooter.shoot_bullet_blockers){
            draw_game_ring_lines(bullet_hint_position + e->up * e->scale.y * 0.5f, 3, 6, 8, Fade(WHITE, 0.5f));                
        }
        
        if (e->jump_shooter.shoot_sword_blockers){
            if (e->jump_shooter.shoot_sword_blockers_immortal){
                Vector2 center = bullet_hint_position + e->up * bullet_hint_scale.y * 0.5f;
                Vector2 triangle1 = {center.x, center.y + 5};
                Vector2 triangle2 = {center.x - 5, center.y - 5};
                Vector2 triangle3 = {center.x + 5, center.y - 5};
                draw_game_triangle_lines(triangle1, triangle2, triangle3, WHITE);
            } else{
                Vector2 scale = {3, 3};
                scale /= current_level_context->cam.cam2D.zoom;
                Texture blocker_texture = e->jump_shooter.blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
                make_texture(blocker_texture, bullet_hint_position + e->up * scale.y * 0.65f, scale, {0.5f, 0.5f}, 0, WHITE);
            }
        }
    } else if (e->flags & AMMO_PACK){
        draw_game_circle(e->position, e->scale.x, RED);
        draw_game_circle(e->position, e->scale.x * 0.5f, ColorBrightness(RED, 0.6f));
    } else if (e->flags & TURRET){ // draw turret
        Color color = e->color;
        if (!e->enemy.turret.activated){
            color = ColorBrightness(color, -0.4f);
        }
        draw_game_triangle_strip(e, color);
        if (e->enemy.turret.homing && e->enemy.turret.activated){
            // Drawing charge line
            Vector2 start = e->position - e->up * e->scale.y * (1.0f - e->pivot.y); 
            f32 length = e->scale.y * e->pivot.y;
            if (game_state == GAME && !state_context.in_pause_editor){
                f32 t = get_turret_charge_progress(&e->enemy.turret);
                length = lerp(0.0f, length, t * t * t);
            }
            draw_game_line(start, start + e->up * length, e->scale.x * 0.2f, RED);
        }
    } else if (e->flags & ENEMY && e->flags & TRIGGER){
        Color color = e->color;
        if (e->enemy.dead_man){
            color = ColorTint(color, ColorBrightness(BROWN, 0.15f));
            color = ColorBrightness(color, 0.1f);
            // color = color_fade(color, 0.6f);
        }
        draw_game_triangle_strip(e, color);
    } else if (e->flags & ENEMY){ // draw enemy
        draw_game_triangle_strip(e);
    }
    
    // draw enemy barrier
    if (e->flags & ENEMY_BARRIER){
        f32 w = e->scale.x;
        f32 h = e->scale.y;
        // Assuming pivot for barriers {0.5, 0.5}.
        
        draw_game_line(e->position + e->up * h * 0.35f - e->right * w * 0.45f, e->position + e->up * h * 0.45f - e->right * w * 0.35f, 1.0f, BLUE);
        draw_game_line(e->position + e->up * h * 0.35f + e->right * w * 0.45f, e->position + e->up * h * 0.45f + e->right * w * 0.35f, 1.0f, BLUE);
        draw_game_line(e->position - e->up * h * 0.35f - e->right * w * 0.45f, e->position - e->up * h * 0.45f - e->right * w * 0.35f, 1.0f, BLUE);
        draw_game_line(e->position - e->up * h * 0.35f + e->right * w * 0.45f, e->position - e->up * h * 0.45f + e->right * w * 0.35f, 1.0f, BLUE);
        
        draw_game_line(e->position + e->up * h * 0.35f + e->right * w * 0.25f, e->position - e->up * h * 0.35f + e->right * w * 0.25f, 0.5f, ColorBrightness(BLUE, 0.2f));
        draw_game_line(e->position + e->up * h * 0.35f - e->right * w * 0.25f, e->position - e->up * h * 0.35f - e->right * w * 0.25f, 0.5f, ColorBrightness(BLUE, 0.2f));
    }
    
    // draw multiple hits
    if (e->flags & MULTIPLE_HITS){
        f32 width = fmaxf(e->scale.x * 0.5f, 10.0f);
        f32 height = 5;
        Vector2 position = e->position - Vector2_up * 5 - Vector2_right * width * 0.5f;
        draw_game_rect(position, {width, height}, {0, 0}, 0, Fade(BROWN, 0.9f));
        
        f32 progress = (f32)e->enemy.multiple_hits.made_hits / (f32)e->enemy.multiple_hits.required_hits;
        width *= progress;
        draw_game_rect(position, {width, height}, {0, 0}, 0, PINK);
    }

    if (e->flags & WIN_BLOCK){
    }
    
    if (e-> flags & DRAW_TEXT){
        draw_game_text(e->position, e->text_drawer.text, e->text_drawer.size, RED);
    }
    
    if (e->flags & SPIKES && (!e->hidden || game_state == EDITOR || state_context.in_pause_editor)){
        draw_spikes(e, e->right, e->up, e->scale.x, e->scale.y);
    }
    
    if (e->flags & PLATFORM){
        // draw platform
        line_strip_points.clear();
        f32 frequency = 6;
        Vector2 start_position = e->position - e->right * e->scale.x * 0.5f + e->up * e->scale.y * 0.5f;
        Vector2 end_position   = e->position + e->right * e->scale.x * 0.5f + e->up * e->scale.y * 0.5f;
        
        Vector2 vertical_removal = e->up * e->scale.y * -0.35f;
        
        Vector2 vec = end_position - start_position;
        Vector2 dir = normalized(vec);
        f32 len = magnitude(vec);
        
        line_strip_points.add(start_position - e->up * e->scale.y);
        
        b32 move = false;
        for (f32 ii = 0; ii <= len - frequency * 0.8f; ii += frequency * 0.8f){
            line_strip_points.add(start_position + dir * ii);
            line_strip_points.add(start_position + dir * (ii + frequency * 0.8f));
            line_strip_points.add(start_position + dir * (ii + frequency * 0.9f) + vertical_removal);
            line_strip_points.add(start_position + dir * (ii + frequency));
        
            move = !move;
            
            ii += frequency * 0.2f;
        }
        
        line_strip_points.add(end_position - e->up * e->scale.y);
        
        draw_game_line_strip(line_strip_points.data, line_strip_points.count, BROWN);
    }
    
    // draw trigger
    if (e->flags & TRIGGER){
        if (should_draw_editor_hints()){
            draw_game_line_strip(e, e->color);
            draw_game_triangle_strip(e, Fade(e->color, 0.1f));
        }
    }
    
    if (e->flags & EXPLOSIVE){
        if (e->light_index > -1){
        }
    }
    
    if (e->flags & PROPELLER && (game_state == EDITOR || state_context.in_pause_editor || debug.draw_areas_in_game)){
        draw_game_line_strip(e, e->color);
        draw_game_triangle_strip(e, e->color * 0.1f);
    }
    
    if (e->flags & BLOCKER && (game_state == EDITOR) && !e->enemy.blocker_immortal){
        Texture texture = e->enemy.blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
        
        draw_game_texture(texture, e->position, {10.0f, 10.0f}, {0.5f, 0.5f}, 0, Fade(WHITE, 0.6f));
    }
    if (e->flags & BLOCKER && e->enemy.blocker_immortal){
        Vector2 triangle1 = {e->position.x, e->position.y + 3};
        Vector2 triangle2 = {e->position.x - 3, e->position.y - 3};
        Vector2 triangle3 = {e->position.x + 3, e->position.y - 3};
        draw_game_triangle_lines(triangle1, triangle2, triangle3, WHITE);
    }
    
    if (e->flags & SWORD_SIZE_REQUIRED && (game_state == EDITOR)){
        Texture texture = e->enemy.big_sword_killable ? big_sword_killable_texture : small_sword_killable_texture;
        
        draw_game_texture(texture, e->position, {10.0f, 10.0f}, {0.5f, 0.5f}, 0, WHITE);
    }

    
    if (e->flags & SHOOT_BLOCKER){
        // draw shoot blockers
        if (e->enemy.shoot_blocker_immortal){
            draw_game_ring_lines(e->position, 3, 6, 8, WHITE);                
        } else{
            Vector2 direction = get_rotated_vector(e->enemy.shoot_blocker_direction, e->rotation);
            f32 len        = e->scale.x * 0.4f + e->scale.y * 0.4f;
            
            Vector2 start_position = e->position - direction * len;
            Vector2 end_position   = e->position + direction * len;
            make_line(start_position, end_position, 1.5f, VIOLET);
            
            Vector2 right  = get_rotated_vector_90(direction, -1);
            
            make_line(end_position, end_position - right * len * 0.2f - direction * len * 0.32f, 1.0f, ColorBrightness(VIOLET, 0.2f));
            make_line(end_position, end_position + right * len * 0.2f - direction * len * 0.32f, 1.0f, ColorBrightness(VIOLET, 0.2f));
            make_line(start_position, start_position - right * len * 0.2f + direction * len * 0.32f, 1.0f, ColorBrightness(VIOLET, 0.2f));
            make_line(start_position, start_position + right * len * 0.2f + direction * len * 0.32f, 1.0f, ColorBrightness(VIOLET, 0.2f));
        }
    }
    
    if ((game_state == EDITOR || debug.draw_up_right) && editor.selected_entity && editor.selected_entity->id == e->id){
        make_line(e->position, e->position + e->right * 3, RED);
        make_line(e->position, e->position + e->up    * 3, GREEN);
    }
    
    if (debug.draw_bounds || editor.selected_entity && (game_state == EDITOR || state_context.in_pause_editor) && e->id == editor.selected_entity->id){
        make_rect_lines(e->position + e->bounds.offset, e->bounds.size, e->pivot, 1.0f / current_level_context->cam.cam2D.zoom, GREEN);
    }
}

void draw_entities(){
    fill_entities_draw_queue();

    //Hash_Table_Int<Entity> *entities = &current_level_context->entities;
    Dynamic_Array<Entity> *entities = &session_context.entities_draw_queue;
    
    for (i32 entity_index = 0; entity_index < entities->count; entity_index++){
        Entity *e = entities->get_ptr(entity_index);
        
        if (game_state == GAME && !session_context.updated_today){
            //
            // To do that we must know globally that it's imaginary update and don't do anything stupid on this update.
            // We don't want to: React to player actions besides movement. Kill player.
            // Kill other guys (even if this is imaginary entities). Spawn entities. Spawn particles. Spawn anything actually.
            // Play sound. Do screenshake. We don't want to do anything outside scope of this particular entity. Load level.
            // It will be simpler if we just put these checks in functions that do something outside scope of entity.
            // Like when we call play_sound we'll check in there that it's not real guys.
            //
            // update_entity(e, core.time.not_updated_accumulated_dt);
            e->position += get_entity_velocity(e) * core.time.not_updated_accumulated_dt;
            if (e->flags & SWORD){
                rotate(e, player_data->sword_angular_velocity * core.time.not_updated_accumulated_dt);
            }
            
            if (e->flags & STICKY_TEXTURE){
                update_sticky_texture(e, core.time.not_updated_accumulated_dt);
            }
        }
        
        draw_entity(e);
    }
}

void draw_editor(){
    f32 closest_len = 1000000;
    Entity *closest = NULL;

    Hash_Table_Int<Entity> *entities = &current_level_context->entities;

    for (i32 i = 0; i < entities->max_count; i++){
        Entity *e = entities->get_ptr(i);
        
        if (!current_level_context->entities.has_index(i)){
            continue;
        }
    
        if (!e->enabled || e->flags == -1){
            continue;
        }
        
        draw_game_circle(current_level_context->player_spawn_point, 3, BLUE);
        
        b32 draw_circles_on_vertices = IsKeyDown(KEY_LEFT_ALT);
        // draw vertices
        if (draw_circles_on_vertices){
            for (i32 v = 0; v < e->vertices.count; v++){
                Vector2 global_vertex_position = global(e, e->vertices.get(v));
                if (editor.selected_entity && editor.selected_entity->id == e->id){
                    const char *text = v == 0 ? "T" : (v == 1 ? "Y" : (v == 2 ? "F" : "G"));
                    draw_game_text(global_vertex_position, text, 22.0f / current_level_context->cam.cam2D.zoom, YELLOW);
                }
                draw_game_circle(global_vertex_position, 1.0f * (0.4f / current_level_context->cam.cam2D.zoom), PINK);
                //draw unscaled vertices
                if (IsKeyDown(KEY_LEFT_SHIFT)){    
                    draw_game_circle(global(e, e->unscaled_vertices.get(v)), 1.0f * 0.4f, PURPLE);
                }
            }
        }
        
        if (debug.draw_position){
            draw_game_text(e->position + (cast(Vector2){0, -3}), text_format("POS:   {%.2f, %.2f}", e->position.x, e->position.y), 20, RED);
        }
        
        if (debug.draw_rotation){
            draw_game_text(e->position, text_format("%d", (i32)e->rotation), 20, RED);
        }
        
        if (debug.draw_scale){
            draw_game_text(e->position + (cast(Vector2){0, -6}), text_format("SCALE:   {%.2f, %.2f}", e->scale.x, e->scale.y), 20, RED);
        }
        
        if (debug.draw_directions){
            draw_game_text(e->position + (cast(Vector2){0, -6}), text_format("UP:    {%.2f, %.2f}", e->up.x, e->up.y), 20, RED);
            draw_game_text(e->position + (cast(Vector2){0, -9}), text_format("RIGHT: {%.2f, %.2f}", e->right.x, e->right.y), 20, RED);
        }
        
        if (editor.dragging_entity != NULL && e->id != editor.dragging_entity->id){
            f32 len = magnitude(e->position - editor.dragging_entity->position);
            if (len < closest_len){
                closest_len = len;
                closest = e;
            }
        }
        
        b32 draw_normals = false;
        if (draw_normals){
            global_normals.count = 0;
            fill_arr_with_normals(&global_normals, e->vertices);
            
            for (i32 n = 0; n < global_normals.count; n++){
                Vector2 start = e->position + global_normals.get(n) * 4; 
                Vector2 end   = e->position + global_normals.get(n) * 8; 
                make_line(start, end, 0.5f, PURPLE);
                draw_game_rect(end, {1, 1}, {0.5f, 0.5f}, PURPLE * 0.9f);
            }
        }
    }
    
    if (editor.dragging_entity != NULL && closest){
        make_line(editor.dragging_entity->position, closest->position, 0.1f, PINK);
    }
    
    //editor ruler drawing
    if (editor.ruler_active){
        make_line(editor.ruler_start_position, input.mouse_position, 0.3f, BLUE * 0.9f);
        Vector2 vec_to_mouse = input.mouse_position - editor.ruler_start_position;
        f32 length = magnitude(vec_to_mouse);
        
        draw_game_text(editor.ruler_start_position + (vec_to_mouse * 0.5f), text_format("%.2f", length), 24.0f / current_level_context->cam.cam2D.zoom, RED);
        draw_game_text(input.mouse_position + Vector2_up, text_format("{%.2f, %.2f}", input.mouse_position.x, input.mouse_position.y), 26.0f / current_level_context->cam.cam2D.zoom, GREEN); 
        
    }
}

void draw_particles(){
    //@TODO: When we'll start considering particles draw order we'll want this to work with entity drawing.
    for (int emitter_index = 0; emitter_index < current_level_context->particle_emitters.max_count; emitter_index++){
        Particle_Emitter *emitter = current_level_context->particle_emitters.get_ptr(emitter_index);
        //@OPTIMIZATION: Make emitter occlusion culling.
        if (!emitter->occupied/* || !emitter.visible*/){
            continue;              
        }
        
        for (i32 emitter_index = emitter->particles_start_index; emitter_index < emitter->particles_max_index; emitter_index++){
            Particle particle = current_level_context->particles.get(emitter_index);
            if (!particle.enabled){
                continue;   
            }
            
            switch (particle.shape){
                case PARTICLE_TEXTURE:{
                    draw_game_texture(emitter->texture, particle.position, particle.scale, {0.5f, 0.5f}, particle.rotation, particle.color, false);                
                } break;
                case PARTICLE_LINE:{
                    Vector2 target_position = particle.position + particle.velocity * 0.1f * emitter->line_length_multiplier * particle.scale.y;
                    draw_game_line(particle.position, target_position, emitter->line_width * particle.scale.y, particle.color);
                } break;
                case SQUARE:{
                    draw_game_rect(particle.position, particle.scale, {0.5f, 0.5f}, particle.rotation, particle.color);
                } break;
                default: draw_game_rect(particle.position, particle.scale, {0.5f, 0.5f}, particle.rotation, particle.color);
            }
            
            if (particle.line_trail_index != -1){
                Line_Trail *line_trail = get_line_trail(particle.line_trail_index);
                if (line_trail && line_trail->occupied){
                    // We go from start index to count becasuse line trail could loop-into-itself. 
                    // For first max_count points we just add points to there will be usual array situation, but after
                    // new trail points starts to occupy start_index position and we increasing start_index so it's 
                    // working somewhat like ring buffer. Maybe it's weird. If after some time when i read this i did not 
                    // understand this code from first 20 seconds - it's really weird.
                    i32 drawed_count = 0;
                    for (i32 i = line_trail->start_index; i < line_trail->positions.count + line_trail->start_index - 1; i++){
                            drawed_count += 1;
                            i32 index  = (i % LINE_TRAIL_MAX_POINTS);
                            f32 color_t = clamp01((f32)drawed_count / (f32)line_trail->positions.count);
                            draw_game_line(line_trail->positions.get(index), line_trail->positions.get((index + 1) % LINE_TRAIL_MAX_POINTS), 0.5f, lerp(Fade(particle.color, 0), particle.color, color_t));
                    }
                    
                    i32 last_index = (line_trail->start_index + line_trail->positions.count - 1) % LINE_TRAIL_MAX_POINTS;
                    draw_game_line(line_trail->positions.get(last_index), particle.position, 0.5f, particle.color);
                } else{
                    printf("WARNING: For some reason particle have line trail index, but we could not get line trail by that index: %d (Or it's don't occupied)\n", particle.line_trail_index);
                }
            }
        }
    }
}

void draw_ui(const char *tag){
    // draw spin bar
    if (game_state == GAME){
        if (player_data->timers.slowmo_timer > 0){
            f32 t = player_data->timers.slowmo_timer / 6.0f;
            f32 opacity = lerp(0.0f, 1.0f, t * t);
            f32 width = screen_width * 0.1f;
            
            Texture vignette = get_texture("SlowmoVignette");
            Vector2 size = {(f32)screen_width / vignette.width, (f32)screen_height / vignette.height};
            BeginBlendMode(BLEND_ADDITIVE);
            draw_texture(vignette, {0, 0}, size, {0, 0}, 0, Fade(SKYBLUE, opacity));
            EndBlendMode();
        }
        
        // draw big sword charges
        {
            f32 horizontal = screen_width * 0.01f;
            f32 vertical   = screen_height * 0.2f;
            f32 width      = screen_width * 0.01f;
            f32 height     = screen_height * 0.05f;
            
            f32 spacing    = width * 1.5f;
                      
            for (i32 i = 0; i < player_data->max_big_sword_charges; i++, horizontal += spacing){
                Color color = i < player_data->current_big_sword_charges ? ColorBrightness(GREEN, 0.2f) : Fade(BROWN, 0.3f);
                draw_rect({horizontal, vertical}, {width, height}, {0, 0}, 0, color);
            }
        }
    }

    // Draw speedrun info after last level
    if (state_context.we_got_a_winner){
        // make_ui_text(
    }
    
    i32 tag_len = str_len(tag);

    for (i32 i = 0; i < ui_context.elements.count; i++){
        Ui_Element element = ui_context.elements.get(i);
        
        if (tag_len > 0 && !str_equal(element.tag, tag)){
            continue;
        }
        
        if (element.ui_flags & UI_IMAGE){
            Ui_Image ui_image = element.ui_image;
            
            if (element.has_texture){
                element.texture.width = element.size.x;
                element.texture.height = element.size.y;
                draw_texture(element.texture, element.position, {1, 1}, element.pivot, 0, element.color);
            } else{
                draw_rect(element.position, element.size, element.pivot, 0, element.color);
            }
        }
    }
    for (i32 i = 0; i < ui_context.elements.count; i++){
        Ui_Element element = ui_context.elements.get(i);
        
        if (tag_len > 0 && !str_equal(element.tag, tag)){
            continue;
        }
        
        if (element.ui_flags & BUTTON){
            Button button = element.button;
            
            draw_rect(element.position, element.size, element.pivot, 0, element.color);
            
            if (element.ui_flags & UI_TOGGLE && element.toggle_value){
                Vector2 down_pos = element.position + Vector2_up * element.size.y + Vector2_right * element.size.x * 0.5f;
                draw_line(element.position, down_pos, WHITE);
                draw_line(down_pos, down_pos + Vector2_right * element.size.x * 0.4f - Vector2_up * element.size.y * 0.9f, WHITE);
            }
            
            if (element.ui_flags & UI_COLOR_PICKER){
                draw_rect(element.position, element.size, element.pivot, 0, element.color);
                
                if (element.toggle_value){
                    draw_rect_lines(element.position, element.size * 1.1f, element.size.x * 0.075f, element.color == WHITE ? PINK : WHITE);
                }
            }
        }
        
        if (element.ui_flags & UI_TEXT){
            Ui_Text ui_text = element.text;
            draw_text(ui_text.content, element.position, ui_text.font_size, ui_text.text_color);
        }
    }

    for (i32 i = 0; i < input_fields.count; i++){
        Input_Field input_field = input_fields.get(i);
        
        if (tag_len > 0 && !str_equal(input_field.tag, tag)){
            continue;
        }
        
        Color background_color = Fade(input_field.color, 0.6f);
        if (input_field.in_focus){
            background_color = Fade(ColorTint(background_color, ColorBrightness(SKYBLUE, 0.2f)), 0.7f);
        }
        draw_rect(input_field.position, input_field.size, {0, 0}, 0, background_color);
        
        Vector2 field_position = input_field.position + Vector2_right * 3;
        Rectangle field_rec = {field_position.x, field_position.y, input_field.size.x, input_field.size.y};
        if (input_field.in_focus){
            draw_text_boxed(text_format("%s_", input_field.content), field_rec, input_field.font_size, 3, WHITE * 0.9f);
        } else{
            draw_text_boxed(input_field.content, field_rec, input_field.font_size, 3, WHITE * 0.9f);
        }
    }
    
    if (tag_len == 0){
        ui_context.elements.clear();
        input_fields.clear();
    }
}

inline b32 should_add_immediate_stuff(){
    return drawing_state == CAMERA_DRAWING && !debug.view_only_lightmaps;
}

void make_light(Vector2 position, f32 radius, f32 power, f32 opacity, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    
    Light light = {};
    light.position = position;
    light.radius = radius;
    light.power = power;
    light.opacity = opacity;
    light.color = color;
    light.make_shadows = false;
    light.make_backshadows = false;
    
    add_light_to_draw_queue(light);
}

void make_texture(Texture texture, Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    Immediate_Texture im_texture = {};
    im_texture.texture  = texture;
    im_texture.position = position;
    im_texture.scale    = scale;
    im_texture.pivot    = pivot;
    im_texture.rotation = rotation;
    im_texture.color    = color;
    
    render.textures_to_draw.add(im_texture);
}

void make_line(Vector2 start_position, Vector2 target_position, f32 thick, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    Line line = {};
    line.start_position = start_position;
    line.target_position = target_position;
    line.color = color;
    line.thick = thick;
    render.lines_to_draw.add(line);
}

inline void make_line(Vector2 start_position, Vector2 target_position, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    make_line(start_position, target_position, 0, color);
}   

void make_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    Rect_Lines rect = {};
    rect.position = position;
    rect.scale = scale;
    rect.pivot = pivot;
    rect.thick = thick;
    rect.color = color;
    render.rect_lines_to_draw.add(rect);
}

inline void make_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    make_rect_lines(position, scale, pivot, 0, color);
}

inline void make_outline(Vector2 position, Array<Vector2, MAX_VERTICES> vertices, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    
    Outline outline = {};
    outline.position = position;
    outline.vertices = vertices;
    outline.color = color;
    render.outlines_to_draw.add(outline);
}

void make_ring_lines(Vector2 center, f32 inner_radius, f32 outer_radius, i32 segments, Color color){
    if (!should_add_immediate_stuff()){
        return;
    }
    Ring_Lines ring = {};
    ring.center = center;
    ring.inner_radius = inner_radius;
    ring.outer_radius = outer_radius;
    ring.segments = segments;
    ring.color = color;
    render.ring_lines_to_draw.add(ring);
}

void draw_debug_info(){
    f32 add_vertical_position = screen_height * 0.03f;
    f32 v = screen_height * 0.05f;
    f32 h = screen_width * 0.35f;
    for (i32 i = debug.log_messages_short.count - 1; i >= 0; i--){
        Log_Message *log = debug.log_messages_short.get_ptr(i);
        
        f32 lifetime = core.time.app_time - log->birth_time;
        
        if (lifetime >= 3.0f){
            debug.log_messages_short.remove(i);
            // i++;
            continue;
        }
        
        draw_text(log->data, {h, v}, 26, ColorBrightness(BROWN, 0.3f));
        v += add_vertical_position;
    }
}

void draw_immediate_stuff(){
    for (i32 i = 0; i < render.lines_to_draw.count; i++){
        Line line = render.lines_to_draw.get(i);
        if (line.thick == 0){
            draw_game_line(line.start_position, line.target_position, line.color);
        } else{
            draw_game_line(line.start_position, line.target_position, line.thick, line.color);
        }
    }
    
    for (i32 i = 0; i < render.ring_lines_to_draw.count; i++){
        Ring_Lines ring = render.ring_lines_to_draw.get(i);
        draw_game_ring_lines(ring.center, ring.inner_radius, ring.outer_radius, ring.segments, ring.color);
    }
    
    for (i32 i = 0; i < render.rect_lines_to_draw.count; i++){
        Rect_Lines rect = render.rect_lines_to_draw.get(i);
        if (rect.thick == 0){
            draw_game_rect_lines(rect.position, rect.scale, rect.pivot, rect.color);
        } else{
            draw_game_rect_lines(rect.position, rect.scale, rect.pivot, rect.thick, rect.color);
        }
    }
    
    for (i32 i = 0; i < render.textures_to_draw.count; i++){
        Immediate_Texture im_texture = render.textures_to_draw.get(i);
        draw_game_texture(im_texture.texture, im_texture.position, im_texture.scale, im_texture.pivot, im_texture.rotation, im_texture.color);
    }
    
    for (i32 i = 0; i < render.outlines_to_draw.count; i++){
        Outline outline = render.outlines_to_draw.get(i);
        // Obviously should make this a real outlines and not line strip.
        draw_game_line_strip(outline.position, outline.vertices, outline.color);
    }
    
    if (!debug.drawing_stopped){
        render.lines_to_draw.clear();
        render.ring_lines_to_draw.clear();
        render.rect_lines_to_draw.clear();
        render.textures_to_draw.clear();
        render.outlines_to_draw.clear();
    }
}

void apply_shake(){
    if (state_context.cam_state.trauma <= 0){    
        return;
    }
    
    f32 x_shake_power = 10;
    f32 y_shake_power = 7;
    f32 x_shake_speed = 7;
    f32 y_shake_speed = 10;
    
    f32 x_offset = perlin_noise3(core.time.game_time * x_shake_speed, 0, 1) * x_shake_power;
    f32 y_offset = perlin_noise3(0, core.time.game_time * y_shake_speed, 2) * y_shake_power;
    
    current_level_context->cam.position += (cast(Vector2){x_offset, y_offset}) * state_context.cam_state.trauma * state_context.cam_state.trauma;
}

Cam saved_cam;
Cam with_shake_cam;

inline f32 get_light_zoom(f32 radius){
    return SCREEN_WORLD_SIZE / radius;
}

inline void add_light_to_draw_queue(Light light){
    render.lights_draw_queue.add(light);
}

struct Bake_Settings{
    i32 rays_per_pixel = 128;
    i32 raymarch_steps = 256;
    f32 distance_mod = 1;
};

Bake_Settings light_bake_settings = {128, 256, 1};
Bake_Settings heavy_bake_settings = {512, 1024, 4};
Bake_Settings final_bake_settings = {1024, 2048, 4};

void bake_lightmaps_if_need(){
    // Currently baking one by one so we could see that something happening. 
    // Later we probably should do that in separate thread so everything does not stall, or just show progress.
    local_persist i32 last_baked_index = -1;
    b32 need_to_bake = ((IsKeyPressed(KEY_F9) || IsKeyPressed(KEY_F10) || IsKeyPressed(KEY_F11)) || session_context.app_frame_count == 0);
    local_persist Bake_Settings bake_settings = light_bake_settings;
    if (need_to_bake || (last_baked_index < lightmaps.max_count && last_baked_index != -1)){
        last_baked_index += 1;
        if (last_baked_index >= lightmaps.max_count){
            last_baked_index = -1;
        }
        
        if (IsKeyPressed(KEY_F9)){
            bake_settings = light_bake_settings;
        } else if (IsKeyPressed(KEY_F10)){
            bake_settings = heavy_bake_settings;
        } else if (IsKeyPressed(KEY_F11)){
            bake_settings = final_bake_settings;
        }
    }
    
    for (i32 lightmap_index = 0; lightmap_index < lightmaps.max_count && need_to_bake; lightmap_index++){
        Lightmap_Data *lightmap_data = lightmaps.get_ptr(lightmap_index);
        RenderTexture *emitters_occluders_rt = &lightmap_data->emitters_occluders_rt;
        RenderTexture *distance_field_rt = &lightmap_data->distance_field_rt;

        current_level_context->cam = get_cam_for_resolution(light_texture_width, light_texture_height);
        current_level_context->cam.position = lightmap_data->position;
        current_level_context->cam.view_position = lightmap_data->position;
        current_level_context->cam.cam2D.zoom = get_light_zoom(lightmap_data->game_size.x);
        
        drawing_state= LIGHTING_DRAWING;
        BeginTextureMode(*emitters_occluders_rt);{
        BeginMode2D(current_level_context->cam.cam2D);
        ClearBackground(Fade(BLACK, 0));
        BeginBlendMode(BLEND_ADDITIVE);
            ForEntities(entity, LIGHT){   
                if (entity->flags & LIGHT){
                    Light *light = current_level_context->lights.get_ptr(entity->light_index);
                    if (light->bake_shadows){
                        if (entity->flags & TEXTURE){
                            draw_game_texture(entity->texture, entity->position, entity->scale, entity->pivot, entity->rotation, Fade(light->color, sqrtf(light->opacity)));
                        } else{
                            draw_game_triangle_strip(entity, Fade(light->color, sqrtf(light->opacity)));
                        }
                    }
                }
            }
            ForEntities(entity2, GROUND){   
                if (entity2->flags & DOOR || entity2->flags & PHYSICS_OBJECT || entity2->flags & LIGHT || entity2->flags & MOVE_SEQUENCE){
                    continue;
                }
                
                draw_game_triangle_strip(entity2, BLACK);
                
                if (entity2->flags & NO_MOVE_BLOCK){
                    draw_game_line_strip(entity2->position, entity2->vertices, PURPLE);
                }
            }
        EndBlendMode();
        EndMode2D();
        }EndTextureMode();
        current_level_context->cam = with_shake_cam;
        
        BeginTextureMode(voronoi_seed_rt);{
            ClearBackground({0, 0, 0, 0});
            BeginShaderMode(voronoi_seed_shader);
                draw_render_texture(emitters_occluders_rt->texture, {1.0f, 1.0f}, WHITE);
            EndShaderMode();
        }EndTextureMode();
        
        RenderTexture prev = voronoi_seed_rt;
        RenderTexture next = jump_flood_rt;
        
        //jump flood voronoi render pass
        {
            i32 passes = ceilf(logf(fmaxf(light_texture_width, light_texture_height)) / logf(2.0f));
            
            i32 level_loc     = get_shader_location(jump_flood_shader, "u_level");
            i32 max_steps_loc = get_shader_location(jump_flood_shader, "u_max_steps");
            i32 offset_loc    = get_shader_location(jump_flood_shader, "u_offset");
            i32 tex_loc       = get_shader_location(jump_flood_shader, "u_tex");
            
            for (int i = 0; i < passes; i++){
                f32 offset = powf(2.0f, passes - i - 1);
                BeginTextureMode(next);{
                    BeginShaderMode(jump_flood_shader);
                    set_shader_value(jump_flood_shader, max_steps_loc, passes);
                    set_shader_value(jump_flood_shader, level_loc, i);
                    set_shader_value(jump_flood_shader, offset_loc, offset);
                    
                    i32 screen_pixel_size_loc  = get_shader_location(jump_flood_shader, "u_screen_pixel_size");
            
                    set_shader_value(jump_flood_shader, screen_pixel_size_loc, {(1.0f) / light_texture_width, (1.0f) / light_texture_height});
    
                    // set_shader_value(jump_flood_shader, step_loc, 1 << i);
                    // set_shader_value(jump_flood_shader, pixel_loc, {LIGHT_TEXTURE_SCALING_FACTOR / screen_width, LIGHT_TEXTURE_SCALING_FACTOR / screen_height});
                    set_shader_value_tex(jump_flood_shader, tex_loc, prev.texture);
                    draw_render_texture(voronoi_seed_rt.texture, {1.0f, 1.0f}, WHITE);
                    EndShaderMode();
                }EndTextureMode();
                
                RenderTexture temp = next;
                next = prev;
                prev = temp;
            }
        }
        
        //distance field pass (Render voronoi)
        BeginTextureMode(*distance_field_rt);{
            ClearBackground({0, 0, 0, 0});
            BeginShaderMode(distance_field_shader);
            i32 tex_loc = get_shader_location(distance_field_shader, "u_tex");
            i32 tex2_loc = get_shader_location(distance_field_shader, "obstacles_texture");
            set_shader_value_tex(distance_field_shader, tex_loc, prev.texture);
            set_shader_value_tex(distance_field_shader, tex2_loc, emitters_occluders_rt->texture);
            draw_render_texture(prev.texture, {1.0f, 1.0f}, WHITE);
            EndShaderMode();
        }EndTextureMode();
        
        // lightmap_data->distance_texture_loc = get_shader_location(global_illumination_shader, text_format("lightmaps_data[%i].distance_texture", lightmap_index));        
        // lightmap_data->emitters_occluders_loc = get_shader_location(global_illumination_shader, text_format("lightmaps_data[%i].emitters_occluders_texture", lightmap_index));        
        lightmap_data->distance_texture_loc = get_shader_location(global_illumination_shader, "distance_texture");        
        lightmap_data->emitters_occluders_loc = get_shader_location(global_illumination_shader, "emitters_occluders_texture");        
    }
    
    local_persist f32 bake_progress = 0;
    if (need_to_bake){
        bake_progress = 0;
    }
        
    // At this point we computed emitters/occluders and distnace fields for every lightmap.
    // Now we do real global illumination work and we will need this info for calculating neighbours.
    for (i32 lightmap_index = 0; lightmap_index < lightmaps.max_count; lightmap_index++){
        // Baking one at the time to see that something is happening.
        // if (lightmap_index != last_baked_index){
        //     continue;
        // }
        
        if (!need_to_bake && bake_progress >= 1){
            break;
        }
        
        if (lightmap_index == 0){
            bake_progress += 0.05f;
        }
    
        Lightmap_Data *lightmap_data         = lightmaps.get_ptr(lightmap_index);
        RenderTexture *gi_rt                 = &lightmap_data->global_illumination_rt;
        RenderTexture *my_emitters_occluders_rt = &lightmap_data->emitters_occluders_rt;
        RenderTexture *my_distance_field_rt     = &lightmap_data->distance_field_rt;
        
        //global illumination pass
        BeginTextureMode(*gi_rt);{
            // Means we're just started
            if (need_to_bake){
                ClearBackground(Fade(BLACK, 0));
            }
            
            BeginShaderMode(global_illumination_shader);
            
            set_shader_value_tex(global_illumination_shader, lightmap_data->distance_texture_loc, my_distance_field_rt->texture);
            set_shader_value_tex(global_illumination_shader, lightmap_data->emitters_occluders_loc, my_emitters_occluders_rt->texture);
            
            i32 rays_per_pixel_loc     = get_shader_location(global_illumination_shader, "u_rays_per_pixel");
            i32 emission_multi_loc     = get_shader_location(global_illumination_shader, "u_emission_multi");
            i32 max_raymarch_steps_loc = get_shader_location(global_illumination_shader, "u_max_raymarch_steps");
            i32 time_loc               = get_shader_location(global_illumination_shader, "u_time");
            i32 distance_mod           = get_shader_location(global_illumination_shader, "u_dist_mod");
            i32 screen_pixel_size_loc  = get_shader_location(global_illumination_shader, "u_screen_pixel_size");
            
            set_shader_value(global_illumination_shader, distance_mod, bake_settings.distance_mod);
            set_shader_value(global_illumination_shader, screen_pixel_size_loc, {(1.0f) / light_texture_width, (1.0f) / light_texture_height});
            set_shader_value(global_illumination_shader, time_loc, core.time.app_time + PI * 10);
            
            set_shader_value(global_illumination_shader, rays_per_pixel_loc, bake_settings.rays_per_pixel);
            set_shader_value(global_illumination_shader, emission_multi_loc, 1.5f);
            set_shader_value(global_illumination_shader, max_raymarch_steps_loc, bake_settings.raymarch_steps);
            
            i32 bake_progress_loc = get_shader_location(global_illumination_shader, "bake_progress");
            set_shader_value(global_illumination_shader, bake_progress_loc, bake_progress);
            
            i32 perlin_texture_loc = get_shader_location(global_illumination_shader, "perlin_texture");
            set_shader_value_tex(global_illumination_shader, perlin_texture_loc, perlin_texture);
            
            draw_render_texture(gi_rt->texture, {1.0f, 1.0f}, WHITE);
            
            // draw_rect({1, 1}, {1, 1}, WHITE);
            EndShaderMode();
        } EndTextureMode();
    }
}

void new_render(){
    bake_lightmaps_if_need();

    BeginTextureMode(render.main_render_texture);
    BeginMode2D(current_level_context->cam.cam2D);
    
    Color base_background_color = debug.full_light ? ColorBrightness(GRAY, 0.1f) : Fade(WHITE, 0);
    
    ClearBackground(is_explosion_trauma_active() ? (player_data->dead_man ? RED : WHITE) : base_background_color);
    
    drawing_state = CAMERA_DRAWING;
    draw_entities();
    // ClearBackground(WHITE);
    draw_particles();
    
    if (player_entity && debug.draw_player_collisions){
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            
            make_line(col.point, col.point + col.normal * 4, 0.2f, GREEN);
            draw_game_rect(col.point + col.normal * 4, {1, 1}, {0.5f, 0.5f}, 0, GREEN * 0.9f);
        }
    }
    
    if (debug.draw_collision_grid){
        // draw collision grid
        Collision_Grid grid = session_context.collision_grid;
        Vector2 player_position = player_entity ? player_entity->position : current_level_context->player_spawn_point;
        
        update_entity_collision_cells(&mouse_entity);
        for (f32 row = -grid.size.y * 0.5f + grid.origin.y; row <= grid.size.y * 0.5f + grid.origin.y; row += grid.cell_size.y){
            for (f32 column = -grid.size.x * 0.5f + grid.origin.x; column <= grid.size.x * 0.5f + grid.origin.x; column += grid.cell_size.x){
                Collision_Grid_Cell *cell = get_collision_cell_from_position({column, row});
                
                draw_game_rect_lines({column, row}, grid.cell_size, {0, 1}, 0.5f / current_level_context->cam.cam2D.zoom, (cell && cell->entities_ids.count > 0) ? GREEN : RED);
            }
        }
    }
    EndMode2D();
    EndTextureMode();
    
    // Drawing baked lightmaps on camera plane.
    BeginTextureMode(global_illumination_rt);{
    BeginMode2D(current_level_context->cam.cam2D);
        ClearBackground(BLACK);
        for (i32 lightmap_index = 0; lightmap_index < lightmaps.max_count; lightmap_index++){
            Lightmap_Data *lightmap_data = lightmaps.get_ptr(lightmap_index);
            RenderTexture *gi_rt = &lightmap_data->global_illumination_rt;
    
            draw_game_texture(gi_rt->texture, lightmap_data->position, lightmap_data->game_size, {0.5f, 0.5f}, 0,  WHITE, true);
        }
    EndMode2D();
    } EndTextureMode();

    // Drawing dynamic lights on camera plane.
    local_persist Shader smooth_edges_shader = LoadShader(0, "./resources/shaders/smooth_edges.fs");
    
    drawing_state = LIGHTING_DRAWING;
    
    BeginTextureMode(light_geometry_rt);{
        ClearBackground(Fade(BLACK, 0));
        BeginMode2D(current_level_context->cam.cam2D);
        BeginShaderMode(gaussian_blur_shader);
            i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
            set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / (screen_width), (1.0f) / (screen_height)});

            // ForEntities(entity, GROUND | ENEMY | PLAYER | PLATFORM | SWORD){
            for (i32 i = 0; i < session_context.entities_draw_queue.count; i++){
                Entity *entity = session_context.entities_draw_queue.get_ptr(i);
                if (entity->hidden || should_not_draw_entity(entity, current_level_context->cam)){
                    continue;
                }
                Color prev_color = entity->color;
                entity->color = WHITE;
                draw_entity(entity);
                entity->color = prev_color;
            }
            //draw_particles();
        EndShaderMode();
        EndMode2D();
    }EndTextureMode();

    local_persist Texture smooth_circle_texture = white_pixel_texture;
    
    for (i32 light_index = 0; light_index < current_level_context->lights.max_count; light_index++){
        Light *light_ptr = current_level_context->lights.get_ptr(light_index);
        
        if (!light_ptr->exists || light_ptr->bake_shadows){
            continue;
        }
        
        Entity *connected_entity = get_entity_by_id(light_ptr->connected_entity_id);
        
        // update temp lights
        if (light_index < session_context.temp_lights_count){
            f32 lifetime = core.time.game_time - light_ptr->birth_time;
            if (lifetime < light_ptr->grow_time){
                f32 grow_t        = lifetime / light_ptr->grow_time;
                light_ptr->radius = lerp(0.0f, light_ptr->target_radius, sqrtf(grow_t));
                light_ptr->power  = lerp(4.0f, 2.0f, grow_t * grow_t);
            } else{ //shrinking
                f32 shrink_t       = clamp01((lifetime - light_ptr->grow_time) / light_ptr->shrink_time);
                light_ptr->radius  = lerp(light_ptr->target_radius, light_ptr->target_radius * 0.5f, shrink_t * shrink_t);
                light_ptr->opacity = lerp(light_ptr->start_opacity, 0.0f, shrink_t * shrink_t);
                light_ptr->power   = lerp(2.0f, 1.0f, shrink_t * shrink_t);
            }
            
            if (lifetime > light_ptr->grow_time + light_ptr->shrink_time){
                light_ptr->exists = false;
            }
        }
        
        //update light
        if (connected_entity){
            light_ptr->position = connected_entity->position;
        }
            
        if (light_ptr->fire_effect){
            f32 perlin_rnd = (perlin_noise3(core.time.game_time * 5, light_index, core.time.game_time * 4) + 1) * 0.5f;
            light_ptr->radius = perlin_rnd * 30 + 45;
            light_ptr->power  = perlin_rnd * 1.0f + 0.5f;
        }
        
        Light light = current_level_context->lights.get(light_index);
        
        // Vector2 light_position = light.position;
        Vector2 lightmap_game_scale = {light.radius, light.radius};
        
        b32 should_calculate_light_anyway = light_ptr->bake_shadows && session_context.just_entered_game_state;
        
        Bounds lightmap_bounds = {lightmap_game_scale, {0, 0}};
        if (!should_calculate_light_anyway && (!check_bounds_collision(current_level_context->cam.view_position, light.position, get_cam_bounds(current_level_context->cam, current_level_context->cam.cam2D.zoom), lightmap_bounds) || (connected_entity && connected_entity->hidden && game_state == GAME)) || debug.full_light){
            continue;
        }
        
        Vector2 shadows_texture_size = {(f32)light.shadows_size, (f32)light.shadows_size};
        
        if (light.make_shadows && (!light.bake_shadows || (core.time.app_time - light.last_bake_time > 1 && (game_state == EDITOR) && !session_context.baked_shadows_this_frame) || session_context.just_entered_game_state || !light_ptr->baked && game_state == GAME)){
            light_ptr->last_bake_time = core.time.app_time;
            
            if (light.bake_shadows){
                session_context.baked_shadows_this_frame = true;
                if (game_state == GAME){
                    light_ptr->baked = true;
                }
            }
            
            BeginTextureMode(light.shadowmask_rt);{
                ClearBackground(Fade(WHITE, 0));
                current_level_context->cam = get_cam_for_resolution(shadows_texture_size.x, shadows_texture_size.y);
                current_level_context->cam.position = light.position;
                current_level_context->cam.view_position = light.position;
                current_level_context->cam.cam2D.zoom = get_light_zoom(light.radius);
                BeginMode2D(current_level_context->cam.cam2D);
                ForEntities(entity, GROUND | light.additional_shadows_flags){
                    if (entity->hidden || entity->id == light.connected_entity_id || should_not_draw_entity(entity, current_level_context->cam)){
                        continue;
                    }
                    
                    if (light.bake_shadows && (entity->flags & DOOR || entity->flags & PHYSICS_OBJECT)){
                        continue;
                    }
                    
                    Color prev_color = entity->color;
                    entity->color = BLACK;
                    draw_entity(entity);
                    entity->color = prev_color;
                }
                // draw_particles();
                EndMode2D();
                current_level_context->cam = with_shake_cam;
            }EndTextureMode();
            
            assert(shadows_texture_size.x >= 1);
            f32 mult = 2.0f / shadows_texture_size.x;
            for (; ; mult *= 1.5f){
                BeginTextureMode(light.shadowmask_rt);{
                    // if (0 && !light.bake_shadows){
                        BeginShaderMode(gaussian_blur_shader);
                    // }
                    i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
                    set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / light.shadows_size, (1.0f) / light.shadows_size});
                    draw_texture(light.shadowmask_rt.texture, shadows_texture_size * 0.5f, {1.0f + mult, 1.0f + mult}, {0.5f, 0.5f}, 0, WHITE, true);
                    // if (0 && !light.bake_shadows){
                        EndShaderMode();
                    // }
                }EndTextureMode();
                
                // need to check and think about this threshold
                if (mult >= 1){
                    break;
                }
            }
        }        

        Vector2 backshadows_texture_size = {(f32)light.backshadows_size, (f32)light.backshadows_size};
        if (light.make_backshadows){
            BeginTextureMode(light.backshadows_rt);{
                ClearBackground(Fade(WHITE, 0));
                current_level_context->cam = get_cam_for_resolution(backshadows_texture_size.x, backshadows_texture_size.y);
                current_level_context->cam.position = light.position;
                current_level_context->cam.view_position = light.position;
                current_level_context->cam.cam2D.zoom = get_light_zoom(light.radius);
                BeginMode2D(current_level_context->cam.cam2D);
                ForEntities(entity, ENEMY | BLOCK_ROPE | SPIKES | PLAYER | PLATFORM | SWORD){
                    if (entity->hidden || entity->id == light.connected_entity_id || should_not_draw_entity(entity, current_level_context->cam)){
                        continue;
                    }
                    Color prev_color = entity->color;
                    entity->color = Fade(BLACK, 0.7f);
                    draw_entity(entity);
                    entity->color = prev_color;
    
                }
                // draw_particles();
                EndMode2D();
                
                BeginShaderMode(gaussian_blur_shader);
                    i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
                    set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / light.backshadows_size, (1.0f) / light.backshadows_size});
    
                    draw_texture(light.backshadows_rt.texture, backshadows_texture_size * 0.5f, {1.0f + 0.2f, 1.0f + 0.2f}, {0.5f, 0.5f}, 0, Fade(BLACK, 0.7f), true);
                EndShaderMode();
                current_level_context->cam = with_shake_cam;
            }; EndTextureMode();
        }
        
        add_light_to_draw_queue(light);
    }
    
    BeginTextureMode(global_illumination_rt);{
    BeginShaderMode(smooth_edges_shader);{
    for (i32 i = 0; i <  render.lights_draw_queue.count; i++){
        Light light = render.lights_draw_queue.get(i);
        Vector2 lightmap_game_scale = {light.radius, light.radius};
            Texture shadowmask_texture = light.make_shadows ? light.shadowmask_rt.texture : white_transparent_pixel_texture;
        
            Vector2 lightmap_texture_pos = get_left_down_texture_screen_position(shadowmask_texture, light.position, lightmap_game_scale);
            BeginMode2D(current_level_context->cam.cam2D);{
                local_persist i32 light_power_loc         = get_shader_location(smooth_edges_shader, "light_power");
                local_persist i32 light_color_loc         = get_shader_location(smooth_edges_shader, "light_color");
                local_persist i32 my_pos_loc              = get_shader_location(smooth_edges_shader, "my_pos");
                local_persist i32 my_size_loc             = get_shader_location(smooth_edges_shader, "my_size");
                local_persist i32 gi_size_loc             = get_shader_location(smooth_edges_shader, "gi_size");
                local_persist i32 gi_texture_loc          = get_shader_location(smooth_edges_shader, "gi_texture");
                local_persist i32 geometry_texture_loc    = get_shader_location(smooth_edges_shader, "geometry_texture");
                local_persist i32 light_texture_loc       = get_shader_location(smooth_edges_shader, "light_texture");
                local_persist i32 backshadows_texture_loc = get_shader_location(smooth_edges_shader, "backshadows_texture");
                
                Vector4 color = ColorNormalize(Fade(light.color, light.opacity));
                set_shader_value_color(smooth_edges_shader, light_color_loc, color);
                set_shader_value(smooth_edges_shader, light_power_loc, light.power);
                set_shader_value(smooth_edges_shader, my_pos_loc, lightmap_texture_pos);
                set_shader_value(smooth_edges_shader, my_size_loc, get_texture_pixels_size(shadowmask_texture, lightmap_game_scale));
                set_shader_value(smooth_edges_shader, gi_size_loc, {(f32)global_illumination_rt.texture.width, (f32)global_illumination_rt.texture.height});
                set_shader_value_tex(smooth_edges_shader, gi_texture_loc,          global_illumination_rt.texture);
                set_shader_value_tex(smooth_edges_shader, light_texture_loc,       smooth_circle_texture);
                set_shader_value_tex(smooth_edges_shader, backshadows_texture_loc, light.make_backshadows ? light.backshadows_rt.texture : white_transparent_pixel_texture);
                set_shader_value_tex(smooth_edges_shader, geometry_texture_loc,    light.make_shadows || light.make_backshadows ? light_geometry_rt.texture : black_pixel_texture);
                
                draw_game_texture(shadowmask_texture, light.position, lightmap_game_scale, {0.5f, 0.5f}, 0, WHITE, true);
            } EndMode2D();
            current_level_context->cam = with_shake_cam;
    }
    } EndShaderMode();
    } EndTextureMode();
    
    render.lights_draw_queue.clear();

    // In original lighting there goes blur pass, but we can think about drawing dynamic lights in other render texture and blur
    // it instead, because there's no way we want to blur whole global illumination including lightmaps.

    if (IsKeyPressed(KEY_F1)){
        debug_toggle_lightmap_view();
    }
    
    drawing_state = CAMERA_DRAWING;
    if (debug.view_only_lightmaps){
        BeginMode2D(current_level_context->cam.cam2D);
        for (i32 lightmap_index = 0; lightmap_index < lightmaps.max_count; lightmap_index++){
            Lightmap_Data *lightmap_data = lightmaps.get_ptr(lightmap_index);
            RenderTexture *gi_rt = &lightmap_data->global_illumination_rt;
    
            draw_game_texture(gi_rt->texture, lightmap_data->position, lightmap_data->game_size, {0.5f, 0.5f}, 0,  WHITE, true);
        }
        EndMode2D();
    } else if (debug.full_light){
        draw_render_texture(render.main_render_texture.texture, {1, 1}, WHITE);
    } else{
        ClearBackground(Fade(BLACK, 0));
        BeginBlendMode(BLEND_ADDITIVE);
        BeginShaderMode(env_light_shader);{
            local_persist i32 gi_data_loc = get_shader_location(env_light_shader, "u_gi_data");
            set_shader_value_tex(env_light_shader, gi_data_loc, global_illumination_rt.texture);
            
            draw_render_texture(render.main_render_texture.texture, {1, 1}, WHITE);
        } EndShaderMode();
        EndBlendMode();
    }

    // BeginMode2D(current_level_context->cam.cam2D);
    // for (i32 lightmap_index = 0; lightmap_index < lightmaps.max_count; lightmap_index++){
    //     Lightmap_Data *lightmap_data = lightmaps.get_ptr(lightmap_index);
    //     RenderTexture *gi_rt = &lightmap_data->global_illumination_rt;

    //     draw_game_texture(gi_rt->texture, lightmap_data->position, lightmap_data->game_size, {0.5f, 0.5f}, 0,  WHITE, true);
    // }
    // EndMode2D();
}

void draw_game(){
    saved_cam = current_level_context->cam;

    apply_shake();
    
    with_shake_cam = current_level_context->cam;
    BeginDrawing();

    // old_render();
    
    new_render();
    
    update_input_field();
    
    BeginMode2D(current_level_context->cam.cam2D);{
        if (game_state == EDITOR || state_context.in_pause_editor){
            draw_editor();
        }
        draw_immediate_stuff();
    } EndMode2D();
    
    draw_debug_info();
    
    if (state_context.we_got_a_winner){
        make_ui_text("Finale for now!\nNow you can try speedruns.\nOpen console with \"/\" (slash) button and type help.\ngame_speedrun for full game speedrun.\nlevel_speedrun for current level speedrun.\nfirst for loading first level\nnext for loading next level", {screen_width * 0.3f, screen_height * 0.2f}, 20, GREEN, "win_speedrun_text");
    }
    
    if (game_state == GAME && player_data->dead_man && !state_context.we_got_a_winner){
        f32 since_died = core.time.game_time - player_data->timers.died_time;
        
        f32 t = clamp01((since_died - 3.0f) / 2.0f);
        make_ui_text("T - restart", {screen_width * 0.45f, screen_height * 0.45f}, 40, Fade(GREEN, t * t), "restart_text");
    }
    
    draw_ui("");
    
    current_level_context->cam = saved_cam;
    
    f32 v_pos = 10;
    f32 font_size = 18;
    if (debug.info_fps){
        draw_text(text_format("FPS: %d", GetFPS()), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (game_state == GAME && player_entity){            
        if (debug.info_spin_progress){
            draw_text(text_format("Spin progress: %.2f", player_data->sword_spin_progress), 10, v_pos, font_size, RED);
            v_pos += font_size;
        }
        
        if (debug.info_blood_progress){
            draw_text(text_format("Blood progress: %.2f", player_data->blood_progress), 10, v_pos, font_size, RED);
            v_pos += font_size;
        }
    }
    
    if (debug.info_particle_count){
        draw_text(text_format("Particles count: %d", enabled_particles_count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    // if (debug.info_emitters_count){
    //     // draw_text(text_format("Emitters count: %d", current_level_context->emitters.count), 10, v_pos, font_size, RED);
    //     v_pos += font_size;
    // }
    
    if (debug.info_player_speed){
        draw_text(text_format("Player speed: %.1f", magnitude(player_data->velocity)), 10, v_pos, font_size, RED);
        v_pos += font_size;
        draw_text(text_format("Player Velocity: {%.1f, %.1f}", player_data->velocity.x, player_data->velocity.y), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    v_pos += font_size;
    draw_text(text_format("Ammo: %d", player_data->ammo_count), 10, v_pos, font_size * 1.5f, VIOLET);
    v_pos += font_size * 1.5f;

    
    if (console.is_open){
        //draw console
        Color text_color = lerp(GREEN * 0, GREEN, console.open_progress * console.open_progress);
        f32 y_position = lerp(-screen_height * 0.6f, 0.0f, EaseOutQuint(console.open_progress));
        
        draw_rect({0, y_position}, {(f32)screen_width, screen_height * 0.5f}, BLUE * 0.2f);
        draw_text_boxed(console.str.data, {4, 4 + y_position, (f32)screen_width, screen_height * 0.5f - 30.0f}, 16, 3, text_color, false);
        draw_text(text_format("App time: %.2f", core.time.app_time), {screen_width * 0.46f, 5.0f}, 14, ColorBrightness(lerp(LIME * 0, LIME, console.open_progress * console.open_progress), 0.5f));
        draw_text(text_format("Game time: %.2f", core.time.game_time), {screen_width * 0.46f, 20.0f}, 14, ColorBrightness(lerp(LIME * 0, LIME, console.open_progress * console.open_progress), 0.5f));
    } else{
        f32 since_console_closed = core.time.app_time - console.closed_time;
        
        if (since_console_closed <= 0.4f){
            f32 t = clamp01(since_console_closed / 0.4f);
            Color text_color = lerp(GREEN, GREEN * 0, t * t);
            f32 y_position = lerp(0.0f, -screen_height * 0.6f, EaseOutQuint(t));
            
            draw_rect({0, y_position}, {(f32)screen_width, screen_height * 0.5f}, BLUE * 0.2f);
            draw_text_boxed(console.str.data, {4, 4 + y_position, (f32)screen_width, screen_height * 0.5f - 30.0f}, 16, 3, text_color);
        }
    }
    
    // draw cursor
    if (game_state == GAME){
        draw_line(input.screen_mouse_position - Vector2_right * 10 - Vector2_up * 10, input.screen_mouse_position + Vector2_right * 10 + Vector2_up * 10, WHITE);
        draw_line(input.screen_mouse_position + Vector2_right * 10 - Vector2_up * 10, input.screen_mouse_position - Vector2_right * 10 + Vector2_up * 10, WHITE);
        draw_rect({input.screen_mouse_position.x - 2.5f, input.screen_mouse_position.y - 2.5f}, {5, 5}, GREEN);
    } else{
        draw_circle({input.screen_mouse_position.x, input.screen_mouse_position.y}, 20, Fade(RED, 0.1f));
        draw_rect({input.screen_mouse_position.x - 5, input.screen_mouse_position.y - 5}, {10, 10}, WHITE);
    }
    
    EndDrawing();
}

void setup_color_changer(Entity *entity){
    entity->color_changer.start_color = entity->color;
    entity->color_changer.target_color = Fade(ColorBrightness(entity->color, 0.5f), 0.5f);
}

void check_avaliable_ids_and_set_if_found(i32 *id){
    i32 try_count = 0;
    while ((current_level_context->entities.has_key(*id) && try_count <= MAX_ENTITIES) || *id <= 10){
        *id = ((*id) + 1) % MAX_ENTITIES;
        try_count += 1;
    }
    
    assert(try_count < 1000);
}

Entity* add_entity(Entity *copy, b32 keep_id, Level_Context *copy_entity_level_context){
    Entity e = Entity(copy, keep_id, copy_entity_level_context);
    
    e.level_context = current_level_context;
    current_level_context->entities.add(e.id, e);
    return current_level_context->entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    Entity e = Entity(pos, scale, pivot, rotation, flags);    
    e.id = current_level_context->entities.total_added_count + core.time.app_time * 10000 + 100;
    check_avaliable_ids_and_set_if_found(&e.id);
    e.level_context = current_level_context;
    current_level_context->entities.add(e.id, e);
    return current_level_context->entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Texture texture, FLAGS flags){
    Entity e = Entity(pos, scale, pivot, rotation, texture, flags);    
    e.id = current_level_context->entities.total_added_count + core.time.app_time * 10000 + 100;
    e.level_context = current_level_context;
    check_avaliable_ids_and_set_if_found(&e.id);
    current_level_context->entities.add(e.id, e);
    return current_level_context->entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags){
    Entity *e = add_entity(pos, scale, pivot, rotation, flags);    
    e->color = color;
    setup_color_changer(e);
    return e;
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    Entity e = Entity(pos, scale, pivot, rotation, flags);    
    e.id = id;
    e.level_context = current_level_context;
    check_avaliable_ids_and_set_if_found(&e.id);
    current_level_context->entities.add(e.id, e);
    return current_level_context->entities.last_ptr();
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags){
    Entity *e = add_entity(id, pos, scale, pivot, rotation, flags);    
    e->color = color;
    setup_color_changer(e);
    return e;
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags, Array<Vector2, MAX_VERTICES> vertices){
    Entity *e = add_entity(id, pos, scale, pivot, rotation, color, flags);    
    e->vertices = vertices;
    setup_color_changer(e);
    return e;
}

inline Vector2 global(Entity *e, Vector2 local_pos){
    return e->position + local_pos;
}

inline Vector2 global(Vector2 position, Vector2 local_pos){
    return position + local_pos;
}

inline Vector2 local(Entity *e, Vector2 global_pos){
    return global_pos - e->position;
}

inline Vector2 world_to_screen(Vector2 position){
    Vector2 cam_pos = current_level_context->cam.position;

    Vector2 with_cam = subtract(position, cam_pos);
    Vector2 pixels   = multiply(with_cam, current_level_context->cam.unit_size);
    
    //Horizontal center and vertical bottom
    f32 width_add, height_add;
    
    width_add = current_level_context->cam.width * 0.5f;    
    height_add = current_level_context->cam.height * 0.5f;    
    Vector2 to_center = {pixels.x + width_add, height_add - pixels.y};

    return to_center;
}

//This gives us real screen pixel position
inline Vector2 world_to_screen_with_zoom(Vector2 position){
    Vector2 cam_pos = current_level_context->cam.position;

    Vector2 with_cam = subtract(position, cam_pos);
    Vector2 pixels   = multiply(with_cam, current_level_context->cam.unit_size * current_level_context->cam.cam2D.zoom);
    //Horizontal center and vertical bottom
    
    f32 width_add, height_add;
    
    width_add = current_level_context->cam.width * 0.5f;    
    height_add = current_level_context->cam.height * 0.5f;    
    Vector2 to_center = {pixels.x + width_add, height_add - pixels.y};

    return to_center;
}

inline Vector2 get_texture_pixels_size(Texture texture, Vector2 game_scale){
    Vector2 screen_texture_size_multiplier = transform_texture_scale(texture, game_scale);
    return multiply({(f32)texture.width, (f32)texture.height}, screen_texture_size_multiplier) * current_level_context->cam.cam2D.zoom;
}

inline Vector2 get_left_down_texture_screen_position(Texture texture, Vector2 world_position, Vector2 game_scale){
    Vector2 pixels_size = get_texture_pixels_size(texture, game_scale);
    pixels_size.y *= -1;
    Vector2 texture_pos = world_to_screen_with_zoom(world_position) - pixels_size  * 0.5f;
    texture_pos.y = screen_height - texture_pos.y;
    
    return texture_pos;
}

inline Vector2 rect_screen_pos(Vector2 position, Vector2 scale, Vector2 pivot){
    Vector2 pivot_add = multiply(pivot, scale);
    Vector2 with_pivot_pos = {position.x - pivot_add.x, position.y + pivot_add.y};
    Vector2 screen_pos = world_to_screen(with_pivot_pos);
    
    return screen_pos;
}

inline void draw_game_triangle(Vector2 a, Vector2 b, Vector2 c, Color color){
    draw_triangle(world_to_screen(a), world_to_screen(b), world_to_screen(c), color);
}

inline void draw_game_circle(Vector2 position, f32 radius, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_circle(screen_pos, radius * current_level_context->cam.unit_size, color);
}

inline void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect(screen_pos, multiply(scale, current_level_context->cam.unit_size), color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect_lines(screen_pos, scale * current_level_context->cam.unit_size, thick, color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect_lines(screen_pos, scale * current_level_context->cam.unit_size, color);
}

Array<Vector2, 2048> screen_positions_buffer = Array<Vector2, 2048>();

inline void draw_game_line_strip(Entity *entity, Color color){
    screen_positions_buffer.clear();
    for (i32 i = 0; i < entity->vertices.count; i++){
        screen_positions_buffer.add(world_to_screen(global(entity, entity->vertices.get(i))));
    }
    
    draw_line_strip(screen_positions_buffer.data, screen_positions_buffer.count, color);
}

inline void draw_game_line_strip(Vector2 *points, i32 count, Color color){
    screen_positions_buffer.clear();
    for (i32 i = 0; i < count; i++){
        screen_positions_buffer.add(world_to_screen(points[i]));
    }
    
    draw_line_strip(screen_positions_buffer.data, screen_positions_buffer.count, color);
}

inline void draw_game_line_strip(Vector2 position, Array<Vector2, MAX_VERTICES> vertices, Color color){
    local_persist Array<Vector2, MAX_VERTICES> global_vertices_buffer = Array<Vector2, MAX_VERTICES>();
    global_vertices_buffer.clear();
    
    for (i32 i = 0; i < vertices.count; i++){
        global_vertices_buffer.add(vertices.get(i) + position);
    }
    draw_game_line_strip(global_vertices_buffer.data, global_vertices_buffer.count, color);
}

void draw_game_triangle_strip(Array<Vector2, MAX_VERTICES> vertices, Vector2 position, Color color){
    screen_positions_buffer.clear();
    for (i32 i = 0; i < vertices.count; i++){
        screen_positions_buffer.add(world_to_screen(global(position, vertices.get(i))));
    }
    
    draw_triangle_strip(screen_positions_buffer.data, screen_positions_buffer.count, color);
}

inline void draw_game_triangle_strip(Entity *entity, Color color){
    if (entity->hidden){
        color = color_fade(entity->color, 0.2f);
    }
    draw_game_triangle_strip(entity->vertices, entity->position, color);
}

inline void draw_game_triangle_strip(Entity *entity){
    draw_game_triangle_strip(entity, entity->color);
}

inline void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, {0, 0});
    draw_rect(screen_pos, multiply(scale, current_level_context->cam.unit_size), pivot, rotation, color);
}

inline void draw_game_text(Vector2 position, const char *text, f32 size, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_text(text, screen_pos, size, color);
}

inline void draw_game_texture(Texture tex, Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color, b32 flip){
    Vector2 screen_pos = world_to_screen(position);
    draw_texture(tex, screen_pos, transform_texture_scale(tex, scale), pivot, rotation, color, flip);
}

inline void draw_game_line(Vector2 start, Vector2 end, f32 thick, Color color){
    draw_line(world_to_screen(start), world_to_screen(end), thick * current_level_context->cam.unit_size, color);
}

inline void draw_game_line(Vector2 start, Vector2 end, Color color){
    draw_line(world_to_screen(start), world_to_screen(end), color);
}

inline void draw_game_ring_lines(Vector2 center, f32 inner_radius, f32 outer_radius, i32 segments, Color color, f32 start_angle, f32 end_angle){
    draw_ring_lines(world_to_screen(center), inner_radius * current_level_context->cam.unit_size, outer_radius * current_level_context->cam.unit_size, segments, color);
}

inline void draw_game_triangle_lines(Vector2 v1, Vector2 v2, Vector2 v3, Color color){
    draw_triangle_lines(world_to_screen(v1), world_to_screen(v2), world_to_screen(v3), color);
}

Vector2 get_left_up_no_rot(Entity *e){
    return {e->position.x - e->pivot.x * e->bounds.size.x, e->position.y + e->pivot.y * e->bounds.size.y};
}

Vector2 get_left_up(Entity *e){
    Vector2 lu = get_left_up_no_rot(e);
    
    rotate_around_point(&lu, e->position, e->rotation);
    return lu;
}

Vector2 get_right_down_no_rot(Entity *e){
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x + e->bounds.size.x, lu.y - e->bounds.size.y};
}

Vector2 get_right_down(Entity *e){
    Vector2 lu = get_left_up(e);
    Vector2 rd = lu + e->right * e->bounds.size.x;
    rd -= e->up * e->bounds.size.y;
    return rd;
}

Vector2 get_left_down_no_rot(Entity *e){
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x, lu.y - e->bounds.size.y};
}

Vector2 get_left_down(Entity *e){
    Vector2 rd = get_right_down(e);
    return rd - e->right * e->bounds.size.x;
}

Vector2 get_right_up_no_rot(Entity *e){
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x + e->bounds.size.x, lu.y};
}

Vector2 get_right_up(Entity *e){
    Vector2 lu = get_left_up(e);
    return lu + e->right * e->bounds.size.x;
}