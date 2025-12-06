#pragma once

#ifndef RELEASE_BUILD
    #define RELEASE_BUILD 0
#endif

#ifndef DEBUG_BUILD
    #define DEBUG_BUILD 0
#endif

//#define assert(a) (if (!a) (i32*)void*);
//#define assert(Expression) if(!(Expression)) {*(i32 *)0 = 0;}

global_variable Array <Collision> collisions_buffer; // Should probably reserve data.

#include "game.h"
#include "perlin.h"

// #define ForEntities(entityext_avaliable(table, 0);  xx < table.capacity; xx = table_next_avaliable(table, xx+0))

#define ForEntities(entity, flags) Entity *entity = NULL; for (i32 index = next_entity_avaliable(current_context, 0, &entity, flags); index < current_context->entities.chunks_count * current_context->entities.chunk_size && entity; index = next_entity_avaliable(current_context, index+1, &entity, flags)) 

#define ForEntitiesInContext(context, entity, flags) Entity *entity = NULL; for (i32 index = next_entity_avaliable(context, 0, &entity, flags); index < context->entities.chunks_count * context->entities.chunk_size && entity; index = next_entity_avaliable(context, index+1, &entity, flags)) 

#define ArrayOfStructsToDefaultValues(arr) for (i32 arr_index = 0; arr_index < arr.count; arr_index++) { (*arr.get(arr_index)) = {0};}

//#define For(arr, type, value) for(i32 ii = 0; ii < arr.count; ii++) { type value = arr.get_value(ii);

global_variable Input input = {0};
global_variable Input replay_input = {0};
// global_variable Context editor_context = {0};

#define MOVE_CELL_SIZE 2.5f
#define SCALE_CELL_SIZE 5

#define MAX_LOADED_LEVELS 2
global_variable Context loaded_editor_contexts[MAX_LOADED_LEVELS];
global_variable Context *editor_context = NULL;
i32 current_editor_context_index = 0;
i32 last_loaded_editor_context_index = 0;

global_variable Context game_context = {0};
global_variable Context planning_context = {0};
global_variable Context checkpoint_context = {0};
global_variable Context loaded_context = {0};
global_variable Context undo_context = {0};
global_variable Context copied_entities_context = {0};
global_variable Context *current_context = NULL;
global_variable State_Context state_context = {0};
global_variable Global_Data global_data = {0};

global_variable Entity *checkpoint_player_entity;
global_variable Player checkpoint_player_data;
global_variable Time checkpoint_time;
global_variable State_Context checkpoint_state_context;
global_variable i32 checkpoint_trigger_id = -1;

global_variable Player *player_data = {0};
// global_variable Player real_player_data = {0};
global_variable Player replay_player_data = {0};

global_variable Level_Replay level_replay = {0};
global_variable Render render = {0};
global_variable Editor editor  = {0}; 
global_variable Debug  debug  = {0};

global_variable const char *first_level_name = "new_basics1";

global_variable Array <Vector2> global_normals = {0};

global_variable Entity mouse_entity = {0};
Entity empty_entity = {0};

// global_variable Entity *player_entity;
global_variable b32 need_destroy_player = false;

global_variable f32 frame_rnd;
global_variable Vector2 frame_on_circle_rnd;

global_variable b32 clicked_ui = false;

global_variable b32 enter_game_state_on_new_level = false;

global_variable Array <Texture_Data> loaded_textures = {0};
global_variable Array <Texture_Data> normal_maps = {0};
global_variable Array <Sound_Handler> sounds_array = {0};

global_variable b32 initing_game = false;

RenderTexture global_illumination_rt;

RenderTexture light_geometry_rt;

Shader env_light_shader;

Shader gaussian_blur_shader;

global_variable Array <String> entities_names = {0};

global_variable Image white_pixel_image;
global_variable Texture white_pixel_texture;
global_variable Image white_transparent_pixel_image;
global_variable Texture white_transparent_pixel_texture;
global_variable Image black_pixel_image;
global_variable Texture black_pixel_texture;

global_variable Shader voronoi_seed_shader;
global_variable Shader jump_flood_shader;
global_variable Shader distance_field_shader;
global_variable Shader global_illumination_shader;

global_variable Cam saved_cam;
global_variable Cam with_shake_cam;

Texture spiral_clockwise_texture;
Texture spiral_counterclockwise_texture;
Texture hitmark_small_texture;
Texture jump_shooter_bullet_hint_texture;
Texture big_sword_killable_texture;
Texture small_sword_killable_texture;
Texture perlin_texture;
Texture missing_texture;

Sound_Handler *missing_sound = NULL;

global_variable Array <Spawn_Object> spawn_objects = {0};

Music ambient_theme;
Music wind_theme;
Music tires_theme;
Music relas_music;
f32 tires_volume = 0.0f;

// #include "entity_ids.cpp"

Player last_player_data = {0};
Player death_player_data = {0};

Cam global_cam_data = {0};

inline void remove_flag(FLAGS *flags, FLAGS flag) {
    if ((*flags) & flag) (*flags) ^= flag;
}

inline b32 is_editor_active() {
    return editor_state == EDITOR || state_context.in_pause_editor;
}

#include "logger.cpp"
#include "random.hpp"
#include "particles.hpp"
#include "old_ui.hpp"
#include "immediate_ui.cpp"
#include "lightmaps.cpp"
#include "collisions.cpp"
#include "planning_state.cpp"
#include "dynamic_lights.cpp"
#include "undo.cpp"
#include "saving_loading.cpp"
#include "text_input.hpp"
#include "console.cpp"
#include "player.cpp"
#include "bird_enemy.cpp"

void setup_context_cam(Context *context) {
    context->cam.width = global_cam_data.width;
    context->cam.height = global_cam_data.height;
    context->cam.unit_size = global_cam_data.width / SCREEN_WORLD_SIZE; 
    context->cam.cam2D.target = cast(Vector2) { global_cam_data.width/2.0f, global_cam_data.height/2.0f };
    context->cam.cam2D.offset = cast(Vector2) { global_cam_data.width/2.0f, global_cam_data.height/2.0f };
    // context->cam = global_cam_data;
}

void switch_current_context(Context *target, b32 clear_stuff) {
    if (clear_stuff) {
        clear_multiselected_entities();
    }

    current_context = target;
    setup_context_cam(current_context);
    
    player_data = &current_context->player;
}

inline Color color_fade(Color color, f32 alpha_multiplier) {
    return {color.r, color.g, color.b, (u8)(clamp((f32)color.a * alpha_multiplier, 0.0f, 255.0f))};
}

inline Color color_opacity(Color color, f32 alpha) {
    return {color.r, color.g, color.b, (u8)(clamp01(alpha) * 255)};
}

inline void mark_entity_destroyed(Entity *entity) {
    entity->will_be_destroyed = true;
    
    // @CLEANUP: That will probably be not necessary if we'll make that it's centipede itself who will call update on all 
    // segments. The right moment to do that will be when we'll make updating entities by type and not just going thgough
    // all of em.
    if (entity->flags & CENTIPEDE) {
        for_array (i, &entity->centipede->segments) {
            mark_entity_destroyed(entity->centipede->segments.get_value(i));
        }
    }
}

void free_entity(Entity *e) {
    // Free trigger.
    if (e->flags & TRIGGER) {
        assert(e->trigger && e->trigger->index > -1);
    
        if (e->trigger->connected.capacity > 0) {
            e->trigger->connected.free_data();
        }
        if (e->trigger->tracking.capacity > 0) {
            e->trigger->tracking.free_data();
        }
        
        if (e->trigger->cam_rails_points.capacity > 0) {
            e->trigger->cam_rails_points.free_data();
        }
        
        e->context->triggers.remove(e->trigger->index);
        e->propeller = NULL;
    }
    
    // Free move sequence.
    if (e->flags & MOVE_SEQUENCE) {
        assert(e->move_sequence && e->move_sequence->index >= 0);
        
        e->move_sequence->points.free_data();
        
        e->context->move_sequences.remove(e->move_sequence->index);
        e->move_sequence = NULL;
    }
    
    // Free sticky texture.
    if (e->flags & STICKY_TEXTURE) {
        assert(e->sticky_texture && e->sticky_texture->index > -1);
        
        e->context->sticky_textures.remove(e->sticky_texture->index);
        e->sticky_texture = NULL;
    }
    
    // Free propeller.
    if (e->flags & PROPELLER) {
        assert(e->propeller && e->propeller->index >= 0);
        e->context->propellers.remove(e->propeller->index);
        e->propeller = NULL;
    }
    
    // Free turret->.
    if (e->flags & TURRET) {
        assert(e->turret && e->turret->index >= 0);
        e->context->turrets.remove(e->turret->index);
        e->turret = NULL;
    }
    
    // Free bird enemy.
    if (e->flags & BIRD_ENEMY) {
        assert(e->bird_enemy && e->bird_enemy->index >= 0);
    
        bird_clear_formation(e->bird_enemy);
        
        e->context->bird_enemies.remove(e->bird_enemy->index);
        e->bird_enemy = NULL;
    }
    
    // Free kill switch.
    if (e->flags & KILL_SWITCH) {
        assert(e->kill_switch && e->kill_switch->index >= 0);
    
        e->kill_switch->connected.free_data();    
        
        e->context->kill_switches.remove(e->kill_switch->index);
        e->kill_switch = NULL;
    }
    
    // Free centipede segment.
    if (e->flags & CENTIPEDE_SEGMENT) {
        assert(e->centipede_segment && e->centipede_segment->index >= 0);
        
        e->context->centipede_segments.remove(e->centipede_segment->index);
        e->centipede_segment = NULL;
    }
    
    // Free centipede.
    if (e->flags & CENTIPEDE) {
        assert(e->centipede && e->centipede->index >= 0);
        for_array (i, &e->centipede->segments) {
            Entity *segment = e->centipede->segments.get_value(i);
            mark_entity_destroyed(segment);
        }
        
        e->centipede->segments.free_data();
        
        e->context->centipedes.remove(e->centipede->index);
        e->centipede = NULL;
    }
    
    // Free jump shooter.
    if (e->flags & JUMP_SHOOTER) {
        e->jump_shooter->move_points.free_data();
        
        e->context->jump_shooters.remove(e->jump_shooter->index);
        e->jump_shooter = NULL;
    }
    
    // Free win block.
    if (e->flags & WIN_BLOCK) {
        e->context->win_blocks.remove(e->win_block->index);
        e->win_block = NULL;
    }
    
    // Free light.
    if (e->lights.count > 0) {
        free_lights_connected_to_entity(e);
        e->lights.free_data();
    }
    
    // Free enemy    .
    if (e->flags & ENEMY && e->union_enemy) { 
        // We'll be here if entity is marked as enemy and it was not previously freed, which should mean that all of his data        
        // is contained in base Enemy struct.
        
        assert(e->union_enemy->index >= 0);
        
        e->context->just_enemies.remove(e->union_enemy->index);
        e->union_enemy = NULL;
    }
    
    // Free projectile .
    if (e->flags & PROJECTILE && e->projectile) {
        assert(e->projectile->index >= 0);
        
        e->context->projectiles.remove(e->projectile->index);
        e->projectile = NULL;
    }
    
    e->color_changer.changing = false;
    
    free_entity_particle_emitters(e);
    
    e->context->entities.remove(e->id - 1);
} // Free entity end.

inline void add_rect_vertices(Static_Array <Vector2, MAX_VERTICES> *vertices, Vector2 pivot) {
    vertices->clear();
    vertices->append({1.0f - pivot.x, pivot.y});
    vertices->append({-pivot.x, pivot.y});
    vertices->append({1.0f - pivot.x, pivot.y - 1.0f});
    vertices->append({-pivot.x, pivot.y - 1.0f});
}

void add_triangle_vertices(Static_Array <Vector2, MAX_VERTICES> *vertices, Vector2 pivot) {
    vertices->clear();
    vertices->append({pivot.x, pivot.y});
    vertices->append({-pivot.x, pivot.y});
    vertices->append({pivot.x, pivot.y - 1.0f});
}

void add_sword_vertices(Static_Array <Vector2, MAX_VERTICES> *vertices, Vector2 pivot) {
    add_rect_vertices(vertices, pivot);
    vertices->get(0)->x *= 0.3f;
    vertices->get(1)->x *= 0.3f;
    
    vertices->get(2)->y += 0.15f;
    vertices->get(3)->y += 0.15f;
}

void add_prism_shaped_vertices(Static_Array <Vector2, MAX_VERTICES> *vertices, Vector2 pivot, f32 narrowing = 0.3f) {
    add_rect_vertices(vertices, pivot);
    vertices->get(0)->x *= narrowing;
    vertices->get(1)->x *= narrowing;
}

void add_upsidedown_vertices(Static_Array <Vector2, MAX_VERTICES> *vertices, Vector2 pivot) {
    add_rect_vertices(vertices, pivot);
    vertices->get(2)->x *= 0.3f;
    vertices->get(3)->x *= 0.3f;
}

void add_romb_vertices(Static_Array <Vector2, MAX_VERTICES> *vertices, Vector2 pivot) {
    add_rect_vertices(vertices, pivot);
    vertices->get(0)->x *= 1.5f;
    vertices->get(3)->x *= 1.5f;
    for (i32 i = 0; i < vertices->count; i++) {
        rotate_around_point(vertices->get(i), {0, 0}, -55);
    }
}

void pick_vertices(Entity *entity) {
    if (entity->flags & (SWORD)) {
        add_sword_vertices(&entity->vertices, entity->pivot);
        add_sword_vertices(&entity->unscaled_vertices, entity->pivot);
    } else if (entity->flags & (BIRD_ENEMY | CENTIPEDE | PROJECTILE | HIT_BOOSTER)) {
        f32 narrowing = 0.3f;
        if (entity->flags & HIT_BOOSTER) {
            narrowing = 0.1f;
        }
        add_prism_shaped_vertices(&entity->vertices, entity->pivot, narrowing);
        add_prism_shaped_vertices(&entity->unscaled_vertices, entity->pivot, narrowing);
    } else if (entity->flags & (JUMP_SHOOTER)) {
        add_upsidedown_vertices(&entity->vertices, entity->pivot);
        add_upsidedown_vertices(&entity->unscaled_vertices, entity->pivot);
    } else if (entity->flags & GIVES_BIG_SWORD_CHARGE) {
        add_romb_vertices(&entity->vertices, entity->pivot);
        add_romb_vertices(&entity->unscaled_vertices, entity->pivot);
    } else {
        add_rect_vertices(&entity->unscaled_vertices, entity->pivot);
        add_rect_vertices(&entity->vertices, entity->pivot);
    }
}

Entity make_entity(Vector2 _pos) {
    Entity e = {0};
    e.flags = 0;
    e.position = _pos;
    
    add_rect_vertices(&e.vertices, e.pivot);

    e.rotation = 0;
    e.up = {0, 1};
    e.right = {1, 0};
    e.pivot = {0.5f, 0.5f};
    
    change_scale(&e, {1, 1});
    setup_color_changer(&e);
    
    e.context = current_context;
    
    return e;
}

Entity make_entity(Vector2 _pos, Vector2 _scale) {
    Entity e = {0};
    e.flags = 0;
    e.position = _pos;
    
    add_rect_vertices(&e.vertices, e.pivot);

    e.rotation = 0;
    e.up = {0, 1};
    e.right = {1, 0};
    change_scale(&e, _scale);
    setup_color_changer(&e);
    
    e.context = current_context;
    
    return e;
}

Entity make_entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags) {
    Entity e = {0};
    e.flags    = _flags;
    e.position = _pos;
    pick_vertices(&e);
    e.rotation = 0;
    
    rotate_to(&e, _rotation);
    change_scale(&e, _scale);
    setup_color_changer(&e);
    
    e.context = current_context;
    
    return e;
}

Entity make_entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags) {
    Entity e = {0};
    // *&e = make_entity(_pos, _scale, _rotation, _flags);
    e.flags    = _flags;
    e.position = _pos;
    e.pivot = _pivot;
    
    pick_vertices(&e);
    
    e.rotation = 0;
    
    rotate_to(&e, _rotation);
    
    change_scale(&e, _scale);
    setup_color_changer(&e);
    
    e.context = current_context;
    
    return e;
}

Entity make_entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, Texture _texture, FLAGS _flags) {
    Entity e = {0};
    e.flags    = _flags;
    e.position = _pos;
    e.pivot    = _pivot;
    e.texture  = _texture;
    //scaling_multiplier = {1, 1};
    e.color = WHITE;
    //scale = transform_texture_scale(texture, _scale);
    
    pick_vertices(&e);
    
    e.rotation = 0;
    
    rotate_to(&e, _rotation);
    
    change_scale(&e, _scale);
    setup_color_changer(&e);
    
    e.context = current_context;
    
    return e;
}

void parse_line(const char *line, char *result, i32 *index) { 
    assert(line[*index] == ':');
    
    i32 i;
    i32 added_count = 0;
    for (i = *index + 1; line[i] != NULL && line[i] != ':'; i++) {
        result[added_count] = line[i];
        added_count++;
    }
    
    *index = i;
}

inline void set_particle_emitter_start_and_max_indexes(Particle_Emitter_Count count_type, i32 *start_index, i32 *max_index) {
    switch (count_type) {
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

inline i32 get_particles_count_for_count_type(Particle_Emitter_Count count_type) {
    switch (count_type) {
        case SMALL_PARTICLE_COUNT:  return MAX_SMALL_COUNT_PARTICLES;
        case MEDIUM_PARTICLE_COUNT: return MAX_MEDIUM_COUNT_PARTICLES;
        case BIG_PARTICLE_COUNT:    return MAX_BIG_COUNT_PARTICLES;
        default: return -1;
    }
}

i32 add_particle_emitter(Particle_Emitter *copy, i32 entity_id) {
    i32 start_index = 0;
    i32 max_index = 0;
    
    set_particle_emitter_start_and_max_indexes(copy->count_type, &start_index, &max_index);
    
    f32 particles_count = get_particles_count_for_count_type(copy->count_type);
    
    i32 emitter_index = -1;  
    i32 occupied_count = 0;
    for (i32 i = start_index; i < max_index; i++) {
        Particle_Emitter *emitter = current_context->particle_emitters.get(i);
        if (!emitter->occupied) {
            *emitter = *copy;
            emitter->occupied = true;
            emitter->index = i;
            
            // So small particles starts at 0, medium count starts at MEDIUM_COUNT_PARTICLES_START_INDEX and big at ...
            i32 particles_count_type_start_index = 0;
            if (emitter->count_type == MEDIUM_PARTICLE_COUNT) {
                particles_count_type_start_index = MEDIUM_COUNT_PARTICLES_START_INDEX;
            } else if (emitter->count_type == BIG_PARTICLE_COUNT) {
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
    
    if (emitter_index == -1) {
        printf("WARNING: Could not found particle emitter index to add. Copy emitter tag is: %s. And count type is: %d\n", copy->tag_16, copy->count_type);
    }
    
    return emitter_index;
}

inline i32 add_entity_particle_emitter(Entity *entity, Particle_Emitter *emitter) {
    return *entity->particle_emitters_indexes.append(add_particle_emitter(emitter, entity->id));
}

inline i32 add_and_enable_entity_particle_emitter(Entity *entity, Particle_Emitter *emitter_copy, Vector2 position, b32 need_to_follow) {
    i32 index = add_entity_particle_emitter(entity, emitter_copy);
    Particle_Emitter *emitter = get_particle_emitter(index);
    if (emitter) {
        emitter->follow_entity = need_to_follow;
        enable_emitter(emitter, position);
    }
    
    return index;
}

Particle_Emitter *get_particle_emitter(i32 index) {
    if (index < 0 || index >= current_context->particle_emitters.capacity) {
        printf("WARNING: Tried to get particle emitter with bad index: %d\n", index);
        return NULL;
    }
    
    Particle_Emitter *emitter = current_context->particle_emitters.get(index);
    if (!emitter->occupied) {
        // print("WARNING: In get_particle_emitter we just took un-occupied emitter. Don't think that should happen");
    }
    
    return emitter;
}

i32 add_note(const char *content) {
    i32 note_index = -1;
    for (i32 i = 0; i < current_context->notes.capacity; i++) {
        if (!current_context->notes.get(i)->occupied) {
            current_context->notes.get(i)->occupied = true;
            str_copy(current_context->notes.get(i)->content, content);
            note_index = i;
            break;
        }
    }
    
    if (note_index == -1) {
        print("WARNING: Could not found note index to add");
    }
    
    return note_index;
}

void copy_context(Context *dest, Context *src, b32 should_init_entities) {
    clear_context(dest);

    // *dest = *src;
    Context *original_context = current_context;
    switch_current_context(dest);
    
    // Editor_State original_game_state = editor_state;
    // editor_state = EDITOR;
    
    dest->level_name = copy_string(src->level_name, &dest->memory_arena);
    dest->player_spawn_point = src->player_spawn_point;
    dest->cam = src->cam;
    
    dest->game_time = src->game_time;
    
    dest->turret_state = src->turret_state;
    
    if (should_init_entities) {
        // Particle emitters get's added on each entity init.
        // So when se init entities - we clear particle emitters, because they will be added again. 
        // When we don't init entities - we copy emitters (and entity indexes are staying the same).
        
        ArrayOfStructsToDefaultValues(current_context->particle_emitters);       
    }
    
    for (i32 i = 0; i < src->particle_emitters.capacity; i++) {
        // Particle emitter could be connected to entity - in that case entity will add emitter itself if we init entities.
        // If emitter don't connected to entity - we want to copy it.
        if (src->particle_emitters.get_value(i).connected_entity_id == -1 || !should_init_entities) {
            dest->particle_emitters.data[i] = src->particle_emitters.get_value(i);
        }
    }
    
    // First of all we just copying raw entities and then going through all of them deep copy properly into dest level context.
    dest->entities = copy_chunk_array(&src->entities);
    for_chunk_array(i, (&src->entities)) {
        // NOTE: On copy_and_add_entity happening init_entity. Some entities (like centipede) would want to spawn things on initing
        // so someone might thing that it could be an issue that we're inserting entities here directly with index.
        // Actually before we copied the whole array, so even if some entity will spawn things in dest level context
        // on initing - newly spawned things will go to non-occupied indexes that we're not going to insert into.
        Entity *added = copy_and_add_entity(src->entities.get(i), dest, i + 1);
        
        // Could happen if, for example, entity have SHOULD_NOT_SAVE_OR_COPY flag.
        if (!added) {
            dest->entities.remove(i);           
            continue;
        }
        
        if (added->flags & PLAYER) {
            dest->player_entity = added;
        }
    }
    
    if (dest->player_entity) {
        dest->player = src->player;
    }
    
    for (i32 i = 0; i < src->line_trails.capacity; i++) {
        dest->line_trails.data[i] = src->line_trails.get_value(i);
    }
    
    assert(src->lightmaps_render_textures_loaded == false && "Lightmaps render textures should be unloaded right after baking.");
    assert(dest->lightmaps_render_textures_loaded == false && "Lightmaps render textures should be unloaded right after baking.");
    dest->lightmaps.clear();
    
    // @TODO: check this because I changed lightmaps.count to lightmaps.capacity because we're clearing before.
    // We're actually clearing dest and we should go through src lightmaps count that that's should be fine.
    for (i32 i = 0; i < src->lightmaps.count; i++) {
        dest->lightmaps.append(src->lightmaps.get_value(i));
    }
    
    switch_current_context(original_context);
    // editor_state = original_game_state;
}

void clear_context(Context *context) {
    // Remember that currently we init level context only in very beginnning, so nobody should set inited to false.
    assert(context->inited);

    Context *original_context = current_context;
    switch_current_context(context);
    ForEntities(entity, 0) {
        free_entity(entity);
        *entity = {0};
    }
    
    context->game_time = 0;
    
    context->turret_state = {0};
    
    context->initially_simulated = false;
    
    context->player_entity = NULL;
    context->player = {0};
    
    context->entities.clear();
    
    context->propellers.clear();
    context->triggers.clear();
    context->sticky_textures.clear();
    context->move_sequences.clear();
    context->bird_enemies.clear();
    context->jump_shooters.clear();
    context->kill_switches.clear();
    context->turrets.clear();
    context->win_blocks.clear();
    
    context->projectiles.clear();
    
    context->just_enemies.clear();
    
    context->centipedes.clear();
    context->centipede_segments.clear();
    
    context->level_name.free_data();
    context->level_name = {0};
    
    // context->particles.clear();
    // context->emitters.clear();
    
    // We do that instead of clearing because this things are using it's full capacity and not count. Count should be equal
    // to capactiy for them. That probably should change some time.
    ArrayOfStructsToDefaultValues(context->particle_emitters);
    ArrayOfStructsToDefaultValues(context->particles);
    ArrayOfStructsToDefaultValues(context->notes);
    
    for_chunk_array(i, &context->lights) {
        free_light(get_light(i, context), i, context);
    }
    context->lights.clear();
    
    // for (i32 i = 0; i < context->lights.capacity; i++) {
    //     context->lights.get(i)->exists = false;
    //     if (i >= global_data.entity_lights_start_index) {
    //         free_light(context->lights.get(i));       
    //         *(context->lights.get(i)) = {0};
    //     } else { // So we in temp lights section
    //     }
    // }
    
    current_context->lightmaps.clear();
    
    // context->we_got_a_winner = false;
    // player_data = {0};
    
    clear_allocator(&context->memory_arena);
    
    init_planning_data(context);
    
    switch_current_context(original_context);
}

inline b32 set_next_collision_stuff(i32 current_index, Collision *col, Entity **other) {
    if (current_index >= collisions_buffer.count) {
        col->collided = false;
        col->other_entity = NULL;
        return false;
    }
    
    *col = collisions_buffer.get_value(current_index);
    *other = col->other_entity;
    return true;
}

Entity *spawn_object_by_name(const char* name, Vector2 position, Context *context) {
    for (i32 i = 0; i < spawn_objects.count; i++) {
        Spawn_Object *obj = spawn_objects.get(i);
        if (str_equal(obj->name, name)) {
            Entity *e = copy_and_add_entity(&obj->entity, context);
            e->position = position;
            return e;
        }
    }
    
    printf("No spawn object named %s\n", name);
    return NULL;
}

inline void free_particle_emitter(i32 index) {
    assert(index >= 0 && index < current_context->particle_emitters.capacity);
    
    Particle_Emitter *emitter = get_particle_emitter(index);
    if (emitter) {
        for (i32 i = emitter->particles_start_index; i < emitter->particles_max_index; i++) {
            current_context->particles.get(i)->enabled = false;
        }
    } else {
        // printf("L_WARNING: No emitter existing on free_particle_emitter. Index is %d\n", index);
    }
    
    *current_context->particle_emitters.get(index) = {0};
}

inline void free_particle_emitters(i32 *start_ptr, i32 count) {
    for (i32 i = 0; i < count; i++) {
        free_particle_emitter(*(start_ptr + i));       
    }
}

inline void free_entity_particle_emitters(Entity *entity) {
    Static_Array <i32, MAX_ENTITY_EMITTERS> *emitters_indexes = &entity->particle_emitters_indexes;
    // Free_particle_emitters(emitters_indexes->data, emitters_indexes->count);.
    for (i32 i = 0; i < emitters_indexes->count; i++) {    
        Particle_Emitter *emitter = get_particle_emitter(emitters_indexes->get_value(i));
        if (emitter) {
            emitter->should_extinct = true;
        }
    }
    emitters_indexes->clear();
}

void init_bird_emitters(Entity *entity) {
    assert(entity->flags & BIRD_ENEMY && entity->bird_enemy);

    Static_Array <i32, MAX_ENTITY_EMITTERS> *emitters_indexes = &entity->particle_emitters_indexes;
    free_entity_particle_emitters(entity);
    entity->bird_enemy->trail_emitter_index = add_entity_particle_emitter(entity, entity->flags & EXPLOSIVE ? &little_fire_emitter : &air_dust_emitter);
    enable_emitter(entity->bird_enemy->trail_emitter_index, entity->position);
    entity->bird_enemy->attack_emitter_index = add_entity_particle_emitter(entity, &small_air_dust_trail_emitter_copy);
    entity->bird_enemy->alarm_emitter_index  = add_entity_particle_emitter(entity, &alarm_smoke_emitter_copy);
    entity->bird_enemy->fire_emitter_index = add_entity_particle_emitter(entity, &fire_emitter);
    entity->bird_enemy->smoke_fire_emitter_index = add_entity_particle_emitter(entity, &smoke_fire_emitter_copy);
    entity->bird_enemy->collision_emitter_index = add_entity_particle_emitter(entity, &rifle_bullet_emitter);
}

String get_entity_name(Entity *entity, Allocator *allocator) {
    if (0) {
    } else if (entity->flags & GROUND) {
        return make_string(allocator, "Ground");  
    } else if (entity->flags & BIRD_ENEMY) {
        return make_string(allocator, "Bird_enemy");  
    } else if (entity->flags & STICKY_TEXTURE) {
        return make_string(allocator, "Sticky_texture");  
    } else if (entity->flags & CENTIPEDE) {
        return make_string(allocator, "Centipede");  
    } else if (entity->flags & CENTIPEDE_SEGMENT) {
        return make_string(allocator, "Centipede_segment");  
    } else if (entity->flags & AMMO_PACK) {
        return make_string(allocator, "Ammo_pack");  
    } else if (entity->flags & TRIGGER) {
        return make_string(allocator, "Trigger");  
    } else if (entity->flags & KILL_SWITCH) {
        return make_string(allocator, "Kill_Switch");  
    } else if (entity->flags & TURRET) {
        return make_string(allocator, "Turret");  
    } else if (entity->flags & HIT_BOOSTER) {
        return make_string(allocator, "Hit_Booster");  
    } else if (entity->flags & DUMMY) {
        if (entity->flags & LIGHT) {
            return make_string(allocator, "Light");  
        }
        return make_string(allocator, "Dummy");  
    } else if (entity->flags & TEXTURE) {
        return make_string(allocator, tprintf("Texture_%s", entity->texture_name));  
    } 
        
    return make_string(allocator, "No_name");
}
inline String temp_entity_name(Entity *entity) {
    return get_entity_name(entity, temp);
}

void init_spawn_objects() {
    Entity block_base_entity = make_entity({0, 0}, {50, 10}, {0.5f, 0.5f}, 0, GROUND);
    block_base_entity.color = BROWN;
    setup_color_changer(&block_base_entity);
    
    Spawn_Object block_base_object;
    block_base_object.entity = block_base_entity;
    str_copy(block_base_object.name, "block_base");
    spawn_objects.append(block_base_object);
    
    Entity no_move_block_entity = make_entity({0, 0}, {50, 10}, {0.5f, 0.5f}, 0, GROUND | NO_MOVE_BLOCK | LIGHT);
    no_move_block_entity.color = PURPLE;
    setup_color_changer(&no_move_block_entity);
    
    Spawn_Object no_move_block_object;
    no_move_block_object.entity = no_move_block_entity;
    str_copy(no_move_block_object.name, "no_move_block");
    spawn_objects.append(no_move_block_object);
    
    Entity note_entity = make_entity({0, 0}, {20, 15}, {0.5f, 0.5f}, 0, NOTE | TEXTURE);
    note_entity.color = Fade(WHITE, 0.7f);
    str_copy(note_entity.texture_name, "editor_note.png");
    note_entity.texture = get_texture(note_entity.texture_name);
    setup_color_changer(&note_entity);
    
    Spawn_Object note_object;
    note_object.entity = note_entity;
    str_copy(note_object.name, "note");
    spawn_objects.append(note_object);
    
    Entity dummy_entity = make_entity({0, 0}, {10, 5}, {0.5f, 0.5f}, 0, DUMMY);
    dummy_entity.color  = Fade(GREEN, 0.5f);
    // dummy_entity.hidden = true;
    setup_color_changer(&dummy_entity);
    
    Spawn_Object dummy_object;
    dummy_object.entity = dummy_entity;
    str_copy(dummy_object.name, "dummy_entity");
    spawn_objects.append(dummy_object);
    
    Entity platform_entity = make_entity({0, 0}, {50, 5}, {0.5f, 0.5f}, 0, PLATFORM);
    platform_entity.color = Fade(ColorBrightness(BROWN, -0.1f), 0.1f);
    setup_color_changer(&platform_entity);
    
    Spawn_Object platform_object;
    platform_object.entity = platform_entity;
    str_copy(platform_object.name, "platform");
    spawn_objects.append(platform_object);
    
    Entity enemy_ammo_pack_entity = make_entity({0, 0}, {5, 5}, {0.5f, 0.5f}, 0, ENEMY | AMMO_PACK);
    enemy_ammo_pack_entity.color = ColorBrightness(RED, -0.1f);
    setup_color_changer(&enemy_ammo_pack_entity);
    
    Spawn_Object enemy_ammo_pack_object;
    enemy_ammo_pack_object.entity = enemy_ammo_pack_entity;
    str_copy(enemy_ammo_pack_object.name, "ammo_pack");
    spawn_objects.append(enemy_ammo_pack_object);
    
    Entity big_sword_charge_giver_entity = make_entity({0, 0}, {10, 10}, {0.5f, 0.5f}, 0, ENEMY | GIVES_BIG_SWORD_CHARGE);
    big_sword_charge_giver_entity.color = ColorBrightness(GREEN, 0.5f);
    setup_color_changer(&big_sword_charge_giver_entity);
    
    Spawn_Object big_sword_charge_giver_object;
    big_sword_charge_giver_object.entity = big_sword_charge_giver_entity;
    str_copy(big_sword_charge_giver_object.name, "big_sword_charge_giver");
    spawn_objects.append(big_sword_charge_giver_object);
    
    Entity turret_direct_entity = make_entity({0, 0}, {5, 15}, {0.5f, 1.0f}, 0, ENEMY | TURRET);
    turret_direct_entity.color = ColorBrightness(PURPLE, 0.5f);
    setup_color_changer(&turret_direct_entity);
    
    Spawn_Object turret_direct_object;
    turret_direct_object.entity = turret_direct_entity;
    str_copy(turret_direct_object.name, "turret_direct");
    spawn_objects.append(turret_direct_object);
    
    Entity turret_homing_entity = make_entity({0, 0}, {5, 15}, {0.5f, 1.0f}, 0, ENEMY | TURRET | HOMING_TURRET);
    turret_homing_entity.color = ColorBrightness(PURPLE, 0.1f);
    setup_color_changer(&turret_homing_entity);
    
    Spawn_Object turret_homing_object;
    turret_homing_object.entity = turret_homing_entity;
    str_copy(turret_homing_object.name, "turret_homing");
    spawn_objects.append(turret_homing_object);
    
    Entity bird_entity = make_entity({0, 0}, {6, 10}, {0.5f, 0.5f}, 0, ENEMY | BIRD_ENEMY | PARTICLE_EMITTER);
    
    Spawn_Object enemy_bird_object;
    enemy_bird_object.entity = bird_entity;
    str_copy(enemy_bird_object.name, "bird_enemy");
    spawn_objects.append(enemy_bird_object);
    
    Entity win_block_entity = make_entity({0, 0}, {50, 15}, {0.5f, 0.5f}, 0, WIN_BLOCK | ENEMY | PLAYER_TOUCH_TIMER);
    win_block_entity.color_changer.start_color = win_block_entity.color;
    win_block_entity.color_changer.target_color = win_block_entity.color * 1.5f;
    setup_color_changer(&win_block_entity);
    
    Spawn_Object win_block_object;
    win_block_object.entity = win_block_entity;
    str_copy(win_block_object.name, "win_block");
    spawn_objects.append(win_block_object);
    
    Entity agro_area_entity = make_entity({0, 0}, {20, 20}, {0.5f, 0.5f}, 0, TRIGGER);
    agro_area_entity.color = Fade(VIOLET, 0.6f);
    agro_area_entity.color_changer.start_color = agro_area_entity.color;
    agro_area_entity.color_changer.target_color = agro_area_entity.color * 1.5f;
    setup_color_changer(&agro_area_entity);
    
    Spawn_Object argo_area_object;
    argo_area_object.entity = agro_area_entity;
    str_copy(argo_area_object.name, "agro_area");
    spawn_objects.append(argo_area_object);
    
    Entity trigger_entity = make_entity({0, 0}, {20, 20}, {0.5f, 0.5f}, 0, TRIGGER);
    trigger_entity.color = Fade(GREEN, 0.6f);
    setup_color_changer(&trigger_entity);
    
    Spawn_Object trigger_object;
    trigger_object.entity = trigger_entity;
    str_copy(trigger_object.name, "trigger");
    spawn_objects.append(trigger_object);
    
    Entity bomb_entity = make_entity({0, 0}, {13, 13}, {0.5f, 0.5f}, 0, ENEMY | EXPLOSIVE);
    bomb_entity.color = ColorBrightness(RED, 0.2f);
    setup_color_changer(&bomb_entity);
    
    Spawn_Object bomb_object;
    bomb_object.entity = bomb_entity;
    str_copy(bomb_object.name, "bomb");
    spawn_objects.append(bomb_object);
    
    Entity kill_trigger_entity = make_entity({0, 0}, {20, 20}, {0.5f, 0.5f}, 0, TRIGGER | KILL_TRIGGER);
    kill_trigger_entity.color = Fade(RED, 0.6f);
    setup_color_changer(&kill_trigger_entity);
    
    Spawn_Object kill_trigger_object;
    kill_trigger_object.entity = kill_trigger_entity;
    str_copy(kill_trigger_object.name, "kill_trigger");
    spawn_objects.append(kill_trigger_object);
    
    Entity kill_switch_entity = make_entity({0, 0}, {20, 10}, {0.5f, 0.5f}, 0, ENEMY | KILL_SWITCH);
    kill_switch_entity.color = ColorBrightness(RED, 0.3f);
    setup_color_changer(&kill_switch_entity);
    
    Spawn_Object kill_switch_object;
    kill_switch_object.entity = kill_switch_entity;
    str_copy(kill_switch_object.name, "kill_switch");
    spawn_objects.append(kill_switch_object);
    
    Entity enemy_barrier_entity = make_entity({0, 0}, {20, 80}, {0.5f, 0.5f}, 0, ENEMY | ENEMY_BARRIER | PLAYER_TOUCH_TIMER);
    enemy_barrier_entity.color = ColorBrightness(GRAY, 0.2f);
    setup_color_changer(&enemy_barrier_entity);
    
    Spawn_Object enemy_barrier_object;
    enemy_barrier_object.entity = enemy_barrier_entity;
    str_copy(enemy_barrier_object.name, "enemy_barrier");
    spawn_objects.append(enemy_barrier_object);
    
    Entity spikes_entity = make_entity({0, 0}, {20, 5}, {0.5f, 0.5f}, 0, TRIGGER | SPIKES | KILL_TRIGGER);
    spikes_entity.color = Fade(RED, 0.9f);
    setup_color_changer(&spikes_entity);
    
    Spawn_Object spikes_object;
    spikes_object.entity = spikes_entity;
    str_copy(spikes_object.name, "spikes");
    spawn_objects.append(spikes_object);
    
    Entity propeller_entity = make_entity({0, 0}, {20, 120}, {0.5f, 1.0f}, 0, PROPELLER);
    propeller_entity.color = Fade(BLUE, 0.4f);
    propeller_entity.color_changer.start_color = propeller_entity.color;
    propeller_entity.color_changer.target_color = propeller_entity.color * 1.5f;
    setup_color_changer(&propeller_entity);
    
    Spawn_Object propeller_object;
    propeller_object.entity = propeller_entity;
    str_copy(propeller_object.name, "propeller");
    spawn_objects.append(propeller_object);
    
    Entity door_entity = make_entity({0, 0}, {5, 80}, {0.5f, 0.5f}, 0, DOOR | GROUND | TRIGGER);
    door_entity.color = ColorBrightness(PURPLE, 0.6f);
    setup_color_changer(&door_entity);
    
    Spawn_Object door_object;
    door_object.entity = door_entity;
    str_copy(door_object.name, "door");
    spawn_objects.append(door_object);
    
    Entity enemy_trigger_entity = make_entity({0, 0}, {15, 75}, {0.5f, 0.5f}, 0, ENEMY | TRIGGER);
    enemy_trigger_entity.color = ColorBrightness(BLUE, 0.6f);
    setup_color_changer(&enemy_trigger_entity);
    
    Spawn_Object enemy_trigger_object;
    enemy_trigger_object.entity = enemy_trigger_entity;
    str_copy(enemy_trigger_object.name, "enemy_trigger");
    spawn_objects.append(enemy_trigger_object);
    
    Entity centipede_entity = make_entity({0, 0}, {9, 10}, {0.5f, 0.5f}, 0, CENTIPEDE | MOVE_SEQUENCE | ENEMY);
    centipede_entity.color = ColorBrightness(RED, 0.6f);
    setup_color_changer(&centipede_entity);
    
    Spawn_Object centipede_object;
    centipede_object.entity = centipede_entity;
    str_copy(centipede_object.name, "centipede");
    spawn_objects.append(centipede_object);
    
    Entity centipede_segment_entity = make_entity({0, 0}, {4, 6}, {0.5f, 0.5f}, 0, ENEMY | CENTIPEDE_SEGMENT);
    centipede_segment_entity.color = ColorBrightness(ORANGE, 0.3f);
    setup_color_changer(&centipede_segment_entity);
    
    Spawn_Object centipede_segment_object;
    centipede_segment_object.entity = centipede_segment_entity;
    str_copy(centipede_segment_object.name, "centipede_segment");
    spawn_objects.append(centipede_segment_object);
    
    Entity shoot_stoper_entity = make_entity({0, 0}, {8, 14}, {0.5f, 0.5f}, 0, ENEMY | SHOOT_STOPER);
    shoot_stoper_entity.color = ColorBrightness(BLACK, 0.3f);
    setup_color_changer(&shoot_stoper_entity);
    
    Spawn_Object shoot_stoper_object;
    shoot_stoper_object.entity = shoot_stoper_entity;
    str_copy(shoot_stoper_object.name, "shoot_stoper");
    spawn_objects.append(shoot_stoper_object);
    
    Entity hit_booster_entity = make_entity({0, 0}, {8, 12}, {0.5f, 0.5f}, 0, ENEMY | HIT_BOOSTER);
    hit_booster_entity.color = ColorBrightness(YELLOW, 0.3f);
    setup_color_changer(&hit_booster_entity);
    
    Spawn_Object hit_booster_object;
    hit_booster_object.entity = hit_booster_entity;
    hit_booster_object.flags |= PLANNING_OBJECT;
    str_copy(hit_booster_object.name, "hit_booster");
    spawn_objects.append(hit_booster_object);
    
    Entity explosive_entity = make_entity({0, 0}, {8, 8}, {0.5f, 0.5f}, 0, ENEMY | EXPLOSIVE);
    explosive_entity.color = ColorBrightness(RED, 0.3f);
    setup_color_changer(&explosive_entity);
    
    Spawn_Object explosive_object;
    explosive_object.entity = explosive_entity;
    explosive_object.flags |= PLANNING_OBJECT;
    str_copy(explosive_object.name, "explosive");
    spawn_objects.append(explosive_object);
    
    // we use move sequence on jump shooter only to set jump points
    Entity jump_shooter_entity = make_entity({0, 0}, {10, 14}, {0.5f, 0.5f}, 0, ENEMY | JUMP_SHOOTER | MOVE_SEQUENCE | PARTICLE_EMITTER);
    // jump_shooter_entity.move_sequence->moving = true;
    // jump_shooter_entity.move_sequence->loop = true;
    jump_shooter_entity.color = ColorBrightness(BLACK, 0.3f);
    setup_color_changer(&jump_shooter_entity);
    
    Spawn_Object jump_shooter_object;
    jump_shooter_object.entity = jump_shooter_entity;
    str_copy(jump_shooter_object.name, "jump_shooter");
    spawn_objects.append(jump_shooter_object);
}

struct Tile_Sheet {
    String sheet_name;
    Array <Texture_Data> textures;
};

Array <Tile_Sheet> tile_sheets = {0};

void add_spawn_object_from_texture(Texture texture, const char *name, const char *directory_name = 0) {
    Entity texture_entity = make_entity({0, 0}, {(f32)texture.width * 0.25f, (f32)texture.height * 0.25f}, {0.5f, 0.5f}, 0, texture, TEXTURE);
    texture_entity.color = WHITE;
    texture_entity.color_changer.start_color = texture_entity.color;
    texture_entity.color_changer.target_color = texture_entity.color * 1.5f;
    // str_copy(texture_entity.name, name); 
    
    texture_entity.texture = texture;
    str_copy(texture_entity.texture_name, name);
    
    if (directory_name) {
        i32 tile_sheet_index = -1;
        for (i32 i = 0; i < tile_sheets.count; i++) {
            if (tile_sheets.get(i)->sheet_name == directory_name) {
                tile_sheet_index = i;
                break;
            }
        }
        
        Tile_Sheet *sheet = NULL;
        
        if (tile_sheet_index == -1) {
            sheet = tile_sheets.append({0});
            sheet->sheet_name = make_string(NULL, directory_name);
            sheet->textures = {0};
        } else {
            sheet = tile_sheets.get(tile_sheet_index);
        }
        
        assert(sheet);
        
        Texture_Data *new_data = sheet->textures.append({0});
        str_copy(new_data->name, name);
        new_data->texture = texture;
    }
    
    // assign_texture(&texture_entity, texture, name);
    
    Spawn_Object texture_object;
    texture_object.entity = texture_entity;
    str_copy(texture_object.name, name);
    
    spawn_objects.append(texture_object);
}

Texture get_texture(const char *name) {
    Texture found_texture;
    
    char *trimped_name = get_substring_before_symbol(name, '.');
    
    b32 found = false;
    for (i32 i = 0; i < loaded_textures.count; i++) {
        if (str_equal(loaded_textures.get_value(i).name, trimped_name)) {
            found_texture = loaded_textures.get_value(i).texture;
            found = true;
        }
    }
    if (!found) {
        print(tprintf("WARNING: Texture named %s cannot be found", trimped_name));
        found_texture = missing_texture;
    }
    
    return found_texture;
}

void load_textures(const char* path, b32 in_root_textures_directory) {
    String_Builder path_builder = make_string_builder(256, temp);
    builder_append(&path_builder, tstring(path));
    if (!str_end_with(path_builder.data, "\\")) {
        builder_append(&path_builder, tstring("\\"));
    }

    String path_string = make_string_from_builder(&path_builder, temp);

    FilePathList textures = LoadDirectoryFiles(path);
    for (i32 i = 0; i < textures.count; i++) {
        char *name = textures.paths[i];
        
        if (!IsPathFile(name)) {
            load_textures(name, false);
            continue;
        }
        
        // if (!str_end_with(name, ".png")) {
        //     continue;
        // }
        
        Texture texture = LoadTexture(name);
        
        substring_after_line(name, path_string.data);
        name = get_substring_before_symbol(name, '.');
        
        Texture_Data data = {0};
        str_copy(data.name, name);
        data.texture = texture;
        
        if (str_contains(data.name, "_normal_map")) {
            substring_before_line(data.name, "_normal_map");
            normal_maps.append(data);
        } else {
            loaded_textures.append(data);
            add_spawn_object_from_texture(texture, name, in_root_textures_directory ? 0 : path);
        }
    }
    UnloadDirectoryFiles(textures);
    
    path_string.free_data();
}

void load_all_textures() {
    // load_texrures is recursive and will check all subdirectories.
    load_textures("resources\\textures", true);
        
    missing_texture                 = get_texture("MissingTexture");
    spiral_clockwise_texture        = get_texture("vpravo");
    spiral_counterclockwise_texture = get_texture("levo");
    hitmark_small_texture           = get_texture("hitmark_small");
}

inline i32 next_entity_avaliable(Context *context, i32 start_index, Entity **entity, FLAGS flags) {
    // for (i32 i = start_index; i < context->entities.capacity; i++) {
    //     if (context->entities.has_index(i) && (flags == 0 || context->entities.get(i)->flags & flags)) {
    //         *entity = context->entities.get(i);
    //         return i;
    //     }
    // }
    
    // *entity = NULL;
    // return context->entities.capacity;
    start_index = context->entities.next_occupied_value(start_index, entity);
    // That cycle here only because we need to check flags and skip entities that does not match.
    while (*entity && (!((*entity)->flags & flags) && flags != 0)) {
        start_index = context->entities.next_occupied_value(start_index + 1, entity);
    }
    
    return start_index;
}

// inline void assign_texture(Entity *entity, Texture texture, const char *texture_name) {
//     entity->texture = texture;
//     str_copy(entity->texture_name, texture_name);
// }

void init_propeller_emitter_settings(Entity *e, Particle_Emitter *air_emitter) {
    enable_emitter(air_emitter);
    air_emitter->position            = e->position;
    air_emitter->over_distance       = 0;
    air_emitter->speed_multiplier    = e->propeller->power / 5.0f;
    air_emitter->count_multiplier    = e->propeller->power / 5.0f;
    air_emitter->lifetime_multiplier = (1.8f * (e->scale.y / 120.0f)) / air_emitter->speed_multiplier;
    air_emitter->spawn_offset        = e->up * e->scale.x * 0.5f;
    air_emitter->spawn_area          = {e->scale.x, e->scale.x};
    air_emitter->direction           = e->up;
}

String *register_entity_name(Entity *entity) {
    String name = temp_entity_name(entity);
    
    i32 index = entities_names.find(name);
    if (index < 0) {
        return entities_names.append(get_entity_name(entity, HEAP_ALLOCATOR));
    }
    
    return entities_names.get(index);
}

// NOTE: Currently passing centipede entity and not really using it because later we'll want to make right segment position not 
// just below previous segment, but along centipede path.
inline Vector2 get_centipede_segment_start_position(Entity *segment, Entity *centipede, i32 segment_index) {
    assert(segment->centipede_segment && centipede->centipede && segment_index >= 0);
    
    Entity *previous = segment->centipede_segment->previous;
    
    Vector2 result = previous->position - previous->up * previous->scale.y * (1.0f - previous->pivot.y) - previous->up * segment->scale.y * (1.0f - segment->pivot.y);
    
    return result;
}
inline void put_centipede_segment_at_right_start_position(Entity *segment, Entity *centipede, i32 segment_index) {
    Entity *previous = segment->centipede_segment->previous;
    change_up(segment, previous->up);
    Vector2 target_position = get_centipede_segment_start_position(segment, centipede, segment_index);  
    
    segment->position = target_position;
}

// ignore_existing_types for situations when we want to add new type info even if one is non-zero.
// For example on copy_and_add_entity we're doing thing like *entity = *to_copy, which means that we'll gonna have 
// the same pointers to types as a copy (like trigger, propeller, some enemy etc.) and in that case we don't want to 
// set this types to NULL, because we can forget something, so we're just telling that we should add new element to types 
// array anyway.
void init_entity(Entity *entity, b32 ignore_existing_types) {
    // We're initing only entities that already present in entity array of current level context.
    // That's because other entities or context or lights might want to get entity by it's id and it will fail if it's just 
    // entity that we created localy.
    
    assert(entity->context);
    assert(entity->id > 0 && get_entity(entity->id, entity->context)->id > 0);

    entity->color = entity->color_changer.start_color;

    // Init ammo pack.
    if (entity->flags & AMMO_PACK){
        entity->flags |= TEXTURE;
        entity->texture = get_texture("Prop");
    }
    
    // Init note.
    if (entity->flags & NOTE) {
        entity->flags |= TEXTURE;
        entity->texture = get_texture("editor_note");
    }

    // Init move sequence.
    if (entity->flags & MOVE_SEQUENCE) {
        if (!entity->move_sequence || ignore_existing_types) {
            i32 index = -1;
            entity->move_sequence = entity->context->move_sequences.append({0}, &index);
            entity->move_sequence->index = index;
        }
        
        if (entity->flags & CENTIPEDE) {
            entity->move_sequence->moving = true;
            entity->move_sequence->loop = true;
            entity->move_sequence->rotate = true;
            // entity->move_sequence->speed = 100;
        }
    }

    // Here we're initing concrete enemy types, which could not be mixed (unlike some modifiers like BLOCKER that could 
    // require union_enemy set to itself.
    //
    // It's all in the else-if statements so that we could know for sure if some enemy entity should be put in 
    // just_enemies array, where stored enemies that just use base Enemy struct (for now that's a AMMO_PACK for one, which 
    // don't need separate data besides basic Enemy.
    // Init enemies.
    if (0) {
    } else if (entity->flags & BIRD_ENEMY) { // Init bird enemy.
        entity->collision_flags = (GROUND | PLAYER | BIRD_ENEMY | CENTIPEDE_SEGMENT | ENEMY_BARRIER | NO_MOVE_BLOCK);
        change_color(entity, entity->flags & EXPLOSIVE ? ORANGE * 0.9f : YELLOW * 0.9f);
        
        change_scale(entity, {6, 10});
    
        if (!entity->bird_enemy || ignore_existing_types) {
            i32 index = -1;
            entity->bird_enemy = entity->context->bird_enemies.append({0}, &index);
            entity->bird_enemy->index = index;
        }
    
        entity->bird_enemy->max_hits_taken = 3;
        entity->bird_enemy->sword_kill_speed_modifier = 4;
        
        entity->bird_enemy->initial_position = entity->position;
        
        init_bird_emitters(entity);
    } else if (entity->flags & KILL_SWITCH) { // Init kill switch.
        if (!entity->kill_switch || ignore_existing_types) {
            i32 index = -1;
            entity->kill_switch = entity->context->kill_switches.append({0}, &index);
            entity->kill_switch->index = index;
        }
    
        entity->kill_switch->max_hits_taken = 1;
    } else if (entity->flags & TURRET) {  // Init turret.
        if (!entity->turret || ignore_existing_types) {
            i32 index = -1;
            entity->turret = entity->context->turrets.append({0}, &index);
            Turret *turret = entity->turret;
            
            // We don't want to set things that could be changed in editor (like shoot_every_tick) every time we init entity,
            // because what would cancel any changes made in editor. 
            // So we put that in here, under ignore_existing_types, because that's mean that we want to just put every 
            // default value and that's full initialization.
            turret->index = index;
            if (entity->flags & HOMING_TURRET) {
                turret->homing = true;
                turret->projectile_settings.launch_speed = 150;
                turret->projectile_settings.max_lifetime = 15;
                turret->shoot_every_tick = 8;
            } else {
                turret->homing = false;
                turret->projectile_settings.launch_speed = 75;
                turret->projectile_settings.max_lifetime = 7;
                turret->shoot_every_tick = 3;
            }
        }
    
        entity->turret->player_cannot_kill = true;
        
    } else if (entity->flags & CENTIPEDE) { // Init centipede.
        if (!entity->centipede || ignore_existing_types) {        
            i32 index = -1;
            entity->centipede = entity->context->centipedes.append({0}, &index);
            entity->centipede->index = index;
        }
        
        assert(entity->centipede);     
        Centipede *centipede = entity->centipede;
        
        for_array (i, &centipede->segments) {
            mark_entity_destroyed(centipede->segments.get_value(i));
        }
        centipede->segments.clear();
        for (i32 i = 0; i < centipede->segments_to_spawn; i++) {
            Entity* segment = spawn_object_by_name("centipede_segment", entity->position, entity->context);
            
            segment->runtime_only_flags |= SHOULD_NOT_SAVE | SHOULD_NOT_COPY;
            
            assert(segment->centipede_segment);
            segment->centipede_segment->head = entity;
            change_up(segment, entity->up);
            segment->draw_order = entity->draw_order + 1;
            centipede->segments.append(segment);
            if (i > 0) segment->centipede_segment->previous = centipede->segments.get_value(i-1);
            else       segment->centipede_segment->previous = entity;
            
            // segment->position = get_centipede_segment_start_position(segment, entity, i);
            put_centipede_segment_at_right_start_position(segment, entity, i);
            
            // assert(segment->move_sequence);
            // i32 segment_index = segment->move_sequence->index;
            // *segment->move_sequence = *entity->move_sequence;
            // segment->move_sequence->index = segment_index;
            // Probably we could think about a way to not copy array for every segment, but I'll probably will rewrite 
            // segments logic to work without move_sequence, so that doesn't matter currently.
            // segment->move_sequence->points = copy_array(&entity->move_sequence->points);
            
            segment->hidden = entity->hidden;
            
            segment->flags = (entity->flags) | CENTIPEDE_SEGMENT; // That's for some enemy settings and we'll change that if separate enemy flags will be made.
            remove_flag(&segment->flags, CENTIPEDE);
            remove_flag(&segment->flags, MOVE_SEQUENCE);
            
            i32 my_index = segment->union_enemy->index;
            *segment->union_enemy = *entity->union_enemy; // Just copying enemy settings that were applied to centipede in editor.
            segment->union_enemy->index =my_index;
                        
            assert(segment->flags & CENTIPEDE_SEGMENT);
                        
            init_entity(segment);
        }
        // }
    } else if (entity->flags & CENTIPEDE_SEGMENT) {    // Init centipede segment.
        assert(!(entity->flags & CENTIPEDE));
        if (!entity->centipede_segment || ignore_existing_types) {
            i32 index = -1;
            entity->centipede_segment = entity->context->centipede_segments.append({0}, &index);
            entity->centipede_segment->index = index;
        }
    } else if (entity->flags & JUMP_SHOOTER) { // Init jump shooter.
        if (!entity->jump_shooter || ignore_existing_types) {
            i32 index = -1;
            entity->jump_shooter = entity->context->jump_shooters.append({0}, &index);
            entity->jump_shooter->index = index;
        }
        
        entity->jump_shooter->max_hits_taken = 6;
        free_entity_particle_emitters(entity);
        entity->jump_shooter->trail_emitter_index  = add_entity_particle_emitter(entity, &air_dust_emitter);
        
        Particle_Emitter *trail_emitter = get_particle_emitter(entity->jump_shooter->trail_emitter_index);
        if (trail_emitter) {
            trail_emitter->follow_entity = false;
            enable_emitter(trail_emitter, entity->position);
        }
        
        entity->jump_shooter->flying_emitter_index = add_entity_particle_emitter(entity, &small_air_dust_trail_emitter_copy);
        Particle_Emitter *flying_emitter = get_particle_emitter(entity->jump_shooter->flying_emitter_index);
        if (flying_emitter) {
            flying_emitter->follow_entity = false;
        }
        
        entity->jump_shooter->sword_kill_speed_modifier = 10;
    } else if (entity->flags & WIN_BLOCK) { // Init win block.
        if (!entity->win_block || ignore_existing_types) {
            i32 index = -1;
            entity->win_block = entity->context->win_blocks.append({0}, &index);
            entity->win_block->index = index;
        }
    
        entity->win_block->max_hits_taken = 5;
    } else if (entity->flags & ENEMY) { // Init enemy.
        if (!entity->union_enemy || ignore_existing_types) {
            // We'll be here if entity is makred as enemy but was not previously inited, which means that it's just a enemy
            // without separate type and array.
                     
            i32 index = -1;
            entity->union_enemy = entity->context->just_enemies.append({0}, &index);
            entity->union_enemy->index = index;
        }
    }
    
    // That's for things that we want to init for every enemy.
    if (entity->flags & ENEMY) {
        assert(entity->union_enemy);
        entity->union_enemy->original_scale = entity->scale;
        
        entity->union_enemy->entity = entity;
    }
    
    // Load normal maps.
    if (entity->flags & TEXTURE) {
        for (i32 i = 0; i < normal_maps.count; i++) {        
            // We allow one normal map for different textures, but then texture should start with normal map name
            // and after normal map name *could* be '_'.
            // For example normal map "Brick_normal_map" will go to "Brick" and "Brick_v1", but not to "Brick1".
            Texture_Data *normal_map = normal_maps.get(i);
            if (str_start_with(entity->texture_name, normal_map->name)
                && (entity->texture_name[str_len(normal_map->name)] == '_' || entity->texture_name[str_len(normal_map->name)] == '\0')) {
                entity->have_normal_map = true;
                entity->normal_map_texture = normal_map->texture;
            }
        }
    }

    // Init no move block.
    if (entity->flags & NO_MOVE_BLOCK) {
        Light light = {0};
        light.color = entity->color;
        light.bake_shadows = true;
        light.opacity = 0.5f;
        
        Light *new_light = copy_and_add_light_to_entity(entity, &light, true);
    }
    
    // Init propeller.
    if (entity->flags & PROPELLER) {
        free_entity_particle_emitters(entity);
        
        if (!entity->propeller || ignore_existing_types) {
            i32 index = -1;
            entity->propeller = entity->context->propellers.append({0}, &index);
            entity->propeller->index = index;
        }
        assert(entity->propeller && entity->propeller->index >= 0);
        
        entity->propeller->air_emitter_index = add_entity_particle_emitter(entity, &air_emitter_copy);
        
        Particle_Emitter *air_emitter = get_particle_emitter(entity->propeller->air_emitter_index);
        
        if (air_emitter) {
            init_propeller_emitter_settings(entity, air_emitter);
        }
    }        
    
    // Init door.
    if (entity->flags & DOOR) {
        entity->flags |= TRIGGER;
    }
    
    // Init trigger.
    if (entity->flags & TRIGGER) {
        if (!entity->trigger || ignore_existing_types) {
            i32 index = -1;
            entity->trigger = entity->context->triggers.append({0}, &index);
            entity->trigger->index = index;
        }
    
        if (entity->flags & KILL_TRIGGER) {
            entity->trigger->settings |= KILL_PLAYER;
        }
        
        if (entity->flags & DOOR) {
            remove_flag(&entity->trigger->settings, PLAYER_TOUCH);
            entity->trigger->settings |= DRAW_LINES_TO_TRACKED;
        }
        
        if (entity->flags & ENEMY) {
            remove_flag(&entity->trigger->settings, PLAYER_TOUCH);
        }
        
        entity->trigger->start_tracking_count = entity->trigger->tracking.count;
        
        entity->trigger->entity = entity;
    }
    
    // Init sticky texture.
    if (entity->flags & STICKY_TEXTURE) { 
        if (!entity->sticky_texture || ignore_existing_types) {
            i32 index = -1;
            entity->sticky_texture = entity->context->sticky_textures.append({0}, &index);
            entity->sticky_texture->index = index;
        }
    }
    
    // Init hit booster.
    if (entity->flags & HIT_BOOSTER) {
        assert(entity->union_enemy);
        entity->union_enemy->max_hits_taken = -1;
    }
    
    // Init enemy barrier.
    if (entity->flags & ENEMY_BARRIER) {
        assert(entity->union_enemy);
    
        entity->union_enemy->max_hits_taken = 20; // We don't really want enemy barrier to be killed by the bullets.
    }
    
    // Init explosive.
    if (entity->flags & EXPLOSIVE) {
        assert(entity->union_enemy);
    
        entity->flags |= LIGHT;
        entity->color_changer.change_time = 5.0f;
        Light explosive_light = {0};
        // explosive_light.make_backshadows = false; @WTF screen goes black in game mode with this shit. Should change the way lights stored and way we get access to them so don't bother, but wtf ebat (also render doc don't loading with this shit)
        if (entity->union_enemy->explosive_radius_multiplier >= 3) {
            explosive_light.shadows_size_flags = BIG_LIGHT;
            explosive_light.backshadows_size_flags = BIG_LIGHT;
            explosive_light.make_backshadows = true;
            explosive_light.make_shadows     = true;
        } else if (entity->union_enemy->explosive_radius_multiplier > 1.5f) {
            explosive_light.shadows_size_flags = MEDIUM_LIGHT;
            explosive_light.backshadows_size_flags = MEDIUM_LIGHT;
            explosive_light.make_backshadows = true;
            explosive_light.make_shadows     = true;
        } else {
            explosive_light.make_shadows     = false;
            explosive_light.make_backshadows = false;
        }
        
        Light *new_light = copy_and_add_light_to_entity(entity, &explosive_light, true);
        if (new_light) {
            new_light->radius = 120;
            new_light->color = Fade(ColorBrightness(ORANGE, 0.2f), 1.0f);
            new_light->power = 1.0f;
        }
    }
    
    // Init blocker.
    if (entity->flags & BLOCKER && editor_state == GAME) {
        assert(entity->union_enemy);
    
        if (entity->union_enemy->blocker_sticky_id > 0) {
            mark_entity_destroyed(get_entity(entity->union_enemy->blocker_sticky_id));
        }
        
        if (!entity->union_enemy->blocker_immortal) {
            Texture texture = entity->union_enemy->blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
            Entity *sticky_entity = add_entity(entity->position, {10, 10}, {0.5f, 0.5f}, 0, texture, TEXTURE | STICKY_TEXTURE);
            // str_copy(sticky_entity->name, "blocker_attack_mark");
            sticky_entity->runtime_only_flags |= SHOULD_NOT_SAVE;
            //sticky_entity->texture = texture;
            sticky_entity->draw_order = 1;
            sticky_entity->sticky_texture->max_lifetime   = 0;
            // sticky_entity->sticky_texture->line_color  = Fade(ORANGE, 0.3f);
            sticky_entity->sticky_texture->need_to_follow = true;
            sticky_entity->sticky_texture->follow_id  = entity->id;
            sticky_entity->sticky_texture->birth_time = current_context->game_time;
            
            sticky_entity->sticky_texture->alpha = 0.8f;
            
            entity->union_enemy->blocker_sticky_id = sticky_entity->id;
        }
    }
    
    // Init sword size required.
    if (entity->flags & SWORD_SIZE_REQUIRED && editor_state == GAME) {
        assert(entity->union_enemy);
    
        if (entity->union_enemy->sword_required_sticky_id > 0) {
            mark_entity_destroyed(get_entity(entity->union_enemy->sword_required_sticky_id));
        }
        
        Texture texture = entity->union_enemy->big_sword_killable ? big_sword_killable_texture : small_sword_killable_texture;
        Entity *sticky_entity = add_entity(entity->position, {15, 30}, {0.5f, 0.5f}, 0, texture, TEXTURE | STICKY_TEXTURE);
        
        sticky_entity->sticky_texture->base_size = {4, 8};
        if (!entity->union_enemy->big_sword_killable) {
            sticky_entity->sticky_texture->base_size = {6, 12};
            sticky_entity->sticky_texture->alpha = 0.8f;
        } else {
            sticky_entity->sticky_texture->alpha = 0.4f;
        }
        
        // str_copy(sticky_entity->name, "sword_size_attack_mark");
        sticky_entity->runtime_only_flags |= SHOULD_NOT_SAVE;
        //sticky_entity->texture = texture;
        sticky_entity->draw_order = 1;
        sticky_entity->sticky_texture->max_lifetime   = 0;
        // sticky_entity->sticky_texture->line_color     = Fade(BLUE, 0.3f);
        sticky_entity->sticky_texture->need_to_follow = true;
        // sticky_entity->sticky_texture->draw_line      = true;
        sticky_entity->sticky_texture->follow_id  = entity->id;
        sticky_entity->sticky_texture->birth_time = current_context->game_time;
        
        entity->union_enemy->sword_required_sticky_id = sticky_entity->id;
    }
    
    // Init projectile.
    if (entity->flags & PROJECTILE) {
        if (!entity->projectile || ignore_existing_types) {
            i32 index = -1;
            entity->projectile = entity->context->projectiles.append({0}, &index);
            entity->projectile->index = index;
        }
    }
    
    // Init player.
    if (entity->flags & PLAYER) { 
        if (!entity->player_data || ignore_existing_types) {
            entity->player_data = &entity->context->player;
        }
    }
    
    entity->name = register_entity_name(entity);
    
    calculate_bounds(entity);
    setup_color_changer(entity);
} // end init entity

inline void autosave_level() {
    i32 max_autosaves = 5;
    i32 autosave_index = -1;    
    for (i32 i = 0; i < max_autosaves; i++) {
        String path = tstring("levels/autosaves/AUTOSAVE_%d_%s.level", i, c_str(current_context->level_name));
        if (!directory_exists(path)) {
            autosave_index = i;
            break;
        }
    }
    
    // Means we did not found vacant number so we'll see for oldest.
    if (autosave_index == -1) {
        i64 oldest_time = -1;
        
        for (i32 i = 0; i < max_autosaves; i++) {
            String path = tstring("levels/autosaves/AUTOSAVE_%d_%s.level", i, c_str(current_context->level_name));
            u64 modification_time = get_file_modification_time(path);
            if (oldest_time == -1 || modification_time < oldest_time) {
                oldest_time = modification_time;
                autosave_index = i;
            }
        }
    }
    
    assert(autosave_index != -1);
    
    save_level(tstring("autosaves/AUTOSAVE_%d_%s", autosave_index, c_str(current_context->level_name)));
}

void load_sounds() {
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
    for (i32 i = 0; i < sounds.count; i++) {
        char *name = sounds.paths[i];
        
        if (!str_end_with(name, ".ogg") && !str_end_with(name, ".wav")) {
            continue;
        }
        
        Sound sound = LoadSound(name);
        substring_after_line(name, "resources\\audio\\");
        name = get_substring_before_symbol(name, '.');
        
        Sound_Handler handler = {0};
        str_copy(handler.name, name);
        
        for (i32 s = 0; s < handler.buffer.capacity; s++) {
            handler.buffer.append(LoadSoundAlias(sound));
        }
        
        // i64 hash = hash_str(name);
        //UnloadSound(sound);
        
        sounds_array.append(handler);
        
        if (str_contains(name, "MissingSound")) {
            missing_sound = sounds_array.last();
        }
    }
    
    UnloadDirectoryFiles(sounds);
}

void play_sound(Sound_Handler *handler) {
    assert(handler->buffer.count > 0);
    
    Sound sound = handler->buffer.get_value(handler->current_index);
    handler->current_index = (handler->current_index + 1) % handler->buffer.capacity;
    
    SetSoundVolume(sound, rnd(handler->base_volume - handler->volume_variation, handler->base_volume + handler->volume_variation));
    SetSoundPitch (sound, rnd(handler->base_pitch - handler->pitch_variation, handler->base_pitch + handler->pitch_variation));
    
    PlaySound(sound);
}

void play_sound(Sound_Handler *handler, Vector2 position, f32 volume_multiplier, f32 base_pitch, f32 pitch_variation) {
    assert(handler->buffer.count > 0);
    
    Sound sound = handler->buffer.get_value(handler->current_index);
    handler->current_index = (handler->current_index + 1) % handler->buffer.capacity;
    
    //check vector to camera for volume and pan
    Vector2 to_position = position - current_context->cam.position;
    f32 len = magnitude(to_position);
    f32 max_len = 250;
    
    // Because we could be really not at the center of the screen with locked cam. We'll see how it's gonna be with horizontal rails.
    if (state_context.cam_state.locked || state_context.cam_state.on_rails_vertical) {
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

void play_sound(const char* name, Vector2 position, f32 volume_multiplier, f32 base_pitch, f32 pitch_variation) {
    char *trimped_name = get_substring_before_symbol(name, '.');    
    Sound_Handler *found_handler = NULL;
    
    for (i32 i = 0; i < sounds_array.count; i++) {
        if (str_equal(sounds_array.get(i)->name, trimped_name)) {
            found_handler = sounds_array.get(i);
        }
    }
    if (!found_handler) {
        printf("NO SOUND found %s\n", name);
        found_handler = missing_sound;
        // return;
    }
    
    play_sound(found_handler, position, volume_multiplier, base_pitch, pitch_variation);
}

inline void play_sound(const char* name, f32 volume_multiplier, f32 base_pitch, f32 pitch_variation) {
    play_sound(name, current_context->cam.position, volume_multiplier, base_pitch, pitch_variation);
}

// #define LIGHT_TEXTURE_SCALING_FACTOR 0.25f
// #define LIGHT_TEXTURE_SIZE_MULTIPLIER 2.0f

void init_context(Context *context) {
    assert(!context->inited);

    current_context = context;
    
    init_allocator(&current_context->memory_arena, Megabytes(4));

    init_array(&context->particles, MAX_PARTICLES, HEAP_ALLOCATOR);
    init_array(&context->particle_emitters, MAX_SMALL_COUNT_PARTICLE_EMITTERS + MAX_MEDIUM_COUNT_PARTICLE_EMITTERS + MAX_BIG_COUNT_PARTICLE_EMITTERS, HEAP_ALLOCATOR);
    
    init_array(&context->notes, 64, HEAP_ALLOCATOR);
    
    init_chunk_array(&context->entities, 512, HEAP_ALLOCATOR);
    init_chunk_array(&context->propellers, 16, HEAP_ALLOCATOR);
    init_chunk_array(&context->triggers, 32, HEAP_ALLOCATOR);
    init_chunk_array(&context->sticky_textures, 128, HEAP_ALLOCATOR);
    init_chunk_array(&context->move_sequences, 128, HEAP_ALLOCATOR);
    init_chunk_array(&context->bird_enemies, 64, HEAP_ALLOCATOR);
    init_chunk_array(&context->jump_shooters, 8, HEAP_ALLOCATOR);
    init_chunk_array(&context->kill_switches, 8, HEAP_ALLOCATOR);
    init_chunk_array(&context->turrets, 32, HEAP_ALLOCATOR);
    init_chunk_array(&context->win_blocks, 8, HEAP_ALLOCATOR);
    
    init_chunk_array(&context->projectiles, 256, HEAP_ALLOCATOR);
    
    init_chunk_array(&context->just_enemies, 64, HEAP_ALLOCATOR);
    
    init_chunk_array(&context->centipedes, 8, HEAP_ALLOCATOR);
    init_chunk_array(&context->centipede_segments, 128, HEAP_ALLOCATOR);
    
    
    init_chunk_array(&context->lights, 128, HEAP_ALLOCATOR);

    //init context
    // for (i32 i = 0; i < context->lights.capacity; i++) {
    //     Light *light = context->lights.append({0});
    //     // *(light) = {0};
        
    //     if (i < global_data.temp_lights_count) {
    //         light->make_shadows             = true;
    //         light->make_backshadows         = true;
    //         light->additional_shadows_flags = 0;
    //         light->grow_time                = 0;
    //         light->shrink_time              = 0;
    //         light->birth_time = -12;
            
    //         i32 size = ULTRA_SMALL_LIGHT;
    //         if (i < global_data.big_temp_lights_count) {
    //             size = BIG_LIGHT;
    //         } else if (i < global_data.big_temp_lights_count + global_data.huge_temp_lights_count) {
    //             size = HUGE_LIGHT;
    //         } else { // So it's usual temp lights
    //             light->make_shadows = false;
    //             light->make_backshadows = false;
    //         }
            
    //         light->shadows_size_flags       = size;
    //         light->backshadows_size_flags   = size;

    //         init_light(light);
    //     }
    // }
    
    for (i32 i = 0; i < context->particles.capacity; i++) {
        context->particles.append({0});
    }
    for (i32 i = 0; i < context->particle_emitters.capacity; i++) {
        context->particle_emitters.append({0});
    }
    for (i32 i = 0; i < context->line_trails.capacity; i++) {
        context->line_trails.append({0});
    }
    for (i32 i = 0; i < context->notes.capacity; i++) {
        context->notes.append({0});
    }
    
    // Init collison grid.
    i32 cells_columns = (i32)(current_context->collision_grid.size.x / current_context->collision_grid.cell_size.x);
    i32 cells_rows    = (i32)(current_context->collision_grid.size.y / current_context->collision_grid.cell_size.y);
    size_t cells_count = cells_columns * cells_rows;
    
    init_array(&current_context->collision_grid.cells, cells_count, HEAP_ALLOCATOR);
    
    for (i32 i = 0; i < cells_count; i++) {
        current_context->collision_grid.cells.append({0});
    }
    
    context->inited = true;
    
    // We're clearing level context right after initialization becase init happens only at the very beginning and next
    // level context is ready for new work right after clearing, so that's saves us some duplicate initialization 
    // and guarantees for that level context is the same as it will be in the middle of program execution
    // when it  will be just cleared and used again.
    clear_context(context);
}

Shader load_shader(const char *vertex, const char *fragment) {
    Shader loaded = LoadShader(vertex, fragment);
    if (!IsShaderValid(loaded)) {
        print("WARNIGNG: Shader could not load");
    }
    
    return loaded;
}

void load_render() {
    voronoi_seed_shader        = load_shader(0, "./resources/shaders/voronoi_seed.fs");
    jump_flood_shader          = load_shader(0, "./resources/shaders/jump_flood.fs");
    distance_field_shader      = load_shader(0, "./resources/shaders/distance_field.fs");
    global_illumination_shader = load_shader(0, "./resources/shaders/global_illumination1.fs");
}

void init_game() {
    push_performance_timer();
    log(tstring("Initing game."), PUSH_INDENTATION);

    initing_game = true;
    
    str_copy(loaded_context.name, "loaded_context");
    // str_copy(editor_context.name, "editor_context");
    str_copy(game_context.name, "game_context");
    str_copy(planning_context.name, "planning_context");
    str_copy(checkpoint_context.name, "checkpoint_context");
    str_copy(undo_context.name, "undo_context");
    str_copy(copied_entities_context.name, "copied_entities_context");
    
    // Now we need to init all level contexts once 
    init_context(&loaded_context);
    init_context(&game_context);
    init_context(&planning_context);
    init_context(&checkpoint_context);
    init_context(&undo_context);
    init_context(&copied_entities_context);
    
    for (i32 i = 0; i < MAX_LOADED_LEVELS; i++) {
        str_copy(loaded_editor_contexts[i].name, tprintf("editor_context_%d", i));
        init_context(&loaded_editor_contexts[i]);
    }
    editor_context = &loaded_editor_contexts[0];
    
    // player_data = &real_player_data;
    
    switch_current_context(&loaded_context);

    editor_state = EDITOR;

    global_data.entity_lights_start_index = global_data.temp_lights_count; 
    
    render = {0};
    
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

    input = {0};
    init_console();
    // current_level = {0};
    load_all_textures();
    init_spawn_objects();
    
    jump_shooter_bullet_hint_texture = get_texture("JumpShooterHintBullet.png");
    big_sword_killable_texture        = get_texture("BigSwordSticky.png");
    small_sword_killable_texture       = get_texture("SmallSwordSticky.png");
    perlin_texture = get_texture("PerlinNoise1.png");
    
    load_sounds();
    
    load_render();
    
    mouse_entity = make_entity(input.mouse_position, {1, 1}, {0.5f, 0.5f}, 0, 0);
    
    char level_name_to_load[256] = "\0";
    if (0 && RELEASE_BUILD) {
        str_copy(level_name_to_load, first_level_name);
    } else {
        str_copy(level_name_to_load, "test_level");
    }
    
    load_level_order();
    
    load_level(tstring(level_name_to_load));
    
    initing_game = false;
    
    i64 elapsed = pop_performance_timer_milliseconds();
    log(tstring("Finished initing game. Elapsed: %llums.", elapsed), POP_INDENTATION);
} // end init game end

void destroy_player() {
    if (!current_context->player_entity) {
        log("Destroy player was called when there's no player_entity is present. That should not happen.\n", LOG_ERROR);
        return;
    }

    mark_entity_destroyed(current_context->player_entity);
    current_context->player_entity->enabled   = false;
    
    mark_entity_destroyed(get_entity(current_context->player.connected_entities_ids.ground_checker_id));
    mark_entity_destroyed(get_entity(current_context->player.connected_entities_ids.sword_entity_id));
    
    current_context->player_entity = NULL;
}

void clean_up_scene() {
    if (current_context) {
        // ForEntities(entity, 0) {
        //     // entity->color = entity->color_changer.start_color;
        // }
        
        for (i32 i = 0; i < MAX_BIRD_POSITIONS; i++) {
            current_context->bird_slots[i].occupied = false;
        }
    }

    state_context = {0};
    checkpoint_trigger_id = -1;
    
    global_data.speedrun_timer.paused = false;
    if (!global_data.speedrun_timer.game_timer_active) {
        global_data.speedrun_timer.time = 0;        
    }
    
    global_data.entities_draw_queue.clear();
    
    assign_selected_entity(NULL);
    editor.in_editor_time = 0;
    close_create_box();
}

Entity *add_player_entity(Context *context, Player *data) {
    Context *original_context = current_context;
    switch_current_context(context);

    Entity *new_player_entity = add_entity(current_context->player_spawn_point, {1.0f, 2.0f}, {0.5f, 0.5f}, 0, RED, PLAYER | PARTICLE_EMITTER);
    new_player_entity->collision_flags = GROUND | ENEMY;
    new_player_entity->draw_order = 30;
    
    Entity *ground_checker = add_entity(new_player_entity->position - new_player_entity->up * new_player_entity->scale.y * 0.5f, {new_player_entity->scale.x * 0.9f, new_player_entity->scale.y * 1.5f}, {0.5f, 0.5f}, 0, CONNECTED_TO_PLAYER); 
    ground_checker->collision_flags = GROUND;
    ground_checker->color = Fade(PURPLE, 0.8f);
    ground_checker->draw_order = 31;
    
    Entity *left_wall_checker = add_entity(new_player_entity->position - new_player_entity->right * new_player_entity->scale.x * 0.5f + Vector2_up * new_player_entity->scale.y * 0.65f, {new_player_entity->scale.x * 0.6f, new_player_entity->scale.y * 0.1f}, {0.5f, 0.5f}, 0, CONNECTED_TO_PLAYER); 
    left_wall_checker->collision_flags = GROUND;
    left_wall_checker->color = Fade(PURPLE, 0.8f);
    left_wall_checker->draw_order = 31;
    
    Entity *right_wall_checker = add_entity(new_player_entity->position + new_player_entity->right * new_player_entity->scale.x * 0.5f + Vector2_up * new_player_entity->scale.y * 0.65f, {new_player_entity->scale.x * 0.6f, new_player_entity->scale.y * 0.1f}, {0.5f, 0.5f}, 0, CONNECTED_TO_PLAYER); 
    right_wall_checker->collision_flags = GROUND;
    right_wall_checker->color = Fade(PURPLE, 0.8f);
    right_wall_checker->draw_order = 31;
    
    Entity *sword_entity = add_entity(current_context->player_spawn_point, data->rifle_scale, {0.5f, 1.0f}, 0, GRAY + RED * 0.1f, SWORD);
    sword_entity->collision_flags = ENEMY;
    sword_entity->color   = GRAY + RED * 0.1f;
    sword_entity->draw_order = 25;
    // str_copy(sword_entity->name, "Player_Sword");
    
    data->connected_entities_ids.ground_checker_id     = ground_checker->id;
    data->connected_entities_ids.left_wall_checker_id  = left_wall_checker->id;
    data->connected_entities_ids.right_wall_checker_id = right_wall_checker->id;
    data->connected_entities_ids.sword_entity_id       = sword_entity->id;
    data->dead_man = false;
    
    data->timers = {0};
    
    free_entity_particle_emitters(new_player_entity);
    data->stun_emitter_index        = add_entity_particle_emitter(new_player_entity, &air_dust_emitter);
    data->rifle_trail_emitter_index = add_entity_particle_emitter(new_player_entity, &gunpowder_emitter);
    data->tires_emitter_index       = add_entity_particle_emitter(new_player_entity, &tires_emitter_copy);
    
    Particle_Emitter *tires_emitter = get_particle_emitter(data->tires_emitter_index);
    if (tires_emitter) {
        tires_emitter->follow_entity = false;
    }
    
    Particle_Emitter *rifle_trail_emitter = get_particle_emitter(data->rifle_trail_emitter_index);
    if (rifle_trail_emitter) {
        rifle_trail_emitter->follow_entity = false;
    }
    
    data->ammo_count = last_player_data.ammo_count;
    
    current_context->player_entity = new_player_entity;
    
    switch_current_context(original_context);
    
    return new_player_entity;
}

void game_setup_collisions() { 
    Vector2 grid_target_pos = current_context->player_spawn_point;
    current_context->collision_grid.origin = {(f32)((i32)grid_target_pos.x - ((i32)grid_target_pos.x % (i32)current_context->collision_grid.cell_size.x)), (f32)((i32)grid_target_pos.y - ((i32)grid_target_pos.y % (i32)current_context->collision_grid.cell_size.y))};
    
    b32 update_static_collision_cells = true;
    update_all_collision_cells(update_static_collision_cells);
}

void enter_gaming_state() {
    b32 should_init_entities = false;
    copy_context(&game_context, current_context, should_init_entities);
    
    switch_current_context(&game_context);
    game_state = GAMING;
    
    state_context.we_got_a_winner = false;
    
    if (!current_context->player_entity) {
        current_context->player = {0};
        add_player_entity(current_context, &current_context->player);
    }
    
    game_setup_collisions();
    
    ForEntities(entity, 0) {
        // if (should_init_entities) {
            update_editor_entity(entity);
            // Initing it again because some entities (like centipede) wanna init things only in game state. 
            // Need to change that.
            init_entity(entity);
            // Update_entity_collision_cells(entity, true);.
        // }
    }
    
    current_context->active_win_blocks_count = current_context->win_blocks.get_occupied_count();
    
    current_context->cam.cam2D.zoom = 0.35f;
    current_context->cam.target_zoom = 0.35f;
    current_context->cam.position = current_context->player_spawn_point;
}

void enter_planning_state() {
    assert(editor_state == GAME);
    game_state = GAME_PLANNING;
    
    clear_context(&game_context);
    
    clean_up_scene();
    switch_current_context(&planning_context);
    
    state_context.we_got_a_winner = false;
    
    game_setup_collisions();
    
    if (!current_context->player_entity) { 
        planning_context.player = {0};
        add_player_entity(current_context, &current_context->player);
    }
    
    current_context->cam.cam2D.zoom = 0.35f;
    current_context->cam.target_zoom = 0.35f;
    current_context->cam.position = current_context->player_spawn_point;
    
    if (!current_context->initially_simulated) {
        // Simulating game world for 2 seconds before the start.
        input = {0};
        for (i32 i = 0; i < FIXED_FPS * 2; i++) { 
            update_entities(&planning_context, TARGET_FRAME_TIME);
        }
        
        current_context->initially_simulated = true;
    }
}

void editor_enter_game_state(Context *from_context) {
    state_context = {0};
    global_data.just_entered_game_state = true;
    current_context->game_time = 0;
    core.time.hitstop = 0;
    core.time.previous_dt = 0;
    
    HideCursor();
    DisableCursor();
    
    editor_state = GAME;
    
    clear_context(&planning_context);
    copy_context(&planning_context, from_context, true);
    
    reset_planning_data(&planning_context);
    enter_planning_state();    
}

void kill_player() {
    Player *player_data = &current_context->player;
    b32 is_player_invincible = debug.god_mode || player_data->state_flags & PLAYER_INVINCIBLE;
    if (is_player_invincible && !state_context.we_got_a_winner || !player_data || player_data->dead_man || debug.dragging_player) { 
        return;
    }
    
    Entity *player_entity = current_context->player_entity;
    
    death_player_data = *player_data;

    emit_particles(&big_blood_emitter_copy, player_entity->position, player_entity->up, 1, 1);
    player_data->dead_man = true;
    player_data->timers.died_time = current_context->game_time;
    play_sound("PlayerTakeDamage", player_entity->position);
    
    // @VISUAL: It's better to separate default flash and player death flash.
    state_context.timers.background_flash_time = core.time.app_time;
}

void editor_enter_editor_state() {
    editor_state = EDITOR;
    state_context = {0};
    
    global_data.playing_replay = false;
    
    current_context->player.dead_man = false; 
    
    // We want to enable cursor when user hits escape key.
    HideCursor();
    DisableCursor();
    
    clean_up_scene();
    
    switch_current_context(editor_context);
    current_context->game_time = 0;
    core.time.hitstop = 0;
    core.time.previous_dt = 0;
    
    SetMusicVolume(tires_theme, 0);
    SetMusicVolume(wind_theme, 0);
    
    update_all_collision_cells(true);
}

Vector2 screen_to_world(Vector2 pos) {
    f32 zoom = current_context->cam.cam2D.zoom;

    f32 width = current_context->cam.width   ;
    f32 height = current_context->cam.height ;

    Vector2 screen_pos = pos;
    Vector2 world_pos = {(screen_pos.x - width * 0.5f) / current_context->cam.unit_size, (height * 0.5f - screen_pos.y) / current_context->cam.unit_size};
    world_pos /= zoom;
    world_pos = world_pos + current_context->cam.position;
    
    return world_pos;
}

inline Vector2 game_mouse_pos() {
    return screen_to_world(input.screen_mouse_position);
}

inline b32 maybe_destroy_entity(Entity *entity) {
    if (entity->destroyed) {
        free_entity(entity);
        return true;
    }
    
    // With will_be_destroyed we'll waiting one extra frame before actually be destroyed so every entity that referring
    // to that entity could detect that and remove reference.
    if (entity->will_be_destroyed) {
        entity->destroyed = true;
        return true;
    }
    
    return false;
}

inline void check_entities_that_should_be_destroyed(Context *context) {
    for_chunk_array(i, &context->entities) {
        maybe_destroy_entity(context->entities.get(i));
    }
}

void fixed_game_update(Context *context, f32 dt) {
    frame_rnd = perlin_noise3(context->game_time, core.time.app_time, 5) * 2 - 1.0f;
    frame_on_circle_rnd = get_perlin_in_circle(1.0f);
    
    Entity *player_entity = current_context->player_entity;
    Player *player_data = &current_context->player;

    if (editor_state == GAME && !state_context.in_pause_editor) {
        if (!global_data.playing_replay) {
            //record input
            if (level_replay.input_record.count >= MAX_INPUT_RECORDS - 1) {
                // level_replay.input_record.remove_first_half();
            } else {
                input.rnd_state = rnd_state;
                input.player_position = player_entity->position;
                level_replay.input_record.append({input});
            }
        } else {
            i32 frame = global_data.game_frame_count - level_replay.start_frame;
            if (frame >= level_replay.input_record.count) {
                // debug_set_time_scale(0);
                global_data.playing_replay = false;
            } else {
                replay_input = level_replay.input_record.get_value(frame).frame_input;
                // core.time = level_replay.input_record.get_value(global_data.game_frame_count).frame_time_data;
                rnd_state = replay_input.rnd_state;
            }
        }
    }

    debug.dragging_player = false;
    if (editor_state == GAME && player_entity) {
        if (IsKeyDown(KEY_K) && !console.is_open) {
            player_entity->position = input.mouse_position;
            debug.dragging_player = true;
            player_data->velocity = Vector2_zero;
        } 
    } 

    if (game_state == GAME_PLANNING && editor_state == GAME) {
        if (input.press_flags & ENTER_GAMING_STATE) {
            enter_gaming_state();
        } else {
            // Update_entities(-1);.
        }
            check_entities_that_should_be_destroyed(current_context);
    } else if (game_state == GAMING) {
        update_entities(context, dt);
    }
    update_particle_emitters(dt);
    // Update_particles(dt);.
    
    // Update camera.
    if (0) {
    } else if (editor_state == GAME && game_state == GAME_PLANNING) {
        // Update planning camera.
        current_context->cam.position += input.direction * dt * 300;
    } else if (editor_state == GAME && player_entity && !state_context.free_cam && !state_context.in_pause_editor && (!is_in_death_instinct() || !is_death_instinct_threat_active())) {
        f32 time_since_death_instinct_stop = core.time.app_time - state_context.death_instinct.stop_time;
        
        f32 locked_speed_t = clamp01(time_since_death_instinct_stop);
        f32 locked_speed_multiplier = lerp(0.001f, 1.0f, locked_speed_t * locked_speed_t * locked_speed_t);
    
        if (!state_context.cam_state.locked) {
            Vector2 player_velocity = player_data->velocity;
            f32 target_speed_multiplier = 1;
        
            f32 time_since_heavy_collision = context->game_time - player_data->heavy_collision_time;
            if (magnitude(player_data->velocity) < 80 && context->game_time > 5 && time_since_heavy_collision <= 1.0f) {
                player_velocity = player_data->heavy_collision_velocity;
                target_speed_multiplier = 0.05f;
            }
            
            f32 player_speed = magnitude(player_velocity);
        
            Vector2 target_position_velocity_addition = player_velocity * 0.25f;
            Vector2 target_position = player_entity->position + Vector2_up * 20 + target_position_velocity_addition;
            
            if (state_context.cam_state.on_rails_horizontal || state_context.cam_state.on_rails_vertical) {
                Entity *rails_trigger_entity = get_entity(state_context.cam_state.rails_trigger_id);
                Array <Vector2> *rails_points = &rails_trigger_entity->trigger->cam_rails_points;
                assert(rails_trigger_entity);
                
                b32 on_rails = rails_points->count >= 2;
                if (on_rails) {
                    Vector2 rails_player_position = player_entity->position + target_position_velocity_addition;
                    Vector2 point1 = rails_points->get_value(0);
                    Vector2 point2 = rails_points->get_value(1);
                    #define SECTION_POS(point) (state_context.cam_state.on_rails_horizontal ? point.x : point.y)
                    f32 player_section_pos = state_context.cam_state.on_rails_horizontal ? rails_player_position.x : rails_player_position.y;
                    f32 last_section_pos = state_context.cam_state.on_rails_horizontal ? rails_points->last_value().x : rails_points->last_value().y;
                    b32 is_going_right_or_up = SECTION_POS(point2) > SECTION_POS(point1);
                    
                    if (0) {
                    } else if (is_going_right_or_up && player_section_pos < SECTION_POS(point1)
                     || !is_going_right_or_up && player_section_pos > SECTION_POS(point1)) {
                        target_position = point1;    
                    } else if(is_going_right_or_up && player_section_pos > last_section_pos
                     || !is_going_right_or_up && player_section_pos < last_section_pos) {
                        target_position = rails_points->last_value();
                    } else {
                        for (i32 i = 0; i < rails_points->count - 1; i++) {                
                            point1 = rails_points->get_value(is_going_right_or_up ? i : i+1);
                            point2 = rails_points->get_value(is_going_right_or_up ? i+1 : i); 
                            if (player_section_pos >= SECTION_POS(point1) && player_section_pos <= SECTION_POS(point2)) {
                                break;
                            }
                        }
                        
                        // f32 section_len = magnitude(point2 - point1);
                        f32 section_len = (SECTION_POS(point2) - SECTION_POS(point1));
                        f32 section_t = clamp01((section_len - (SECTION_POS(point2) - player_section_pos)) / section_len);
                    
                        target_position = lerp(point1, point2, section_t);
                    }
                }
            } else {
                // Camera is unlocked completely
                // This section tries to keep enemies in sight. 
                // Currently we count enemies in 4 diagonal directions and each enemy in direction reduces amount of displacement.
                Vector2 additional_position = Vector2_zero;
                
                local_persist i32 direction_enemies_count[4];
                memset(direction_enemies_count, 0, sizeof(direction_enemies_count));
                // @OPTIMIZATION: Of course here we want just to through enemies, not through all entities.
                ForEntities(entity, ENEMY) {
                    assert(entity->union_enemy);
                    if (entity->flags == ENEMY || entity->flags & AMMO_PACK || entity->flags & HIT_BOOSTER || entity->flags & PROJECTILE || !entity->union_enemy->in_agro || entity->union_enemy->dead_man || entity->flags & PROJECTILE) {
                        continue;
                    }
                    
                    if (entity->flags & TURRET && !entity->turret->homing) {
                        continue;
                    }
                    
                    Vector2 vec_to_enemy = entity->position - player_entity->position;
                    Vector2 dir_to_enemy = normalized(vec_to_enemy);
                    f32 distance_to_enemy = magnitude(vec_to_enemy);
                    
                    if (distance_to_enemy > 1000) {
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
                    
                    if (enemies_count > 20) {
                        continue;
                    }
                    
                    Vector2 addition = dir_to_enemy * (((60.0f * 0.35f) / current_context->cam.cam2D.zoom) / enemies_count);
                    // If enemy really close or to far we want to reduce amount of displacement
                    f32 min_threshold = (SCREEN_WORLD_SIZE / current_context->cam.cam2D.zoom);
                    f32 max_threshold = (min_threshold * 2);
                    if (distance_to_enemy <= min_threshold) { 
                        addition *= (distance_to_enemy / min_threshold);
                    } else if (distance_to_enemy >= max_threshold) {
                        f32 t = (max_threshold / distance_to_enemy);
                        addition *= t * t;
                    }
                    
                    if (entity->flags & CENTIPEDE_SEGMENT) {
                        addition *= 0.1f;
                    }
                    
                    additional_position += addition;
                    
                    f32 max_x = (SCREEN_WORLD_SIZE / current_context->cam.cam2D.zoom) * 0.25f;
                    f32 max_y = (SCREEN_WORLD_SIZE / aspect_ratio / current_context->cam.cam2D.zoom) * 0.2f;
                    clamp(&additional_position.x, -max_x, max_x);
                    clamp(&additional_position.y, -max_y, max_y);
                }
                
                target_position += additional_position;
                
                if (additional_position != Vector2_zero) {
                    if (dot(player_data->velocity, additional_position) < 0) {
                        target_position += target_position_velocity_addition * 0.3f;
                    } else {
                        target_position -= target_position_velocity_addition * 0.5f;
                    }
                }
            } 
            
            Vector2 vec_to_target = target_position - current_context->cam.target;
            Vector2 vec_to_player = player_entity->position - current_context->cam.target;
            
            f32 target_dot = dot(vec_to_target, vec_to_player);
            
            f32 speed_t = clamp01(player_speed / 200.0f);
            
            f32 target_speed = lerp(3, 10, speed_t * speed_t);
            target_speed *= target_speed_multiplier;
            
            current_context->cam.target = lerp(current_context->cam.target, target_position, clamp01(dt * target_speed));
            
            f32 cam_speed = lerp(10.0f, 100.0f, speed_t * speed_t);
            
            current_context->cam.position = lerp(current_context->cam.position, current_context->cam.target, clamp01(dt * cam_speed * locked_speed_multiplier));
            
        // Locked camera
        } else if ((!is_in_death_instinct() || !is_death_instinct_threat_active()) || state_context.free_cam) {
            current_context->cam.position = lerp(current_context->cam.position, current_context->cam.target, clamp01(dt * 4 * locked_speed_multiplier));
            if (magnitude(current_context->cam.target - current_context->cam.position) <= EPSILON) {
                current_context->cam.position = current_context->cam.target;
            }
        }
    }
    
    state_context.cam_state.trauma -= dt * state_context.cam_state.trauma_decrease_rate;
    state_context.cam_state.trauma = clamp01(state_context.cam_state.trauma);
    
    state_context.explosion_trauma = clamp01(state_context.explosion_trauma - dt * 20);

    if (!is_in_death_instinct() || !is_death_instinct_threat_active()) {
        f32 zoom_speed = editor_state == GAME ? 3 : 10;
        Cam *cam = &current_context->cam;
        
        f32 target_zoom = cam->target_zoom;
        if (editor_state == GAME && player_data->in_slowmo && !state_context.cam_state.locked) {
            target_zoom *= 1.2f;
        }
        cam->cam2D.zoom = lerp(cam->cam2D.zoom, target_zoom, dt * zoom_speed);
        
        if (core.time.real_dt >= 0.1) {
            cam->cam2D.zoom = cam->target_zoom;
        }
    }
    
    if (abs(current_context->cam.cam2D.zoom - current_context->cam.target_zoom) <= EPSILON) {
        current_context->cam.cam2D.zoom = current_context->cam.target_zoom;
    }

    input.press_flags = 0;
    input.sum_mouse_delta = Vector2_zero;
    input.sum_mouse_wheel = 0;
    
    global_data.game_frame_count += 1;
} // end fixed game update

Cam get_cam_for_resolution(i32 width, i32 height) {
    Cam cam = current_context->cam;
    cam.unit_size = width / SCREEN_WORLD_SIZE; 
    cam.cam2D.target = cast(Vector2) { width/2.0f, height/2.0f };
    cam.cam2D.offset = cast(Vector2) { width/2.0f, height/2.0f };
    cam.width = width;
    cam.height = height;
    
    return cam;
}

void update_game() {
    clear_allocator(temp);

    frame_rnd = rnd01();
    frame_on_circle_rnd = rnd_on_circle();
    
    //update input
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
    
    if (can_player_input) {
        if (IsKeyDown(KEY_RIGHT)) {
            input.direction.x = 1;
            input.hold_flags |= RIGHT;
        } else if (IsKeyDown(KEY_LEFT)) {
            input.direction.x = -1;
            input.hold_flags |= LEFT;
        }
        if (IsKeyDown(KEY_UP)) {
            input.direction.y = 1;
            input.hold_flags |= UP;
        } else if (IsKeyDown(KEY_DOWN)) {
            input.direction.y = -1;
            input.hold_flags |= DOWN;
        }
        if (IsKeyDown(KEY_D)) {
            input.direction.x = 1;
            input.hold_flags |= RIGHT;
        } else if (IsKeyDown(KEY_A)) {
            input.direction.x = -1;
            input.hold_flags |= LEFT;
        }
        if (IsKeyDown(KEY_W)) {
            input.direction.y = 1;
            input.hold_flags |= UP;
        } else if (IsKeyDown(KEY_S)) {
            input.direction.y = -1;
            input.hold_flags |= DOWN;
        }
        
        if (IsKeyDown(KEY_F)) {
            input.hold_flags |= SWORD_BIG_DOWN;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            input.hold_flags |= SHOOT_DOWN;
        }
        
        if (input.direction.x != 0 || input.direction.y != 0) {
            normalize(&input.direction);
        }
        
        if (input.tap_direction.x == 0 && IsKeyPressed(KEY_RIGHT)) {
            input.tap_direction.x = 1;
        } else if (input.tap_direction.x == 0 && IsKeyPressed(KEY_LEFT)) {
            input.tap_direction.x = -1;
        } else {
            input.tap_direction.x = 0;
        }
        if (input.tap_direction.y == 0 && IsKeyPressed(KEY_UP)) {
            input.tap_direction.y = 1;
        } else if (input.tap_direction.y == 0 && IsKeyPressed(KEY_DOWN)) {
            input.tap_direction.y = -1;
        } else {
            input.tap_direction.y = 0;
        }
    
        if (input.hold_flags & RIGHT) {
            input.sum_direction.x = 1;
        } else if (input.hold_flags & LEFT) {
            input.sum_direction.x = -1;
        }
        if (input.sum_direction.x != 0) {
            input.last_non_zero_x = input.sum_direction.x;
        }
        
        if (input.hold_flags & UP) {
            input.sum_direction.y = 1;
        } else if (input.hold_flags & DOWN) {
            input.sum_direction.y = -1;
        }
        
        
        if (IsKeyPressed(KEY_SPACE)) {
            input.press_flags |= JUMP;
        }
        if (IsKeyPressed(KEY_F)) {
            input.press_flags |= SWORD_BIG;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            input.press_flags |= SHOOT;
        }
        
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            input.hold_flags |= SPIN_DOWN;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            input.press_flags |= SPIN;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            input.press_flags |= SPIN_RELEASED;
        }
        
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            input.press_flags |= SHOOT_RELEASED;
        }
        
        if (game_state == GAME_PLANNING) {
            if (IsKeyPressed(KEY_SPACE)) {
                input.press_flags |= ENTER_GAMING_STATE;
            }
        }
    }
    //end update input
    
    mouse_entity.position = input.mouse_position;
    
    if (screen_size_changed) {
        global_cam_data.width = screen_width;
        global_cam_data.height = screen_height;
        global_cam_data.unit_size = screen_width / SCREEN_WORLD_SIZE; 
        global_cam_data.cam2D.target = cast(Vector2) { screen_width/2.0f, screen_height/2.0f };
        global_cam_data.cam2D.offset = cast(Vector2) { screen_width/2.0f, screen_height/2.0f };
        
        if (global_cam_data.width != 0 && global_cam_data.height != 0 && !window_minimized) {
            aspect_ratio = (f32)global_cam_data.width / (f32)global_cam_data.height;
            UnloadRenderTexture(render.main_render_texture);
            UnloadRenderTexture(global_illumination_rt);
            UnloadRenderTexture(light_geometry_rt);
            
            render.main_render_texture = LoadRenderTexture(screen_width, screen_height);
            global_illumination_rt = LoadRenderTexture(screen_width, screen_height);
            light_geometry_rt = LoadRenderTexture(screen_width, screen_height);
        }
        
        setup_context_cam(current_context);
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_SPACE)) {
        if (editor_state == EDITOR) {
            editor_enter_game_state(editor_context);
        } else if (editor_state == GAME) {
            editor_enter_editor_state();
        }
    } 
    
    // editor game pause
    if (IsKeyPressed(KEY_TAB) && !console.is_open && editor_state == GAME) {
        assign_selected_entity(NULL);
        state_context.in_pause_editor = !state_context.in_pause_editor;
        if (state_context.in_pause_editor) {
            editor.in_editor_time = 0;
        }
    }
    
    Player *player_data = &current_context->player;
    
    // In game level restart.
    if (editor_state == GAME && !console.is_open) {
        if (IsKeyPressed(KEY_T)) {
            if (global_data.speedrun_timer.game_timer_active && player_data->dead_man) {
                restart_game();
                global_data.speedrun_timer.time = 0;
            } else if (global_data.speedrun_timer.level_timer_active) {
                // editor_enter_editor_state();
                // enter_and_reload_game_state(editor_context, true);
                global_data.speedrun_timer.time = 0;
            } else {
                b32 is_have_checkpoint = checkpoint_trigger_id > 0;
                global_data.playing_replay = false;            
                // editor_enter_editor_state();
                if (is_have_checkpoint) {
                    // enter_and_reload_game_state(&checkpoint_context, false);
                    current_context->player_entity = checkpoint_player_entity;
                    // real_player_data = checkpoint_player_data;
                    core.time = checkpoint_time;
                    state_context = checkpoint_state_context;
                    
                    player_data->velocity = Vector2_zero;
                    global_data.speedrun_timer.time = 0;
                } else {
                    // enter_and_reload_game_state(editor_context, true);
                    enter_planning_state();
                }
            }
        }
        
        if (state_context.we_got_a_winner && IsKeyPressed(KEY_V)) {
            maybe_load_next_level();
        }
    }
    
    core.time.app_time += GetFrameTime();
    core.time.real_dt = GetFrameTime();
    
    Entity *player_entity = current_context->player_entity;
    
    // Update death instinct.
    if (is_in_death_instinct() && is_death_instinct_threat_active() && editor_state == GAME) {
        f32 time_since_death_instinct = core.time.app_time - state_context.death_instinct.start_time;
        
        Entity *threat_entity = get_entity(state_context.death_instinct.threat_entity_id);
        Vector2 cam_position = player_entity->position + (threat_entity->position - player_entity->position) * 0.5f;
        
        if (state_context.death_instinct.last_reason == ENEMY_ATTACKING) {
            f32 distance_t = (1.0f - clamp01(magnitude(threat_entity->position - player_entity->position) / get_death_instinct_radius(threat_entity->scale)));
            
            f32 t = EaseOutQuint(distance_t);
            core.time.target_time_scale = lerp(0.6f, 0.02f, t * t);
            current_context->cam.position        = lerp(current_context->cam.position, lerp(current_context->cam.target, cam_position, t * t), core.time.real_dt * t * 5);
            current_context->cam.cam2D.zoom      = lerp(current_context->cam.cam2D.zoom, lerp(current_context->cam.target_zoom, 0.55f, t * t), core.time.real_dt * t * 5);
        } else if (state_context.death_instinct.last_reason == SWORD_WILL_EXPLODE) {
            f32 distance_t = (1.0f - clamp01(state_context.death_instinct.angle_till_explode / 150.0f));
            f32 t = EaseOutQuint(distance_t);
            core.time.target_time_scale = lerp(0.6f, 0.015f, t);
            current_context->cam.position        = lerp(current_context->cam.position, lerp(current_context->cam.target, cam_position, t), core.time.real_dt * t * 5);
            current_context->cam.cam2D.zoom      = lerp(current_context->cam.cam2D.zoom, lerp(current_context->cam.target_zoom, 0.55f, t), core.time.real_dt * t * 5);
        } else {
            current_context->cam.position        = lerp(current_context->cam.position, cam_position, clamp01(core.time.real_dt * 5));
            current_context->cam.cam2D.zoom      = lerp(current_context->cam.cam2D.zoom, 0.55f, clamp01(core.time.real_dt * 5));
            core.time.target_time_scale = lerp(core.time.target_time_scale, 0.03f, clamp01(core.time.real_dt * 10));
        }
        
        f32 instinct_t = time_since_death_instinct / state_context.death_instinct.duration;
        make_line(player_entity->position, threat_entity->position, Fade(RED, instinct_t * instinct_t));
        f32 radius_multiplier = lerp(80.0f, 10.0f, sqrtf(instinct_t));
        Color ring_color = Fade(ColorBrightness(RED, abs(sinf(core.time.app_time * lerp(1.0f, 10.0f, instinct_t)) * 0.8f - 0.5f)), instinct_t * 0.4f);
        make_ring_lines(threat_entity->position, 1.0f * radius_multiplier, 2.0f * radius_multiplier, 14, ring_color);
        
        if (time_since_death_instinct >= 0.2f && !state_context.death_instinct.played_effects) {
            play_sound("DeathInstinct", 2);
            state_context.death_instinct.played_effects = true;
        }
        
        state_context.death_instinct.was_in_death_instinct = true;
    } else if (state_context.death_instinct.was_in_death_instinct) {
        stop_death_instinct();
        // core.time.target_time_scale = 1;
        state_context.death_instinct.was_in_death_instinct = false;
    } else if (editor_state == GAME) {
        core.time.target_time_scale = lerp(core.time.target_time_scale, 1.0f, clamp01(core.time.real_dt * 2));        
        if (1.0f - core.time.target_time_scale <= 0.01f) {
            core.time.target_time_scale = 1;
        }
    }
    
    // Hitstop logic.
    if (editor_state == GAME && !state_context.in_pause_editor) {
        core.time.unscaled_dt = GetFrameTime();
        if (core.time.hitstop > 0) {
            core.time.time_scale = fminf(core.time.time_scale, 0.1f);
            core.time.hitstop -= core.time.real_dt;
        } 
        
        if (core.time.hitstop <= 0) {
            if (IsKeyDown(KEY_LEFT_SHIFT) && core.time.target_time_scale > 0.4f) {
                core.time.time_scale = 0.25f;
                player_data->timers.slowmo_timer = fminf(player_data->timers.slowmo_timer + core.time.real_dt, 10.0f);
                player_data->in_slowmo = true;
            } else {
                core.time.time_scale = core.time.target_time_scale;
                player_data->timers.slowmo_timer = fmaxf(player_data->timers.slowmo_timer - core.time.real_dt, 0);
                player_data->in_slowmo = false;
            }
        }
        
        if (core.time.debug_target_time_scale != 1 && (core.time.debug_target_time_scale < core.time.target_time_scale || core.time.hitstop <= 0)) {
           core.time.time_scale = core.time.debug_target_time_scale; 
        }
        
        core.time.dt = GetFrameTime() * core.time.time_scale;
    } else if (editor_state == EDITOR || state_context.in_pause_editor) {
        core.time.unscaled_dt = 0;
        core.time.dt          = 0;
    }

    
    if (editor_state == EDITOR || state_context.in_pause_editor) {
        update_editor_ui();
        update_editor();
    }
    
    update_console();
    
    if (editor_state == GAME && game_state == GAME_PLANNING) { 
        update_planning(current_context);
    }
    
    if (editor_state == GAME && !state_context.in_pause_editor) {
        f32 full_delta = core.time.dt + core.time.previous_dt;
        core.time.previous_dt = 0;
        
        full_delta = Clamp(full_delta, 0, 0.5f * core.time.time_scale);
        global_data.updated_today = false;
        while (full_delta >= TARGET_FRAME_TIME) {
            core.time.fixed_dt = TARGET_FRAME_TIME;
            fixed_game_update(current_context, core.time.fixed_dt);
            global_data.updated_today = true;
            full_delta -= TARGET_FRAME_TIME;
        }
        
        if (global_data.updated_today) {
            input.hold_flags = 0;
            core.time.not_updated_accumulated_dt = 0;
            input.sum_direction = Vector2_zero;
        } else { 
            core.time.not_updated_accumulated_dt = full_delta;
        }
        
        core.time.previous_dt = full_delta;
    } else {
        fixed_game_update(current_context, core.time.real_dt);
    }
    
    // Update speedrun timer.
    if (editor_state == GAME && (global_data.speedrun_timer.level_timer_active || global_data.speedrun_timer.game_timer_active)) {
        Color color = WHITE;
        if (state_context.we_got_a_winner) {
            global_data.speedrun_timer.paused = true;
            color = GREEN;
        } else if (player_data->dead_man) {
            color = RED;
            global_data.speedrun_timer.paused = true;
        }
        
        if (!global_data.speedrun_timer.paused) {
            global_data.speedrun_timer.time += core.time.dt;
        }
        
        const char *title_and_time = tprintf("%s\n%.4f", global_data.speedrun_timer.level_timer_active ? c_str(current_context->level_name) : "Game speedrun", global_data.speedrun_timer.time);
        Old::make_ui_text(title_and_time, {screen_width * 0.46f, 5}, "speedrun_timer", color, 22);
    }
    
    if (IsKeyPressed(KEY_PAGE_UP)) {
        state_context.free_cam = !state_context.free_cam;
        if (!state_context.free_cam) {
            current_context->cam.target_zoom = debug.last_zoom;
        } else {
            debug.last_zoom = current_context->cam.target_zoom;
        }
    }
    
    if (IsKeyPressed(KEY_L) && IsKeyDown(KEY_RIGHT_ALT)) {
        debug_unlock_camera();
    }
    
    if (editor_state == GAME && player_entity && !state_context.free_cam && !state_context.in_pause_editor) {
    } else {
        f32 zoom = current_context->cam.target_zoom;

        // Update editor camera.
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            current_context->cam.position += (cast(Vector2) {-input.mouse_delta.x / zoom, input.mouse_delta.y / zoom}) / (current_context->cam.unit_size);
        }
        if (input.mouse_wheel != 0 && !console.is_open && !editor.create_box_active) {
            f32 max_zoom = 5;
            f32 min_zoom = 0.03f;
            if (input.mouse_wheel > 0 || input.mouse_wheel < 0) {
                current_context->cam.target_zoom += input.mouse_wheel * 0.05f;
                clamp(&current_context->cam.target_zoom, min_zoom, max_zoom);
            }
        }
    }
    
    if (editor.update_cam_view_position) {
        current_context->cam.view_position = current_context->cam.position;
    }
    
    draw_game();
    
    // #if RELEASE_BUILD
    // UpdateMusicStream(ambient_theme);
    // #endif
    UpdateMusicStream(wind_theme);
    UpdateMusicStream(tires_theme);
    
    if (state_context.playing_relax || state_context.we_got_a_winner) {
        UpdateMusicStream(relas_music);
    }
    
    global_data.just_entered_game_state = false;
    
    
    // We do this so lights don't bake all at one frame 
    global_data.baked_shadows_this_frame = false;
    
    global_data.app_frame_count += 1;
} // Update game end.

void update_color_changer(Entity *entity, f32 dt) {
    Color_Changer *changer = &entity->color_changer;
    
    if (changer->changing || changer->frame_changing) {
        f32 t = abs(sinf(core.time.app_time * changer->change_time));
        entity->color = lerp(changer->start_color, changer->target_color, t);
    } else if (entity->flags & EXPLOSIVE) {
        Color target_color = ColorBrightness(changer->start_color, 2);
        f32 t = abs(sinf(current_context->game_time * changer->change_time));
        entity->color = lerp(changer->start_color, target_color, t);
        
        if (entity->lights.count > 0) {
            Light *light = get_light(entity->lights.get_value(0));
            light->color  = lerp(Fade(target_color, 1), Fade(ColorBrightness(ORANGE, 0.3f), 0.9f), t);
            light->radius = get_explosion_radius(entity) * 2 * lerp(0.9f, 1.3f, t);
        }
    } else if (changer->interpolating) {
        entity->color = lerp(changer->start_color, changer->target_color, changer->progress);
    } else {
        if (editor_state == EDITOR || state_context.in_pause_editor) {
            entity->color = changer->start_color;
        }
    }
    
    if (entity->flags & BLOCKER) {
        entity->color = ColorTint(entity->color, RAYWHITE);
    }
    
    changer->frame_changing = false;
}

b32 check_col_point_rec(Vector2 point, Entity *e) {
    Vector2 l_u = get_left_up_no_rot(e);
    Vector2 r_d = get_right_down_no_rot(e);

    return ((point.x >= l_u.x) && (point.x <= r_d.x) && (point.y >= r_d.y) && (point.y <= l_u.y));
}

inline b32 check_col_circles(Circle a, Circle b) {
    f32 distance = sqr_magnitude(a.position - b.position);
    
    return distance < a.radius * a.radius + b.radius * b.radius;
}

inline Vector2 get_rotated_vector_90(Vector2 v, f32 clockwise) {
    return {-v.y * clockwise, v.x * clockwise};
}

inline Light *get_light(i32 index, Context *context) {        
    if (!context) context = current_context;
    assert(index > -1);
    
    return context->lights.get(index);
}

inline Entity *get_entity(i32 id, Context *context) {
    if (!context) context = current_context;
    
    Entity *entity = context->entities.get(id - 1);
    if (!entity) {
        log("Wrong entity id! Returning NULL in get_entity function!", LOG_ERROR);
    }
    // Entity ids are index + 1, so 0 is always invalid.
    return entity;
}

// This function is basically the same as usual get_entity, but we're not logging error if failed to get entity.
inline Entity *maybe_get_entity(i32 id, Context *context) {
    if (!context) context = current_context;
    
    Entity *entity = context->entities.get(id - 1);
    // Entity ids are index + 1, so 0 is always invalid.
    return entity;
}

inline b32 entity_array_contains_id(Entity **arr, i32 count, i32 id) {
    for (i32 i = 0; i < count; i++) {
        if (arr[i]->id == id) {
            return true;
        }
    }
    
    return false;
}

void assign_moving_vertex_entity(Entity *e, i32 vertex_index) {
    Vector2 *vertex = e->vertices.get(vertex_index);

    assign_selected_entity(e);
    editor.moving_vertex = vertex;
    editor.moving_vertex_index = vertex_index;
    editor.moving_vertex_entity = e;
    editor.moving_vertex_entity_id = e->id;
    
    editor.dragging_entity = NULL;
}

void move_vertex(Entity *entity, Vector2 target_position, i32 vertex_index) {
    Vector2 *vertex = entity->vertices.get(vertex_index);
    Vector2 *unscaled_vertex = entity->unscaled_vertices.get(vertex_index);
    
    Vector2 local_target = local_position(entity, target_position);

    *vertex = local_target;
    *unscaled_vertex = {vertex->x / entity->scale.x, vertex->y / entity->scale.y};
    
    calculate_bounds(entity);
}

void editor_destroy_entity(Entity *entity) {
    mark_entity_destroyed(entity);
    
    if (is_editor_active()) {
        undo_mark_entity_changed(entity);
        editor.just_deleted_entity = true; // That thing beign used only for undo for now.
    }
    
    assign_selected_entity(NULL);
    editor.dragging_entity = NULL;
    editor.cursor_entity   = NULL;
}

void destroy_multiselected_entities() {    
    for (i32 i = 0; i < editor.multiselection.entities.count; i++) {
        Entity *entity = get_entity(editor.multiselection.entities.get_value(i));
        if (!entity) {
            continue;    
        }
        
        editor_destroy_entity(entity);
    }
    
    clear_multiselected_entities();
}

void editor_destroy_entity(i32 entity_id) {
    editor_destroy_entity(get_entity(entity_id));
}

// New selected could be NULL, which means that we're not selecting anyone anymore.
void assign_selected_entity(Entity *new_selected) {
    // We're just not allowing copying things for that flag so it could not be unchanged copy. Maybe will try to work around
    // it eventually, but probably will leave it as constrained as it is.
    if (new_selected && new_selected->runtime_only_flags & SHOULD_NOT_COPY) {
        return;
    }
    
    if (editor.selected) {
        editor.selected->color_changer.changing = 0;
        editor.selected->color = editor.selected->color_changer.start_color;
        
        assert(editor.selected_unchanged_copy);
        free_entity(editor.selected_unchanged_copy);
    }
    
    editor.selected_unchanged_copy = NULL;
    
    if (new_selected) {
        new_selected->color_changer.changing = 1;
        
        editor.selected_unchanged_copy = copy_and_add_entity(new_selected, &undo_context);
    }
    
    editor.selected = new_selected;
    
    focus_input_field.in_focus = false;
}

void start_closing_create_box() {
    editor.create_box_closing = true;
    editor.create_box_lifetime = editor.create_box_slide_time;
}

void close_create_box() {
    editor.create_box_active = false;
    editor.create_box_closing = false;
    if (str_equal(focus_input_field.tag, "create_box")) {
        focus_input_field.in_focus = false;
    }

    editor.create_box_lifetime = 0;
}

void make_color_picker(Vector2 inspector_position, Vector2 inspector_size, f32 v_pos, Color *color_ptr) {
    Old::make_ui_text("Color: ", {inspector_position.x + 25, v_pos}, "light_bake_shadows");
    f32 color_h_pos_mult = 0.2f;
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, WHITE, *color_ptr == WHITE, "light_color_picker_white")) {
        *color_ptr = WHITE;
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(RED, 0.3f), *color_ptr == ColorBrightness(RED, 0.3f), "light_color_picker_RED")) {
        *color_ptr = ColorBrightness(RED, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(ORANGE, 0.3f), *color_ptr == ColorBrightness(ORANGE, 0.3f), "light_color_picker_ORANGE")) {
        *color_ptr = ColorBrightness(ORANGE, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(SKYBLUE, 0.3f), *color_ptr == ColorBrightness(SKYBLUE, 0.3f), "light_color_picker_SKYBLUE")) {
        *color_ptr = ColorBrightness(SKYBLUE, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(BLUE, 0.3f), *color_ptr == ColorBrightness(BLUE, 0.3f), "light_color_picker_BLUE")) {
        *color_ptr = ColorBrightness(BLUE, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(GREEN, 0.3f), *color_ptr == ColorBrightness(GREEN, 0.3f), "light_color_picker_GREEN")) {
        *color_ptr = ColorBrightness(GREEN, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(LIME, 0.3f), *color_ptr == ColorBrightness(LIME, 0.3f), "light_color_picker_LIME")) {
        *color_ptr = ColorBrightness(LIME, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(PINK, 0.3f), *color_ptr == ColorBrightness(PINK, 0.3f), "light_color_picker_PINK")) {
        *color_ptr = ColorBrightness(PINK, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(VIOLET, 0.3f), *color_ptr == ColorBrightness(VIOLET, 0.3f), "light_color_picker_VIOLET")) {
        *color_ptr = ColorBrightness(VIOLET, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(MAGENTA, 0.3f), *color_ptr == ColorBrightness(MAGENTA, 0.3f), "light_color_picker_MAGENTA")) {
        *color_ptr = ColorBrightness(MAGENTA, 0.3f);
    }
    color_h_pos_mult += 0.05f;
    
    if (Old::make_ui_color_picker({inspector_position.x + inspector_size.x * color_h_pos_mult, v_pos}, ColorBrightness(BROWN, 0.3f), *color_ptr == ColorBrightness(BROWN, 0.3f), "light_color_picker_MAGENTA")) {
        *color_ptr = ColorBrightness(BROWN, 0.3f);
    }
    color_h_pos_mult += 0.05f;
}

void make_light_size_picker(Vector2 inspector_position, Vector2 inspector_size, f32 v_pos, f32 height_add, i32 *size_flags, Entity *selected) {
    // Currently making light settings only for the first light. Maybe that's the way we'll do it to the end because other lights
    // is mostly added during gameplay and in editor we want to setup only main light.
    assert(selected->lights.count > 0);
    // There we're just updating our own light - that's where that name came from.  
    Light *light_to_update = get_light(selected->lights.get_value(0));
    f32 h_pos_mult = 0.05f;
    if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & ULTRA_SMALL_LIGHT, "ultra_small_size_flag")) {
        *size_flags = ULTRA_SMALL_LIGHT;
        copy_and_add_light_to_entity(selected, light_to_update, true);
    }
    Old::make_ui_text("(64): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "ultra_small_size_flag");
    h_pos_mult += 0.15f;
    
    if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & SMALL_LIGHT, "small_size_flag")) {
        *size_flags = SMALL_LIGHT;
        copy_and_add_light_to_entity(selected, light_to_update, true);
    }
    Old::make_ui_text("(128): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "small_size_flag");
    h_pos_mult += 0.15f;
    
    if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & MEDIUM_LIGHT, "medium_light_flag")) {
        *size_flags = MEDIUM_LIGHT;
        copy_and_add_light_to_entity(selected, light_to_update, true);
    }
    Old::make_ui_text("(256): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "medium_light_flag");
    h_pos_mult += 0.15f;

    if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & BIG_LIGHT, "big_light_flag")) {
        *size_flags = BIG_LIGHT;
        copy_and_add_light_to_entity(selected, light_to_update, true);
    }
    Old::make_ui_text("(512): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "big_light_flag");
    h_pos_mult += 0.15f;

    if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & HUGE_LIGHT, "huge_light_flag")) {
        *size_flags = HUGE_LIGHT;
        copy_and_add_light_to_entity(selected, light_to_update, true);
    }
    Old::make_ui_text("(1024): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "huge_light_flag");
    h_pos_mult += 0.15f;

    if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * h_pos_mult, v_pos}, *size_flags & GIANT_LIGHT, "giant_light_flag")) {
        *size_flags = GIANT_LIGHT;
        copy_and_add_light_to_entity(selected, light_to_update, true);
    }
    Old::make_ui_text("(2048): ", {inspector_position.x + inspector_size.x * h_pos_mult, v_pos + height_add}, "giant_light_flag");
    h_pos_mult += 0.15f;
}

#ifndef INSPECTOR_MACRO
#define INSPECTOR_MACRO
    #define INSPECTOR_UI_TOGGLE_COLOR(text, tag, bool_to_change, color, additional_action) { \
        Old::make_ui_text(text, {inspector_position.x + h_pos, v_pos}, tag, color); \
        if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, bool_to_change, tag)) { \
            bool_to_change = !bool_to_change; \
            additional_action; \
        } \
        v_pos += height_add; \
    }   
    #define INSPECTOR_UI_TOGGLE(text, tag, bool_to_change, additional_action) { \
        INSPECTOR_UI_TOGGLE_COLOR(text, tag, bool_to_change, WHITE, additional_action); \
    }   
    #define INSPECTOR_UI_TOGGLE_FLAGS(text, tag, flags, flag, additional_action) { \
        Old::make_ui_text(text, {inspector_position.x + h_pos, v_pos}, tag); \
        if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, flags & flag, tag)) { \
            flags ^= flag; \
            additional_action; \
        } \
        v_pos += height_add; \
    }   
    
    #define INSPECTOR_UI_INPUT_FIELD_COLOR(text, tag, format, value_to_change, convert_function, color, additional_action) { \
        Old::make_ui_text(text, {inspector_position.x + h_pos, v_pos}, tag, color); \
        if (make_input_field(tprintf(format, value_to_change), {inspector_position.x + inspector_size.x * 0.4f, v_pos}, 100, tag)) { \
            value_to_change = convert_function(focus_input_field.content); \
            additional_action; \
        } \
        v_pos += height_add;\
    }
    #define INSPECTOR_UI_INPUT_FIELD(text, tag, format, value_to_change, convert_function, additional_action) { \
        INSPECTOR_UI_INPUT_FIELD_COLOR(text, tag, format, value_to_change, convert_function, WHITE, additional_action); \
    }
#endif

void update_editor_ui() {
    //inspector logic
    
    // move entity points hint
    if (editor.selected || editor.multiselection.entities.count > 0) {
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F3)) {
            editor.move_entity_points = !editor.move_entity_points;
        }
        Old::make_ui_text(tprintf("Ctrl+F3:\nMove entity points: %s", editor.move_entity_points ? "YES" : "NO"), {10, screen_height * 0.5f}, 30, Fade(GREEN, 0.6f), "move_entity_points_hint");
    }
    
    Entity *selected = editor.selected;
    if (selected) {
        Vector2 inspector_size = {screen_width * 0.2f, screen_height * 0.6f};
        Vector2 inspector_position = {screen_width - inspector_size.x - inspector_size.x * 0.1f, 0 + inspector_size.y * 0.05f};
        Old::make_ui_image(inspector_position, inspector_size, {0, 0}, SKYBLUE * 0.7f, "inspector_window");
        f32 height_add = 30 * UI_SCALING;
        f32 v_pos = inspector_position.y + height_add + 40;
        f32 h_pos = 5;
        
        Old::make_ui_text(tprintf("ID: %d", selected->id), {inspector_position.x + inspector_size.x * 0.4f, inspector_position.y - 10}, 18, WHITE, "inspector_id"); 
        
        Old::make_ui_text(c_str(*selected->name), {inspector_position.x, inspector_position.y + 10}, 24, BLACK, "inspector_name"); 
        Old::make_ui_text("POSITION", {inspector_position.x + inspector_size.x * 0.4f, inspector_position.y + 40}, 24, WHITE * 0.9f, "inspector_pos");
        Old::make_ui_text("X:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_pos_x");
        Old::make_ui_text("Y:", {inspector_position.x + 5 + 35 + 100, v_pos}, 22, BLACK * 0.9f, "inspector_pos_y");
        if (make_input_field(tprintf("%.3f", selected->position.x), {inspector_position.x + 30, v_pos}, {100, 25}, "inspector_pos_x")
            || make_input_field(tprintf("%.3f", selected->position.y), {inspector_position.x + 30 + 100 + 35, v_pos}, {100, 25}, "inspector_pos_y")
            ) {
            Vector2 old_position = selected->position;
            if (str_equal(focus_input_field.tag, "inspector_pos_x")) {
                selected->position.x = to_f32(focus_input_field.content);
            } else if (str_equal(focus_input_field.tag, "inspector_pos_y")) {
                selected->position.y = to_f32(focus_input_field.content);
            } else {
                assert(false);
            }
            
            undo_mark_entity_changed(editor.selected);
        }
        // v_pos += height_add;

        if (editor.multiselection.entities.count > 1) {
            Old::make_ui_text("Multiselected editing (only draw order)", {inspector_position.x + inspector_size.x * 0.05f, inspector_position.y + v_pos}, 24, RED * 0.9f, "inspector_pos");
            v_pos += height_add;
        }
        
        Old::make_ui_text("SCALE", {inspector_position.x + inspector_size.x * 0.4f, inspector_position.y + v_pos}, 24, WHITE * 0.9f, "inspector_scale");
        v_pos += height_add * 2;
        Old::make_ui_text("X:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_scale_x");
        Old::make_ui_text("Y:", {inspector_position.x + 5 + 35 + 100, v_pos}, 22, BLACK * 0.9f, "inspector_scale_y");
        if (make_input_field(tprintf("%.3f", editor.selected->scale.x), {inspector_position.x + 30, v_pos}, {100, 25}, "inspector_scale_x")
            || make_input_field(tprintf("%.3f", editor.selected->scale.y), {inspector_position.x + 30 + 100 + 35, v_pos}, {100, 25}, "inspector_scale_y")
            ) {
            Vector2 old_scale = editor.selected->scale;
            Vector2 new_scale = old_scale;
            // undo_remember_vertices_start(editor.selected);
            
            if (str_equal(focus_input_field.tag, "inspector_scale_x")) {
                new_scale.x = to_f32(focus_input_field.content);
            } else if (str_equal(focus_input_field.tag, "inspector_scale_y")) {
                new_scale.y = to_f32(focus_input_field.content);
            } else {
                assert(false);
            }
            
            Vector2 scale_add = new_scale - old_scale;
            if (scale_add != Vector2_zero) {
                add_scale(editor.selected, scale_add);
            }
            
            
            undo_mark_entity_changed(editor.selected);
        }
        v_pos += height_add;
        
        Old::make_ui_text("Rotation:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_rotation");
        if (make_input_field(tprintf("%.2f", editor.selected->rotation), {inspector_position.x + 150, v_pos}, {75, 25}, "inspector_rotation")
            ) {
            f32 old_rotation = editor.selected->rotation;
            f32 new_rotation = old_rotation;
            
//             undo_remember_vertices_start(editor.selected);
            
            if (str_equal(focus_input_field.tag, "inspector_rotation")) {
                new_rotation = to_f32(focus_input_field.content);
            } else {
                assert(false);
            }
            
            f32 rotation_add = new_rotation - old_rotation;
            if (rotation_add != 0) {
                rotate(editor.selected, rotation_add);
            }
            
            undo_mark_entity_changed(editor.selected);
        }
        v_pos += height_add;
        
        Old::make_ui_text("Draw Order:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_rotation");
        if (make_input_field(tprintf("%d", editor.selected->draw_order), {inspector_position.x + 150, v_pos}, {75, 25}, "inspector_draw_order")
            ) {
            
            if (editor.multiselection.entities.count > 1) {
                for (i32 i = 0; i < editor.multiselection.entities.count; i++) {                
                    Entity *entity = get_entity(editor.multiselection.entities.get_value(i));
                    entity->draw_order = to_i32(focus_input_field.content);
                }
            } else {
                i32 old_draw_order = editor.selected->draw_order;
                i32 new_draw_order = old_draw_order;
                
                if (str_equal(focus_input_field.tag, "inspector_draw_order")) {
                    new_draw_order = to_i32(focus_input_field.content);
                } else {
                    assert(false);
                }
                
                i32 draw_order_add = new_draw_order - old_draw_order;
                if (draw_order_add != 0) {
                    editor.selected->draw_order += draw_order_add;
                }
                
                undo_mark_entity_changed(editor.selected);
            }
        }
        v_pos += height_add;
        
        height_add = 16;// * fmax(UI_SCALING, 0.5f);
        f32 type_font_size = 24;
        f32 type_info_v_pos = type_font_size;
        
        for (f32 line_pos = v_pos; line_pos < inspector_size.y; line_pos += height_add) {
            Old::make_ui_image({inspector_position.x + 5, line_pos}, {inspector_size.x - 10, 1}, {0, 0}, Fade(ColorBrightness(SKYBLUE, 0.4f), 0.5f), "ui_line");
        }

        //entity settings overall
        if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Entity settings", "entity_settings")) {
            editor.draw_entity_settings = !editor.draw_entity_settings;
        }
        v_pos += height_add;
        
        if (editor.draw_entity_settings) {
            INSPECTOR_UI_TOGGLE("Hidden: ", "entity_hidden", selected->hidden, );
            
            INSPECTOR_UI_TOGGLE("Spawn enemy when no ammo: ", "spawn_no_ammo", selected->spawn_enemy_when_no_ammo, );
            
            INSPECTOR_UI_TOGGLE_FLAGS("No move block: ", "no_move_block", selected->flags, NO_MOVE_BLOCK, ); 
            INSPECTOR_UI_TOGGLE_FLAGS("Move sequence: ", "entity_move_sequence", selected->flags, MOVE_SEQUENCE, init_entity(selected));
            
            // move sequence inspector ui
            if (selected->flags & MOVE_SEQUENCE) {
                f32 h_pos = 15;
                INSPECTOR_UI_TOGGLE("Moving: ", "move_sequence_moving", selected->move_sequence->moving, );
                INSPECTOR_UI_TOGGLE("Loop: ", "move_sequence_loop", selected->move_sequence->loop, );
                INSPECTOR_UI_TOGGLE("Rotate: ", "move_sequence_rotate", selected->move_sequence->rotate, );
                
                INSPECTOR_UI_INPUT_FIELD("Speed: ", "move_sequence_speed", "%.1f", selected->move_sequence->speed, to_f32, );  
                
                INSPECTOR_UI_TOGGLE("Speed related player distance: : ", "move_sequence_speed_related_player_distance", selected->move_sequence->speed_related_player_distance, );
                if (selected->move_sequence->speed_related_player_distance) {
                    f32 h_pos = 25;
                    INSPECTOR_UI_INPUT_FIELD("Min distance: ", "move_sequence_min_distance", "%.1f", selected->move_sequence->min_distance, to_f32, );  
                    INSPECTOR_UI_INPUT_FIELD("Max distance: ", "move_sequence_max_distance", "%.1f", selected->move_sequence->max_distance, to_f32, );  
                    INSPECTOR_UI_INPUT_FIELD("Max distance speed: ", "move_sequence_max_distance_speed", "%.1f", selected->move_sequence->max_distance_speed, to_f32, );  
                }
                
                Old::make_ui_text(tprintf("Points count: %d", selected->move_sequence->points.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_count");
                type_info_v_pos += type_font_size;
                Old::make_ui_text("Ctrl+L clear points", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_clear");
                type_info_v_pos += type_font_size;
                Old::make_ui_text("Ctrl+M Remove point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_remove");
                type_info_v_pos += type_font_size;
                Old::make_ui_text("Ctrl+N Add point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_add_point");
                type_info_v_pos += type_font_size;
                
                Old::make_ui_text("Move sequence settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "move_sequence_settings");
                type_info_v_pos += type_font_size;
            }
            
            if (selected->flags & PROPELLER) {            
                INSPECTOR_UI_INPUT_FIELD("Propeller power: ", "propeller_power", "%.0f", selected->propeller->power, to_f32, );
                INSPECTOR_UI_TOGGLE("Spin sensitive :", "propeller_spin_sensitive", selected->propeller->spin_sensitive, );
            }
        } // entity inspector end
        
        if (selected->flags & NOTE) {
            f32 h_pos = 15;
            Note *note = current_context->notes.get(selected->note_index);
            INSPECTOR_UI_TOGGLE("NOTE draw in game: ", "note_draw_in_game", note->draw_in_game, );
        }
        
        // inspector light inspector
        if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Light settings", "light_settings")) {
            editor.draw_light_settings = !editor.draw_light_settings;
        }
        v_pos += height_add;
        
        if (editor.draw_light_settings) {
            INSPECTOR_UI_TOGGLE_FLAGS("Make light: ", "make_light", selected->flags, LIGHT, 
                if (selected->flags & LIGHT) {
                    Light empty_light = {0};
                    copy_and_add_light_to_entity(selected, &empty_light, true);                    
                } else {
                    free_lights_connected_to_entity(selected);
                }
            );
            if (selected->flags & LIGHT && selected->lights.count > 0) {
                Light *light = get_light(selected->lights.get_value(0));
                
                make_color_picker(inspector_position, inspector_size, v_pos, &light->color);
                v_pos += height_add;
                
                INSPECTOR_UI_INPUT_FIELD("Light radius: ", "light_radius", "%.2f", light->radius, to_f32, );
                INSPECTOR_UI_INPUT_FIELD("Light opacity: ", "light_opacity", "%.2f", light->opacity, to_f32, );
                INSPECTOR_UI_INPUT_FIELD("Light power: ", "light_power", "%.2f", light->power, to_f32, );
                
                // Leave this without macro for colors.
                Old::make_ui_text("Bake shadows: ", {inspector_position.x + 5, v_pos}, "light_bake_shadows", 17, ColorBrightness(light->bake_shadows ? GREEN : RED, 0.5f));
                if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, light->bake_shadows, "light_bake_shadows")) {
                    light->bake_shadows = !light->bake_shadows;
                    copy_and_add_light_to_entity(selected, light, true);
                }
                v_pos += height_add;
                
                INSPECTOR_UI_TOGGLE("Make shadows: ", "light_make_shadows", light->make_shadows, copy_and_add_light_to_entity(selected, light, true));

                if (light->make_shadows) {
                    Old::make_ui_text("Shadows Size flags: ", {inspector_position.x + 5, v_pos}, "shadows_size_flags");
                    v_pos += height_add;
                    make_light_size_picker(inspector_position, inspector_size, v_pos, height_add, &light->shadows_size_flags, selected);
                    v_pos += height_add * 2;
                }
                
                INSPECTOR_UI_TOGGLE("Make backshadows: ", "light_make_backshadows", light->make_backshadows, copy_and_add_light_to_entity(selected, light, true));

                if (light->make_backshadows) {
                    Old::make_ui_text("Backshadows Size flags: ", {inspector_position.x + 5, v_pos}, "backshadows_size_flags");
                    v_pos += height_add;
                    make_light_size_picker(inspector_position, inspector_size, v_pos, height_add, &light->backshadows_size_flags, selected);
                    v_pos += height_add * 2;
                }
            }
        }
        
        // trigger inspector
        if (selected->flags & TRIGGER) {
            if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Trigger settings", "trigger_settings")) {
                editor.draw_trigger_settings = !editor.draw_trigger_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_trigger_settings) {
                if (Old::make_button({inspector_position.x + inspector_size.x * 0.2f, v_pos + 3}, {inspector_size.x * 0.6f, height_add - 4}, "Trigger (in game)", "trigger_now_button", SKYBLUE, ColorBrightness(BROWN, -0.3f)) && editor_state == GAME) {
                    selected->trigger->debug_should_trigger_now = true;
                }
                v_pos += height_add;
            
                INSPECTOR_UI_TOGGLE_FLAGS("Activate on player: ", "trigger_player_touch",               selected->trigger->settings, PLAYER_TOUCH, );
                INSPECTOR_UI_TOGGLE_FLAGS("Die after trigger: ", "trigger_die_after_trigger",           selected->trigger->settings, DIE_AFTER_TRIGGER, );
                INSPECTOR_UI_TOGGLE_FLAGS("Kill player: ", "trigger_kill_player",                       selected->trigger->settings, KILL_PLAYER, );
                INSPECTOR_UI_TOGGLE_FLAGS("Kill enemies: ", "trigger_kill_enemies",                     selected->trigger->settings, KILL_ENEMIES, );
                INSPECTOR_UI_TOGGLE_FLAGS("Doors Open(1) Close(0): ", "trigger_open_doors",             selected->trigger->settings, OPEN_DOORS, );
                INSPECTOR_UI_TOGGLE_FLAGS("Start physics: ", "trigger_start_physics_simulation",        selected->trigger->settings, START_PHYSICS_SIMULATION, );
                INSPECTOR_UI_TOGGLE_FLAGS("Lines to tracked: ", "trigger_draw_lines_to_tracked",        selected->trigger->settings, DRAW_LINES_TO_TRACKED, );
                INSPECTOR_UI_TOGGLE_FLAGS("Agro enemies: ", "trigger_agro_enemies",                     selected->trigger->settings, AGRO_ENEMIES, );
                INSPECTOR_UI_TOGGLE_FLAGS("Show(1) Hide(0) entities: ", "trigger_shows_entities",       selected->trigger->settings, SHOWS_ENTITIES, );
                INSPECTOR_UI_TOGGLE_FLAGS("Starts moving sequence: ", "trigger_starts_moving_sequence", selected->trigger->settings, STARTS_MOVING_SEQUENCE, );
                
                Color cam_section_color = ColorBrightness(PINK, 0.4f);
                INSPECTOR_UI_TOGGLE_FLAGS("Change zoom: ", "trigger_change_zoom", selected->trigger->settings, CHANGE_ZOOM, );
                if (selected->trigger->settings & CHANGE_ZOOM) {
                    f32 h_pos = 10;
                    INSPECTOR_UI_INPUT_FIELD_COLOR("Zoom value: ", "trigger_zoom_value", "%.2f", selected->trigger->zoom_value, to_f32, ColorBrightness(cam_section_color, -0.1f), );
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Cam rails horizontal: ", "trigger_start_cam_rails_horizontal", selected->trigger->settings, START_CAM_RAILS_HORIZONTAL, init_entity(selected));
                INSPECTOR_UI_TOGGLE_FLAGS("Cam rails vertical: ", "trigger_start_cam_rails_vertical",     selected->trigger->settings, START_CAM_RAILS_VERTICAL, init_entity(selected));
                INSPECTOR_UI_TOGGLE_FLAGS("Stop cam rails: ", "trigger_stop_cam_rails",                   selected->trigger->settings, STOP_CAM_RAILS, init_entity(selected));
                
                INSPECTOR_UI_TOGGLE_FLAGS("Lock camera: ", "trigger_lock_camera", selected->trigger->settings, LOCK_CAMERA,
                    if ((selected->trigger->settings & LOCK_CAMERA) && selected->trigger->locked_camera_position == Vector2_zero) {
                        selected->trigger->locked_camera_position = selected->position;
                    }
                );
                
                INSPECTOR_UI_TOGGLE_FLAGS("Unlock camera: ", "trigger_unlock_camera",             selected->trigger->settings, UNLOCK_CAMERA, );
                
                INSPECTOR_UI_TOGGLE_FLAGS("Allow player shoot: ", "trigger_allow_player_shoot",   selected->trigger->settings, ALLOW_PLAYER_SHOOT, );
                INSPECTOR_UI_TOGGLE_FLAGS("Forbid player shoot: ", "trigger_forbid_player_shoot", selected->trigger->settings, FORBID_PLAYER_SHOOT, );
                
                INSPECTOR_UI_TOGGLE_FLAGS("Play sound: ", "trigger_play_sound",                   selected->trigger->settings, PLAY_SOUND, );
                if (selected->trigger->settings & PLAY_SOUND) {
                    Old::make_ui_text("Sound name: ", {inspector_position.x + 5, v_pos}, "trigger_play_sound_name_text");
                    if (make_input_field(selected->trigger->sound_name, {inspector_position.x + inspector_size.x * 0.4f, v_pos}, {inspector_size.x * 0.25f, 20}, "trigger_sound_name") ) {
                        str_copy(selected->trigger->sound_name, focus_input_field.content);
                    }
                    v_pos += height_add;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Load level: ", "trigger_load_level", selected->trigger->settings, LOAD_LEVEL, );
                if (selected->trigger->settings & LOAD_LEVEL) {
                    Old::make_ui_text("Level name: ", {inspector_position.x + 5, v_pos}, "trigger_load_level_name_text");
                    if (make_input_field(selected->trigger->level_name, {inspector_position.x + inspector_size.x * 0.4f, v_pos}, {inspector_size.x * 0.6f, 20}, "trigger_load_level_name") ) {
                        str_copy(selected->trigger->level_name, focus_input_field.content);
                    }
                    v_pos += height_add;
                }
                INSPECTOR_UI_TOGGLE_FLAGS("Play replay: ", "trigger_play_replay", selected->trigger->settings, PLAY_REPLAY, );
                if (selected->trigger->settings & PLAY_REPLAY) {
                    Old::make_ui_text("Replay name: ", {inspector_position.x + 5, v_pos}, "trigger_replay_name");
                    if (make_input_field(selected->trigger->replay_name, {inspector_position.x + inspector_size.x * 0.4f, v_pos}, {inspector_size.x * 0.6f, 20}, "trigger_replay_name") ) {
                        str_copy(selected->trigger->replay_name, focus_input_field.content);
                    }
                    v_pos += height_add;
                }
            }
        
            if (selected->trigger->settings & (START_CAM_RAILS_HORIZONTAL | START_CAM_RAILS_VERTICAL)) {
                Old::make_ui_text("Ctrl+L rails clear points", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "cam_rails_clear");
                type_info_v_pos += type_font_size;
                Old::make_ui_text("Ctrl+M Rails Remove point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "cam_rails_remove");
                type_info_v_pos += type_font_size;
                Old::make_ui_text("Ctrl+N Rails Add point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "cam_rails_add_point");
                type_info_v_pos += type_font_size;
                Old::make_ui_text(tprintf("Rails points count: %d", selected->trigger->cam_rails_points.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "trigger_rails_points_count");
            type_info_v_pos += type_font_size;

            }
            if (selected->trigger->settings & CHANGE_ZOOM) {
                Old::make_ui_text("Ctrl+R: Camera position", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "locked_cam_position");
                type_info_v_pos += type_font_size;
            }
            Old::make_ui_text("Clear ALL Connected: Ctrl+L", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_clear");
            type_info_v_pos += type_font_size;
            Old::make_ui_text("Remove selected: Ctrl+D", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_remove");
            type_info_v_pos += type_font_size;
            Old::make_ui_text("Assign New: Ctrl+A", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_assign");
            type_info_v_pos += type_font_size;
            Old::make_ui_text("Assign tracking enemy: Ctrl+Q", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_assign");
            type_info_v_pos += type_font_size;
            Old::make_ui_text(tprintf("Connected count: %d", selected->trigger->connected.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "trigger_connected_count");
            type_info_v_pos += type_font_size;
            Old::make_ui_text(tprintf("Tracking count: %d", selected->trigger->tracking.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "trigger_tracking_count");
            type_info_v_pos += type_font_size;
            Old::make_ui_text("Trigger settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "trigger_settings");
            type_info_v_pos += type_font_size;
        }
        
        if (selected->flags & KILL_SWITCH) {
            Old::make_ui_text("Clear ALL Connected: Ctrl+L", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "kill_switch_clear");
            type_info_v_pos += type_font_size;
            Old::make_ui_text("Remove selected: Ctrl+D", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "kill_switch_remove");
            type_info_v_pos += type_font_size;
            Old::make_ui_text("Assign New: Ctrl+A", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "kill_switch_assign");
            type_info_v_pos += type_font_size;
            Old::make_ui_text(tprintf("Connected count: %d", selected->kill_switch->connected.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, 0.2f), "kill_switch_connected_count");
            type_info_v_pos += type_font_size;
            Old::make_ui_text("Kill switch settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "kill_switch_settings");
            type_info_v_pos += type_font_size;
        }
        
        // enemy inspector
        if (selected->flags & ENEMY) {
            assert(selected->union_enemy);
        
            if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Enemy settings", "enemy_settings")) {
                editor.draw_enemy_settings = !editor.draw_enemy_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_enemy_settings) {
                INSPECTOR_UI_TOGGLE("Gives ammo: ", "enemy_gives_ammo", selected->union_enemy->gives_ammo, );
                
                INSPECTOR_UI_TOGGLE_FLAGS("Explosive: ", "enemy_explosive", selected->flags, EXPLOSIVE, 
                    if (!(selected->flags & EXPLOSIVE)) {
                        free_lights_connected_to_entity(selected);
                        
                        // We're just not allowing EXPLOSIVE to have another light, because he's light on it's own.
                        if (selected->flags & LIGHT) {
                            selected->flags ^= LIGHT;
                        }
                    }
                    init_entity(selected);
                );
                
                if (selected->flags & EXPLOSIVE) {
                    h_pos = 25;
                    INSPECTOR_UI_INPUT_FIELD("Explosion radius: ", "explosive_radius_multiplier", "%.1f", selected->union_enemy->explosive_radius_multiplier, to_f32, 
                        selected->union_enemy->explosive_radius_multiplier = fmaxf(selected->union_enemy->explosive_radius_multiplier, 0);
                        init_entity(selected);
                    );
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Blocker: ", "enemy_blocker", selected->flags, BLOCKER, init_entity(selected)); 
                
                if (selected->flags & BLOCKER) {
                    h_pos = 25;
                    INSPECTOR_UI_TOGGLE("Blocker immortal: ", "blocker_immortal", selected->union_enemy->blocker_immortal, init_entity(selected));
                    if (!selected->union_enemy->blocker_immortal) {
                        INSPECTOR_UI_TOGGLE("Blocker clockwise: ", "blocker_clockwise", selected->union_enemy->blocker_clockwise, init_entity(selected));
                    }
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Shoot blocker: ", "enemy_shoot_blocker", selected->flags, SHOOT_BLOCKER, init_entity(selected)); 
                if (selected->flags & SHOOT_BLOCKER) {
                    h_pos = 25;
                    INSPECTOR_UI_TOGGLE("Shoot blocker immortal: ", "shoot_blocker_immortal", selected->union_enemy->shoot_blocker_immortal, init_entity(selected));
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Sword size required: ", "enemy_sword_size_required", selected->flags, SWORD_SIZE_REQUIRED, init_entity(selected)); 
                if (selected->flags & SWORD_SIZE_REQUIRED) {
                    h_pos = 25;
                    INSPECTOR_UI_TOGGLE("Big (1) or small (0) killable: ", "enemy_big_or_small_killable", selected->union_enemy->big_sword_killable, init_entity(selected));
                
                    h_pos = 5;
                }
                
                INSPECTOR_UI_TOGGLE_FLAGS("Multiple hits: ", "enemy_multiple_hits", selected->flags, PLAYER_TOUCH_TIMER, init_entity(selected)); 
            }
        
            Old::make_ui_text(tprintf("Ctrl+O/P Sword kill speed: %.1f", selected->union_enemy->sword_kill_speed_modifier), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "sword_kill_speed_modifier_change");
            type_info_v_pos += type_font_size;
            
            if (selected->flags & SHOOT_BLOCKER) {
                if (!selected->union_enemy->shoot_blocker_immortal) {
                    Old::make_ui_text(tprintf("Ctrl+F/G Shoot Block Vector: {%.2f, %.2f}", selected->union_enemy->shoot_blocker_direction.x, selected->union_enemy->shoot_blocker_direction.y), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "shoot_blocker_direction");
                    type_info_v_pos += type_font_size;
                }
            }

            Old::make_ui_text("Enemy settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "enemy_settings");
            type_info_v_pos += type_font_size;
        } // enemy inspector end
        
        if (selected->flags & PROPELLER) {
            Old::make_ui_text(tprintf("Ctrl+Q/E Power change: %.0f", selected->propeller->power), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "propeller_power");
            type_info_v_pos += type_font_size;
            
            Old::make_ui_text("Propeller settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "propeller_settings");
            type_info_v_pos += type_font_size;

        }
        
        if (selected->flags & CENTIPEDE) {
            if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Centipede settings", "centipede_settings")) {
                editor.draw_centipede_settings = !editor.draw_centipede_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_centipede_settings) {
                INSPECTOR_UI_TOGGLE("Spikes on right: ", "spikes_on_right", selected->centipede->spikes_on_right, );
                INSPECTOR_UI_TOGGLE("Spikes on left: ", "spikes_on_left", selected->centipede->spikes_on_left, );

                INSPECTOR_UI_INPUT_FIELD("Segments count:", "segments_to_spawn", "%d", selected->centipede->segments_to_spawn, to_i32,
                    selected->centipede->segments_to_spawn = fminf(selected->centipede->segments_to_spawn, 128);
                );
            }
        }
        
        // jumps shooter inspector
        if (selected->flags & JUMP_SHOOTER) {
            if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Jump shooter settings", "jump_shooter_settings")) {
                editor.draw_jump_shooter_settings = !editor.draw_jump_shooter_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_jump_shooter_settings) {
                INSPECTOR_UI_INPUT_FIELD("Shots count:", "jump_shooter_shots_count", "%d", selected->jump_shooter->shots_count, to_i32, );
                INSPECTOR_UI_INPUT_FIELD("Spread:", "jump_shooter_spread", "%.1f", selected->jump_shooter->spread, to_f32,
                    selected->jump_shooter->spread = clamp(selected->jump_shooter->spread, 0.0f, 180.0f);
                );   
                
                INSPECTOR_UI_INPUT_FIELD("Explosive count:", "jump_shooter_explosive_count", "%d", selected->jump_shooter->explosive_count, to_i32, 
                    selected->jump_shooter->explosive_count = fmin(fmin(selected->jump_shooter->explosive_count, 64), selected->jump_shooter->shots_count);
                );

                INSPECTOR_UI_TOGGLE("Shoot sword blockers: ", "shoot_sword_blockers", selected->jump_shooter->shoot_sword_blockers, );
                
                if (selected->jump_shooter->shoot_sword_blockers) {
                    h_pos = 15;
                    INSPECTOR_UI_TOGGLE("Sword blockers immortal: ", "shoot_sword_blockers_immortal", selected->jump_shooter->shoot_sword_blockers_immortal, );
                    h_pos = 5;
                }
                
                Old::make_ui_text("Shoot bullet blockers: ", {inspector_position.x + 5, v_pos}, "shoot_bullet_blockers");
                if (Old::make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->jump_shooter->shoot_bullet_blockers, "shoot_bullet_blockers")) {
                    selected->jump_shooter->shoot_bullet_blockers = !selected->jump_shooter->shoot_bullet_blockers;
                }
                v_pos += height_add;
            }
        }
        
        // inspector turret inspector
        if (selected->flags & TURRET) {
            if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Turret settings", "turret_settings")) {
                editor.draw_turret_settings = !editor.draw_turret_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_turret_settings) {
                Turret *turret = selected->turret;
                
                INSPECTOR_UI_TOGGLE_FLAGS("Shoot blockers: ", "turret_shoot_blockers", turret->projectile_settings.enemy_flags, BLOCKER, );
                
                if (turret->projectile_settings.enemy_flags & BLOCKER) {
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
                if (turret->homing) {
                    INSPECTOR_UI_INPUT_FIELD("Shoot width: ", "turret_shoot_width", "%.0f", turret->shoot_width, to_f32, );
                    INSPECTOR_UI_INPUT_FIELD("Shoot height: ", "turret_shoot_height", "%.0f", turret->shoot_height, to_f32, );
                }
            }
        }

        if (selected->flags & DOOR) {
            if (Old::make_button({inspector_position.x + inspector_size.x * 0.05f, v_pos}, {inspector_size.x * 0.9f, height_add}, "Door settings", "door_settings")) {
                editor.draw_door_settings = !editor.draw_door_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_door_settings) {
                INSPECTOR_UI_TOGGLE("Open: ", "door_open_closed", selected->door.is_open, );
            }
        
            Old::make_ui_text(tprintf("Ctrl+T Trigger: %s", selected->door.is_open ? "Open" : "Close"), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "door_trigger");
            type_info_v_pos += type_font_size;
            
            Old::make_ui_text("Door settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "door_settings");
            type_info_v_pos += type_font_size;
        }
        
        //type info background
        Old::make_ui_image({inspector_position.x - 160, (f32)screen_height - type_info_v_pos}, {(f32)screen_width * 0.5f, type_info_v_pos}, {0, 0}, SKYBLUE * 0.7f, "inspector_type_info_background");
    }
    
    // Create box.
    b32 writing_other_input_field = focus_input_field.in_focus && !editor.create_box_active;
    b32 can_control_create_box = !console.is_open && !writing_other_input_field;
    b32 need_close_create_box = false;
    
    if (can_control_create_box && IsKeyPressed(KEY_SPACE) && editor.in_editor_time > 0.05f) {
        if (editor.create_box_active && !editor.create_box_closing) {
            need_close_create_box = true;
        } else { //open create box
            editor.create_box_open_mouse_position = input.mouse_position;
            
            editor.create_box_scrolled = 0;
            editor.create_box_active = true;
            editor.create_box_closing = false;
            editor.create_box_lifetime = 0;
            make_next_input_field_in_focus("create_box");
            assign_selected_entity(NULL);
        }
    }
    
    if (can_control_create_box && IsKeyPressed(KEY_ESCAPE)) {
        if (editor.create_box_active) {
            need_close_create_box = true;
        } else if (editor.selected) {
            assign_selected_entity(NULL);
        }
    }
    
    if (can_control_create_box && editor.create_box_active) {
        if (IsKeyPressed(KEY_DOWN)) {
            editor.create_box_selected_index++;
            if (editor.create_box_selected_index < 0) {
                editor.create_box_selected_index = 0;
            }
        }
        if (IsKeyPressed(KEY_UP)) {
            editor.create_box_selected_index--;
            if (editor.create_box_selected_index < 0) {
                editor.create_box_selected_index = 0;
            }
        }
    
        Vector2 field_size = {600, 50};
        Vector2 field_target_position = {screen_width * 0.5f - field_size.x * 0.5f, 100};
        Vector2 field_start_position = field_target_position - Vector2_up * field_size.y * 6;
        
        if (editor.create_box_closing) {
            editor.create_box_lifetime -= core.time.real_dt;
            if (editor.create_box_lifetime <= 0) {
                need_close_create_box = true;
            }
        } else {
            editor.create_box_lifetime += core.time.real_dt;
        }
        
        f32 create_t = clamp01(editor.create_box_lifetime / editor.create_box_slide_time);
        
        Vector2 field_position = lerp(field_start_position, field_target_position, EaseOutBack(create_t));
        
        i32 input_len = str_len(focus_input_field.content);
        i32 fitting_count = 0;
        f32 alpha_multiplier = lerp(0.0f, 1.0f, clamp01(create_t * 2));
        
        for (i32 i = 0; i < spawn_objects.count; i++) {
            Spawn_Object obj = spawn_objects.get_value(i);
            if (input_len > 0 && !str_contains(obj.name, focus_input_field.content)) {
                continue;
            }
            
            editor.create_box_scrolled += GetMouseWheelMove();
            Vector2 obj_position = field_position + Vector2_up * field_size.y * (fitting_count + 1) + Vector2_up * editor.create_box_scrolled + Vector2_right * field_size.x * 0.2f;
            Vector2 obj_size = {field_size.x * 0.6f, field_size.y};
            
            b32 this_object_selected = editor.create_box_selected_index == fitting_count;
            
            Color button_color = lerp(BLACK * 0, BLACK * 0.9f, clamp01(create_t * 2));
            Color text_color   = lerp(WHITE * 0, WHITE * 0.9f, clamp01(create_t * 2));
            
            if (Old::make_button(obj_position, obj_size, {0, 0}, obj.name, 24, "create_box", button_color, text_color) || (this_object_selected && IsKeyPressed(KEY_ENTER))) {
                obj.entity.position = editor.create_box_open_mouse_position; // So that on init_entity it has real position.
                Entity *entity = copy_and_add_entity(&obj.entity, current_context);
                need_close_create_box = true;
                
                editor.just_spawned_ids.append(entity->id);
            }
            
            if (obj.entity.flags & TEXTURE) {
                Vector2 texture_position = obj_position - Vector2_right * field_size.y;
                Old::make_ui_image(obj.entity.texture, texture_position, {field_size.y, field_size.y}, {0, 0}, Fade(WHITE, alpha_multiplier), "create_box_obj_texture");
            }
            
            if (this_object_selected) {
                f32 color_multiplier = lerp(0.7f, 0.9f, (sinf(core.time.app_time * 3) + 1) * 0.5f);
                Color selected_color = lerp(WHITE * 0, WHITE * color_multiplier, clamp01(create_t * 2));
                Old::make_ui_image(obj_position, {obj_size.x * 0.2f, obj_size.y}, {1, 0}, selected_color, "create_box");
            }
            
            fitting_count++;
        }
        
        if (fitting_count > 0 && editor.create_box_selected_index > fitting_count - 1) {
            editor.create_box_selected_index = fitting_count - 1;   
        }
    
        if (make_input_field("", field_position, field_size, "create_box", GRAY, false)) {
            need_close_create_box = true;
        }
    }
    
    if (need_close_create_box) {
        if (editor.create_box_closing) {
            close_create_box();
        } else {
            start_closing_create_box();
        }
    }
} //editor ui end

Entity *get_cursor_entity() {
    Entity *cursor_entity_candidate = NULL;
    
    b32 mouse_on_selected_entity = editor.selected && check_entities_collision(&mouse_entity, editor.selected).collided;
    
    fill_collisions(&mouse_entity, &collisions_buffer, 0);
    
    for (i32 i = 0; i < collisions_buffer.count; i++) {
        Entity *e = collisions_buffer.get_value(i).other_entity;
        if (editor.last_click_position == input.mouse_position) {
            //If we long enough on one entity we assume that we want to pick it up and not to cycle
            f32 time_since_last_click = core.time.app_time - editor.last_click_time;
            if (time_since_last_click >= 0.5f && mouse_on_selected_entity) {
                if (e->id == editor.selected->id) {
                    cursor_entity_candidate = e;
                }
            } else if (!cursor_entity_candidate && editor.selected && e->id != editor.selected->id) {
                cursor_entity_candidate = e;
            } else if (!editor.place_cursor_entities.contains(e)) {
                cursor_entity_candidate = e;
            }
        } else { //Mouse moved from last click
            //If mouse moved we always want cursor entity to be a selected entity
            if (mouse_on_selected_entity) {
                if (e->id == editor.selected->id) {
                    cursor_entity_candidate = e;
                }
            } else {
                b32 other_entity_preferable = (e->flags & GROUND || e->flags & PLATFORM || e->flags & ENEMY || e->flags & PROPELLER || e->flags & TRIGGER || e->flags & DUMMY || e->flags & (CENTIPEDE | CENTIPEDE_SEGMENT));
                //We want to pick most valuable entity for selection and not background if there's something more around
                if (!cursor_entity_candidate || other_entity_preferable) {
                    cursor_entity_candidate = e;
                }
            }
            editor.place_cursor_entities.clear();
        }
    }        
    
    return cursor_entity_candidate;
}

Entity *editor_spawn_entity(const char *name, Vector2 position) {
    Entity *entity = spawn_object_by_name(name, round_to_factor(input.mouse_position, 5), current_context);
    
    if (entity) {
        editor.just_spawned_ids.append(entity->id);
    }
    
    return entity;
}

b32 snap_vertex_to_closest(Entity *entity, Vector2 *entity_vertex, i32 vertex_index) { 
    if (!editor.selected) {
        return false;
    }

    Vector2 closest_vertex_global = Vector2_zero;
    f32 distance_to_closest_vertex = INFINITY;

    // for (i32 i = 0; i < current_context->entities.capacity; i++) {        
    for_chunk_array(i, (&current_context->entities)) {
        Entity *e = current_context->entities.get(i);
        
        if (!e->enabled) {
            continue;
        }

        for (i32 v = 0; v < e->vertices.count; v++) {
            Vector2 *vertex = e->vertices.get(v);
            
            Vector2 vertex_global = global_position(e, *vertex);
                
            if (e->id != editor.selected->id) {
                f32 sqr_distance = sqr_magnitude(global_position(editor.selected, *entity_vertex) - vertex_global);
                if (sqr_distance < distance_to_closest_vertex) {
                    distance_to_closest_vertex = sqr_distance;
                    closest_vertex_global = vertex_global;
                }
            }
        }
    }
    
    Vector2 new_position = closest_vertex_global - *entity_vertex;
    Vector2 position_change = new_position - entity->position;
    entity->position = new_position;     
    if (position_change != Vector2_zero) undo_mark_entity_changed(entity);
    
    // // Because when we start moving vertex we remembering these vertices already. 
    // // So if we do that here aswell - on undo vertices will go on place where we pressed button.
    // // Really need to change undo system though.
    // if (!editor.moving_vertex_entity) {
//     //     undo_remember_vertices_start(editor.selected);
    // }
    // move_vertex(editor.selected, closest_vertex_global, vertex_index);

//     // undo_add_vertices_change(editor.selected);
    
    return true;
}

inline b32 is_vertex_on_mouse(Vector2 vertex_global) {
    return check_col_circles({input.mouse_position, 1}, {vertex_global, 0.5f * (0.4f / current_context->cam.cam2D.zoom)});
}

void editor_move_entity_points(Entity *entity, Vector2 displacement) {
    if (entity->flags & MOVE_SEQUENCE) {
        for (i32 i = 0; i < entity->move_sequence->points.count; i++) {
            *entity->move_sequence->points.get(i) += displacement;
        }
    }
    if (entity->flags & TRIGGER) {
        for (i32 i = 0; i < entity->trigger->cam_rails_points.count; i++) {
            *entity->trigger->cam_rails_points.get(i) += displacement;
        }
    }
}

inline Vector2 get_editor_mouse_move() {
    f32 zoom = current_context->cam.cam2D.zoom;
    return cast(Vector2) {input.mouse_delta.x / zoom, -input.mouse_delta.y / zoom} / (current_context->cam.unit_size);
}

inline f32 round_to_factor(f32 number, f32 quantization_factor) {
    return roundf(number / quantization_factor) * quantization_factor;
}
inline Vector2 round_to_factor(Vector2 vec, f32 quantization_factor) {
    return {round_to_factor(vec.x, quantization_factor), round_to_factor(vec.y, quantization_factor)};
}

void editor_mouse_move_entity(Entity *entity) {
    Vector2 move_delta = get_editor_mouse_move();
    
    b32 moving_without_cell_bound = IsKeyDown(KEY_LEFT_ALT);
    if (!moving_without_cell_bound) {
        move_delta = input.mouse_position - (entity->position + editor.dragging_start_mouse_offset);
    }
    
    if (moving_without_cell_bound) {
        entity->position += move_delta;
    } else if (sqr_magnitude(move_delta) >= (MOVE_CELL_SIZE * 0.5f * MOVE_CELL_SIZE * 0.5f)) {
        Vector2 next_position = entity->position + move_delta;
        Vector2 cell_position = {round_to_factor(next_position.x, MOVE_CELL_SIZE), round_to_factor(next_position.y, MOVE_CELL_SIZE)};
        move_delta = cell_position - entity->position;
        entity->position += move_delta;
    } else {
        move_delta = Vector2_zero;
    }
    
    if (editor.move_entity_points) {
        editor_move_entity_points(entity, move_delta);
    }
}

inline void add_to_multiselection(i32 id) {
    if (!editor.multiselection.entities.contains(id)) {
        Entity *entity = get_entity(id);
        if (entity->runtime_only_flags & SHOULD_NOT_COPY) return;
    
        editor.multiselection.entities.append(id);
        
        editor.multiselection.unchanged_copies.append(copy_and_add_entity(get_entity(id), &undo_context));
    }
}

void add_to_multiselection(Array <i32> *ids) {
    for (i32 i = 0 ; i < ids->count; i++) {
        i32 id = ids->get_value(i);
                
        add_to_multiselection(id);
    }
}

void remove_id_from_multiselection(i32 id) {
    i32 index = editor.multiselection.entities.find(id);
    if (index >= 0) {
        editor.multiselection.entities.remove(index);
        
        Entity *unchanged_copy = editor.multiselection.unchanged_copies.get_value(index);
        free_entity(unchanged_copy);
        editor.multiselection.unchanged_copies.remove(index);
    }
}

void remove_ids_from_multiselection(Array <i32> *ids) {
    // Backwards because if we'll got multiselection.entities array itself - things will broke if go beginning to end.
    for (i32 i = ids->count - 1; i >= 0; i--) {
        i32 id = ids->get_value(i);
        remove_id_from_multiselection(id);
    }
}

inline void clear_multiselected_entities() {
    remove_ids_from_multiselection(&editor.multiselection.entities);
}

b32 clicked_on_entity_edge(f32 rotation, Vector2 edge_center, b32 is_horizontal, f32 orthogonal_size, f32 radius_multiplier) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return false;
    }

    Static_Array <Vector2, MAX_VERTICES> edge_vertices = Static_Array <Vector2, MAX_VERTICES>();
    add_rect_vertices(&edge_vertices, {0.5f, 0.5f});
    f32 selection_radius = fmaxf(3.0f, 1.5f / current_context->cam.cam2D.zoom) * radius_multiplier;
    for (i32 i = 0; i < edge_vertices.count; i++) {
        if (is_horizontal) {
            edge_vertices.get(i)->x *= selection_radius;
            edge_vertices.get(i)->y *= orthogonal_size;
        } else {
            edge_vertices.get(i)->y *= selection_radius;
            edge_vertices.get(i)->x *= orthogonal_size;
        }
        rotate_around_point(edge_vertices.get(i), Vector2_zero, rotation);
    }
    
    b32 clicked_edge = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) 
                        && check_collision(edge_center, mouse_entity.position, edge_vertices, mouse_entity.vertices, {0.5f, 0.5f}, mouse_entity.pivot).collided;
    return clicked_edge;
}

void try_move_entity_edges(Entity *e) {
    if (editor.moving_entity_edge_type == NONE && !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }
    
    if (editor.moving_entity_edge_type != NONE && editor.moving_entity_edge_id != e->id) {
        return;
    }
    
    // These settings for left edge just for example. We will set it again on switch anyway.
    Vector2 scale_side = Vector2_right * -1;
    Vector2 position_change_direction = e->right * -1;
    Vector2 edge_center = e->position + position_change_direction * e->scale.x * e->pivot.x;
    f32 scale_modifier = 1;
    
    if (editor.moving_entity_edge_type == NONE) {
        if (0) {
        } else if (clicked_on_entity_edge(e->rotation, e->position + e->right * e->scale.x * e->pivot.x, true, e->scale.y, 0.1f)) {
            editor.moving_entity_edge_type = RIGHT_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position - e->right * e->scale.x * e->pivot.x, true, e->scale.y, 0.1f)) {
            editor.moving_entity_edge_type = LEFT_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position + e->up * e->scale.y * e->pivot.y, false, e->scale.x, 0.1f)) {
            editor.moving_entity_edge_type = TOP_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position - e->up * e->scale.y * e->pivot.y, false, e->scale.x, 0.1f)) {
            editor.moving_entity_edge_type = BOTTOM_EDGE;
        // Two pass for that we detect precisely which edge was clicked. First more precise.
        // That needs in situations where two edges really clear and radius are too big so we detect right-left first.
        } else if (clicked_on_entity_edge(e->rotation, e->position + e->right * e->scale.x * e->pivot.x, true, e->scale.y, 1)) {
            editor.moving_entity_edge_type = RIGHT_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position - e->right * e->scale.x * e->pivot.x, true, e->scale.y, 1)) {
            editor.moving_entity_edge_type = LEFT_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position + e->up * e->scale.y * e->pivot.y, false, e->scale.x, 1)) {
            editor.moving_entity_edge_type = TOP_EDGE;
        } else if (clicked_on_entity_edge(e->rotation, e->position - e->up * e->scale.y * e->pivot.y, false, e->scale.x, 1)) {
            editor.moving_entity_edge_type = BOTTOM_EDGE;
        } else {
            return;
        }
        
        if (editor.moving_entity_edge_type != NONE) {
            // So we really clicked on edge this frame and we should remember vertices because I am just writed retarded 
            // undo system long ago and this shit needs full rewrite.
//             undo_remember_vertices_start(e);
            editor.moving_edge_start_entity_position = e->position;
            editor.moving_edge_start_entity_scale = e->scale;
        }
    }
    
    switch (editor.moving_entity_edge_type) {
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
    while (abs(edge_mouse_dot) >= scale_amount * 0.75f) {
    
        Vector2 next_scale = round_to_factor(e->scale + scale_side * scale_modifier * normalized(edge_mouse_dot) * scale_amount, scale_amount);
        
        if (next_scale.x >= scale_amount && next_scale.y >= scale_amount) {
            Vector2 position_change = position_change_direction * normalized(edge_mouse_dot) * scale_amount * 0.5f;
            change_scale(e, next_scale);
            e->position += position_change;
        }
        
        edge_mouse_dot -= scale_amount * normalized(edge_mouse_dot);
    }
}

void rotate_multiselected(f32 to_rotate) {
    for (i32 i = 0; i < editor.multiselection.entities.count; i++) {                
        Entity *entity = get_entity(editor.multiselection.entities.get_value(i));
        
        f32 next_rotation = round_to_factor(entity->rotation + to_rotate, 15);
        to_rotate = next_rotation - entity->rotation;
        
        rotate(entity, to_rotate);
        
        Vector2 before_position = entity->position;
        rotate_around_point(&entity->position, editor.multiselection.center, to_rotate);
        undo_mark_entity_changed(entity);
    }
}

b32 is_action_queued(Repeat_Action *repeat_data, b32 pressed, b32 hold) {
    b32 result = false;
    
    if (pressed) {
        result = true;
        repeat_data->action_time = core.time.app_time;
        repeat_data->hold_time = 0;
        repeat_data->pressed_in_beginning = true;
    } else if (hold && repeat_data->pressed_in_beginning) {
        f32 since_press = core.time.app_time - repeat_data->action_time;
        repeat_data->hold_time += core.time.real_dt;
        
        f32 repeat_delay = repeat_data->start_repeat_action_delay;
        if (repeat_data->hold_time > 1.5f && repeat_data->should_sped_up) {
            repeat_delay = repeat_data->sped_up_repeat_action_delay;
        }
        
        if (!repeat_data->repeating && since_press > repeat_data->hold_time_to_action) {
            repeat_data->repeating = true;
            result = true;
            repeat_data->action_time = core.time.app_time;
        } else if (repeat_data->repeating && since_press > repeat_delay) {
            result = true;
            repeat_data->action_time = core.time.app_time;
        }
    } else {
        repeat_data->hold_time = 0;
        repeat_data->repeating = false;
        repeat_data->pressed_in_beginning = false;
    }
    
    return result;
}

// This can be called not only when editor_state is EDITOR, but even when we're in pause for example.
void update_editor() {
    if (editor.selected && editor.selected->will_be_destroyed) {
        assign_selected_entity(NULL);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        EnableCursor();
        ShowCursor();
        SetMousePosition(input.screen_mouse_position.x, input.screen_mouse_position.y);
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        HideCursor();
        DisableCursor();
    }
    
    Multiselection *multiselection = &editor.multiselection;

    // levels switching context stitch
    if (editor_state == EDITOR) {
        if (IsKeyPressed(KEY_ONE) && IsKeyDown(KEY_LEFT_CONTROL) && !console.is_open) {
            current_editor_context_index += 1;    
            current_editor_context_index %= MAX_LOADED_LEVELS;    
            
            Context *next_context = &loaded_editor_contexts[current_editor_context_index];
            // i32 cycled = 0;
            // while (cycled <= MAX_LOADED_LEVELS && next_context->level_name.count == 0) {
            //     cycled += 1;
            //     current_editor_context_index += 1;    
            //     current_editor_context_index %= MAX_LOADED_LEVELS;    
            //     next_context = &loaded_editor_contexts[current_editor_context_index];
            // }
            
            if (next_context->level_name.count > 0) {
                editor_context = next_context;
                switch_current_context(editor_context, true);
            }
            
            // setup_context_cam(current_context);
        }
    
        // We need grid to be at camera center because levels could be quite big and even our mouse collision detection does not work
        // without grid at that place. 
        // BUT i've recently (08.03.2025 currently) made that origin is on player spawn point in editor. Don't remember why. 
        // There's could be other scary reason.
        // Vector2 grid_target_pos = current_context->player_spawn_point;
        
        // We're changing grid origin in editor just because right now we have static size grid and we want to be able to 
        // click on entities that far away from {zero, zero}.
        //
        // NOTE that we can do this in editor beacuse we're recalculating static entities collision cells every frame, whereas
        // in game mode we're not doing that and grid origin in game mode should stay at where we've put it in beginning.
        Vector2 grid_target_pos = current_context->cam.position;
        current_context->collision_grid.origin = {(f32)((i32)grid_target_pos.x - ((i32)grid_target_pos.x % (i32)current_context->collision_grid.cell_size.x)), (f32)((i32)grid_target_pos.y - ((i32)grid_target_pos.y % (i32)current_context->collision_grid.cell_size.y))};
    }
    // Undo_Action undo_action;
    // b32 something_in_undo = false;
    b32 can_control_with_single_button = !focus_input_field.in_focus && !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT);
    b32 can_select = !clicked_ui;
    
    f32 dt = core.time.real_dt;
    
    editor.in_editor_time += dt;

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_L)) {
        editor.update_cam_view_position = !editor.update_cam_view_position;
    }
    
    b32 moving_editor_cam = false;
    
    f32 zoom = current_context->cam.target_zoom;

    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        moving_editor_cam = true;
    }
    
    // b32 need_move_vertices = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select;
    // b32 need_snap_vertex = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_V);
    
    i32 selected_vertex_index;
    
    i32 cursor_entities_count = 0;
    
    Entity *moving_vertex_entity_candidate = NULL;
    i32 moving_vertex_candidate = -1;
    
    // Spawn shortcuts
    if (IsKeyDown(KEY_LEFT_ALT)) {
        if (IsKeyPressed(KEY_ONE)) {
            editor_spawn_entity("block_base", input.mouse_position);
        }
        if (IsKeyPressed(KEY_TWO)) {
            editor_spawn_entity("dummy_entity", input.mouse_position);
        }
        if (IsKeyPressed(KEY_THREE)) {
            editor_spawn_entity("ammo_pack", input.mouse_position);
        }
        if (IsKeyPressed(KEY_FOUR)) {
            editor_spawn_entity("enemy_bird", input.mouse_position);
        }
        if (IsKeyPressed(KEY_FIVE)) {
            assign_selected_entity(editor_spawn_entity("note", input.mouse_position));
            make_next_input_field_in_focus("note");
        }
    }
    
    update_entities(current_context, core.time.real_dt);
    
    // Editor entities loop.
    for_chunk_array(i, (&current_context->entities)) {
        Entity *e = current_context->entities.get(i);
        
        if (!e->enabled) {
            continue;
        }
        
        if ((check_entities_collision(&mouse_entity, e)).collided) {
            cursor_entities_count++;
        }
        
        // //editor vertices
        // for (i32 v = 0; v < e->vertices.count && need_move_vertices; v++) {
        //     Vector2 *vertex = e->vertices.get(v);
            
        //     Vector2 vertex_global = global_position(e, *vertex);
            
        //     if (need_move_vertices && (!moving_vertex_entity_candidate || (editor.selected && e->id == editor.selected->id))) {
        //         if (is_vertex_on_mouse(vertex_global)) {
        //             moving_vertex_entity_candidate = e;
        //             moving_vertex_candidate = v;
        //         }
        //     }
        // }
        
        b32 maybe_want_to_move_edges = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if (maybe_want_to_move_edges && !editor.is_scaling_entity && editor.selected && editor.selected->id == e->id) {
            try_move_entity_edges(e);
        } else if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) { 
            // Check for mouse button not released this frame is for that we keep dragging edge state for one frame
            // and could consider that in editor code. (Was added so that releasing button not de-selected entity on dragging code).
            // This else is for clearing moving edge state after we finished and also for remembering undo.
            if (editor.moving_entity_edge_type != NONE && e->id == editor.moving_entity_edge_id) {
                if (e->scale != editor.moving_edge_start_entity_scale) {
                    undo_mark_entity_changed(e);
                }
                editor.moving_entity_edge_type = NONE;
                editor.moving_entity_edge_id = -1;
            }
        }
        
        //editor move sequence points        
        // We don't want to move points if selected entity already is move sequence or if selected is trigger with cam rails.
        b32 cannot_move_points = editor.selected && ((editor.selected->flags & MOVE_SEQUENCE || (editor.selected->flags & TRIGGER && editor.selected->trigger->cam_rails_points.count > 0)) && editor.selected->id != e->id);
        for (i32 p = 0; e->flags & MOVE_SEQUENCE && IsKeyDown(KEY_LEFT_ALT) && p < e->move_sequence->points.count && !cannot_move_points; p++) {
            Vector2 *point = e->move_sequence->points.get(p);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && check_col_circles({input.mouse_position, 1}, {*point, 0.5f / current_context->cam.cam2D.zoom})) {
                *point = input.mouse_position;
            }
        }
        
        //editor move cam rails points        
        for (i32 p = 0; e->flags & TRIGGER && (e->trigger->settings & (START_CAM_RAILS_HORIZONTAL | START_CAM_RAILS_VERTICAL)) && IsKeyDown(KEY_LEFT_ALT) && p < e->trigger->cam_rails_points.count && !cannot_move_points; p++) {
            Vector2 *point = e->trigger->cam_rails_points.get(p);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && check_col_circles({input.mouse_position, 1}, {*point, 0.5f / current_context->cam.cam2D.zoom})) {
                *point = input.mouse_position;
            }
        }
    } // End editor entities loop.
    
    //assign move vertex
    // if (need_move_vertices && moving_vertex_entity_candidate) {
    //     assign_moving_vertex_entity (moving_vertex_entity_candidate, moving_vertex_candidate);
//     //     undo_remember_vertices_start(moving_vertex_entity_candidate);
    // }
    
    // if (need_snap_vertex && editor.moving_vertex && editor.moving_vertex_entity) {
    //     snap_vertex_to_closest(editor.moving_vertex_entity, editor.moving_vertex, editor.moving_vertex_index);
        
    //     editor.moving_vertex = NULL;
    //     editor.moving_vertex_entity = NULL;
    // }
    
    if (editor.selected && IsKeyDown(KEY_LEFT_ALT)) {
        i32 vertex_snap_index = -1;
        if (IsKeyPressed(KEY_T))   vertex_snap_index = 0;
        if (IsKeyPressed(KEY_Y))   vertex_snap_index = 1;
        if (IsKeyPressed(KEY_F)) vertex_snap_index = 2;
        if (IsKeyPressed(KEY_G))  vertex_snap_index = 3;
        
        if (vertex_snap_index != -1 && vertex_snap_index < editor.selected->vertices.count) {
            Vector2 *vertex = editor.selected->vertices.get(vertex_snap_index);
            snap_vertex_to_closest(editor.selected, vertex, vertex_snap_index);
        }
    }
    
    //This means we clicked all entities in one mouse position, so we want to cycle
    if (cursor_entities_count <= editor.place_cursor_entities.count) {
        editor.place_cursor_entities.clear();
    }
    
    editor.cursor_entity = get_cursor_entity();
    
    if (editor.cursor_entity) {
        // editor.cursor_entity->color_changer.frame_changing = true;    
    }
    
    b32 need_start_dragging = false;
    
    // mouse select editor
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select) {
        if (editor.cursor_entity) { //select entity
            b32 is_same_selected_entity = editor.selected != NULL && editor.selected->id == editor.cursor_entity->id;
            need_start_dragging = is_same_selected_entity;
            if (!is_same_selected_entity) {
                // multiselect exclude multiselect remove
                b32 removed = false;
                if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    if (multiselection->entities.contains(editor.cursor_entity->id)) {
                        remove_id_from_multiselection(editor.cursor_entity->id);
                        removed = true;
                    } else {
                        add_to_multiselection(editor.cursor_entity->id);
                        
                        if (editor.selected && !multiselection->entities.contains(editor.selected->id)) {
                            add_to_multiselection(editor.selected->id);
                        }
                        
                        // So if it was first multiselected and we do not have selected - wanna make first one selected.
                        if (multiselection->entities.count == 1) {
                            assign_selected_entity(editor.cursor_entity);
                        }
                    }
                } else {
                    assign_selected_entity(editor.cursor_entity);
                    clear_multiselected_entities();
                }
                
                if (!removed) {
                    editor.place_cursor_entities.append(editor.selected); // @CLEANUP: Do not know what is place_cursor_entities. Maybe we should make it just temp array.
                    
                    editor.selected_this_click = true;
                }
            }
        }
    } 
    
    // multiselect
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        multiselection->excluding = true;     
        multiselection->selection_entities.clear();
        multiselection->start_point = input.mouse_position;
    } else if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        multiselection->selection_entities.clear();
        
        if (!multiselection->selecting) {
            clear_multiselected_entities();
        }
        multiselection->selecting = false;
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && multiselection->excluding) {
        multiselection->excluding = false;
        
        remove_ids_from_multiselection(&multiselection->selection_entities);
        
        multiselection->selection_entities.clear();
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        multiselection->selecting = true;       
        multiselection->start_point = input.mouse_position;
        multiselection->selection_entities.clear();
    }
    
    if ((multiselection->selecting || multiselection->excluding) && sqr_magnitude(input.mouse_position - multiselection->start_point) > 1) {
        multiselection->selection_entities.clear();
    
        Vector2 pivot = Vector2_zero;    
        if (input.mouse_position.x >= multiselection->start_point.x) pivot.x = 0;
        else pivot.x = 1;
        if (input.mouse_position.y >= multiselection->start_point.y) pivot.y = 1;
        else pivot.y = 0;
        
        Vector2 scale = {abs(input.mouse_position.x - multiselection->start_point.x), abs(input.mouse_position.y - multiselection->start_point.y)};
        fill_collisions_rect(multiselection->start_point, scale, pivot, &collisions_buffer, 0);
        
        for (i32 i = 0; i < collisions_buffer.count; i++) {
            Entity *other = collisions_buffer.get(i)->other_entity;
            if (multiselection->selection_entities.contains(other->id)) {
                continue;
            }
            
            multiselection->selection_entities.append(other->id);
            
            if (!multiselection->excluding) {
                other->color_changer.frame_changing = true;
                make_rect_lines(other->position + other->bounds.offset, other->bounds.size, other->pivot, 2.0f / current_context->cam.cam2D.zoom, BLUE); 
            }
        }
        
        Color color = multiselection->excluding ? RED : BLUE;
        make_rect_lines(multiselection->start_point, scale, pivot, 2.0f / (current_context->cam.cam2D.zoom), color);
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && multiselection->selecting) {
        multiselection->selecting = false;
        
        add_to_multiselection(&multiselection->selection_entities);
    }
    
    // Update multiselected.
    if (multiselection->entities.count > 0) {
        for_array_backwards (i, &multiselection->entities) {
            i32 id = multiselection->entities.get_value(i);
            if (get_entity(id)->will_be_destroyed) {
                remove_id_from_multiselection(multiselection->entities.get_value(i));
            }
        }
    
        local_persist b32 was_moving_multiselected = false;
        b32 should_move_multiselected = IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL);
        
        if (!was_moving_multiselected && should_move_multiselected) {
            multiselection->total_displacement_for_undo = Vector2_zero;   
            editor.dragging_start = input.mouse_position;
        }
        if (was_moving_multiselected && !should_move_multiselected && multiselection->total_displacement_for_undo != Vector2_zero) {
            
//             undo_add_multiselect_position_change(multiselection->total_displacement_for_undo);
            for_array (i, &multiselection->entities) {
                undo_mark_entity_changed(get_entity(multiselection->entities.get_value(i)));
            }
        }
        
        was_moving_multiselected = should_move_multiselected;
    
        Vector2 most_right_entity_position;
        Vector2 most_left_entity_position;
        Vector2 most_top_entity_position;
        Vector2 most_bottom_entity_position;
        
        // Detecting required movement for multiselected.
        Vector2 moving_displacement = Vector2_zero;
        if (IsKeyDown(KEY_LEFT_ALT)) {
            moving_displacement = get_editor_mouse_move();
            multiselection->total_displacement_for_undo += moving_displacement;
        } else {
            if (sqr_magnitude(input.mouse_position - editor.dragging_start) >= (MOVE_CELL_SIZE * 0.5f * MOVE_CELL_SIZE * 0.5f)) {
                // While moving multiselected entities we canot directly set position like we do in editor_mouse_move_entity,
                // so here we calculate current quantized displacement from last moving.
                // Vector2 cell_mouse_position = round_to_factor(input.mouse_position, MOVE_CELL_SIZE);
                moving_displacement = round_to_factor(input.mouse_position - editor.dragging_start, MOVE_CELL_SIZE);
                editor.dragging_start += moving_displacement;
                multiselection->total_displacement_for_undo += moving_displacement;
            }
        }

        for (i32 entity_index = 0; entity_index < multiselection->entities.count; entity_index++) {
            Entity *entity = get_entity(multiselection->entities.get_value(entity_index));
            if (!entity) {
                continue;
            }
            
            b32 excluding_this_entity = multiselection->excluding && multiselection->selection_entities.contains(entity->id);
            if (excluding_this_entity) {
                continue;
            }
            
            if (should_move_multiselected) {
                // We want to move entity points on multiselect moving.
                b32 was_moving_entity_points = editor.move_entity_points;
                if (multiselection->entities.count > 1) {
                    editor.move_entity_points = true;
                }
                assign_selected_entity(NULL);
                
                // editor_mouse_move_mulentity(entity);
            
                if (moving_displacement != Vector2_zero) {
                    entity->position += moving_displacement;
                    
                    if (editor.move_entity_points) {
                        editor_move_entity_points(entity, moving_displacement);
                    }
                }
                
                editor.move_entity_points = was_moving_entity_points;
            }
            
            entity->color_changer.frame_changing = true;
            make_rect_lines(entity->position + entity->bounds.offset, entity->bounds.size, entity->pivot, 2.0f / current_context->cam.cam2D.zoom, BLUE); 
            
            if (entity_index == 0) {
                most_right_entity_position = entity->position;
                most_left_entity_position = entity->position;
                most_top_entity_position = entity->position;
                most_bottom_entity_position = entity->position;
            } else {
                if (entity->position.x > most_right_entity_position.x) most_right_entity_position = entity->position;
                if (entity->position.x < most_left_entity_position.x) most_left_entity_position = entity->position;
                if (entity->position.y > most_top_entity_position.y) most_top_entity_position = entity->position;
                if (entity->position.y < most_bottom_entity_position.y) most_bottom_entity_position = entity->position;
            }
        }
        
        multiselection->center = {most_left_entity_position.x + (most_right_entity_position.x - most_left_entity_position.x) * 0.5f, most_bottom_entity_position.y + (most_top_entity_position.y - most_bottom_entity_position.y) * 0.5f};
        
        if (IsKeyPressed(KEY_X)) {
            destroy_multiselected_entities();
        }
    }
    
    if (editor.dragging_entity == NULL && !editor.selected_this_click && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL) && editor.selected != NULL && need_start_dragging && can_select) { // assign dragging entity
        if (editor.cursor_entity != NULL) {
            if (editor.moving_vertex == NULL && editor.selected->id == editor.cursor_entity->id && editor.moving_entity_edge_type == NONE) {
                editor.dragging_entity = editor.selected;
                editor.dragging_entity_id = editor.selected->id;
                editor.dragging_start = editor.dragging_entity->position;
                editor.dragging_start_mouse_offset = input.mouse_position - editor.dragging_start;
                editor.dragging_start_entity_position = editor.selected->position;
            }
        }
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && can_select) { // stop dragging entity
        if (editor.selected && !editor.selected_this_click && editor.cursor_entity) {
            if (editor.dragging_time <= 0.1f && editor.cursor_entity->id == editor.selected->id && editor.moving_entity_edge_type == NONE) {
                assign_selected_entity(NULL);
            }
        }
        
        editor.dragging_time = 0;
        editor.selected_this_click = false;
        
        if (editor.dragging_entity) {
            if (editor.selected && editor.selected->position != editor.dragging_start_entity_position) {
                undo_mark_entity_changed(editor.selected);
            }
        }
        
        editor.dragging_entity = NULL;
        
        if (editor.moving_vertex_entity) {
            // something_in_undo = true;
            // undo_action.entity_id = editor.moving_vertex_entity->id;
            // undo_apply_vertices_change(editor.moving_vertex_entity, &undo_action);
        }
        
        editor.moving_vertex = NULL;
        editor.moving_vertex_entity = NULL;
    }
    
    //entity tap moving
    if (editor.selected) {
        Vector2 tap_move = {input.tap_direction.x * MOVE_CELL_SIZE, input.tap_direction.y * MOVE_CELL_SIZE};
        if (tap_move.x != 0 || tap_move.y != 0) {
            Vector2 next_position = round_to_factor(editor.selected->position + tap_move, MOVE_CELL_SIZE);
            Vector2 move_amount = next_position - editor.selected->position;
            editor.selected->position += move_amount;
            // undo_action.position_change = move_amount;
            // undo_action.entity_id = editor.selected->id;
            // something_in_undo = true;
        }
    }
    
    if (editor.moving_vertex != NULL) {
        move_vertex(editor.moving_vertex_entity, input.mouse_position, editor.moving_vertex_index);
    }
    
    //editor copy
    if ((editor.selected || multiselection->entities.count > 0) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) {
        Context *original_context = current_context;
        switch_current_context(&copied_entities_context);
        
        // Do not forgetting to free every entity that was previously copied and added to copied entities level context.
        for (i32 i = 0; i < editor.copied_entities.count; i++) {
            free_entity(editor.copied_entities.get_value(i));
        }
        editor.copied_entities.clear();
        
        if (multiselection->entities.count > 0) {
            for (i32 i = 0; i < multiselection->entities.count; i++) {
                Entity *entity_to_copy = get_entity(multiselection->entities.get_value(i), original_context);   
                // We keep id here so later we could verify different connected entities by ids. 
                // @LEAK: Check that we're actually freeing copied entities.
                editor.copied_entities.append(copy_and_add_entity(entity_to_copy, &copied_entities_context, entity_to_copy->id));
            }
            editor.copied_entities_center = multiselection->center;
        } else {
            Entity *entity_to_copy = get_entity(editor.selected->id, original_context);   
            editor.copied_entities.append(copy_and_add_entity(entity_to_copy, &copied_entities_context, entity_to_copy->id));
            // copy_entity(&editor.copied_entity, editor.selected);
            editor.copied_entities_center = entity_to_copy->position;
        }
        
        switch_current_context(original_context);
        
        editor.context_on_last_copy = original_context;
        
        editor.is_copied = true;
    }
    
    // editor paste
    if (editor.is_copied && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V) && editor.copied_entities.count > 0) {
        Vector2 paste_position = {round_to_factor(input.mouse_position.x, 5), round_to_factor(input.mouse_position.y, 5)};
        
        assign_selected_entity(NULL);
        
        Array <i32> spawned_ids = {.allocator = temp};
        
        clear_multiselected_entities();
        for (i32 i = 0; i < editor.copied_entities.count; i++) {
            Entity *to_spawn = editor.copied_entities.get_value(i);
            Entity *spawned = copy_and_add_entity(to_spawn, current_context);
            spawned_ids.append(spawned->id);
            spawned->position += paste_position - editor.copied_entities_center;
            editor_move_entity_points(spawned, paste_position - editor.copied_entities_center);
            
            editor.just_spawned_ids.append(spawned->id);
            
            if (editor.copied_entities.count == 1) {
                assign_selected_entity(spawned);
            } else {
                add_to_multiselection(spawned->id);
            }
        }
        assert(spawned_ids.count == editor.copied_entities.count);
        
        
        // Right now we want to verify connected entities only to triggers.
        // Again - that's because when we copy trigger and in multiselected was his connected guys - they will have different
        // ids. We know original ids (in copied_entiies we keep ids because they in different level context)
        // and we will know which ids they have now, because we track spawned entities and they 
        // have the same indexes as copied entities. If that was a bad explanation I've explained it also in do-list 
        // in 'Loading multiple levels' task.
        for (i32 i = 0; i < spawned_ids.count; i++) {
            Entity *spawned =  get_entity(spawned_ids.get_value(i));
            Entity *copied = editor.copied_entities.get_value(i);
            if (spawned->flags & TRIGGER) {
                // We have original trigger connected and tracking in copied_entities.
                Trigger *spawned_trigger = spawned->trigger;
                
                spawned_trigger->connected.clear();                                      
                spawned_trigger->tracking.clear();
                
                Trigger *copied_trigger = copied->trigger;
                assert(copied_trigger); // That should work because spawned_ids indexes should be the same as the copied indexes.
                // Here we want to go through all copied entities and find entities with ids from copied trigger->
                // Then we want to add entity from spawned with the same index to connected and tracked of new trigger->
                // That's confusing because it's just is. Not sure if it's even possible to make simpler.
                // But on the bright side - that's not so much code.
                //
                // UPDATE after ~3 months - completely understandable. Making same thing for KILL_SWITCH now.
                for (i32 x = 0; x < spawned_ids.count; x++) {
                    Entity *other_copied_entity = editor.copied_entities.get_value(x);
                    if (copied_trigger->connected.contains(other_copied_entity->id)) {
                        spawned_trigger->connected.append(spawned_ids.get_value(x));
                    }
                    if (copied_trigger->tracking.contains(other_copied_entity->id)) {
                        spawned_trigger->tracking.append(spawned_ids.get_value(x));
                    }
                }
            }
            
            if (spawned->flags & KILL_SWITCH) { 
                Kill_Switch *spawned_kill_switch = spawned->kill_switch;
                Kill_Switch *copied_kill_switch = copied->kill_switch;
                assert(copied_kill_switch); // That should work because spawned_ids indexes should be the same as the copied indexes.
                spawned_kill_switch->connected.clear();
                
                for (i32 x = 0; x < spawned_ids.count; x++) {
                    Entity *other_copied = editor.copied_entities.get_value(x);
                    if (copied_kill_switch->connected.contains(other_copied->id)) {
                        spawned_kill_switch->connected.append(spawned_ids.get_value(x));
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
            // (10 is just arbitrary number).
            //
            // And we do all that only for copying pasting in the same level context.
            if (spawned_ids.count < 10 && str_equal(spawned->context->name, editor.context_on_last_copy->name)) {
                for_chunk_array(j, &current_context->triggers) {
                    Trigger *trigger = current_context->triggers.get(j);
                    
                    if (trigger->connected.contains(copied->id)) {
                        assert(!trigger->connected.contains(spawned->id));
                        trigger->connected.append(spawned->id);
                    }
                    if (trigger->tracking.contains(copied->id)) {
                        assert(!trigger->tracking.contains(spawned->id));
                        trigger->tracking.append(spawned->id);
                    }
                }
                for_chunk_array(j, &current_context->kill_switches) {
                    Kill_Switch *kill_switch = current_context->kill_switches.get(j);
                    
                    if (kill_switch->connected.contains(copied->id)) {
                        assert(!kill_switch->connected.contains(spawned->id));
                        kill_switch->connected.append(spawned->id);
                    }
                }
            }
        }
    }
    
    //editor ruler
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && IsKeyDown(KEY_LEFT_ALT)) {
        editor.ruler_active = true;
        editor.ruler_start_position = input.mouse_position;
    } else if (editor.ruler_active && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        editor.ruler_active = false;
    }
    
    //editor Delete entity
    if (can_control_with_single_button && IsKeyPressed(KEY_X) && editor.selected) {
        editor_destroy_entity(editor.selected);
    }
    
    if (editor.dragging_entity != NULL) {
        editor.dragging_time += dt;
    }
    
    if (editor.dragging_entity != NULL && !moving_editor_cam) {
        editor_mouse_move_entity(editor.dragging_entity);
    }
    
    //editor Entity to mouse or go to entity
    if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.dragging_entity) {
        editor.dragging_entity->position = input.mouse_position;
    } else if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.selected) {
        current_context->cam.position = editor.selected->position;
    }
    
    //editor free entity rotation
    if (editor.selected && IsKeyDown(KEY_LEFT_ALT)) {
        f32 rotation = 0;
        f32 speed = 50;
        if (!editor.is_rotating_entity && (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_Q))) {
            editor.rotating_start = editor.selected->rotation;
//             undo_remember_vertices_start(editor.selected);
            editor.is_rotating_entity = true;
        } 
        
        if (IsKeyDown(KEY_E)) {
            rotation = dt * speed;
        } else if (IsKeyDown(KEY_Q)) {
            rotation = -dt * speed;
        }
        
        if (rotation != 0 && editor.is_rotating_entity) {
            rotate(editor.selected, rotation);
        }
        
        if (editor.is_rotating_entity && (IsKeyUp(KEY_E) && IsKeyUp(KEY_Q))) {
            undo_mark_entity_changed(editor.selected);
            editor.is_rotating_entity = false;
        } 
    } else if ((editor.selected || multiselection->entities.count > 1) && can_control_with_single_button) {
        // editor snap entity rotation.
        local_persist f32 holding_time = 0;
        f32 to_rotate = 0;
        if (IsKeyPressed(KEY_E)) {
            to_rotate = 15;
        }
        if (IsKeyPressed(KEY_Q)) {
            to_rotate = -15;
        }
        
        if (to_rotate != 0) {
            if (multiselection->entities.count > 1) {
                rotate_multiselected(to_rotate);
            } else {
                f32 next_rotation = round_to_factor(editor.selected->rotation + to_rotate, 15);
                to_rotate = next_rotation - editor.selected->rotation;
                rotate(editor.selected, to_rotate);
                undo_mark_entity_changed(editor.selected);
            }
        }
        
        if (IsKeyReleased(KEY_E) || IsKeyReleased(KEY_Q)) {
            holding_time = 0;
        }
        
        if (IsKeyDown(KEY_E) || IsKeyDown(KEY_Q)) {
            holding_time += dt;
            if (holding_time >= 0.2f) {
                f32 direction = IsKeyDown(KEY_E) ? 15 : -15;
                if (multiselection->entities.count > 1) {
                    rotate_multiselected(direction);                    
                } else {
                    rotate(editor.selected, direction);
                    undo_mark_entity_changed(editor.selected);
                }
                holding_time = 0;
            }
        }
    }
    
    // Editor free entity scaling.
    if (editor.selected && IsKeyDown(KEY_LEFT_ALT) && editor.moving_entity_edge_type == NONE) {
        Vector2 scaling = {0};
        f32 speed = 80;
        
        if (!editor.is_scaling_entity && (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_A))) {
            editor.scaling_start = editor.selected->scale;
//             undo_remember_vertices_start(editor.selected);
            editor.is_scaling_entity = true;
        }
        if      (IsKeyDown(KEY_W)) scaling.y += speed * dt;
        else if (IsKeyDown(KEY_S)) scaling.y -= speed * dt;
        if      (IsKeyDown(KEY_D)) scaling.x += speed * dt;
        else if (IsKeyDown(KEY_A)) scaling.x -= speed * dt;

        if (scaling != Vector2_zero && editor.is_scaling_entity) {
            add_scale(editor.selected, scaling);
        }
        
        if (editor.is_scaling_entity && (IsKeyUp(KEY_W) && IsKeyUp(KEY_S) && IsKeyUp(KEY_A) && IsKeyUp(KEY_D))) {
            Vector2 scale_change = editor.selected->scale - editor.scaling_start;
            
            undo_mark_entity_changed(editor.selected);
            editor.is_scaling_entity = false;
        } 
    } else if (editor.selected && can_control_with_single_button && editor.moving_entity_edge_type == NONE) {
        local_persist f32 holding_time = 0;
        Vector2 scaling = Vector2_zero;
        f32 scale_amount = SCALE_CELL_SIZE;
        
        if      (IsKeyPressed(KEY_W)) scaling.y += scale_amount;
        else if (IsKeyPressed(KEY_S)) scaling.y -= scale_amount;
        if      (IsKeyPressed(KEY_D)) scaling.x += scale_amount;
        else if (IsKeyPressed(KEY_A)) scaling.x -= scale_amount;


        if (scaling != Vector2_zero) {
            Vector2 next_scale = editor.selected->scale + scaling;
            next_scale = {round_to_factor(next_scale.x, scale_amount), round_to_factor(next_scale.y, scale_amount)};
            scaling = next_scale - editor.selected->scale;
        
            add_scale(editor.selected, scaling);
            undo_mark_entity_changed(editor.selected);
        }
        
        if (IsKeyReleased(KEY_W) || IsKeyReleased(KEY_S) || IsKeyReleased(KEY_A) || IsKeyReleased(KEY_D)) {
            holding_time = 0;
        }
        
        local_persist i32 hold_scale_times = 0;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D)) {
            f32 delay = 0.2f;
            if (hold_scale_times >= 1) {
                delay = 0.05f;
            }
            holding_time += dt;
            if (holding_time >= delay) {
                hold_scale_times += 1;
                if (hold_scale_times > 10) {
                    scale_amount = 20;
                }
                if      (IsKeyDown(KEY_W)) scaling.y += scale_amount;
                else if (IsKeyDown(KEY_S)) scaling.y -= scale_amount;
                if      (IsKeyDown(KEY_D)) scaling.x += scale_amount;
                else if (IsKeyDown(KEY_A)) scaling.x -= scale_amount;
                
//                 undo_remember_vertices_start(editor.selected);
                add_scale(editor.selected, scaling);
                undo_mark_entity_changed(editor.selected);
                holding_time = 0;
            }
        } else {
            hold_scale_times = 0;
        }
    }
    
    //editor components management
    if (editor.selected) {
        Entity *selected = editor.selected;
        if (selected->flags & TRIGGER) {
            b32 wanna_assign = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A);
            b32 wanna_assign_tracking_enemy = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Q);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D);
            b32 wanna_change_locked_camera_position = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_R);
            
            b32 wanna_add_cam_rails_point    = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N);
            b32 wanna_remove_cam_rails_point = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_M);
            b32 wanna_clear_cam_rails_points = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L);
            //trigger assign or remove
            if (wanna_assign || wanna_remove) {
                fill_collisions(&mouse_entity, &collisions_buffer, DOOR | ENEMY | SPIKES | GROUND | PLATFORM | MOVE_SEQUENCE | TRIGGER | DUMMY | TEXTURE);
                
                for (i32 i = 0; i < collisions_buffer.count; i++) {
                    Collision col = collisions_buffer.get_value(i);
                    
                    if (wanna_assign && !wanna_remove && !selected->trigger->connected.contains(col.other_entity->id)) {
                        selected->trigger->connected.append(col.other_entity->id);
                        undo_mark_entity_changed(selected);
                        break;
                    } else if (wanna_remove && !wanna_assign) {
                        if (selected->trigger->connected.contains(col.other_entity->id)) {
                            selected->trigger->connected.remove_first_encountered(col.other_entity->id);
                            undo_mark_entity_changed(selected);
                        } else if (selected->trigger->tracking.contains(col.other_entity->id)) {
                            selected->trigger->tracking.remove_first_encountered(col.other_entity->id);
                            undo_mark_entity_changed(selected);
                        }
                        break;
                    }
                }
            }
            
            if (wanna_change_locked_camera_position) {
                selected->trigger->locked_camera_position = input.mouse_position;
            }
            
            if (wanna_assign_tracking_enemy) {
                fill_collisions(&mouse_entity, &collisions_buffer, ENEMY | CENTIPEDE);
                for (i32 i = 0; i < collisions_buffer.count; i++) {
                    Collision col = collisions_buffer.get_value(i);
                    
                    if (!selected->trigger->tracking.contains(col.other_entity->id)) {
                        selected->trigger->tracking.append(col.other_entity->id);
                        undo_mark_entity_changed(selected);
                    }
                }
            }
            
            //trigger clear
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)) {
                selected->trigger->connected.clear();
                undo_mark_entity_changed(selected);
            }
            
            if (wanna_remove_cam_rails_point) {
                for (i32 i = 0; i < selected->trigger->cam_rails_points.count; i++) {
                    Vector2 point = selected->trigger->cam_rails_points.get_value(i);   
                    
                    if (check_col_circles({input.mouse_position, 1}, {point, 0.5f  * (0.4f / current_context->cam.cam2D.zoom)})) {       
                        selected->trigger->cam_rails_points.remove(i);
                        break;
                    }
                }
            }
            if (wanna_add_cam_rails_point) {
                selected->trigger->cam_rails_points.append(input.mouse_position);
                undo_mark_entity_changed(selected);
            }
            if (wanna_clear_cam_rails_points) {
                selected->trigger->cam_rails_points.clear();
                undo_mark_entity_changed(selected);
            }
        }
        
        if (selected->flags & KILL_SWITCH) {
            b32 wanna_assign = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D);
            
            Kill_Switch *kill_switch = selected->kill_switch;
            //kill switch assign or remove
            if (wanna_assign || wanna_remove) {
                fill_collisions(&mouse_entity, &collisions_buffer, ENEMY);
                for (i32 i = 0; i < collisions_buffer.count; i++) {
                    Collision col = collisions_buffer.get_value(i);
                    
                    if (wanna_assign && !wanna_remove && !kill_switch->connected.contains(col.other_entity->id)) {
                        kill_switch->connected.append(col.other_entity->id);
                        undo_mark_entity_changed(selected);
                        break;
                    } else if (wanna_remove && !wanna_assign) {
                        if (kill_switch->connected.contains(col.other_entity->id)) {
                            kill_switch->connected.remove_first_encountered(col.other_entity->id);
                            undo_mark_entity_changed(selected);
                        }
                        break;
                    }
                }
            }
            
            //kill switch clear
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)) {
                kill_switch->connected.clear();
                undo_mark_entity_changed(selected);
            }
        }
        
        // editor enemy components
        if (selected->flags & ENEMY) {
            assert(selected->union_enemy);
        
            b32 wanna_increase_sword_kill_speed  = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P);
            b32 wanna_decrease_sword_kill_speed  = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O);
            
            b32 wanna_rotate_shoot_blocker_direction   = IsKeyDown(KEY_LEFT_CONTROL) && (IsKeyDown(KEY_F) || IsKeyDown(KEY_G));
            
            if (wanna_increase_sword_kill_speed || wanna_decrease_sword_kill_speed) {   
                f32 speed_change = wanna_increase_sword_kill_speed ? 0.5f : -0.5f;
                selected->union_enemy->sword_kill_speed_modifier += speed_change;
            }
            
            if (wanna_rotate_shoot_blocker_direction && selected->flags & SHOOT_BLOCKER) {
                f32 speed = IsKeyDown(KEY_F) ? -40 : 40;
                selected->union_enemy->shoot_blocker_direction = get_rotated_vector(selected->union_enemy->shoot_blocker_direction, speed * core.time.real_dt);
            }
        }
        
        // propeller change
        if (selected->flags & PROPELLER) {
            b32 wanna_increase_power = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E);
            b32 wanna_decrease_power = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Q);
            
            if (wanna_increase_power || wanna_decrease_power) {
                f32 power_change = wanna_increase_power ? 100 : -100;
                selected->propeller->power += power_change;
            }
        }
        
        // door settings
        if (selected->flags & DOOR) {
            b32 wanna_trigger = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_T);
            
            if (wanna_trigger) {
                activate_door(selected, !selected->door.is_open);
            }
        }
        
        // move sequence settings
        if (selected->flags & MOVE_SEQUENCE) {
            b32 wanna_clear    = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L);
            b32 wanna_add    = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_N);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_M);
            
            if (wanna_remove) {
                for (i32 i = 0; i < selected->move_sequence->points.count; i++) {
                    Vector2 point = selected->move_sequence->points.get_value(i);   
                    
                    if (check_col_circles({input.mouse_position, 1}, {point, 0.5f  * (0.4f / current_context->cam.cam2D.zoom)})) {       
                        selected->move_sequence->points.remove(i);
                        break;
                    }
                }
            }
            if (wanna_add) {
                selected->move_sequence->points.append(input.mouse_position);
            }
            if (wanna_clear) {
                selected->move_sequence->points.clear();
            }
        }
    }
    
    
    update_undo_logic();
    
    // Tile sheets logic.
    if (editor.selected && editor.selected->flags & TEXTURE) {
        if (IsKeyPressed(KEY_PERIOD) || IsKeyPressed(KEY_COMMA)) {
            // We don't really want to save tile sheet name on every entity, so we just take a little performance hit of 
            // going through every texture of every tile sheet to look up the current tile texture.
            for (i32 i = 0; i < tile_sheets.count; i++) {
                Tile_Sheet *sheet = tile_sheets.get(i);
                for (i32 j = 0; j < sheet->textures.count; j++) {
                    const char *sheet_texture_name = sheet->textures.get(j)->name;
                    if (str_equal(sheet_texture_name, editor.selected->texture_name)) {
                        i32 increment_direction = IsKeyPressed(KEY_PERIOD) ? 1 : -1;
                        i32 next_index = (j + increment_direction);
                        if (next_index >= sheet->textures.count) next_index = 0;
                        if (next_index < 0) next_index = sheet->textures.count - 1;
                        
                        Texture_Data *next_data = sheet->textures.get(next_index);
                        editor.selected->texture = next_data->texture;
                        str_copy(editor.selected->texture_name, next_data->name);
                        init_entity(editor.selected);
                        break;
                    }
                }
            }
        }
    }
    
    //editor Save level
    if (IsKeyPressed(KEY_J) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL)) {
        save_current_level();
    }
    if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) {
        save_current_level();
        state_context.timers.background_flash_time = core.time.app_time;
    }
    
    f32 time_since_autosave = core.time.app_time - editor.last_autosave_time;
    if (time_since_autosave > 40 && editor_state == EDITOR) {
        autosave_level();
        editor.last_autosave_time = core.time.app_time;
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        editor.last_click_position = input.mouse_position;
        editor.last_click_time = current_context->game_time;
    }
    
    if (can_control_with_single_button && IsKeyPressed(KEY_P) && !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL)) {
        current_context->player_spawn_point = input.mouse_position;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clicked_ui = false;
    }
} // Update editor end.

void change_color(Entity *entity, Color new_color) {
    entity->color = new_color;
    setup_color_changer(entity);
}

Bounds get_bounds(Static_Array <Vector2, MAX_VERTICES> vertices, Vector2 pivot) {
    f32 top_vertex    = -INFINITY;
    f32 bottom_vertex =  INFINITY;
    f32 right_vertex  = -INFINITY;
    f32 left_vertex   =  INFINITY;
    
    for (i32 i = 0; i < vertices.count; i++) {
        Vector2 *vertex = vertices.get(i);
        
        if (vertex->y > top_vertex) {
            top_vertex = vertex->y;
        }
        if (vertex->y < bottom_vertex) {
            bottom_vertex = vertex->y;
        }
        if (vertex->x > right_vertex) {
            right_vertex = vertex->x;
        }
        if (vertex->x < left_vertex) {
            left_vertex = vertex->x;
        }
    }    
    
    Vector2 middle_position = {.x = (1.0f - pivot.x) * left_vertex + pivot.x * right_vertex,
                               .y = pivot.y * bottom_vertex + (1.0f - pivot.y) * top_vertex};
    
    return {{right_vertex - left_vertex, top_vertex - bottom_vertex}, middle_position};
}

inline void calculate_bounds(Entity *entity) {
    entity->bounds = get_bounds(entity->vertices, entity->pivot);
}

void calculate_vertices(Entity *entity) {
    for (i32 i = 0; i < entity->vertices.count; i++) {
        Vector2 *vertex = entity->vertices.get(i);
        Vector2 unscaled_vertex = entity->unscaled_vertices.get_value(i);
        
        f32 up_dot    = dot(entity->up,    unscaled_vertex);
        f32 right_dot = dot(entity->right, unscaled_vertex);

        *vertex = normalized(unscaled_vertex) + (entity->up * up_dot * entity->scale.y) + (entity->right * right_dot * entity->scale.x);
    }
}

void change_scale(Entity *entity, Vector2 new_scale) {
    Vector2 old_scale = entity->scale;
    
    entity->scale = new_scale;
    
    clamp(&entity->scale.x, 0.01f, 10000);
    clamp(&entity->scale.y, 0.01f, 10000);

    calculate_vertices(entity);    
    calculate_bounds(entity);
}

void add_scale(Entity *entity, Vector2 added) {
    change_scale(entity, entity->scale + added);
}

void change_up(Entity *entity, Vector2 new_up) {
    rotate_to(entity, (atan2f(new_up.x, new_up.y) * RAD2DEG));
}

void change_right(Entity *entity, Vector2 new_right) {
    rotate_to(entity, atan2f(-new_right.y, new_right.x) * RAD2DEG);
}

void rotate_around_point(Vector2 *target, Vector2 origin, f32 rotation) {
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

void rotate_to(Entity *entity, f32 new_rotation, b32 add_to_undo) {
    if (add_to_undo) assert(editor_state == EDITOR || state_context.in_pause_editor);

    while (new_rotation >= 360) {
        new_rotation -= 360;
    }
    while (new_rotation < 0) {
        new_rotation += 360;
    }

    f32 old_rotation = entity->rotation;

    entity->rotation = new_rotation;
    
    entity->up    = {sinf(new_rotation * DEG2RAD),  cosf(new_rotation * DEG2RAD)};
    normalize(&entity->up);
    entity->right = {cosf(new_rotation * DEG2RAD), -sinf(new_rotation * DEG2RAD)};
    normalize(&entity->right);
    
    for (i32 i = 0; i < entity->vertices.count; i++) {
        Vector2 *vertex = entity->vertices.get(i);
        rotate_around_point(vertex, {0, 0}, entity->rotation - old_rotation);
        rotate_around_point(entity->unscaled_vertices.get(i), {0, 0}, entity->rotation - old_rotation);
    }
    
    calculate_bounds(entity);
}

inline void rotate(Entity *entity, f32 rotation, b32 add_to_undo) {
    if (add_to_undo) assert(editor_state == EDITOR || state_context.in_pause_editor);

    rotate_to(entity, entity->rotation + rotation);
}

void add_hitstop(f32 added, b32 can_go_over_limit) {
    b32 was_over_limit = core.time.hitstop > 0.1f;
    if (core.time.hitstop < 0) {
        core.time.hitstop = 0;
    }

    core.time.hitstop += added;
    
    if (can_go_over_limit && core.time.hitstop > 0.5f) {
        clamp(&core.time.hitstop, 0, 0.5f);
    } else if (core.time.hitstop > 0.1f && !was_over_limit) {
        clamp(&core.time.hitstop, 0,  0.1f);
    }
}

void shake_camera(f32 trauma) {
    state_context.cam_state.trauma += trauma;
    state_context.cam_state.trauma = clamp01(state_context.cam_state.trauma);
}

void win_level() {
    if (!state_context.we_got_a_winner) {    
        state_context.we_got_a_winner = true;
        kill_player();
    }
}

b32 is_type(Entity *entity, FLAGS flags) {
    return entity->flags & flags;
}

inline void cut_rope(Entity *entity, Vector2 point) {
    if (point == Vector2_zero) {
        point = entity->position;
    }
    mark_entity_destroyed(entity);
    emit_particles(&rifle_bullet_emitter, point, entity->up, 6, 50);
    play_sound("RopeCut", point);
}

inline void calculate_collisions(void (respond_func)(Entity*, Collision), Entity *entity) {
    fill_collisions(entity, &collisions_buffer, entity->collision_flags);
    
    for (i32 i = 0; i < collisions_buffer.count; i++) {
        Collision col = collisions_buffer.get_value(i);
        respond_func(entity, col);
    }
}

void respond_jump_shooter_collision(Entity *shooter_entity, Collision col) {
    assert(shooter_entity->flags & JUMP_SHOOTER);

    Jump_Shooter *shooter = shooter_entity->jump_shooter;
    Entity *other = col.other_entity;
    f32 speed   = magnitude(shooter->velocity);
    f32 speed_t = clamp01(speed / 300.0f);
    
    b32 is_high_velocity = speed > 100;
    
    b32 should_respond = true;
    
    if (!shooter->dead_man && other->flags & PLAYER && shooter->states.flying_to_point) {
        if (can_sword_damage_enemy(shooter_entity)) {
            try_sword_damage_enemy(shooter_entity, shooter_entity->position);
        } else {
            kill_player();
        }
    }
    
    if (other->flags & GROUND) {
        resolve_collision(shooter_entity, col);
        
        if (shooter->dead_man) {
            emit_particles(&fire_emitter, shooter_entity->position, col.normal, 4, 3);
            play_sound("Explosion", shooter_entity->position, 0.3f);
            add_explosion_light(shooter_entity->position, rnd(100.0f, 250.0f), 0.15f, 0.4f, ColorBrightness(RED, 0.5f));
            mark_entity_destroyed(shooter_entity);
            shooter_entity->enabled = false;
            shake_camera(0.6f);
            return;
        }
        
        // jump shooter stop on ground
        if (shooter->states.flying_to_point) {
            shooter->current_index = (shooter->current_index + 1) % shooter->move_points.count;
            shooter->states.flying_to_point = false;
            shooter->states.standing = true;
            shooter->states.standing_start_time = current_context->game_time;
            shooter->velocity = Vector2_zero;
            emit_particles(&ground_splash_emitter, col.point, col.normal, 6, 2.5f);
            
            disable_emitter(shooter->flying_emitter_index);
        } else if (!shooter->states.standing) {
            shooter->velocity = reflected_vector(shooter->velocity * 0.7f, col.normal);
            emit_particles(&ground_splash_emitter, col.point, col.normal, 1, 0.5f);
            
            if (shooter->was_in_stun) {
                shooter->stun_start_time = -234;
            }
        }
    }
}

void move_by_velocity_with_collisions(Entity *entity, Vector2 velocity, f32 max_frame_move_len, void (respond_collision_func)(Entity*, Collision), f32 dt) {
    Vector2 this_frame_move_direction = normalized(velocity);
    f32 this_frame_move_len = magnitude(velocity * dt); 
    
    if (this_frame_move_len > max_frame_move_len * 10) {
        print("PHYSICS ERROR: Some objects moves too fast and will require heavy simulation, so it stopped.");
        return;
    }
    
    while(this_frame_move_len > max_frame_move_len) {
        entity->position += this_frame_move_direction * max_frame_move_len;
        calculate_collisions(respond_collision_func, entity);
        this_frame_move_len -= max_frame_move_len;
        this_frame_move_direction = normalized(velocity);
    }
    
    entity->position += this_frame_move_direction * this_frame_move_len;
    calculate_collisions(respond_collision_func, entity);
}

inline f32 get_explosion_radius(Entity *entity, f32 base_radius) {
    assert(entity->flags & EXPLOSIVE);
    assert(entity->union_enemy);

    base_radius *= entity->union_enemy->explosive_radius_multiplier;

    f32 scale_sum = (entity->scale.x * 0.5f + entity->scale.y * 0.5f);
    f32 scale_progress = clamp01(scale_sum / 200.0f);
    
    return lerp(base_radius, base_radius * 6, scale_progress);
}

b32 is_explosion_trauma_active() {
    return core.time.app_time - state_context.timers.background_flash_time <= 0.1f;
}

void add_explosion_trauma(f32 explosion_radius) {
    if (explosion_radius < 100 || is_explosion_trauma_active()) {
        return;
    }
    
    state_context.explosion_trauma += explosion_radius / 500.0f;
    if (state_context.explosion_trauma >= 1) {
        state_context.timers.background_flash_time = core.time.app_time;
        state_context.explosion_trauma = 0;
    }
}

void kill_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, b32 can_wait, f32 particles_speed_modifier) {
    assert(enemy_entity->flags & ENEMY);
    assert(enemy_entity->union_enemy);
    
    Enemy *enemy = enemy_entity->union_enemy;
    if (!enemy->dead_man) {
        if (can_wait) {
            if (enemy_entity->flags & EXPLOSIVE && core.time.app_time - state_context.timers.last_explosion_app_time < 0.01f) {
                enemy->should_explode = true;
                return;
            }
        }
    
        enemy->stun_start_time = current_context->game_time;
        emit_particles(get_sword_kill_particle_emitter(enemy_entity), kill_position, kill_direction, 1, particles_speed_modifier, 1);
    
        enemy->dead_man = true;
        enemy->died_time = current_context->game_time;
        b32 should_be_destroyed = !(enemy_entity->flags & (TRIGGER | CENTIPEDE_SEGMENT));
        if (should_be_destroyed) {
            enemy_entity->enabled = false;
            mark_entity_destroyed(enemy_entity);
    
            if (enemy_entity->flags & SHOOT_STOPER) {
                // assert(state_context.shoot_stopers_count >= 0);
                if (state_context.shoot_stopers_count > 0) {
                    state_context.shoot_stopers_count--;
                } else {
                    print("WARNING: Shoot stopers count could go below zero. That may be because we skipped trigger and kill it, so no assertion, just warning");            
                }
            }
        }
        
        // Kill kill switch death.
        if (enemy_entity->flags & KILL_SWITCH) {
            Kill_Switch *kill_switch = enemy_entity->kill_switch;
            for (i32 i = 0; i < kill_switch->connected.count; i++) {
                Entity *connected = get_entity(kill_switch->connected.get_value(i));
                if (!connected) {
                    continue;
                }
                
                kill_enemy(connected, connected->position, connected->up);
            }
        }
        
        // Kill win block.
        if (enemy_entity->flags & WIN_BLOCK) {
            assert(current_context->active_win_blocks_count > 0);
            
            current_context->active_win_blocks_count -= 1;
            if (current_context->active_win_blocks_count <= 0) {
                win_level();
            }
        }
        
        if (enemy_entity->flags & MOVE_SEQUENCE && !(enemy_entity->flags & CENTIPEDE_SEGMENT)) {
            enemy_entity->move_sequence->moving = false;
        }
        
        // Explosion.
        if (enemy_entity->flags & EXPLOSIVE) {
            assert(enemy_entity->union_enemy);
        
            state_context.timers.last_explosion_app_time = core.time.app_time;
            f32 explosion_radius = get_explosion_radius(enemy_entity);
            
            Particle_Emitter *explosion_emitter = &explosion_emitter_copy;
            if (enemy->explosive_radius_multiplier > 1.5f) {
                explosion_emitter = &big_explosion_emitter_copy;
            }
            
            emit_particles(explosion_emitter, enemy_entity->position, Vector2_up, explosion_radius / 40.0f);
            play_sound("BigExplosion", enemy_entity->position, 0.5f);
            
            i32 light_size_flag = SMALL_LIGHT;
            // @VISUAL: Probably will consider that when I'll return dynamic shadows.
            // if (enemy_entity->light_index != -1) light_size_flag = current_context->lights.get_value(enemy_entity->light_index).shadows_size_flags;
            add_explosion_light(enemy_entity->position, explosion_radius * rnd(3.0f, 6.0f), 0.15f, fminf(enemy->explosive_radius_multiplier, 3.0f), ColorBrightness(ORANGE, 0.3f), light_size_flag);
            
            f32 explosion_add_speed = 80;
            i32 spawned_particles_count = 0;
            ForEntities(other_entity, 0) {
                if (other_entity->flags & TURRET || other_entity->flags & WIN_BLOCK || other_entity->flags & CENTIPEDE) {
                    continue;
                }
            
                Vector2 vec_to_other = other_entity->position - enemy_entity->position;
                f32 distance_to_other = magnitude(vec_to_other);
                
                if (distance_to_other > explosion_radius) {
                    continue;
                }
                
                Vector2 dir_to_other = normalized(vec_to_other);
                
                Collision obstacle_collision = raycast(enemy_entity->position, dir_to_other, distance_to_other - 5, GROUND | CENTIPEDE_SEGMENT | CENTIPEDE, distance_to_other - 5, enemy_entity->id);
                if (obstacle_collision.collided) {
                    if (spawned_particles_count < 3) {
                        emit_particles(&ground_splash_emitter, obstacle_collision.point, obstacle_collision.normal, 4, 5.5f);
                        spawned_particles_count += 1;
                    }
                    continue;
                }
                
                if (other_entity->flags & ENEMY && other_entity->union_enemy->max_hits_taken >= 0) {
                    if (!other_entity->union_enemy->dead_man) {
                        stun_enemy(other_entity, other_entity->position, dir_to_other, true);
                    }
                    
                    if (other_entity->flags & BIRD_ENEMY) {
                        other_entity->bird_enemy->velocity += dir_to_other * explosion_add_speed;
                    }
                    if (other_entity->flags & JUMP_SHOOTER) {
                        other_entity->jump_shooter->velocity += dir_to_other * explosion_add_speed;
                    }
                } else if (other_entity->flags & PROJECTILE) {
                    mark_entity_destroyed(other_entity);
                }
                
                if (other_entity->flags & BLOCK_ROPE) {
                    cut_rope(other_entity);
                }
                
                if (other_entity->flags & PLAYER && !current_context->player.dead_man && distance_to_other < explosion_radius * 0.75f) {
                    kill_player();
                }
            }
            
            add_hitstop(0.1f * fmaxf(1.0f, enemy->explosive_radius_multiplier * 0.5f), true);
            shake_camera(0.5f * fmaxf(1.0f, enemy->explosive_radius_multiplier * 0.5f));
            
            // Centipede explode segments.
            if (enemy_entity->flags & CENTIPEDE_SEGMENT) {
                // If we don't explode all segments at once then weird things occur when some segments in ground.
                Centipede *head = enemy_entity->centipede_segment->head->centipede;
                if (!head->all_segments_dead) {
                    head->all_segments_dead = true;
                    for (i32 i = 0; i < head->segments.count; i++) {
                        Entity *segment = head->segments.get_value(i);
                        if (segment && segment->id != enemy_entity->id) {
                            kill_enemy(segment, segment->position, segment->up);
                        } else if (!segment) {
                            print("WARNING: For some reason on exploding all centipede segments some segment was not here AT ALL!!");
                        }
                    }
                }
            }
        } // Kill explosive end.
    }
}

inline b32 is_enemy_can_take_damage(Entity *enemy_entity, b32 check_for_last_hit_time) {
    assert(enemy_entity->flags & ENEMY);
    assert(enemy_entity->union_enemy);

    if (enemy_entity->flags & CENTIPEDE_SEGMENT && enemy_entity->union_enemy->dead_man) {
        return false;
    }
    if (enemy_entity->flags & CENTIPEDE) {
        return false;
    }
    
    if (enemy_entity->union_enemy->player_cannot_kill) {
        return false;
    }
    
    if (enemy_entity->flags & TRIGGER && enemy_entity->union_enemy->dead_man) {
        return false;
    }
    
    if (!check_for_last_hit_time) {
        return true;
    }
    
    f32 immune_time = 0.2f;
    if (enemy_entity->flags & PLAYER_TOUCH_TIMER) {
        immune_time = 0.078f;
    }
    
    b32 recently_got_hit = (current_context->game_time - enemy_entity->union_enemy->last_hit_time) <= immune_time;
    return !recently_got_hit;
}

void agro_enemy(Entity *entity) {
    assert(entity->union_enemy);

    if (entity->union_enemy->in_agro || entity->union_enemy->dead_man) {
        return;
    }

    entity->union_enemy->in_agro = true;
    
    add_explosion_light(entity->position, (entity->scale.y + entity->scale.x) * 10, 0.1f, 2.2f, Fade(ColorBrightness(RED, 0.5f), 0.2f), SMALL_LIGHT, entity->id);
    
    if (entity->flags & SHOOT_STOPER) {
        state_context.shoot_stopers_count++;
    }
}

inline Particle_Emitter *get_sword_kill_particle_emitter(Entity *enemy_entity) {
    Particle_Emitter *emitter = &blood_pop_emitter_copy;
    // Should just do a enemy flag for serious enemies instead of picking everyone individualy.
    if (enemy_entity->flags & BIRD_ENEMY) {
        emitter = &sword_kill_medium_emitter_copy;
    } else if (enemy_entity->flags & JUMP_SHOOTER) {
        emitter = &sword_kill_big_emitter_copy;
    }
    
    return emitter;
}

void stun_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, b32 serious) {
    assert(enemy_entity->flags & ENEMY);
    assert(enemy_entity->union_enemy);
    
    Enemy *enemy = enemy_entity->union_enemy;
    
    if (enemy_entity->flags & EXPLOSIVE) {
        kill_enemy(enemy_entity, kill_position, kill_direction);
        return;
    }
    
    if (is_enemy_can_take_damage(enemy_entity)) {
        if (enemy_entity->flags & MOVE_SEQUENCE && !(enemy_entity->flags & CENTIPEDE_SEGMENT)) {
            enemy_entity->move_sequence->moving = false;
        }
        agro_enemy(enemy_entity);
    
        enemy->stun_start_time = current_context->game_time;
        enemy->last_hit_time   = current_context->game_time;
        enemy->hits_taken++;
        b32 should_die_in_one_hit = enemy_entity->flags & BIRD_ENEMY && enemy_entity->bird_enemy->attacking;
        
        if ((enemy->hits_taken >= enemy->max_hits_taken || serious || should_die_in_one_hit) && !enemy->dead_man) {
            emit_particles(get_sword_kill_particle_emitter(enemy_entity), kill_position, kill_direction, 1, 1, 1);
        
            enemy->died_time = current_context->game_time;
        
            if (enemy_entity->flags & BIRD_ENEMY) {
                enemy->dead_man = true;
                // birds handle dead state by themselves
                enable_emitter(enemy_entity->bird_enemy->fire_emitter_index, enemy_entity->position);
                enable_emitter(enemy_entity->bird_enemy->smoke_fire_emitter_index, enemy_entity->position);
                add_fire_light_to_entity(enemy_entity);
            } else if (enemy_entity->flags & JUMP_SHOOTER) {
                enemy->dead_man = true;
                Particle_Emitter *dead_fire_emitter = get_particle_emitter(add_entity_particle_emitter(enemy_entity, &fire_emitter));
                if (dead_fire_emitter) {
                    dead_fire_emitter->position = enemy_entity->position;
                    enable_emitter(dead_fire_emitter);
                }
                add_fire_light_to_entity(enemy_entity);
            } else if (enemy_entity->flags & CENTIPEDE_SEGMENT) {
                enemy->dead_man = true;
            } else {
                kill_enemy(enemy_entity, kill_position, kill_direction);
            }
        } else {
            enemy->stun_start_time = current_context->game_time;
            enemy->last_hit_time   = current_context->game_time;
        }
        add_hitmark(enemy_entity, true); 
    }
}

void add_rifle_projectile(Vector2 start_position, Vector2 velocity) {
    Entity *projectile_entity = add_entity(start_position, {2, 2}, {0.5f, 0.5f}, 0, PINK, PROJECTILE | PARTICLE_EMITTER);
    projectile_entity->projectile->type  = PLAYER_RIFLE_PROJECTILE;
    projectile_entity->projectile->birth_time = current_context->game_time;
    projectile_entity->projectile->velocity = velocity;
    projectile_entity->projectile->max_lifetime = 7;
    
    projectile_entity->projectile->trail_emitter_index = add_and_enable_entity_particle_emitter(projectile_entity, &bullet_trail_emitter_copy, start_position, true);
    add_and_enable_entity_particle_emitter(projectile_entity, &bullet_trail_emitter_copy, start_position, true);
    add_and_enable_entity_particle_emitter(projectile_entity, &magical_sparks_emitter_copy, start_position, true);
    add_and_enable_entity_particle_emitter(projectile_entity, &white_sparks_emitter_copy, start_position, true);
}

inline Vector2 transform_texture_scale(Texture texture, Vector2 wish_scale) {
    Vector2 real_scale = {(f32)texture.width / current_context->cam.unit_size, (f32)texture.height / current_context->cam.unit_size};
    
    return {wish_scale.x / real_scale.x, wish_scale.y / real_scale.y};
}

void add_hitmark(Entity *entity, b32 need_to_follow, f32 scale_multiplier, Color tint) {
    Entity *hitmark = add_entity(entity->position, transform_texture_scale(hitmark_small_texture, {45, 45}) * scale_multiplier, {0.5f, 0.5f}, rnd(-90.0f, 90.0f), hitmark_small_texture, TEXTURE | STICKY_TEXTURE);
    hitmark->runtime_only_flags |= SHOULD_NOT_SAVE;
    change_color(hitmark, tint);
    hitmark->draw_order = 1;
    
    hitmark->sticky_texture->need_to_follow   = need_to_follow;
    hitmark->sticky_texture->draw_line        = true;
    hitmark->sticky_texture->follow_id        = entity->id;
    hitmark->sticky_texture->birth_time       = current_context->game_time;
    hitmark->sticky_texture->should_draw_until_expires = true;
    hitmark->sticky_texture->max_distance     = 1000;
    hitmark->sticky_texture->base_size = hitmark->scale;
}

Vector2 get_entity_velocity(Entity *entity) {
    if (entity->flags & PLAYER) {
        return entity->player_data->velocity;
    }
    if (entity->flags & BIRD_ENEMY) {
        return entity->bird_enemy->velocity;
    }
    if (entity->flags & JUMP_SHOOTER) {    
        return entity->jump_shooter->velocity;
    }
    if (entity->flags & PROJECTILE) {
        return entity->projectile->velocity;
    }
    return Vector2_zero;
}

inline b32 compare_difference(f32 first, f32 second, f32 allowed_difference = EPSILON) {
    return abs(first - second) <= allowed_difference;
}

inline f32 get_death_instinct_radius(Vector2 threat_scale) {
    // return fmaxf(fminf((18.0f / current_context->cam.cam2D.zoom), 60), 40) * fmaxf(magnitude(velocity) / 200.0f, 1.0f);
    return 40 + (threat_scale.x + threat_scale.y) - 8.0f;
}

b32 is_death_instinct_threat_active() {
    Entity *threat_entity = get_entity(state_context.death_instinct.threat_entity_id);
    assert(threat_entity->union_enemy);
    
    b32 entity_alive = threat_entity && !threat_entity->destroyed && !threat_entity->union_enemy->dead_man;
    
    f32 since_death_instinct = core.time.app_time - state_context.death_instinct.start_time;
    b32 is_no_cooldown_on_stop = since_death_instinct <= state_context.death_instinct.allowed_duration_without_cooldown; 
    
    if (entity_alive) {
        switch (state_context.death_instinct.last_reason) {
            case ENEMY_ATTACKING:{
                Vector2 vec_to_player = current_context->player_entity->position - threat_entity->position;
                Vector2 dir_to_player = normalized(vec_to_player);
                f32 distance_to_player = magnitude(vec_to_player);
                
                // We want instinct to stop if player evaded enemy in the beginning, but if we'll do that with cooldown there
                // could be confusions.
                b32 check_for_flying_towards = is_no_cooldown_on_stop; 
                return is_enemy_should_trigger_death_instinct(threat_entity, get_entity_velocity(threat_entity), dir_to_player, distance_to_player, check_for_flying_towards);
            } break;
            case SWORD_WILL_EXPLODE:{
                return current_context->player.is_sword_will_hit_explosive;     
            } break;
            default: return true;
        }
    } else {
        return false;
    }
}

inline b32 is_in_death_instinct() {
    return core.time.app_time - state_context.death_instinct.start_time <= state_context.death_instinct.duration;
}

inline b32 is_death_instinct_in_cooldown() {
    return core.time.app_time - state_context.death_instinct.cooldown_start_time <= state_context.death_instinct.cooldown;
}

void stop_death_instinct() {
    f32 time_since_death_instinct = core.time.app_time - state_context.death_instinct.start_time;

    Entity *threat_entity = get_entity(state_context.death_instinct.threat_entity_id);
    assert(threat_entity->union_enemy);

    b32 is_threat_status_gives_cooldown = true;
    if (state_context.death_instinct.last_reason == ENEMY_ATTACKING) {
        // We start cooldown if there was flying guy if he's not here anymore so player really used that instinct and don't just
        // evaded enemy.
        // is_threat_status_gives_cooldown = (!threat_entity || threat_entity->enemy.dead_man || is_enemy_should_trigger_death_instinct(threat_entity, get_entity_velocity(threat_entity), normalized(player_entity->position - threat_entity->position), magnitude(player_entity->position - threat_entity->position), true));
        if (threat_entity) {
            is_threat_status_gives_cooldown = !is_death_instinct_threat_active();
        }
    } else if (state_context.death_instinct.last_reason == SWORD_WILL_EXPLODE) {
        // If there was explosive we want start cooldown if threat is still alive so player evaded explosion. 
        // That's because if player killed it - he taked risk and succeeded.
        is_threat_status_gives_cooldown = (threat_entity && !threat_entity->union_enemy->dead_man);
    }
    b32 should_start_cooldown = time_since_death_instinct >= state_context.death_instinct.allowed_duration_without_cooldown && is_threat_status_gives_cooldown;

    if (should_start_cooldown) {
        state_context.death_instinct.cooldown_start_time = core.time.app_time;            
    }
    if (time_since_death_instinct >= 0.2f) {
        state_context.death_instinct.stop_time = core.time.app_time;
    }
    state_context.death_instinct.start_time = -12;
    state_context.death_instinct.threat_entity_id = -1;
    state_context.death_instinct.played_effects = false;
}

// @CLEANUP: Death instinct mechanic is not present right now, so that a candidate for removal.
b32 is_enemy_should_trigger_death_instinct(Entity *entity, Vector2 velocity, Vector2 dir_to_player, f32 distance_to_player, b32 check_if_flying_towards) {
    return false;

    b32 flying_towards = true;
    // @TODO: We definetely want to better check if enemy is flying towards. For example we can just simulate enemy some steps forward.
    if (check_if_flying_towards) {
        flying_towards = distance_to_player < get_death_instinct_radius(entity->scale) && dot(dir_to_player, normalized(velocity)) >= 0.9f;
    }
    
    if (!flying_towards) {
        return false;
    }
    
    b32 will_kill_on_hit = true;
    if (!will_kill_on_hit) {
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

b32 start_death_instinct(Entity *threat_entity, Death_Instinct_Reason reason) {
    if (is_in_death_instinct() || is_death_instinct_in_cooldown() || current_context->player.dead_man) {
        return false;
    }
    
    state_context.death_instinct.start_time = core.time.app_time;
    state_context.death_instinct.threat_entity_id = threat_entity->id;
    state_context.death_instinct.last_reason = reason;
    return true;
}

void calculate_projectile_collisions(Entity *entity) {
    Projectile *projectile = entity->projectile;
    
    Entity *player_entity = current_context->player_entity;
    
    // Player projectile collisions.
    if (projectile->type == PLAYER_RIFLE_PROJECTILE) {
        fill_collisions(entity, &collisions_buffer, GROUND | ENEMY | WIN_BLOCK | ROPE_POINT);
        // Player *player = player_data;
        
        b32 damaged_enemy = false;
        
        b32 should_start_dying = true;
        
        for (i32 i = 0; i < collisions_buffer.count; i++) {
            Collision col = collisions_buffer.get_value(i);
            Entity *other = col.other_entity;
            
            // Dying player rifle projectile is just slow shit and that happens only after bounce. So we destroy it at any collision.
            if (projectile->dying) {
                mark_entity_destroyed(entity);
                return;
            }
            
            if (projectile->last_hit_id == other->id) {
                continue;
            }
            
            // b32 need_bounce = false;
            
            Vector2 velocity_dir = normalized(projectile->velocity);
            f32 sparks_speed = 1;
            f32 sparks_count = 1;
            f32 hitstop_add = 0;
            
            if (other->flags & ENEMY && is_enemy_can_take_damage(other, false) && !projectile->dying) {
                // projectile->already_hit_ids.append(other->id);
                projectile->last_hit_id = other->id;
                projectile->hits_count += 1;
                b32 killed = false;
                b32 can_damage = true;
                
                assert(other->union_enemy);
                Enemy *enemy = other->union_enemy;
                
                if (other->flags & SHOOT_BLOCKER) {
                    Vector2 shoot_blocker_direction = get_rotated_vector(enemy->shoot_blocker_direction, other->rotation);
                    f32 velocity_dot_direction = dot(velocity_dir, shoot_blocker_direction);    
                        
                    can_damage = !enemy->shoot_blocker_immortal && (compare_difference(velocity_dot_direction, 1, 0.1f) || compare_difference(velocity_dot_direction, -1, 0.1f));
                    sparks_speed += 2;
                    sparks_count += 2;
                    
                    if (!can_damage) {
                        // need_bounce = true;
                        enemy->last_hit_time = current_context->game_time;
                        play_sound("ShootBlock", col.point);
                    }
                }
                if (other->flags & ENEMY_BARRIER) {
                    can_damage = false;
                    enemy->last_hit_time = current_context->game_time;
                    // need_bounce = true;
                    play_sound("ShootBlock", col.point);
                }
                
                if (other->flags & PROJECTILE && !(other->flags & EXPLOSIVE)) {
                    can_damage = false;
                    // need_bounce = true;
                    play_sound("ShootBlock", col.point);
                }

                
                if (other->flags & BIRD_ENEMY && can_damage) {
                    other->bird_enemy->velocity += projectile->velocity * 0.05f;
                }
                if (other->flags & JUMP_SHOOTER && can_damage) {
                    other->jump_shooter->velocity += projectile->velocity * 0.05f;
                }
                
                // We don't want projectiles to hit triggers. That's just not cool.
                if (other->flags & TRIGGER) {
                    can_damage = false;
                }
                
                if (other->flags & WIN_BLOCK) {
                    can_damage = false;
                }
                
                if (other->flags & HIT_BOOSTER) {
                    entity->position = other->position;
                    projectile->velocity = normalized(other->up) * magnitude(projectile->velocity);
                    should_start_dying = false;
                }
                
                if (should_start_dying) {
                    projectile->velocity = reflected_vector(projectile->velocity * 0.6f, col.normal);
                    projectile->bounced = true;
                    projectile->birth_time = current_context->game_time;
                    // need_bounce = true;
                }
                
                if (can_damage) {
                    if (enemy->max_hits_taken > 1) {
                        stun_enemy(other, entity->position, col.normal);    
                    } else if (enemy->max_hits_taken <= -1) {
                        
                    } else {
                        kill_enemy(other, entity->position, col.normal, false);
                        killed = true;
                    }
                }
                
                if (other->flags & TRIGGER && can_damage) {
                    sparks_count += 20;
                    hitstop_add = 0.1f;
                }
                
                if (other->flags & BIRD_ENEMY || other->flags & JUMP_SHOOTER) {
                    emit_particles(&bullet_strong_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
                } else {
                    emit_particles(&bullet_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
                }
                
                // Just for avoiding annoyence.
                if (projectile->hits_count < 8) {
                    add_hitstop(0.03f + hitstop_add);
                    shake_camera(0.1f);
                }
                
                if (projectile->hits_count > 30) {
                    // @TODO: We should explode here. Also could check that hits were made fast, like calculating average delay between hits for last 10.
                }
                
                
                if (can_damage) {
                    f32 time_since_last_hit = current_context->game_time - state_context.timers.last_projectile_hit_time;
                    if (time_since_last_hit <= 0.05f) {
                        state_context.contiguous_projectile_hits_count += 1;
                    } else {
                        state_context.contiguous_projectile_hits_count = 0;
                    }
                
                    f32 base_pitch = 1.0f + state_context.contiguous_projectile_hits_count * 0.025f;
                    state_context.timers.last_projectile_hit_time = current_context->game_time;
                    play_sound("RifleHit", col.point, 0.4f + state_context.contiguous_projectile_hits_count * 0.01f, base_pitch);
                }
            
                damaged_enemy = can_damage;
            }
            
            if (other->flags & GROUND) {
                mark_entity_destroyed(entity);
                emit_particles(&bullet_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
            }
            
            if (other->flags & ROPE_POINT) {
                // cut rope point
                mark_entity_destroyed(other);
                emit_particles(&rifle_bullet_emitter, col.point, col.normal, 6, 10);
                emit_particles(&bullet_hit_emitter_copy, col.point, velocity_dir, sparks_count, sparks_speed);
                play_sound("RopeCut", col.point);
            }
            
            // if (need_bounce && !) {
            //     projectile->velocity = reflected_vector(projectile->velocity * 0.5f, col.normal);
            // }
            
            if (!projectile->dying && current_context->game_time - projectile->last_light_spawn_time >= 0.1f) {
                add_explosion_light(col.point, 75, 0.05f, 0.1f, ColorBrightness(damaged_enemy ? RED : SKYBLUE, 0.4f));
                projectile->last_light_spawn_time = current_context->game_time;
            }
        }
    } else if (projectile->type == JUMP_SHOOTER_PROJECTILE) {
        fill_collisions(entity, &collisions_buffer, GROUND | PLAYER | CENTIPEDE_SEGMENT | ENEMY_BARRIER | NO_MOVE_BLOCK);
        // @CLEANUP We don't need JUMP_SHOOTER_PROJECTILE anymore because we don't want jump shooter.        
        assert(entity->union_enemy);
        Enemy *enemy = entity->union_enemy;
        
        for (i32 i = 0; i < collisions_buffer.count; i++) {
            Collision col = collisions_buffer.get_value(i);
            Entity *other = col.other_entity;
            
            if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT) {
                assert(!(other->flags & CENTIPEDE));
                kill_enemy(entity, col.point, col.normal);
                emit_particles(&bullet_hit_emitter_copy, col.point, col.normal * -1, 1);
            }
            
            if (other->flags & PLAYER && !current_context->player.dead_man && !enemy->dead_man) {
                // It's a good thing that we don't kill player when projectile is blocker or explosive, 
                // but of course we need to better tell player what exactly will kill him on touch. 
                // While projectiles are flying - they're leave particle trail and all flying projectiles 
                // will kill us. So when projectile stopped we should still produce particles for 
                // base projectiles so player can know what he need to know.
                // That also works for enemies - they're kill player on touch when they're producing certain particles.
                b32 can_kill_player = !projectile->dying || !(entity->flags & (BLOCKER | EXPLOSIVE));
                if (can_kill_player) {
                    if (can_sword_damage_enemy(entity)) {
                        kill_enemy(entity, col.point, col.normal);
                    } else {
                        kill_player();
                    }
                }
            }
        }
    } else if (projectile->type == TURRET_DIRECT_PROJECTILE || projectile->type == TURRET_HOMING_PROJECTILE) {
        fill_collisions(entity, &collisions_buffer, GROUND | PLAYER | CENTIPEDE_SEGMENT | ENEMY_BARRIER | NO_MOVE_BLOCK);
        assert(entity->union_enemy);
        Enemy *enemy = entity->union_enemy;
        
        for (i32 i = 0; i < collisions_buffer.count; i++) {
            Collision col = collisions_buffer.get_value(i);
            Entity *other = col.other_entity;
            
            if (other->flags & PLAYER) {
                b32 should_kill_player = !entity->context->player.dead_man && !enemy->dead_man;
                if (should_kill_player) {
                    kill_player();
                    kill_enemy(entity, col.point, col.normal);
                    emit_particles(&bullet_hit_emitter_copy, col.point, col.normal * -1, 1);
                }
            } else {
                kill_enemy(entity, col.point, col.normal);
                emit_particles(&bullet_hit_emitter_copy, col.point, col.normal * -1, 1);
            }
        }
    }
}

inline void update_projectile(Entity *entity, f32 dt) {
    assert(entity->flags & PROJECTILE);
    
    Projectile *projectile = entity->projectile;
    f32 lifetime = current_context->game_time - projectile->birth_time;
    
    Vector2 player_position = current_context->player_entity ? current_context->player_entity->position : Vector2_zero;
    
    if (projectile->max_lifetime > 0 && lifetime> projectile->max_lifetime) {
        if (entity->flags & ENEMY) {
            // kill_enemy(entity, entity->position, entity->up);
            mark_entity_destroyed(entity);    
        } else {
            mark_entity_destroyed(entity);    
        }
        return;
    }
    
    f32 sqr_distance_to_player = sqr_magnitude(entity->position - player_position);
    if (projectile->type == PLAYER_RIFLE_PROJECTILE) {
        if (sqr_distance_to_player > 1000 * 1000) {
            mark_entity_destroyed(entity);
            return;
        }
        
        if (projectile->bounced) {
            // Projectile flies for some time after bounce, but then we want it to settle down.
            f32 feel_strong_after_bounce = 0.1f;
            if (lifetime > feel_strong_after_bounce) {
                f32 life_overshoot = lifetime - feel_strong_after_bounce;
                projectile->dying = true;
                
                clamp_magnitude(&projectile->velocity, 60);
                projectile->velocity.y -= current_context->player.gravity * dt;
            }
        }
    }
    
    if (projectile->type == TURRET_HOMING_PROJECTILE) {
        Vector2 vec_to_player = player_position - entity->position;
        Vector2 dir = normalized(vec_to_player);
        
        if (dot(dir, entity->up) > 0) {
            change_up(entity, move_towards(entity->up, dir, 2, dt));
            f32 projectile_speed = magnitude(projectile->velocity);
            projectile->velocity = entity->up * projectile_speed;
        }
    }
    
    Vector2 move = projectile->velocity * dt;
    Vector2 move_dir = normalized(move);
    f32 move_len = magnitude(move);
    f32 max_move_len = entity->scale.y * 0.5f;
    
    for (i32 i = 0; i < entity->particle_emitters_indexes.count; i++) {
        Particle_Emitter *emitter = get_particle_emitter(entity->particle_emitters_indexes.get_value(i));
        if (emitter) {
            if (sqr_distance_to_player >= 500 * 500) {
                disable_emitter(emitter);
            } else {
                emitter->direction = move_dir * 1.0f;
                enable_emitter(emitter, entity->position);
            }
        }
    }
    
    if (projectile->type == JUMP_SHOOTER_PROJECTILE) {
        if (lifetime >= 0.5f && !projectile->dying) {
            Collision ray = raycast(entity->position + entity->up * entity->scale.y * 0.5f, entity->up, 10, GROUND | CENTIPEDE_SEGMENT | BLOCKER | SHOOT_BLOCKER, 5, entity->id);
            if (ray.collided) {
                projectile->dying = true;
            }
        }
    
        if (lifetime >= 3 || projectile->dying) {
            f32 damping_factor = projectile->dying ? 25 : 4;
            projectile->velocity *= 1.0f - (dt * damping_factor);
            projectile->dying = true;
            if (entity->flags & EXPLOSIVE || entity->flags & BLOCKER) {
                disable_emitter(entity->union_enemy->alarm_emitter_index);
            }
        } else {
        }
    }
    
    while (move_len > max_move_len) {
        entity->position += move_dir * max_move_len;
        calculate_projectile_collisions(entity);
        move_len -= max_move_len;
    }
    
    entity->position += move_dir * move_len;
    calculate_projectile_collisions(entity);
    
    change_up(entity, projectile->velocity);
}

void update_sticky_texture(Entity *entity, f32 dt) {
    Sticky_Texture *st = entity->sticky_texture;
    
    f32 lifetime = current_context->game_time - st->birth_time;
    f32 lifetime_t = 0;
    if (st->max_lifetime > EPSILON) {
        lifetime_t = lifetime / st->max_lifetime;
        if (lifetime >= st->max_lifetime) {
            mark_entity_destroyed(entity);
        } else {
            entity->color = lerp(entity->color_changer.start_color, Fade(WHITE, 0), EaseOutExpo(lifetime_t));
        }
    }
    
    Entity *follow_entity = NULL;
    if (st->follow_id > 0) {
        follow_entity = get_entity(st->follow_id);
        if (follow_entity->will_be_destroyed) {
            st->follow_id = 0;
            follow_entity = NULL;
        }
    }
    
    b32 need_to_follow = false;
    if (st->need_to_follow && follow_entity) {
        need_to_follow = true;
        Vector2 target_position = follow_entity->position;
        if (follow_entity->flags & SHOOT_STOPER) {
            target_position = get_shoot_stoper_cross_position(follow_entity);
        }
        entity->position = lerp(entity->position, target_position, dt * 40);
        
        if (follow_entity->flags & ENEMY && follow_entity->union_enemy->dead_man && !st->should_draw_until_expires) {
           st->should_draw_texture = false;
       }
    }
    
    if (!need_to_follow && st->max_lifetime <= EPSILON) {
        mark_entity_destroyed(entity);
    }
    
    st->need_to_follow = need_to_follow;
}

inline b32 verify_trigger_connected(Entity *entity) {
    if (!(entity->flags & TRIGGER)) {
        return false;   
    }
      
    b32 removed_something = false;
      
    Trigger *trigger = entity->trigger;
    for_array (i, &trigger->connected) {
        Entity *connected = get_entity(trigger->connected.get_value(i));
        if (connected->will_be_destroyed) {
            trigger->connected.remove(i);
            i--;
            removed_something = true;
        }
    }
    for_array (i, &trigger->tracking) {
        Entity *tracking = get_entity(trigger->tracking.get_value(i));
        if (tracking->will_be_destroyed) {
            trigger->tracking.remove(i);
            i--;
            removed_something = true;
        }
    }
    
    if (removed_something && is_editor_active()) {
        undo_mark_entity_changed(entity);
    }
    
    return removed_something;
}

inline b32 verify_kill_switch_connected(Entity *entity) {
    if (!(entity->flags & KILL_SWITCH)) {
        return false;
    }

    b32 removed_something = false;
    
    Kill_Switch *kill_switch = entity->kill_switch;
    for_array (i, &kill_switch->connected) {
        Entity *connected = get_entity(kill_switch->connected.get_value(i));
        if (connected->will_be_destroyed) {
            kill_switch->connected.remove(i);
            i--;
            removed_something = true;
        }
    }
    
    if (removed_something && is_editor_active()) {
        undo_mark_entity_changed(entity);
    }
    
    return removed_something;
}

void update_editor_entity(Entity *e) {
    if (e->flags & LIGHT) {
        if (e->lights.count == 0) {
            printf("WARNING: Entity with flag LIGHT don't have corresponding light index (name: %s; id: %d)\n", temp_entity_name(e).data, e->id);
        } else {
            Light *light = get_light(e->lights.get_value(0));
            light->position = e->position;
        }
    }
    
    if (e->flags & DOOR) {
        e->door.closed_position = e->door.is_open ? e->position - e->up * e->scale.y : e->position;
        e->door.open_position   = e->door.is_open ? e->position : e->position + e->up * e->scale.y;
    }
    
    // Update turret editor.
    if (e->flags & TURRET) {
        e->turret->original_up = e->up;
    }
    
    if (e->flags & PROPELLER) {
        Particle_Emitter *air_emitter = get_particle_emitter(e->propeller->air_emitter_index);
        if (air_emitter) {
            init_propeller_emitter_settings(e, air_emitter);
        }
    }
    
    if (e->flags & CENTIPEDE) {
        if (e->centipede->segments.count != e->centipede->segments_to_spawn) {
            init_entity(e); // On init entity centipede will destroy all existing segments and respawn them with proper count.
        }
        for_array (i, &e->centipede->segments) {
            Entity *segment = e->centipede->segments.get_value(i);
            Vector2 right_position = get_centipede_segment_start_position(segment, e, i);
            if (segment->position != right_position) {
                put_centipede_segment_at_right_start_position(segment, e, i);
            }
        }
    }
}

void activate_turret(Entity *entity) {
    entity->turret->activated = true;
    entity->turret->last_shot_tick = entity->context->turret_state.current_tick - entity->turret->start_tick_delay;
}

void trigger_entity(Entity *trigger_entity, Entity *connected) {
    connected->hidden = !(trigger_entity->trigger->settings & SHOWS_ENTITIES);
    
    b32 should_agro = (trigger_entity->trigger->settings & AGRO_ENEMIES) && debug.enemy_ai;
    if (should_agro) {
        if (connected->flags & ENEMY) {
            agro_enemy(connected);
        }
        
    }
    
    if (connected->flags & CENTIPEDE) {
        assert(connected->flags & MOVE_SEQUENCE); // While we move centipede by move sequence we want that to be checked.
        for (i32 i = 0; i < connected->centipede->segments.count; i++) {
            Entity *segment = connected->centipede->segments.get_value(i);
            assert(segment);
            segment->hidden = connected->hidden;
            if (should_agro) {
                segment->move_sequence->moving = connected->move_sequence->moving;
                agro_enemy(segment);
            }
        }
    }
    
    if (connected->flags & DOOR) {
        b32 trigger_opens_doors =  trigger_entity->trigger->settings & OPEN_DOORS;
        if (connected->door.is_open != trigger_opens_doors) {
            activate_door(connected, trigger_opens_doors);
        }
    }
    
    if (connected->flags & MOVE_SEQUENCE) {
        connected->move_sequence->moving = trigger_entity->trigger->settings & STARTS_MOVING_SEQUENCE;
    }
    
    if (connected->flags & TURRET) {
        activate_turret(connected);
    }
}

i32 update_trigger(Entity *e) {
    assert(e->flags & TRIGGER);
    
    if (game_state == GAME_PLANNING) return 0;
    
    Trigger *trigger = e->trigger;
    
    b32 trigger_now = false;
    
    if (trigger->debug_should_trigger_now) {
        trigger->debug_should_trigger_now = false;
        trigger_now = true;
    }
    
    if (e->flags & ENEMY && e->union_enemy->dead_man) {
        if (trigger->triggered) {
            return TRIGGER_SOME_ACTION;
        }
    
        trigger_now = true;
    }
    
    if (trigger->settings & KILL_ENEMIES) {
        fill_collisions(e, &collisions_buffer, ENEMY);
        for (i32 i = 0; i < collisions_buffer.count; i++) {
            Collision col = collisions_buffer.get_value(i);
            kill_enemy(col.other_entity, col.point, col.normal);            
        }
    }
    
    // Here we check for enemies that would be assigned as dead_man and will not be automatically removed from tracking 
    // array. Below we check for original count and if enemy was completely destroyed and count becomes zero - we'll detect that.
    if (trigger->tracking.count > 0 && !trigger->triggered) {
        b32 found_enemy = false;
        for (i32 i = 0; i < trigger->tracking.count; i++) {
            i32 id = trigger->tracking.get_value(i);
            
            Entity *tracking_entity = get_entity(id);

            if (!tracking_entity->union_enemy->dead_man) {
                found_enemy = true;
                break;
            } 
        }
        
        if (!found_enemy) {
            trigger_now = true;            
        }
    }
    
    if (trigger->start_tracking_count > 0 && trigger->tracking.count == 0 && !trigger->triggered) {
        trigger_now = true;
    }
    
    Player *player_data = &current_context->player;
    if (trigger_now || (trigger->settings & PLAYER_TOUCH) && current_context->player_entity && check_entities_collision(e, current_context->player_entity).collided) {
        if (trigger->settings & FORBID_PLAYER_SHOOT) {
            player_data->can_shoot = false;
        }
        if (trigger->settings & ALLOW_PLAYER_SHOOT) {
            player_data->can_shoot = true;
        }
    
        if (str_contains(temp_entity_name(e).data, "checkpoint") && checkpoint_trigger_id != e->id) {
            clear_context(&checkpoint_context);
            copy_context(&checkpoint_context, current_context, false);
            checkpoint_player_entity = current_context->player_entity;
            checkpoint_player_data = *player_data;
            checkpoint_time = core.time;
            checkpoint_state_context = state_context;
            
            checkpoint_trigger_id = e->id;
        }
        
        if (str_equal(temp_entity_name(e).data, "relax")) {
            state_context.playing_relax = true;
        }
    
        if (trigger->settings & LOAD_LEVEL) {
            b32 we_on_last_level = str_equal(trigger->level_name, "LAST_LEVEL_MARK");
            if (we_on_last_level || global_data.speedrun_timer.level_timer_active) {
                win_level();
            } else {
                enter_game_state_on_new_level = true;
                last_player_data = *player_data;
                load_level(tstring(trigger->level_name));
                return TRIGGER_LEVEL_LOAD;
            }
        }
        
        if (trigger->settings & PLAY_REPLAY) {
            if (!global_data.playing_replay) {
                load_replay(trigger->replay_name);
            }
        }
        
        if (trigger->settings & START_CAM_RAILS_HORIZONTAL) {
            state_context.cam_state.on_rails_horizontal = true;
            state_context.cam_state.on_rails_vertical = false;
            state_context.cam_state.locked = false;
            state_context.cam_state.rails_trigger_id = e->id;
        }
        if (trigger->settings & START_CAM_RAILS_VERTICAL) {
            state_context.cam_state.on_rails_vertical = true;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.locked = false;
            state_context.cam_state.rails_trigger_id = e->id;
        }
        if (trigger->settings & STOP_CAM_RAILS) {
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical   = false;
            state_context.cam_state.rails_trigger_id = -1;
        }
        
        if (trigger->settings & PLAY_SOUND && !trigger->triggered) {
            play_sound(trigger->sound_name);
        }
        
        if (trigger->settings & CHANGE_ZOOM) {
            // With wide monitors happening cut in vertical space so we need to calculate zoom with aspect ratio.
            // 16:9 it's 1.777777 aspect_ratio
            // 21:9 it's 2.333333 aspect_ratio
            f32 target_zoom = trigger->zoom_value;
            target_zoom /= (aspect_ratio / 1.77777f);
            current_context->cam.target_zoom = target_zoom;
        }
        
        if (trigger->settings & UNLOCK_CAMERA) {
            state_context.cam_state.locked = false;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical = false;
        } else if (trigger->settings & LOCK_CAMERA) {
            state_context.cam_state.locked = true;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical = false;
            current_context->cam.target = trigger->locked_camera_position;
        }
    
        if (e->flags & DOOR) {
            trigger_entity(e, e);
        }
    
        if (trigger->settings & KILL_PLAYER) {
            kill_player();
        }
        
        for (i32 i = 0; i < trigger->connected.count; i++) {
            i32 id = trigger->connected.get_value(i);
            
            Entity *connected_entity = get_entity(id);
                        
            trigger_entity(e, connected_entity);
        }
        
        trigger->triggered = true;
        trigger->triggered_time = current_context->game_time;
        
        if (trigger->settings & DIE_AFTER_TRIGGER) {
            e->enabled = false;
        }
    }
    
    return TRIGGER_SOME_ACTION;
}

void update_door(Entity *entity) {
    Door *door = &entity->door;

    f32 since_triggered = current_context->game_time - door->triggered_time;
    f32 move_time       = door->is_open ? door->time_to_open : door->time_to_close;
    
    if (since_triggered <= move_time) {
        Vector2 target_position = door->is_open ? door->open_position   : door->closed_position;
        Vector2 start_position  = door->is_open ? door->closed_position : door->open_position;
        
        f32 t = clamp01(since_triggered / move_time);
        
        entity->position = lerp(start_position, target_position, EaseOutElastic(t));
    }
}

void activate_door(Entity *entity, b32 is_open) {
    if (entity->door.is_open != is_open) { 
        play_sound("OpenDoor", entity->position);
        entity->door.is_open = is_open;
        entity->door.triggered_time = current_context->game_time;
    }
}

Collision get_nearest_ground_collision(Vector2 point, f32 radius) {
    Static_Array <Vector2, MAX_VERTICES> vertices;
    f32 radius_step = 4;
    f32 current_radius = 0;
    
    add_rect_vertices(&vertices, {0.5f, 0.5f});    
    while (current_radius <= radius) {
        current_radius += radius_step        ;
        for (i32 i = 0; i < vertices.count; i++) {
            *vertices.get(i) = normalized(vertices.get_value(i)) * current_radius;
            rotate_around_point(vertices.get(i), {0, 0}, 33);
        }
        
        Bounds bounds = get_bounds(vertices, {0.5f, 0.5f});
        
        fill_collisions(point, vertices, bounds, {0.5f, 0.5f}, &collisions_buffer, GROUND);
        
        for (i32 i = 0; i < collisions_buffer.count; i++) {
            Collision col = collisions_buffer.get_value(i);
            if (col.collided) {
                return col;
            }
        }
    }
    
    return {0};
}

void update_move_sequence(Entity *entity, f32 dt) {
    Move_Sequence *sequence = entity->move_sequence;
    
    if (!sequence->moving || sequence->points.count == 0) {
        sequence->moved_last_frame = Vector2_zero;
        return;
    }
    
    if (!sequence->loop && sequence->current_index >= sequence->points.count-1 && sqr_magnitude(entity->position - sequence->points.get_value(sequence->current_index)) <= EPSILON) {
        sequence->moved_last_frame = Vector2_zero;
        return;
    }
    
    Vector2 target = sequence->points.get_value((sequence->current_index + 1) % sequence->points.count);
    
    if (sequence->current_index >= sequence->points.count-1 && !sequence->loop) {
        target = sequence->points.last_value();
    }
    
    f32 speed = sequence->speed;
    
    if (sequence->speed_related_player_distance && editor_state == GAME) {
        f32 distance_to_player = magnitude(current_context->player_entity->position - entity->position);
        f32 distance_t = clamp01((distance_to_player + sequence->min_distance) / sequence->max_distance);
        speed = lerp(sequence->speed, sequence->max_distance_speed, distance_t * distance_t);
    }
    
    if (sequence->just_born) {
        sequence->velocity = normalized(target - entity->position) * speed;
        sequence->wish_position = entity->position;
        
        sequence->just_born = false;
    }
        
    if (entity->flags & JUMP_SHOOTER) {
        // Jump_Shooter *shooter = &entity->jump_shooter;   
    } else {
        Vector2 previous_position = entity->position;
        sequence->wish_position = move_towards(sequence->wish_position, target, speed, dt);
        
        if (!sequence->loop && sequence->current_index >= sequence->points.count - 2) {
            entity->position = move_towards(entity->position, sequence->wish_position, speed, dt);
        } else {
            Vector2 wish_vec = sequence->wish_position - entity->position;
            f32 wish_len = magnitude(wish_vec);
            if (wish_len > 0) {
                sequence->wish_velocity = (wish_vec / wish_len) * speed;
                sequence->velocity = move_towards(sequence->velocity, sequence->wish_velocity, speed * 4, dt);
                entity->position += sequence->velocity * dt;
            }
        }
        
        
        if (sequence->rotate) {
            change_up(entity, normalized(sequence->velocity));
        }
        
        sequence->moved_last_frame = entity->position - previous_position;
        
        if (magnitude(target - sequence->wish_position) <= EPSILON) {
            sequence->current_index = sequence->current_index + 1;
            if (sequence->current_index >= sequence->points.count && sequence->loop) {
                sequence->current_index = 0;
            }
        }
    }
}

void update_all_collision_cells(b32 update_cells_for_static_entities) {
    for (i32 i = 0; i < current_context->collision_grid.cells.count; i++) {        
        current_context->collision_grid.cells.get(i)->dynamic_entities.clear();
    }
    if (update_cells_for_static_entities) {
        for (i32 i = 0; i < current_context->collision_grid.cells.count; i++) {        
            current_context->collision_grid.cells.get(i)->static_entities.clear();
        }
    }
    
    ForEntities(entity, 0) {
        if (entity->will_be_destroyed) {
            continue;
        }
    
        update_entity_collision_cells(entity, update_cells_for_static_entities);
    }
}

void shoot_projectile(Vector2 position, Vector2 direction, Projectile_Settings settings, Projectile_Type type, Color color) {
    Vector2 scale = {3, 8};
    
    if (type == TURRET_HOMING_PROJECTILE) {
        scale *= 2;
    }
    
    // @CLEANUP: Right now we set additional projectile enemy flags directly to entity, but when we redo entity system we will 
    // want to set that on enemy of spawned projectile->
    Entity *projectile_entity = add_entity(position, scale, {0.5f, 0.5f}, 0, PROJECTILE | ENEMY | PARTICLE_EMITTER | settings.enemy_flags);
    assert(projectile_entity->projectile->index >= 0);
    change_color(projectile_entity, color);
    projectile_entity->projectile->birth_time = current_context->game_time;
    projectile_entity->projectile->type = type;
    projectile_entity->projectile->velocity = direction * settings.launch_speed;
    change_up(projectile_entity, direction);
    projectile_entity->projectile->max_lifetime = settings.max_lifetime;
    
    if (projectile_entity->flags & BLOCKER) {
        projectile_entity->union_enemy->blocker_clockwise = settings.blocker_clockwise;
    }
    
    if (type == TURRET_HOMING_PROJECTILE) {
        add_and_enable_entity_particle_emitter(projectile_entity, &small_air_dust_trail_emitter_copy, projectile_entity->position, true);
    }
    projectile_entity->union_enemy->alarm_emitter_index = add_and_enable_entity_particle_emitter(projectile_entity, &alarm_smoke_emitter_copy, projectile_entity->position, true);
    init_entity(projectile_entity);
}

global_variable f32 turret_max_angle_diversion = 70;
inline void update_turret(Entity *entity, f32 dt) {
    Turret *turret = entity->turret;
    assert(turret);
    
    if (!turret->activated) {
        // if (turret->homing && entity->rotation != turret->original_angle - turret_max_angle_diversion) {
        //     rotate_to(entity, turret->original_angle - turret_max_angle_diversion);
        // }
        return;
    }
    
    Projectile_Type projectile_type = TURRET_DIRECT_PROJECTILE;
    Color projectile_color = ColorBrightness(RED, 0.5f);
    
    Entity *player_entity = current_context->player_entity;
    
    b32 player_in_range = true;
    b32 player_in_angle_range = true;
    
    
    turret->see_player = false;
    if (turret->homing && player_entity) {
        // projectile_type = TURRET_HOMING_PROJECTILE;
        projectile_color = ColorBrightness(ORANGE, -0.2f);
        
        // f32 sqr_distance = sqr_magnitude(vec_to_player);
            
        if (!check_rectangles_collision(player_entity->position, player_entity->scale, entity->position, {turret->shoot_width, turret->shoot_height})) {
            turret->in_agro = false;
            player_in_range = false;
        } else {
            turret->in_agro = true;
            Vector2 vec_to_player = player_entity->position - entity->position;
            Vector2 dir = normalized(vec_to_player);
            // This (- 2) because of the shitty raycast where we could hit obstacle *behind* player and count that as we don't 
            // see player, even though we have direct line of sight.
            f32 distance = magnitude(vec_to_player) - entity->scale.y * entity->pivot.y - 2;
            
            Collision ray_collision = raycast(entity->position + entity->up * entity->scale.y * entity->pivot.y, dir, distance, GROUND | ENEMY_BARRIER | NO_MOVE_BLOCK, distance);
            
            if (ray_collision.collided) {
                turret->see_player = false;
            } else {
                turret->see_player = true;
            }
            
            f32 dot_original_current = dot(turret->original_up, dir);
            f32 max_allowed_dot = 1.0f - (turret_max_angle_diversion / 90.0f);          
            if (dot_original_current < max_allowed_dot) {
                turret->see_player = false;
                player_in_angle_range = false;
            } else {
                f32 rotation_speed = 30;
                // f32 angle_change = normalized(desired_angle - current_angle) * rotation_speed * dt * -1;
                // rotate(entity, angle_change);
                change_up(entity, move_towards(entity->up, dir, 1.0f, dt));
            }
        }
    } else {
        // We currently don't care about non-homing turret seeing player.
        turret->see_player = true;
    }
    
    Turret_State *state = &entity->context->turret_state;
    
    // We apply delay only for first shot ever, because next it will just work as intended because of last_shot_tick.
    i32 seconds_between_ticks = turret->last_shot_tick == 0 ? turret->start_tick_delay : 0;
    // b32 is_my_tick = state->ticked_this_frame && (state->current_tick - turret->last_shot_tick - seconds_between_ticks) >= turret->shoot_every_tick;
    b32 is_my_tick = state->ticked_this_frame && ((state->current_tick - turret->start_tick_delay) % turret->shoot_every_tick) == 0 && turret->last_shot_tick != state->current_tick;
    
    // We don't set last shot tick on real shot because homing turrets will not always see player when tick happens, so 
    // we just tracking ticks for that to work correctly.
    if (is_my_tick) {
        turret->last_shot_tick = state->current_tick;
    }
    
    if (is_my_tick && player_in_range && player_in_angle_range /*turret->see_player*/) {
        Vector2 start_position = entity->position + entity->up * entity->scale.y * entity->pivot.y;
        shoot_projectile(start_position, entity->up, turret->projectile_settings, projectile_type, projectile_color);
        if (turret->homing) {
            play_sound("BirdAttack", entity->position);
        }
    }
}

// We return false if we should stop updating entities this frame. Shoulda make result flag or something, but for now comment will do.
inline b32 update_entity(Entity *e, f32 dt) {
    update_color_changer(e, dt);            
    
    Entity *player_entity = current_context->player_entity;
    
    //update light on entity (Lights itself updates in separate place).
    if (e->flags & LIGHT) {
        if (e->lights.count == 0) {
            printf("WARNING: Entity with flag LIGHT don't have corresponding light index. Name: %s, id: %d\n", temp_entity_name(e).data, e->id);
        }
    }
    
    // Update player.
    if (e->flags & PLAYER && !debug.dragging_player) {
        if (e->flags & REPLAY_PLAYER) {
            if (!global_data.playing_replay) {
                mark_entity_destroyed(e);
                return true;
            }
            // @HACK if we'll use replay characters more - we should really look into where we use player_data->
            // player_data = &replay_player_data;
            // e->position = replay_input.player_position;
            // update_player(e, dt, replay_input);
            // player_data = &real_player_data;
        } else {
            // player_data = &real_player_data;
            update_player(e, &e->context->player, input, dt);
        }
    }
      
    // Update explosive.
    if (e->flags & EXPLOSIVE) {
        if (e->union_enemy->should_explode && !e->union_enemy->dead_man) {
            if (core.time.app_time - state_context.timers.last_explosion_app_time >= 0.01f) {    
                kill_enemy(e, e->position, e->up, false);
            }
        }
    }
        
    // Update bird enemy.
    if (e->flags & BIRD_ENEMY && debug.enemy_ai) {
        update_bird_enemy(e, dt);
    }
    
    if (e->flags & TURRET && debug.enemy_ai) {
        update_turret(e, dt);
    }
    
    // Update player touch timer.
    if (e->flags & PLAYER_TOUCH_TIMER && e->context->player_entity) {
        Player_Touch_Timer *touch = &e->union_enemy->player_touch_timer;
        // Collision player_collision = check_entities_collision(e, e->context->player);
        auto player_collisions = get_tcollisions(e, PLAYER | CONNECTED_TO_PLAYER);
        
        if (player_collisions.count > 0) {
            touch->regen_timer = 0;
            touch->touched_timer += dt;
            
            if (touch->touched_timer >= touch->seconds_until_death) {
                kill_enemy(e, e->position, e->up, false, 1);
            }
        } else {
            touch->regen_timer += dt;
            if (touch->regen_timer >= touch->seconds_until_regen && touch->touched_timer > 0) {
                touch->touched_timer -= dt;
                clamp(&touch->touched_timer, 0, touch->seconds_until_death);
            }
        }
    }
    
    // Update projectile.
    if (e->flags & PROJECTILE) {
        update_projectile(e, dt);
    }
    
    for (i32 em = 0; em < e->particle_emitters_indexes.count; em++) {
        Particle_Emitter *emitter = get_particle_emitter(e->particle_emitters_indexes.get_value(em));
        if (emitter && emitter->follow_entity) {
            emitter->position = e->position;
        }
        // update_emitter(e->emitters.get(em), dt);
    }
    
    // Update sticky texture.
    if (e->flags & STICKY_TEXTURE) {
        update_sticky_texture(e, dt);
    }
    
    // Update trigger.
    if (e->flags & TRIGGER) {
        i32 trigger_action_flags = update_trigger(e);
        if (trigger_action_flags & TRIGGER_LEVEL_LOAD) {
            return false;
        }
    }
    
    // Update move sequence.
    if (e->flags & MOVE_SEQUENCE) {
        update_move_sequence(e, dt);
    }
    
    // Update centipede.
    if (e->flags & CENTIPEDE && debug.enemy_ai && !e->centipede->dead_man) {
        Centipede *centipede = e->centipede;
        
        i32 alive_count = 0;
        for_array (i, &centipede->segments) {
            Entity *segment = centipede->segments.get_value(i);
            
            Entity *previous = segment->centipede_segment->previous;
            Vector2 previous_bottom = previous->position - previous->up * previous->scale.y * (1.0f - previous->pivot.y);
            
            Vector2 vec = previous_bottom - segment->position;
            f32 length = magnitude(vec);
            f32 half_segment_height = segment->scale.y * 0.5f;
            if (length > half_segment_height) {
                Vector2 dir = vec / length;
                change_up(segment, dir);
                segment->position = previous_bottom - dir * half_segment_height;
            } 
            
            if (!segment->centipede_segment->dead_man) {
                alive_count += 1;
            }
        }
        
        if (alive_count == 0) {
            centipede->all_segments_dead = true;
        
            mark_entity_destroyed(e);
        }
        // end update centipede end
    }

    if (e->flags & JUMP_SHOOTER && debug.enemy_ai) {
        // Update jump shooter.
        Jump_Shooter *shooter = e->jump_shooter;
        
        if (!shooter->in_agro) {
            shooter->states = {0};
            shooter->states.standing_start_time = current_context->game_time;
        }
        
        f32 in_stun_time = current_context->game_time - shooter->stun_start_time;
        
        b32 is_stunned = in_stun_time <= shooter->max_stun_time;
        
        if (shooter->dead_man || is_stunned) {
            shooter->velocity.y -= GRAVITY * dt;
            rotate(e, shooter->velocity.x * 30 * dt);
            shooter->states = {0};
            disable_emitter(shooter->trail_emitter_index);
            shooter->states.standing = false;
            shooter->states.standing_start_time = current_context->game_time;
            
            if (is_stunned) {
                shooter->was_in_stun = true;
            }
        } else if (shooter->was_in_stun) {
            shooter->states.standing = true;
            shooter->states.standing_start_time = current_context->game_time;
            
            shooter->was_in_stun = false;
        }
        
        f32 block_direction_switch_time = 1.5f;
        
        Particle_Emitter *trail_emitter = get_particle_emitter(shooter->trail_emitter_index);
        Particle_Emitter *flying_emitter = get_particle_emitter(shooter->flying_emitter_index);
        
        Vector2 vec_to_player = player_entity->position - e->position;
        Vector2 dir_to_player = normalized(vec_to_player);
        f32 distance_to_player = magnitude(vec_to_player);
        
        if (shooter->states.standing) {
            f32 standing_time = current_context->game_time - shooter->states.standing_start_time;
            f32 max_standing_time = 3.0f;
            
            Collision nearest_ground = get_nearest_ground_collision(e->position, e->scale.x * 0.7f + e->scale.y * 0.7f);
            
            if (nearest_ground.collided) {
                shooter->not_found_ground_timer = 0;
                shooter->velocity = Vector2_zero;
                
              //landing animation
                if (standing_time <= 1.0f) {
                    f32 landing_t = clamp01(standing_time / 1.0f);
                    
                    Vector2 target_scale = {shooter->original_scale.x * 1.5f, shooter->original_scale.y * 0.5f};
                    if (landing_t <= 0.20f) {
                        f32 t = clamp01(landing_t / 0.25f);
                        change_scale(e, lerp(shooter->original_scale, target_scale, EaseOutElastic(t)));
                    } else {
                        f32 t = clamp01((landing_t - 0.25f) / (1.0f - 0.25f));
                        change_scale(e, lerp(target_scale, shooter->original_scale, EaseInOutElastic(t)));
                    }
                }
                // squeezing animation
                if (standing_time >= max_standing_time - 1.0f) {
                    f32 anim_t = clamp01((standing_time - (max_standing_time - 1.0f)) / 1.0f);
                    
                    Vector2 target_scale = {shooter->original_scale.x * 1.4f, shooter->original_scale.y * 0.7f};
                    if (anim_t <= 0.85f) {
                        f32 t = anim_t / 0.85f;
                        change_scale(e, lerp(shooter->original_scale, target_scale, t * t));
                    } else {
                        f32 t = (anim_t - 0.85f) / 0.3f;
                        change_scale(e, lerp(target_scale, shooter->original_scale, sqrtf(t)));
                    }
                } 
                //jump shooter jump
                if (standing_time >= max_standing_time) {
                    shooter->states.standing = false;
                    shooter->states.jumping = true;
                    shooter->states.jump_start_time = current_context->game_time;
                    shooter->jump_direction = e->up;
                    
                    shooter->velocity = shooter->jump_direction * 250;
                    emit_particles(&ground_splash_emitter, e->position - e->up * e->scale.y * 0.5f, e->up, 4, 1.5f);
                } else {
                    Collision ray_collision = raycast(e->position, normalized(nearest_ground.point - e->position), (e->scale.x + e->scale.y) * 2, GROUND, 1.0f);
                    Vector2 point = Vector2_zero;
                    Vector2 normal = Vector2_up;
                    if (ray_collision.collided) {
                        point = ray_collision.point;
                        normal = ray_collision.normal;
                    } else {
                        point = nearest_ground.point;
                        normal = nearest_ground.normal;
                    }
                    
                    Vector2 dir = normalized(e->position - point);
                    e->position = move_towards(e->position, point + dir * e->scale.y * 0.5f, 50, dt);
                    change_up(e, move_towards(e->up, normal, 10, dt));
                    
                    if (nearest_ground.other_entity->flags & MOVE_SEQUENCE) {
                        e->position += nearest_ground.other_entity->move_sequence->moved_last_frame;
                    }
                }
            } else { // If not found ground
                shooter->not_found_ground_timer += dt;
                shooter->velocity.y -= GRAVITY * dt;
                
                if (shooter->not_found_ground_timer >= 0.4f) {
                    shooter->states.standing_start_time = current_context->game_time;
                }
            }
        }
        
        if (shooter->states.jumping) {
            f32 jumping_time = current_context->game_time - shooter->states.jump_start_time;
            f32 max_jumping_time = 1.5f;
            f32 jump_t = clamp01((jumping_time / max_jumping_time));
            
            rotate(e, shooter->velocity.x * 20 * dt);
            
            f32 gravity_multiplier = shooter->jump_direction.y > 0 ? lerp(3.0f, 2.0f, jump_t * jump_t) : lerp(-0.5f, 0.0f, jump_t * jump_t);
            shooter->velocity.y -= GRAVITY * gravity_multiplier * dt;
            shooter->velocity.x = lerp(shooter->velocity.x, 0.0f, jump_t * dt * 6);
            
            if (jumping_time >= max_jumping_time || (jumping_time >= max_jumping_time * 0.5f && shooter->velocity.y < 40)) {
                shooter->states.jumping = false;
                shooter->states.charging = true;
                shooter->states.charging_start_time = current_context->game_time;
                
                shooter->blocker_clockwise = /*rnd01() >= 0.5f*/ (current_context->game_time - (i32)current_context->game_time) >= 0.5f;
            }
        }
        
        if (shooter->states.charging) {
            f32 charging_time = current_context->game_time - shooter->states.charging_start_time;
            f32 charging_t = clamp01(charging_time / shooter->max_charging_time);
            
            // Visual hints above jumper head happening in first half of charging
            if (charging_t <= 0.5f) {
                f32 t = charging_t * 2;
                
                block_direction_switch_time = 0.2f;
            }
            
            move_vec_towards(&shooter->velocity, Vector2_zero, lerp(0.0f, 100.0f, sqrtf(charging_t)), dt);
            shooter->velocity.x = lerp(shooter->velocity.x, 0.0f, charging_t * dt * 5);
            
            f32 look_speed = lerp(0.0f, 10.0f, charging_t * charging_t);
            change_right(e, move_towards(e->right, dir_to_player.x > 0 ? dir_to_player : dir_to_player * -1, look_speed, dt));
            
            // jump shooter shoot
            if (charging_time >= shooter->max_charging_time && current_context->game_time - state_context.timers.last_jump_shooter_attack_time >= 0.3f) {                    
                f32 angle = -shooter->spread * 0.5f;
                f32 angle_step = shooter->spread / shooter->shots_count;
                
                local_persist Static_Array <i32, 64> explosive_indexes;
                explosive_indexes.clear();
                
                for (i32 i = 0; i < shooter->explosive_count; i++) {
                    i32 explosive_index = /*rnd(0, shooter->shots_count)*/ (i32)current_context->game_time * (explosive_indexes.count + 1) % shooter->shots_count;
                    while (explosive_indexes.contains(explosive_index)) {
                        explosive_index = (explosive_index+1) % shooter->shots_count;
                    }
                    explosive_indexes.append(explosive_index);
                }
                
                i32 explosive_shots = 0;
                
                for (i32 i = 0; i < shooter->shots_count; i++) {
                    // @CLEANUP All of this is now in the shoot_projectile function. We don't use jump shooter anymore so it stays
                    // till the time comes.
                    Vector2 direction = get_rotated_vector(dir_to_player, angle);
                    angle += angle_step;
                    f32 speed = 100;
                    
                    FLAGS additional_flags = 0;
                    if (explosive_indexes.contains(i)) {
                        additional_flags |= EXPLOSIVE;        
                    }
                    if (shooter->shoot_sword_blockers) {
                        additional_flags |= BLOCKER;
                    }
                    if (shooter->shoot_bullet_blockers) {
                        additional_flags |= SHOOT_BLOCKER;
                    }
                    
                    Entity *projectile_entity = add_entity(e->position, {2, 4}, {0.5f, 0.5f}, 0, PROJECTILE | ENEMY | PARTICLE_EMITTER | additional_flags);
                    assert(projectile_entity->union_enemy);
                    change_color(projectile_entity, ColorBrightness(RED, 0.4f));
                    projectile_entity->projectile->birth_time = current_context->game_time;
                    projectile_entity->projectile->type = JUMP_SHOOTER_PROJECTILE;
                    projectile_entity->projectile->velocity = direction * speed;
                    projectile_entity->projectile->max_lifetime = 15;
                    projectile_entity->union_enemy->gives_ammo = false;
                    
                    if (shooter->shoot_sword_blockers) {
                        projectile_entity->union_enemy->blocker_clockwise = shooter->blocker_clockwise;
                        projectile_entity->union_enemy->blocker_immortal  = shooter->shoot_sword_blockers_immortal;
                    }
                    if (shooter->shoot_bullet_blockers) {
                        projectile_entity->union_enemy->shoot_blocker_immortal = true;
                    }
                    
                    add_and_enable_entity_particle_emitter(projectile_entity, &small_air_dust_trail_emitter_copy, projectile_entity->position, true);
                    projectile_entity->union_enemy->alarm_emitter_index = add_and_enable_entity_particle_emitter(projectile_entity, &alarm_smoke_emitter_copy, projectile_entity->position, true);
                    init_entity(projectile_entity);
                }
                
                shooter->velocity = dir_to_player * -30 + Vector2_up * 100;
                
                shooter->states.charging = false;
                shooter->states.in_recoil = true;
                shooter->states.recoil_start_time = current_context->game_time;
                
                state_context.timers.last_jump_shooter_attack_time = current_context->game_time;
            }
        }
        
        if (shooter->shoot_sword_blockers) {
            // Mostly visual change direction above blocker head
            f32 time_since_block_direction_change = current_context->game_time - shooter->last_visual_blocker_direction_change_time;
            if (time_since_block_direction_change >= block_direction_switch_time) {
                shooter->blocker_clockwise = !shooter->blocker_clockwise;
                shooter->last_visual_blocker_direction_change_time = current_context->game_time;
            }
        }
        
        if (shooter->states.in_recoil) {
            f32 in_recoil_time = current_context->game_time - shooter->states.recoil_start_time;
            f32 max_recoil_time = 1.0f;
            
            rotate(e, shooter->velocity.x * 20 * dt);
            
            f32 gravity_multiplier = shooter->velocity.y > 0 ? 1.5f : 0.7f;
            shooter->velocity.y -= GRAVITY * gravity_multiplier * dt;
            
            if (in_recoil_time >= max_recoil_time) {
                shooter->states.in_recoil = false;
                shooter->states.picking_point = true;
                shooter->states.picking_point_start_time = current_context->game_time;
            }
        }
        
        if (shooter->states.picking_point) {
            f32 picking_point_time = current_context->game_time - shooter->states.picking_point_start_time;
            f32 picking_point_t = clamp01(picking_point_time / shooter->max_picking_point_time);
            
            Move_Point next_point = shooter->move_points.get_value((shooter->current_index + 1) % shooter->move_points.count);
            
            Vector2 vec_to_point = next_point.position - e->position;
            Vector2 dir = normalized(vec_to_point);
            
            move_vec_towards(&shooter->velocity, Vector2_zero, lerp(0.0f, 100.0f, sqrtf(picking_point_t)), dt);
            
            f32 look_speed = lerp(0.0f, 14.0f, picking_point_t * picking_point_t);
            change_up(e, move_towards(e->up, dir, look_speed, dt));
            
            Vector2 target_scale = {shooter->original_scale.x * 1.4f, shooter->original_scale.y * 1.7f};
            change_scale(e, lerp(shooter->original_scale, target_scale, picking_point_t * picking_point_t));
            
            if (trail_emitter) {
                trail_emitter->direction = e->up * -1;
                trail_emitter->count_multiplier = lerp(1.0f, 10.0f, sqrtf(picking_point_t));
                trail_emitter->speed_multiplier = lerp(1.0f, 10.0f, sqrtf(picking_point_t));
                trail_emitter->over_time = 1.0f;
            }   
            // jump shooter fly to next
            if (picking_point_time >= shooter->max_picking_point_time) {
                shooter->states.picking_point = false;
                shooter->states.flying_to_point = true;
                shooter->states.flying_start_time = current_context->game_time;
                
                shooter->velocity = dir * 400;
                if (flying_emitter) {
                    flying_emitter->position = e->position - e->up * e->scale.y * 0.5f;
                    enable_emitter(flying_emitter);
                }
                
                if (trail_emitter) {
                    trail_emitter->count_multiplier = 1;
                    trail_emitter->speed_multiplier = 1;
                    trail_emitter->over_time = 2.0f;
                }
            }
        }
        
        if (shooter->states.flying_to_point) {
            f32 flying_time = current_context->game_time - shooter->states.flying_start_time;
            f32 max_flying_time = 1.2f;
            f32 flying_t = clamp01(flying_time / max_flying_time);
            // when we fly we just wait for ground collision to change state and if it took too long - we should die
            
            Vector2 target_scale = {shooter->original_scale.x * 1.4f, shooter->original_scale.y * 1.7f};
            change_scale(e, lerp(target_scale, shooter->original_scale, sqrtf(flying_t)));
            
            if (flying_time >= 10.0f) {
                kill_enemy(e, e->position, e->up);
            }
            
            Move_Point target_point = shooter->move_points.get_value((shooter->current_index + 1) % shooter->move_points.count);
            Vector2 vec_to_point = target_point.position - e->position;
            Vector2 dir = normalized(vec_to_point);
            f32 len = magnitude(vec_to_point);
            
            change_up(e, lerp(dir, target_point.normal, clamp01(EaseInOutQuad(flying_t) + lerp(0.0f, 1.0f, 1.0f - clamp01(len / 30.0f)))));
            
            if (is_enemy_should_trigger_death_instinct(e, shooter->velocity, dir_to_player, distance_to_player, true)) {
                start_death_instinct(e, ENEMY_ATTACKING);          
            }
        }
        
        move_by_velocity_with_collisions(e, shooter->velocity, e->scale.x * 0.5f + e->scale.y * 0.5f, &respond_jump_shooter_collision, dt);
        
        if (trail_emitter) {
            trail_emitter->position = e->position - e->up * e->scale.y * 0.5f;
            if (!shooter->states.picking_point && shooter->velocity != Vector2_zero) {
                trail_emitter->direction = normalized(shooter->velocity * -1);
            }
        }
        
        if (shooter->states.flying_to_point) {
            if (flying_emitter) {
                flying_emitter->position  = e->position - e->up * e->scale.y * 0.5f;
                if (shooter->velocity != Vector2_zero) {
                    flying_emitter->direction = normalized(shooter->velocity * -1);
                }
            }
        }                
    } // end update jump shooter
    
    if (e->flags & DOOR) {
        update_door(e);
    }
    
    return true;
} //update entity end

void update_entities(Context *context, f32 dt) {
    context->game_time += core.time.fixed_dt;

    // Update turrets ticks.
    context->turret_state.ticked_this_frame = false;
    context->turret_state.tick_countdown -= dt;
    if (context->turret_state.tick_countdown <= 0) {
        // 0.2f it's just arbitrary value for turret tick.
        context->turret_state.tick_countdown += 0.2f;
        context->turret_state.current_tick += 1;
        context->turret_state.ticked_this_frame = true;
    }

    Chunk_Array <Entity> *entities = &current_context->entities;
    
    b32 update_static_entities_collision_cells = editor_state == EDITOR || state_context.in_pause_editor;
    update_all_collision_cells(update_static_entities_collision_cells);        
    
    // for (i32 entity_index = 0; entity_index < entities->capacity; entity_index++) {
    for_chunk_array(entity_index, entities) {
        Entity *e = entities->get(entity_index);
        
        if (e->flags & PLAYER) {
            // @CLEANUP: Why that's here? It could be in free_entity.
            if (need_destroy_player) {
                destroy_player();   
                need_destroy_player = false;
            }
        }
        
        if (maybe_destroy_entity(e)) {
            continue;
        }
                
        if (e->enabled && editor_state == GAME && e->spawn_enemy_when_no_ammo && player_data->ammo_count <= 0 && (/*!current_context->entities.has_key(e->spawned_enemy_id) || */e->spawned_enemy_id == -1)) { 
            Entity *spawned = spawn_object_by_name("ammo_pack", e->position, current_context);
            e->spawned_enemy_id = spawned->id;
        }
        
        if (!e->enabled || (e->hidden && editor_state == GAME && !state_context.in_pause_editor)) {
            continue;
        }
        
        if (e->flags & TRIGGER) {
            verify_trigger_connected(e);
        }
        if (e->flags & KILL_SWITCH) {
            verify_kill_switch_connected(e);
        }
        
        if (editor_state == EDITOR || state_context.in_pause_editor) {
            update_color_changer(e, dt);
            if (editor_state == EDITOR) {
                update_editor_entity(e);
            }
            continue;
        }
    
        // Update_entity_collision_cells(e);.
        if (!update_entity(e, dt)) {
            break;
        }
    } // Update entities end.
}

void move_vec_towards(Vector2 *current, Vector2 target, f32 speed, f32 dt) {
    *current = move_towards(*current, target, speed, dt);
}

Vector2 move_towards(Vector2 current, Vector2 target, f32 speed, f32 dt) {
    Vector2 vec = (target - current);
    f32 len = magnitude(vec);
    Vector2 dir = normalized(vec);
    
    f32 target_move_len = speed * dt;
    if (target_move_len > len) {
        target_move_len = len;
    }
    
    current += dir * target_move_len;
    
    return current;
}

f32 move_towards(f32 current, f32 target, f32 speed, f32 dt) {
    f32 required = target - current;
    
    f32 move_len = speed * dt;
    
    if (move_len > abs(required)) {
        move_len = abs(required);
    }
    
    current += normalized(required) * move_len;
    
    return current;
}

inline void draw_player(Entity *entity) {
    assert(entity->flags & PLAYER);
    
    if (player_data->dead_man) {
        return;
    }
    
    draw_game_triangle_strip(entity);
    
    if (is_death_instinct_in_cooldown()) {
        f32 cooldown_left = state_context.death_instinct.cooldown_start_time + state_context.death_instinct.cooldown - core.time.app_time;
        draw_game_text(entity->position + Vector2_up * 8, tprintf("%.1f", cooldown_left), 44, YELLOW);
        state_context.death_instinct.was_in_cooldown = true;
    } else {
        if (state_context.death_instinct.was_in_cooldown) {
            state_context.death_instinct.was_in_cooldown = false;
            play_sound("ScifyOne", 1.5f);
        }
        
        draw_game_ring_lines(entity->position, entity->scale.y * 1.05f, entity->scale.y * 2.0f, 5, YELLOW, current_context->game_time * 4, current_context->game_time * 2 + 360);
    }
}

inline Vector2 get_perlin_in_circle(f32 speed) {
    return {perlin_noise3_seed(current_context->game_time * speed, 1, 2, rnd(0, 10000)), perlin_noise3_seed(1, current_context->game_time * speed, 3, rnd(0, 10000))};
}

inline Vector2 get_perlin_in_circle(f32 speed, f32 seed1, f32 seed2) {
    return {perlin_noise3_seed(current_context->game_time * speed, seed1, seed2, rnd(0, 10000)), perlin_noise3_seed(seed2, current_context->game_time * speed, seed1, rnd(0, 10000))};
}

inline void draw_sword(Entity *entity) {
    assert(entity->flags & SWORD);
    
    Entity visual_entity = *entity;
    
    Vector2 handle_end = visual_entity.position + visual_entity.up * visual_entity.scale.y * 0.2f;
    draw_game_line(visual_entity.position, handle_end, visual_entity.scale.x * 0.2f, BLACK);
    
    draw_game_triangle_strip(&visual_entity);
}

inline void draw_rifle(Entity *entity) {
    Entity visual_entity = *entity;
    
    f32 time_since_shake = current_context->game_time - player_data->timers.rifle_shake_start_time;
    
    if (time_since_shake <= 0.2f) {
        Vector2 perlin_rnd = {perlin_noise3(current_context->game_time * 30, 1, 2), perlin_noise3(1, current_context->game_time * 30, 3)};
        visual_entity.position += perlin_rnd * 1.8f;
    }
    
    visual_entity.color = ColorBrightness(GREEN, 0.3f);
    
    // Vector2 handle_end = visual_entity.position + visual_entity.up * visual_entity.scale.y * 0.2f;
    // draw_game_line(visual_entity.position, handle_end, visual_entity.scale.x * 0.2f, BLACK);
    
    draw_game_triangle_strip(&visual_entity);
    
    draw_game_line_strip(&visual_entity, WHITE);
}

inline Collision get_ray_collision_to_player(Entity *entity, FLAGS collision_flags, f32 reduced_len) {
    if (!current_context->player_entity) {
        print("WARNING: Tried to get ray collision to player, but player is not present");
        return {0};     
    }
    
    Vector2 vec_to_player = current_context->player_entity->position - entity->position;
    Vector2 dir = normalized(vec_to_player);
    f32 len = magnitude(vec_to_player);
    return raycast(entity->position, dir, len - reduced_len, collision_flags, 6, entity->id);
}

inline void draw_bird_enemy(Entity *entity) {
    assert(entity->flags & BIRD_ENEMY);
    
    Entity visual_entity = *entity;
    if (entity->bird_enemy->charging) {
        f32 charging_time = current_context->game_time - entity->bird_enemy->charging_start_time;
        f32 t = clamp01(charging_time / entity->bird_enemy->max_charging_time);
        visual_entity.position += get_perlin_in_circle(30) * lerp(0.0f, 1.0f, t * t);
    }
    
    draw_game_triangle_strip(&visual_entity);
    // draw_game_line_strip(&visual_entity, RED);
    make_outline(visual_entity.position, visual_entity.vertices, RED);
}

int compare_entities_draw_order(const void *first, const void *second) {
    Entity *entity1 = (Entity*)first;
    Entity *entity2 = (Entity*)second;
    
    if (entity1->draw_order == entity2->draw_order) {
        return 0;
    }
    
    return entity1->draw_order < entity2->draw_order ? 1 : -1;
}

Bounds get_cam_bounds(Cam cam, f32 zoom) {
    Bounds cam_bounds;
    cam_bounds.size = {(f32)screen_width, (f32)screen_height};
    cam_bounds.size /= zoom;
    cam_bounds.size /= current_context->cam.unit_size;
    
    cam_bounds.offset = {0, 0};
    
    return cam_bounds;
}

inline b32 should_not_draw_entity(Entity *e, Cam cam) {
    Bounds cam_bounds = get_cam_bounds(cam, cam.cam2D.zoom);
    return e->will_be_destroyed || !check_bounds_collision(cam.view_position, cam_bounds, e) || !e->enabled;
}

inline f32 get_turret_charge_progress(Turret *turret) {
    Turret_State *state = &current_context->turret_state;
    f32 between_tick_time       = (f32)(turret->shoot_every_tick) * state->seconds_between_ticks;
    f32 from_previous_tick_time = (f32)(state->current_tick - turret->last_shot_tick) * state->seconds_between_ticks + (state->seconds_between_ticks - state->tick_countdown);
    return clamp01(from_previous_tick_time / between_tick_time);
}

void fill_entities_draw_queue() {
    global_data.entities_draw_queue.clear();
    
    Entity *player_entity = current_context->player_entity;
    
    // That also acts entities loop on draw update call. For example we use it for some immediate stuff that should
    // work on occluded entities.
    ForEntities(entity, 0) {
        if (!entity->enabled) {
            continue;
        }
        
        if (entity->hidden && editor_state == GAME && !state_context.in_pause_editor/* && !should_draw_entity_anyway(&e)*/) {
            continue;
        }
        
        // always draw bird
        if (entity->flags & BIRD_ENEMY) { 
            Bird_Enemy *bird = entity->bird_enemy;
            local_persist Color charging_line_color  = Fade(ORANGE, 0.3f);
            local_persist Color attacking_line_color = Fade(RED, 0.6f);
            local_persist f32 charging_line_width = 1.5f;
            local_persist f32 attacking_line_width = 7.0f;
            if (bird->charging && !bird->dead_man) {
                f32 charging_time = current_context->game_time - entity->bird_enemy->charging_start_time;
                f32 t = clamp01(charging_time / entity->bird_enemy->max_charging_time);
                Color attack_line_color = color_fade(charging_line_color, t * t);
                f32 attack_line_width = lerp(0.0f, charging_line_width, t * t);
                Vector2 attack_line_target_position = player_entity->position;
                // @TODO Should make this ray collision check so that line would stop when bird will not fly all the way to player.
                // Will do that when we'll perform collision optimizations.
                // Collision ray_collision = get_ray_collision_to_player(entity, entity->collision_flags, 2);
                // if (ray_collision.collided) {
                //     attack_line_target_position = ray_collision.point;
                //     attack_line_color = color_fade(attack_line_color, 0.5f);
                // }
                make_line(entity->position, attack_line_target_position, attack_line_width, attack_line_color);
            }
            
            if (bird->attacking && !bird->dead_man) {
                f32 attacking_time = current_context->game_time - bird->attack_start_time;
                f32 t = clamp01(attacking_time / bird->max_attack_time);
                
                Color attack_line_color = color_fade(attacking_line_color, (1.0f - t) * (1.0f - t));
                if (t <= 0.1f) {
                    attack_line_color = lerp(charging_line_color, attack_line_color, t * 10);
                }
                
                f32 attack_line_width = 0;
                if (t <= 0.1f) {
                    attack_line_width = lerp(charging_line_width, attacking_line_width, EaseOutElastic(t * 10.0f));
                } else {
                    attack_line_width = lerp(attacking_line_width, 0.5f, EaseOutElastic((t - 0.1f) / 0.9f));
                }
                
                Vector2 target_position = player_entity->position;
                if (dot(target_position - entity->position, entity->up) <= 0) {
                    target_position = entity->position + entity->up * 200;
                    attack_line_color = color_fade(attack_line_color, 0.2f);
                }
                
                make_line(entity->position, target_position, attack_line_width, attack_line_color);
            }
        }
        
        // Always draw turret.
        if (entity->flags & TURRET) {
            Turret *turret = entity->turret;
            if (turret->homing && turret->see_player) {
                f32 t = get_turret_charge_progress(turret);
                
                Color line_color = color_fade(Fade(RED, 0.6f), t);
                f32 line_width = lerp(0.0f, 3.0f, t * t * t);
                
                Vector2 target_position = player_entity->position;
                
                make_line(entity->position + entity->up * entity->scale.y * entity->pivot.y, target_position, line_width, line_color);
            }
            
            if (should_draw_editor_hints() && editor.selected && entity->id == editor.selected->id && turret->homing) {
                // draw_game_circle(entity->position, turret->shoot_radius, Fade(RED, 0.2f));
                draw_game_rect(entity->position, {turret->shoot_width, turret->shoot_height}, {0.5f, 0.5f}, 0, Fade(RED, 0.2f));
            }
        }
        
        // always draw move sequence
        if (entity->flags & MOVE_SEQUENCE && should_draw_editor_hints()) {
            if (entity->move_sequence->speed_related_player_distance && editor.selected && editor.selected->id == entity->id) {
                draw_game_circle(entity->position, entity->move_sequence->max_distance, Fade(RED, 0.05f));
                draw_game_circle(entity->position, entity->move_sequence->min_distance, Fade(BLUE, 0.2f));
            }
                
            for (i32 ii = 0; ii < entity->move_sequence->points.count; ii++) {
                Vector2 point = entity->move_sequence->points.get_value(ii);
                
                Color color = editor.selected && editor.selected->id == entity->id ? ColorBrightness(GREEN, 0.2f) : Fade(BLUE, 0.2f);
                
                if (IsKeyDown(KEY_LEFT_ALT)) {
                    draw_game_circle(point, 1  * (0.4f / current_context->cam.cam2D.zoom), SKYBLUE);
                    draw_game_text(point - Vector2_up, tprintf("%d", ii), 18 / current_context->cam.cam2D.zoom, RED);
                    
                    if (entity->flags & JUMP_SHOOTER) {
                        Collision nearest_ground = get_nearest_ground_collision(point, 20);
                        if (nearest_ground.collided) {
                            Collision ray_collision = raycast(point, normalized(nearest_ground.point - point), magnitude(nearest_ground.point - point), GROUND, 1);
                            if (ray_collision.collided) {
                                make_line(ray_collision.point, ray_collision.point + ray_collision.normal * 5, GREEN);
                            }
                        } else {
                            draw_game_circle(point, 1 * (0.4f / current_context->cam.cam2D.zoom), RED);
                        }
                    }
                }
                if (ii < entity->move_sequence->points.count - 1) {
                    make_line(point, entity->move_sequence->points.get_value(ii+1), color);
                } else if (entity->move_sequence->loop) {
                    make_line(point, entity->move_sequence->points.get_value(0), color);
                }
            }
        }
        
        // always draw explosive
        if (entity->flags & EXPLOSIVE) {
            if (editor_state == EDITOR) {
                draw_game_circle(entity->position, get_explosion_radius(entity), Fade(ORANGE, 0.1f));
            }
        }
        
        // always draw trigger
        if (entity->flags & TRIGGER) {
            Trigger *trigger = entity->trigger;
            if (should_draw_editor_hints()) {
                // draw cam zoom trigger draw trigger zoom draw trigger cam
                if (trigger->settings & CHANGE_ZOOM) {
                    Bounds cam_bounds = get_cam_bounds(current_context->cam, trigger->zoom_value);
                    Vector2 position = entity->position;
                    draw_game_circle(trigger->locked_camera_position, 2, PINK);
                    
                    Color cam_border_color = Fade(PINK, 0.15f);
                    if (editor.selected && editor.selected->id == entity->id) {
                        cam_border_color = Fade(ColorBrightness(PINK, 0.3f), 0.45f);
                    }
                    position = trigger->locked_camera_position;
                    make_rect_lines(position + cam_bounds.offset, cam_bounds.size, {0.5f, 0.5f}, 2.0f / (current_context->cam.cam2D.zoom), cam_border_color);
                    draw_game_text((position + cam_bounds.offset) - cam_bounds.size * 0.5f, tprintf("%.2f", trigger->zoom_value), 18.0f / current_context->cam.cam2D.zoom, ColorBrightness(color_fade(cam_border_color, 1.5f), 0.5f));
                }
                
                if (trigger->settings & (START_CAM_RAILS_HORIZONTAL | START_CAM_RAILS_VERTICAL)) {
                    for (i32 ii = 0; ii < trigger->cam_rails_points.count; ii++) {
                        Vector2 point = trigger->cam_rails_points.get_value(ii);
                        
                        Color color = editor.selected && editor.selected->id == entity->id ? ColorBrightness(WHITE, 0.2f) : ColorBrightness(Fade(WHITE, 0.1f), 0.05f);
                        
                        if (IsKeyDown(KEY_LEFT_ALT)) {
                            draw_game_circle(point, 1  * (0.4f / current_context->cam.cam2D.zoom), SKYBLUE);
                            draw_game_text(point - Vector2_up, tprintf("%d", ii), 18 / current_context->cam.cam2D.zoom, RED);
                        }
                        if (ii < trigger->cam_rails_points.count - 1) {
                            make_line(point, trigger->cam_rails_points.get_value(ii+1), color);
                        } 
                    }
                }
            }
            
            b32 is_trigger_selected = editor.selected && editor.selected->id == entity->id || (IsKeyDown(KEY_LEFT_ALT) && should_draw_editor_hints());
            f32 since_triggered = current_context->game_time - trigger->triggered_time;
            for (i32 ii = 0; ii < trigger->connected.count; ii++) {
                Entity *connected = get_entity(trigger->connected.get_value(ii));
                
                if (!connected) {
                    continue;
                }
                
                if (connected->flags & DOOR && ((entity->flags ^ TRIGGER) > 0 || editor_state != GAME)) {
                    Color color = connected->door.is_open == (trigger->settings & OPEN_DOORS) ? SKYBLUE : ORANGE;
                    f32 width = connected->door.is_open == (trigger->settings & OPEN_DOORS) ? 1.0f : 0.2f;
                    make_line(entity->position, connected->position, width, Fade(ColorBrightness(color, 0.2f), 0.3f));
                } else if (is_trigger_selected && should_draw_editor_hints()) {
                    make_line(entity->position, connected->position, RED);
                }
                
                // Draw trigger lines.
                if (trigger->triggered && since_triggered <= 1.5f) {
                    f32 full_t = since_triggered / 1.5f;
                    Color start_color = Fade(PURPLE, 0);
                    Color target_color = Fade(ColorBrightness(PURPLE, 0.15f), 0.6f);
                    Color line_color;
                    
                    f32 target_thick = 2.0f;
                    f32 thick;
                    if (full_t <= 0.3f) {
                        f32 t = full_t / 0.3f;
                        line_color = lerp(start_color, target_color, t);
                        thick = lerp(0.0f, target_thick, EaseInOutElastic(t));
                    } else {
                        f32 t = (full_t - 0.3f) / 0.7f;
                        line_color = color_fade(target_color, 1.0f - t);
                        thick = lerp(target_thick, 0.0f, t * t);
                    }
                    
                    make_line(entity->position, connected->position, thick, line_color);
                }
            }
            for (i32 ii = 0; ii < trigger->tracking.count; ii++) {
                i32 id = trigger->tracking.get_value(ii);
                Entity *tracked_entity = get_entity(id);
                if (!tracked_entity) {
                    continue;
                }
                                
                if (is_trigger_selected && should_draw_editor_hints()) {
                    make_line(entity->position, tracked_entity->position, GREEN);
                } else if ((trigger->settings & DRAW_LINES_TO_TRACKED) && editor_state != EDITOR) {
                    if ((tracked_entity->flags & ENEMY | CENTIPEDE) && !tracked_entity->union_enemy->dead_man) {
                        make_line(entity->position, tracked_entity->position, 1.0f, Fade(PINK, 0.3f));
                    }
                }
            }
        }
        
        // always draw sticky texture
        if (entity->flags & STICKY_TEXTURE) {
            Sticky_Texture *st = entity->sticky_texture;
            
            f32 lifetime = current_context->game_time - st->birth_time;
            f32 lifetime_t = 0.5f;
            if (st->max_lifetime > EPSILON) {
                lifetime_t = lifetime / st->max_lifetime;
            }
            
            Entity *follow_entity = NULL;
            if (entity->sticky_texture->follow_id > 0) {
                follow_entity = get_entity(entity->sticky_texture->follow_id);
                if (follow_entity->will_be_destroyed) {
                    entity->sticky_texture->follow_id = 0;
                    follow_entity = NULL;
                } else {
                }
            }
            if (follow_entity) {
                // We make lights only for immortal sticky textures - like blocker sign.
                if (st->max_lifetime <= 0) {
                    if (follow_entity->flags & BLOCKER && st->should_draw_texture) {
                        make_light(follow_entity->position, 75, 1, 1.0f, WHITE);
                    }
                    
                    if (follow_entity->flags & SWORD_SIZE_REQUIRED && st->should_draw_texture) {
                        make_light(follow_entity->position, 75, 1.5, 0.7f, follow_entity->union_enemy->big_sword_killable ? ColorBrightness(RED, 0.4f) : BLUE);
                    }
                }
                
                if (st->draw_line && st->need_to_follow && player_entity) {
                    Color line_color = st->line_color;
                    if (follow_entity && follow_entity->flags & ENEMY && st->max_lifetime > 0 && !(follow_entity->flags & SHOOT_STOPER)) {
                        line_color = follow_entity->union_enemy->dead_man ? color_fade(SKYBLUE, 0.3f) : color_fade(RED, 0.3f);
                    }
        
                    Vector2 vec_to_follow = entity->position - player_entity->position;
                    f32 len = magnitude(vec_to_follow);
                    if (len <= st->max_distance || st->max_distance <= 0) {
                        make_line(player_entity->position, entity->position, st->line_width, lerp(line_color, color_fade(line_color, 0), lifetime_t * lifetime_t));
                    }
                }
            }
        }
        
        // Always draw hit booster
        if (entity->flags & HIT_BOOSTER) {
            Vector2 v1 = entity->position + entity->up * entity->scale.y * entity->pivot.y;
            Vector2 v2 = v1 + entity->up * 100;
            
            make_line(v1, v2, 0.1f, color_fade(YELLOW, 0.28f));
        }
        
        // Always draw kill switch.
        if (entity->flags & KILL_SWITCH) {
            Kill_Switch *kill_switch = entity->kill_switch;
            for (i32 i = 0; i < kill_switch->connected.count; i++) {
                Entity *connected = get_entity(kill_switch->connected.get_value(i));                
                if (!connected) {
                    continue;
                }
                
                f32 width = 3.0f;
                Vector2 first = entity->position;
                Vector2 second = {connected->position.x, entity->position.y};
                Vector2 third = connected->position;
                Color color = Fade(YELLOW, 0.7f);
                make_line(first, second, width, color);
                make_line(second, third, width, color);
            }
        }
        
        // This checks for occlusion.
        if (should_not_draw_entity(entity, current_context->cam)) {
            entity->visible = false;
            continue;
        } else {
            entity->visible = true;
        }
        
        global_data.entities_draw_queue.append(*entity);
    }
    
    qsort(global_data.entities_draw_queue.data, global_data.entities_draw_queue.count, sizeof(Entity), compare_entities_draw_order);
}

#define MAX_LINE_STRIP_POINTS 1024

Static_Array <Vector2, MAX_LINE_STRIP_POINTS> line_strip_points;

void draw_spikes(Entity *e, Vector2 side_direction, Vector2 up_direction, f32 width, f32 height) {
    if (drawing_state != CAMERA_DRAWING) {
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
    for (f32 ii = -frequency; ii <= len + frequency; ii += frequency) {
        Vector2 position = start_position + dir * ii + (spike ? vertical_addition : Vector2_zero);
        line_strip_points.append(position);
        spike = !spike;
        
        // Vector2 a = start_position + dir * ii;
        // Vector2 b = a + dir * frequency + vertical_addition;
        // Vector2 c = b + dir * frequency - vertical_addition;
        // draw_game_triangle(c, b, a, e->hidden ? Fade(RED, 0.3f) : RED);
    }
    
    Color color = Fade(e->color, 0.1f);
    if (e->hidden) {
        e->color = color_fade(color, 0.2f);
    }
    
    draw_game_triangle_strip(e, color);
    draw_game_line_strip(line_strip_points.data, line_strip_points.count, e->hidden ? Fade(RED, 0.3f) : RED);
}

inline Vector2 get_shoot_stoper_cross_position(Entity *entity) {
    return entity->position + entity->up * entity->scale.y * 0.85f;
}

inline b32 should_draw_editor_hints() {
    return (editor_state == EDITOR || state_context.in_pause_editor || debug.draw_areas_in_game);
}

void draw_entity(Entity *e) {
    if (!e->enabled || e->will_be_destroyed) {
        return;
    }
    
    if (e->flags & TEXTURE) {
        // draw texture
        i32 exclude_flags = NOTE & AMMO_PACK;
        if (!(e->flags & exclude_flags)) {
            Vector2 position = e->position;
            // draw sticky texture texture
            if (e->flags & STICKY_TEXTURE) {
                if (e->sticky_texture->should_draw_texture) {
                    change_scale(e, (e->sticky_texture->base_size) / fminf(current_context->cam.cam2D.zoom, 0.35f)); 
                    make_texture(e->texture, position, e->scale, e->pivot, e->rotation, Fade(e->color, ((f32)e->color.a / 255.0f) * e->sticky_texture->alpha));
                }
            } else {
                b32 should_draw = true;
                
                // Currently in game mode we don't want to draw textures that goes into light baking as light emitters,
                // just because it messes up the looks.
                if (editor_state == GAME && !state_context.in_pause_editor) {
                    if (e->flags & LIGHT) {
                        assert(e->lights.count > 0);
                        Light *light = get_light(e->lights.get_value(0));
                        if (light->bake_shadows) {
                            should_draw = false;
                        }
                    }
                    // if (e->flags == TEXTURE) {
                    //     should_draw = false;
                    // }
                }
                
                if (should_draw) {
                    draw_game_texture(e->texture, position, e->scale, e->pivot, e->rotation, e->color);
                }
            }
        }
    }
    
    // draw note
    if (e->flags & NOTE) {
        assert(e->note_index != -1);
        Note *note = current_context->notes.get(e->note_index);
        if (editor_state == EDITOR || state_context.in_pause_editor) {
            make_texture(e->texture, e->position, e->scale, e->pivot, e->rotation, e->color);
            // draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
            if (editor.selected && editor.selected->id == e->id || (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_ALT)) || focus_input_field.in_focus && str_contains(focus_input_field.tag, tprintf("%d", e->id))) {
                Vector2 note_size = {screen_width * 0.2f, screen_height * 0.2f};
                i32 content_count = str_len(note->content);
                f32 chars_scaling_treshold = 200 * UI_SCALING;
                if (content_count > chars_scaling_treshold) {
                    note_size *= lerp(1.0f, 2.5f, clamp01(((f32)content_count - chars_scaling_treshold) / (chars_scaling_treshold * 4)));
                }
                if (make_input_field(note->content, world_to_screen_with_zoom(e->position + (cast(Vector2) {e->scale.x * 0.5f, e->scale.y * -0.5f})), note_size, tprintf("note%d", e->id))) {
                    str_copy(note->content, focus_input_field.content);
                }
            }
        } else if (note->draw_in_game && editor_state == GAME) {
            draw_game_text(e->position, note->content, 50, note->in_game_color);
        }
    }
    
    if (e->flags & GROUND || e->flags & PLATFORM || e->flags == 0 || e->flags & PROJECTILE) {
        // draw ground
        if (e->vertices.count > 0) {
            draw_game_triangle_strip(e);
        } else {
            draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
        }
    }
    
    if (e->flags & NO_MOVE_BLOCK) {
        make_outline(e->position, e->vertices, PURPLE);
    }
    
    if (e->flags & DOOR) {
        if (editor.selected && editor.selected->id == e->id) {
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
    
    if (e->flags & BLOCK_ROPE) {
        draw_game_triangle_strip(e);
    }
    
    if (e->flags & ROPE_POINT) {
        draw_game_circle(e->position, e->scale.x * 0.8f, e->color);
    }
    
    if (e->flags & DUMMY) {
        // draw dummy
        if (editor_state == EDITOR || state_context.in_pause_editor) {
            draw_game_triangle_strip(e);
            draw_game_line_strip(e, SKYBLUE);
        }
    }
    
    if (e->flags & PLAYER) {
        // draw player
        draw_player(e);
    }
    
    if (e->flags & SWORD) {
        // draw sword
        draw_sword(e);
    }
    
    if (e->flags & SHOOT_STOPER) {
        // draw shoot stoper
        f32 line_width = e->scale.x * 0.1f;
        Vector2 top = e->position + e->up * e->scale.y * 0.5f;
        Vector2 cross_position = get_shoot_stoper_cross_position(e);
        
        draw_game_line(top, top + e->up * e->scale.y * 0.5f, line_width, BLACK);
        draw_game_line(cross_position - e->right * e->scale.x * 0.6f, cross_position + e->right * e->scale.x * 0.6f, line_width, BLACK);
    }
    
    // draw enemies
    if (e->flags & BIRD_ENEMY) {
        draw_bird_enemy(e);
    } else if (e->flags & CENTIPEDE_SEGMENT) {
        assert(!(e->flags & CENTIPEDE));
        Entity *segment = e;
        Color color = segment->color;
        if (segment->union_enemy->dead_man) {
            //color = Fade(color, 0.3f);
            color = Fade(BLACK, 0.3f);
        }
        draw_game_triangle_strip(segment, color);
        if (e->centipede_segment->head->centipede->spikes_on_right) {
            draw_spikes(segment, segment->up, segment->right, segment->scale.y, segment->scale.x);
        } else {
            if (!segment->union_enemy->dead_man) {
                draw_game_circle(segment->position + segment->right * segment->scale.x * 0.5f, 2.0f, GREEN);
            }
        }
        if (e->centipede_segment->head->centipede->spikes_on_left) {
            draw_spikes(segment, segment->up, segment->right * -1.0f, segment->scale.y, segment->scale.x);
        } else {
            if (!segment->union_enemy->dead_man) {
                draw_game_circle(segment->position - segment->right * segment->scale.x * 0.5f, 2.0f, GREEN);
            }
        }
    } else if (e->flags & CENTIPEDE) {
        assert(e->centipede);
    
        // First draw segments.
        // for_array (i, &e->centipede->segments) {
        //     Entity *segment = e->centipede->segments.get_value(i);
        //     Color color = segment->color;
        //     if (segment->union_enemy->dead_man) {
        //         //color = Fade(color, 0.3f);
        //         color = Fade(BLACK, 0.3f);
        //     }
        //     draw_game_triangle_strip(segment, color);
        //     if (e->centipede->spikes_on_right) {
        //         draw_spikes(segment, segment->up, segment->right, segment->scale.y, segment->scale.x);
        //     } else {
        //         if (!segment->union_enemy->dead_man) {
        //             draw_game_circle(segment->position + segment->right * segment->scale.x * 0.5f, 2.0f, GREEN);
        //         }
        //     }
        //     if (e->centipede->spikes_on_left) {
        //         draw_spikes(segment, segment->up, segment->right * -1.0f, segment->scale.y, segment->scale.x);
        //     } else {
        //         if (!segment->union_enemy->dead_man) {
        //             draw_game_circle(segment->position - segment->right * segment->scale.x * 0.5f, 2.0f, GREEN);
        //         }
        //     }
        // }
    
        // Now draw centipede (head).
        draw_game_triangle_strip(e);
    } else if (e->flags & JUMP_SHOOTER) {
        // draw jump shooter
        
        if (e->jump_shooter->states.charging) {
            f32 charging_time = current_context->game_time - e->jump_shooter->states.charging_start_time;
            f32 charging_t = charging_time / e->jump_shooter->max_charging_time;
            e->position += get_perlin_in_circle(50) * lerp(0.0f, 1.0f, charging_t * charging_t);
        }
        if (e->jump_shooter->states.picking_point) {
            f32 picking_point_time = current_context->game_time - e->jump_shooter->states.picking_point_start_time;
            f32 picking_point_t = picking_point_time / e->jump_shooter->max_picking_point_time;
            e->position += get_perlin_in_circle(50) * lerp(0.0f, 1.0f, picking_point_t * picking_point_t);
        }
        if (e->jump_shooter->states.flying_to_point) {
            e->position += get_perlin_in_circle(25);
        }

        draw_game_triangle_strip(e);
        
        Color hint_color = Fade(ColorBrightness(WHITE, -0.2f), 0.8f);
        Vector2 bullet_hint_position = e->position + e->up * e->scale.y * 0.6f;
        Vector2 bullet_hint_scale = {e->scale.x * 0.7f, e->scale.y * 1.1f};
        
        if (e->jump_shooter->explosive_count > 0) {
            Color target_color = ColorBrightness(WHITE, 4);
            f32 color_t = abs(sinf(current_context->game_time * 3));
            hint_color = lerp(hint_color, target_color, color_t);
            
            if (e->jump_shooter->states.charging) {
                f32 charging_time = current_context->game_time - e->jump_shooter->states.charging_start_time;
                f32 charging_t = charging_time / e->jump_shooter->max_charging_time;
                
                f32 radius = lerp(0.0f, 40.0f, charging_t * charging_t);
                draw_game_circle(bullet_hint_position + e->up * bullet_hint_scale.y * 0.5f, radius, Fade(ORANGE, 0.1f));
            }
        }
        
        draw_game_texture(jump_shooter_bullet_hint_texture, bullet_hint_position, bullet_hint_scale, {0.5f, 1.0f}, e->rotation, hint_color);
        
        if (e->jump_shooter->shoot_bullet_blockers) {
            draw_game_ring_lines(bullet_hint_position + e->up * e->scale.y * 0.5f, 3, 6, 8, Fade(WHITE, 0.5f));                
        }
        
        if (e->jump_shooter->shoot_sword_blockers) {
            if (e->jump_shooter->shoot_sword_blockers_immortal) {
                Vector2 center = bullet_hint_position + e->up * bullet_hint_scale.y * 0.5f;
                Vector2 triangle1 = {center.x, center.y + 5};
                Vector2 triangle2 = {center.x - 5, center.y - 5};
                Vector2 triangle3 = {center.x + 5, center.y - 5};
                draw_game_triangle_lines(triangle1, triangle2, triangle3, WHITE);
            } else {
                Vector2 scale = {3, 3};
                scale /= current_context->cam.cam2D.zoom;
                Texture blocker_texture = e->jump_shooter->blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
                make_texture(blocker_texture, bullet_hint_position + e->up * scale.y * 0.65f, scale, {0.5f, 0.5f}, 0, WHITE);
            }
        }
    } else if (e->flags & AMMO_PACK) {
        // draw_game_circle(e->position, e->scale.x, RED);
        // draw_game_circle(e->position, e->scale.x * 0.5f, ColorBrightness(RED, 0.6f));
        draw_game_texture(e->texture, e->position, e->scale * 2, e->pivot, e->rotation, WHITE);
    } else if (e->flags & TURRET) { // draw turret
        Color color = e->color;
        if (!e->turret->activated) {
            color = ColorBrightness(color, -0.4f);
        }
        draw_game_triangle_strip(e, color);
        if (e->turret->homing && e->turret->activated) {
            // Drawing charge line
            Vector2 start = e->position - e->up * e->scale.y * (1.0f - e->pivot.y); 
            f32 length = e->scale.y * e->pivot.y;
            if (editor_state == GAME && !state_context.in_pause_editor) {
                f32 t = get_turret_charge_progress(e->turret);
                length = lerp(0.0f, length, t * t * t);
            }
            draw_game_line(start, start + e->up * length, e->scale.x * 0.2f, RED);
        }
    } else if (e->flags & ENEMY && e->flags & TRIGGER) {
        Color color = e->color;
        if (e->union_enemy->dead_man) {
            color = ColorTint(color, ColorBrightness(BROWN, 0.15f));
            color = ColorBrightness(color, 0.1f);
            // color = color_fade(color, 0.6f);
        }
        draw_game_triangle_strip(e, color);
    } else if (e->flags & WIN_BLOCK) {
        // draw win block  
        draw_game_triangle_strip(e, ColorBrightness(GREEN, -0.2f));
    } else if (e->flags & ENEMY) { // Draw enemy.
        draw_game_triangle_strip(e);
    }
    
    // draw enemy barrier
    if (e->flags & ENEMY_BARRIER) {
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
    
    // draw player touch timer
    if (e->flags & PLAYER_TOUCH_TIMER) {
        f32 width = fmaxf(e->scale.x * 0.5f, 10.0f);
        f32 height = 5;
        Vector2 position = e->position - Vector2_up * 5 - Vector2_right * width * 0.5f;
        draw_game_rect(position, {width, height}, {0, 0}, 0, Fade(BROWN, 0.9f));
        
        auto touch = &e->union_enemy->player_touch_timer;
        f32 progress = touch->touched_timer / touch->seconds_until_death;
        width *= progress;
        draw_game_rect(position, {width, height}, {0, 0}, 0, PINK);
    }

    if (e->flags & SPIKES && (!e->hidden || editor_state == EDITOR || state_context.in_pause_editor)) {
        draw_spikes(e, e->right, e->up, e->scale.x, e->scale.y);
    }
    
    if (e->flags & PLATFORM) {
        // draw platform
        line_strip_points.clear();
        f32 frequency = 6;
        Vector2 start_position = e->position - e->right * e->scale.x * 0.5f + e->up * e->scale.y * 0.5f;
        Vector2 end_position   = e->position + e->right * e->scale.x * 0.5f + e->up * e->scale.y * 0.5f;
        
        Vector2 vertical_removal = e->up * e->scale.y * -0.35f;
        
        Vector2 vec = end_position - start_position;
        Vector2 dir = normalized(vec);
        f32 len = magnitude(vec);
        
        line_strip_points.append(start_position - e->up * e->scale.y);
        
        for (f32 ii = 0; ii <= len - frequency * 0.8f; ii += frequency * 0.8f) {
            line_strip_points.append(start_position + dir * ii);
            line_strip_points.append(start_position + dir * (ii + frequency * 0.8f));
            line_strip_points.append(start_position + dir * (ii + frequency * 0.9f) + vertical_removal);
            line_strip_points.append(start_position + dir * (ii + frequency));
            
            ii += frequency * 0.2f;
        }
        
        line_strip_points.append(end_position - e->up * e->scale.y);
        
        draw_game_line_strip(line_strip_points.data, line_strip_points.count, BROWN);
    }
    
    // draw trigger
    if (e->flags & TRIGGER) {
        if (should_draw_editor_hints()) {
            draw_game_line_strip(e, e->color);
            draw_game_triangle_strip(e, Fade(e->color, 0.1f));
        }
    }
    
    if (e->flags & EXPLOSIVE) {
    }
    
    if (e->flags & PROPELLER && (editor_state == EDITOR || state_context.in_pause_editor || debug.draw_areas_in_game)) {
        draw_game_line_strip(e, e->color);
        draw_game_triangle_strip(e, e->color * 0.1f);
    }
    
    if (e->flags & BLOCKER && (editor_state == EDITOR) && !e->union_enemy->blocker_immortal) {
        Texture texture = e->union_enemy->blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
        
        draw_game_texture(texture, e->position, {10.0f, 10.0f}, {0.5f, 0.5f}, 0, Fade(WHITE, 0.6f));
    }
    if (e->flags & BLOCKER && e->union_enemy->blocker_immortal) {
        Vector2 triangle1 = {e->position.x, e->position.y + 3};
        Vector2 triangle2 = {e->position.x - 3, e->position.y - 3};
        Vector2 triangle3 = {e->position.x + 3, e->position.y - 3};
        draw_game_triangle_lines(triangle1, triangle2, triangle3, WHITE);
    }
    
    if (e->flags & SWORD_SIZE_REQUIRED && (editor_state == EDITOR)) {
        Texture texture = e->union_enemy->big_sword_killable ? big_sword_killable_texture : small_sword_killable_texture;
        
        draw_game_texture(texture, e->position, {10.0f, 10.0f}, {0.5f, 0.5f}, 0, WHITE);
    }

    
    if (e->flags & SHOOT_BLOCKER) {
        // draw shoot blockers
        if (e->union_enemy->shoot_blocker_immortal) {
            draw_game_ring_lines(e->position, 3, 6, 8, WHITE);                
        } else {
            Vector2 direction = get_rotated_vector(e->union_enemy->shoot_blocker_direction, e->rotation);
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
}

void draw_entities() {
    fill_entities_draw_queue();

    //Hash_Table_Int<Entity> *entities = &current_context->entities;
    Array <Entity> *entities = &global_data.entities_draw_queue;
    
    for (i32 entity_index = 0; entity_index < entities->count; entity_index++) {
        Entity *e = entities->get(entity_index);
        
        if (e->destroyed || e->will_be_destroyed) {
            continue;
        }
        
        if (editor_state == GAME && !global_data.updated_today) {
            //
            // To do that we must know globally that it's imaginary update and don't do anything stupid on this update.
            // We don't want to: React to player actions besides movement. Kill player.
            // Kill other guys (even if this is imaginary entities). Spawn entities. Spawn particles. Spawn anything actually.
            // Play sound. Do screenshake. We don't want to do anything outside scope of this particular entity. Load level.
            // It will be simpler if we just put these checks in functions that do something outside scope of entity.
            // Like when we call play_sound we'll check in there that it's not real guys.
            //
            // Update_entity(e, core.time.not_updated_accumulated_dt);.
            e->position += get_entity_velocity(e) * core.time.not_updated_accumulated_dt;
            if (e->flags & SWORD) {
                // rotate(e, player_data->sword_angular_velocity * core.time.not_updated_accumulated_dt);
            }
            
            if (e->flags & STICKY_TEXTURE) {
                update_sticky_texture(e, core.time.not_updated_accumulated_dt);
            }
        }
        
        draw_entity(e);
    }
    
    if (game_state == GAME_PLANNING) {
        planning_draw(current_context);
    }
}

void draw_game_space_editor() {
    Chunk_Array <Entity> *entities = &current_context->entities;

    for_chunk_array(i, entities) {
        Entity *e = entities->get(i);
        
        if (!e->enabled || e->flags == -1) {
            continue;
        }
        
        draw_game_circle(current_context->player_spawn_point, 3, BLUE);
        
        b32 draw_circles_on_vertices = IsKeyDown(KEY_LEFT_ALT);
        // draw vertices
        if (draw_circles_on_vertices) {
            for (i32 v = 0; v < e->vertices.count; v++) {
                Vector2 global_vertex_position = global_position(e, e->vertices.get_value(v));
                if (editor.selected && editor.selected->id == e->id) {
                    const char *text = v == 0 ? "T" : (v == 1 ? "Y" : (v == 2 ? "F" : "G"));
                    draw_game_text(global_vertex_position, text, 22.0f / current_context->cam.cam2D.zoom, YELLOW);
                }
                draw_game_circle(global_vertex_position, 1.0f * (0.4f / current_context->cam.cam2D.zoom), PINK);
                //draw unscaled vertices
                if (IsKeyDown(KEY_LEFT_SHIFT)) {    
                    draw_game_circle(global_position(e, e->unscaled_vertices.get_value(v)), 1.0f * 0.4f, PURPLE);
                }
            }
        }
        
        if (debug.draw_position) {
            draw_game_text(e->position + (cast(Vector2) {0, -3}), tprintf("POS:   {%.2f, %.2f}", e->position.x, e->position.y), 20, RED);
        }
        
        if (debug.draw_rotation) {
            draw_game_text(e->position, tprintf("%d", (i32)e->rotation), 20, RED);
        }
        
        if (debug.draw_scale) {
            draw_game_text(e->position + (cast(Vector2) {0, -6}), tprintf("SCALE:   {%.2f, %.2f}", e->scale.x, e->scale.y), 20, RED);
        }
        
        if (debug.draw_directions) {
            draw_game_text(e->position + (cast(Vector2) {0, -6}), tprintf("UP:    {%.2f, %.2f}", e->up.x, e->up.y), 20, RED);
            draw_game_text(e->position + (cast(Vector2) {0, -9}), tprintf("RIGHT: {%.2f, %.2f}", e->right.x, e->right.y), 20, RED);
        }
        
        b32 draw_normals = false;
        if (draw_normals) {
            global_normals.clear();
            fill_arr_with_normals(&global_normals, e->vertices);
            
            for (i32 n = 0; n < global_normals.count; n++) {
                Vector2 start = e->position + global_normals.get_value(n) * 4; 
                Vector2 end   = e->position + global_normals.get_value(n) * 8; 
                make_line(start, end, 0.5f, PURPLE);
                draw_game_rect(end, {1, 1}, {0.5f, 0.5f}, PURPLE * 0.9f);
            }
        }
        
        // Draw direction lines.
        if ((editor_state == EDITOR || debug.draw_up_right) && editor.selected && editor.selected->id == e->id) {
            make_line(e->position, e->position + e->right * 3, RED);
            make_line(e->position, e->position + e->up    * 3, GREEN);
        }
        
        if (debug.draw_bounds || editor.selected && (editor_state == EDITOR || state_context.in_pause_editor) && e->id == editor.selected->id) {
            make_rect_lines(e->position + e->bounds.offset, e->bounds.size, e->pivot, 1.0f / current_context->cam.cam2D.zoom, GREEN);
        }
    }
    
    //editor ruler drawing
    if (editor.ruler_active) {
        make_line(editor.ruler_start_position, input.mouse_position, 0.3f, BLUE * 0.9f);
        Vector2 vec_to_mouse = input.mouse_position - editor.ruler_start_position;
        f32 length = magnitude(vec_to_mouse);
        
        draw_game_text(editor.ruler_start_position + (vec_to_mouse * 0.5f), tprintf("%.2f", length), 24.0f / current_context->cam.cam2D.zoom, RED);
        draw_game_text(input.mouse_position + Vector2_up, tprintf("{%.2f, %.2f}", input.mouse_position.x, input.mouse_position.y), 26.0f / current_context->cam.cam2D.zoom, GREEN); 
        
    }
    
    draw_game_lightmap_editing();
} // draw game space editor end

void draw_particles() {
    //@TODO: When we'll start considering particles draw order we'll want this to work with entity drawing.
    for (int emitter_index = 0; emitter_index < current_context->particle_emitters.capacity; emitter_index++) {
        Particle_Emitter *emitter = current_context->particle_emitters.get(emitter_index);
        //@OPTIMIZATION: Make emitter occlusion culling.
        if (!emitter->occupied/* || !emitter.visible*/) {
            continue;              
        }
        
        for (i32 emitter_index = emitter->particles_start_index; emitter_index < emitter->particles_max_index; emitter_index++) {
            Particle particle = current_context->particles.get_value(emitter_index);
            if (!particle.enabled) {
                continue;   
            }
            
            switch (particle.shape) {
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
            
            if (particle.line_trail_index != -1) {
                Line_Trail *line_trail = get_line_trail(particle.line_trail_index);
                if (line_trail && line_trail->occupied) {
                    // We go from start index to count becasuse line trail could loop-into-itself. 
                    // For first max_count points we just add points to there will be usual array situation, but after
                    // new trail points starts to occupy start_index position and we increasing start_index so it's 
                    // working somewhat like ring buffer. Maybe it's weird. If after some time when i read this i did not 
                    // understand this code from first 20 seconds - it's really weird.
                    i32 drawed_count = 0;
                    for (i32 i = line_trail->start_index; i < line_trail->positions.count + line_trail->start_index - 1; i++) {
                            drawed_count += 1;
                            i32 index  = (i % LINE_TRAIL_MAX_POINTS);
                            f32 color_t = clamp01((f32)drawed_count / (f32)line_trail->positions.count);
                            draw_game_line(line_trail->positions.get_value(index), line_trail->positions.get_value((index + 1) % LINE_TRAIL_MAX_POINTS), 0.5f, lerp(Fade(particle.color, 0), particle.color, color_t));
                    }
                    
                    i32 last_index = (line_trail->start_index + line_trail->positions.count - 1) % LINE_TRAIL_MAX_POINTS;
                    draw_game_line(line_trail->positions.get_value(last_index), particle.position, 0.5f, particle.color);
                } else {
                    printf("WARNING: For some reason particle have line trail index, but we could not get line trail by that index: %d (Or it's don't occupied)\n", particle.line_trail_index);
                }
            }
        }
    }
}

void draw_ui(const char *tag) {
    // draw spin bar
    if (editor_state == GAME) {
        if (player_data->timers.slowmo_timer > 0) {
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
                      
            for (i32 i = 0; i < player_data->max_big_sword_charges; i++, horizontal += spacing) {
                Color color = i < player_data->current_big_sword_charges ? ColorBrightness(GREEN, 0.2f) : Fade(BROWN, 0.3f);
                draw_rect({horizontal, vertical}, {width, height}, {0, 0}, 0, color);
            }
        }
        
        if (game_state == GAME_PLANNING) {
            planning_draw_ui(current_context);
        }
    }

    // Draw speedrun info after last level
    if (state_context.we_got_a_winner) {
        // Old::make_ui_text(
    }
    
    i32 tag_len = str_len(tag);

    for (i32 i = 0; i < Old::ui_context.elements.count; i++) {
        Old::Ui_Element element = Old::ui_context.elements.get_value(i);
        
        if (tag_len > 0 && !str_equal(element.tag, tag)) {
            continue;
        }
        
        if (element.ui_flags & Old::UI_IMAGE) {
            Old::Ui_Image ui_image = element.ui_image;
            
            if (element.has_texture) {
                element.texture.width = element.size.x;
                element.texture.height = element.size.y;
                draw_texture(element.texture, element.position, {1, 1}, element.pivot, 0, element.color);
            } else {
                draw_rect(element.position, element.size, element.pivot, 0, element.color);
            }
        }
    }
    for (i32 i = 0; i < Old::ui_context.elements.count; i++) {
        Old::Ui_Element element = Old::ui_context.elements.get_value(i);
        
        if (tag_len > 0 && !str_equal(element.tag, tag)) {
            continue;
        }
        
        if (element.ui_flags & Old::BUTTON) {
            Old::Button button = element.button;
            
            draw_rect(element.position, element.size, element.pivot, 0, element.color);
            
            if (element.ui_flags & Old::UI_TOGGLE && element.toggle_value) {
                Vector2 down_pos = element.position + Vector2_up * element.size.y + Vector2_right * element.size.x * 0.5f;
                draw_line(element.position, down_pos, WHITE);
                draw_line(down_pos, down_pos + Vector2_right * element.size.x * 0.4f - Vector2_up * element.size.y * 0.9f, WHITE);
            }
            
            if (element.ui_flags & Old::UI_COLOR_PICKER) {
                draw_rect(element.position, element.size, element.pivot, 0, element.color);
                
                if (element.toggle_value) {
                    draw_rect_lines(element.position, element.size * 1.1f, element.size.x * 0.075f, element.color == WHITE ? PINK : WHITE);
                }
            }
        }
        
        if (element.ui_flags & Old::UI_TEXT) {
            Old::Ui_Text ui_text = element.text;
            draw_text(ui_text.content, element.position, ui_text.font_size, ui_text.text_color);
        }
    }

    for (i32 i = 0; i < input_fields.count; i++) {
        Input_Field input_field = input_fields.get_value(i);
        
        if (tag_len > 0 && !str_equal(input_field.tag, tag)) {
            continue;
        }
        
        Color background_color = Fade(input_field.color, 0.6f);
        if (input_field.in_focus) {
            background_color = Fade(ColorTint(background_color, ColorBrightness(SKYBLUE, 0.2f)), 0.7f);
        }
        draw_rect(input_field.position, input_field.size, {0, 0}, 0, background_color);
        
        Vector2 field_position = input_field.position + Vector2_right * 3;
        Rectangle field_rec = {field_position.x, field_position.y, input_field.size.x, input_field.size.y};
        if (input_field.in_focus) {
            draw_text_boxed(tprintf("%s_", input_field.content), field_rec, input_field.font_size, 3, WHITE * 0.9f);
        } else {
            draw_text_boxed(input_field.content, field_rec, input_field.font_size, 3, WHITE * 0.9f);
        }
    }
    
    if (tag_len == 0) {
        Old::ui_context.elements.clear();
        input_fields.clear();
    }
}

inline b32 should_add_immediate_stuff() {
    return drawing_state == CAMERA_DRAWING && !debug.view_only_lightmaps;
}

void make_texture(Texture texture, Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color) {
    if (!should_add_immediate_stuff()) {
        return;
    }
    Immediate_Texture im_texture = {0};
    im_texture.texture  = texture;
    im_texture.position = position;
    im_texture.scale    = scale;
    im_texture.pivot    = pivot;
    im_texture.rotation = rotation;
    im_texture.color    = color;
    
    render.textures_to_draw.append(im_texture);
}

void make_line(Vector2 start_position, Vector2 target_position, f32 thick, Color color, f32 lifetime) {
    if (!should_add_immediate_stuff()) {
        return;
    }
    Line line = {0};
    line.start_position = start_position;
    line.target_position = target_position;
    line.color = color;
    line.thick = thick;
    line.lifetime = lifetime;
    
    if (line.lifetime <= 0) {
        render.lines_to_draw.append(line);
    } else {
        render.lines_to_draw_persistent.append(line);
    }
}

inline void make_line(Vector2 start_position, Vector2 target_position, Color color) {
    if (!should_add_immediate_stuff()) {
        return;
    }
    make_line(start_position, target_position, 0, color);
}   

void make_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color) {
    if (!should_add_immediate_stuff()) {
        return;
    }
    Rect_Lines rect = {0};
    rect.position = position;
    rect.scale = scale;
    rect.pivot = pivot;
    rect.thick = thick;
    rect.color = color;
    render.rect_lines_to_draw.append(rect);
}

inline void make_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, Color color) {
    if (!should_add_immediate_stuff()) {
        return;
    }
    make_rect_lines(position, scale, pivot, 0, color);
}

inline void make_outline(Vector2 position, Static_Array <Vector2, MAX_VERTICES> vertices, Color color) {
    if (!should_add_immediate_stuff()) {
        return;
    }
    
    Outline outline = {0};
    outline.position = position;
    outline.vertices = vertices;
    outline.color = color;
    render.outlines_to_draw.append(outline);
}

void make_ring_lines(Vector2 center, f32 inner_radius, f32 outer_radius, i32 segments, Color color) {
    if (!should_add_immediate_stuff()) {
        return;
    }
    Ring_Lines ring = {0};
    ring.center = center;
    ring.inner_radius = inner_radius;
    ring.outer_radius = outer_radius;
    ring.segments = segments;
    ring.color = color;
    render.ring_lines_to_draw.append(ring);
}

void draw_screen_space_editor() {
    // draw logs
    f32 add_vertical_position = screen_height * 0.03f;
    f32 v = screen_height * 0.05f;
    f32 h = screen_width * 0.35f;
    for (i32 i = debug.log_messages_short.count - 1; i >= 0; i--) {
        Log_Message *log = debug.log_messages_short.get(i);
        
        f32 lifetime = core.time.app_time - log->birth_time;
        
        if (lifetime >= 3.0f) {
            debug.log_messages_short.remove(i);
            // i++;
            continue;
        }
        
        draw_text(log->data, {h, v}, 26, ColorBrightness(BROWN, 0.3f));
        v += add_vertical_position;
    }
    
    bool draw_horizontal_vertical_lines = true;
    if (draw_horizontal_vertical_lines && editor.selected) {
        Vector2 screen_position = world_to_screen_with_zoom(editor.selected->position);
        draw_line({(f32)0, screen_position.y}, {(f32)screen_width, screen_position.y}, Fade(RED, 0.5f));
        draw_line({screen_position.x, (f32)0}, {screen_position.x, (f32)screen_height}, Fade(GREEN, 0.5f));
    }
}

void draw_immediate_stuff() {
    for (i32 i = 0; i < render.lines_to_draw.count; i++) {
        Line line = render.lines_to_draw.get_value(i);
        if (line.thick == 0) {
            draw_game_line(line.start_position, line.target_position, line.color);
        } else {
            draw_game_line(line.start_position, line.target_position, line.thick, line.color);
        }
    }
    
    for (i32 i = 0; i < render.ring_lines_to_draw.count; i++) {
        Ring_Lines ring = render.ring_lines_to_draw.get_value(i);
        draw_game_ring_lines(ring.center, ring.inner_radius, ring.outer_radius, ring.segments, ring.color);
    }
    
    for (i32 i = 0; i < render.rect_lines_to_draw.count; i++) {
        Rect_Lines rect = render.rect_lines_to_draw.get_value(i);
        if (rect.thick == 0) {
            draw_game_rect_lines(rect.position, rect.scale, rect.pivot, rect.color);
        } else {
            draw_game_rect_lines(rect.position, rect.scale, rect.pivot, rect.thick, rect.color);
        }
    }
    
    for (i32 i = 0; i < render.textures_to_draw.count; i++) {
        Immediate_Texture im_texture = render.textures_to_draw.get_value(i);
        draw_game_texture(im_texture.texture, im_texture.position, im_texture.scale, im_texture.pivot, im_texture.rotation, im_texture.color);
    }
    
    for (i32 i = 0; i < render.outlines_to_draw.count; i++) {
        Outline outline = render.outlines_to_draw.get_value(i);
        // Obviously should make this a real outlines and not line strip.
        draw_game_line_strip(outline.position, outline.vertices, outline.color);
    }
    
    if (!debug.drawing_stopped) {
        render.lines_to_draw.clear();
        render.ring_lines_to_draw.clear();
        render.rect_lines_to_draw.clear();
        render.textures_to_draw.clear();
        render.outlines_to_draw.clear();
    }
    
    for_array_backwards (i, &render.lines_to_draw_persistent) {
        Line *line = render.lines_to_draw_persistent.get(i);
        
        line->lifetime_timer += core.time.dt;
        f32 t = clamp01(line->lifetime_timer / line->lifetime);
        
        Color color = lerp(line->color, Fade(line->color, 0), t * t * t);
        f32 thick = lerp(line->thick, 0.0f, EaseInOutQuad(t));
        
        draw_game_line(line->start_position, line->target_position, thick, color);
        
        if (t >= 1) {
            render.lines_to_draw_persistent.remove(i);
        }
    }
}

void apply_shake() {
    if (state_context.cam_state.trauma <= 0) {    
        return;
    }
    
    f32 x_shake_power = 10;
    f32 y_shake_power = 7;
    f32 x_shake_speed = 7;
    f32 y_shake_speed = 10;
    
    f32 x_offset = perlin_noise3(current_context->game_time * x_shake_speed, 0, 1) * x_shake_power;
    f32 y_offset = perlin_noise3(0, current_context->game_time * y_shake_speed, 2) * y_shake_power;
    
    current_context->cam.position += (cast(Vector2) {x_offset, y_offset}) * state_context.cam_state.trauma * state_context.cam_state.trauma;
}

void new_render() {
    Entity *player_entity = current_context->player_entity;

    bake_lightmaps_if_need();

    // Drawing baked lightmaps on camera plane.
    BeginTextureMode(global_illumination_rt); {
    BeginMode2D(current_context->cam.cam2D); {
        ClearBackground(BLACK);
        for (i32 lightmap_index = 0; lightmap_index < current_context->lightmaps.count; lightmap_index++) {
            Lightmap_Data *lightmap_data = current_context->lightmaps.get(lightmap_index);
            Texture lightmap_texture = lightmap_data->lightmap_texture;
            
            if (!lightmap_data->has_loaded_texture) {
                lightmap_texture = lightmap_data->global_illumination_rt.texture;
            }
    
            draw_game_texture(lightmap_texture, lightmap_data->position, lightmap_data->game_size, {0.5f, 0.5f}, 0,  WHITE, false);
        }
    } EndMode2D();
    } EndTextureMode();

    update_dynamic_lights();
    draw_dynamic_lights(&global_illumination_rt);

    if (IsKeyPressed(KEY_F1)) {
        debug_toggle_lightmap_view();
    }
    
    BeginTextureMode(render.main_render_texture); {
    BeginMode2D(current_context->cam.cam2D); {
        Color base_background_color = debug.full_light ? ColorBrightness(GRAY, 0.1f) : Fade(WHITE, 0);
        
        ClearBackground(is_explosion_trauma_active() ? (player_data->dead_man ? RED : WHITE) : base_background_color);
        
        drawing_state = CAMERA_DRAWING;
        draw_entities();
        draw_particles();
        
        if (player_entity && debug.draw_player_collisions) {
            for (i32 i = 0; i < collisions_buffer.count; i++) {
                Collision col = collisions_buffer.get_value(i);
                
                make_line(col.point, col.point + col.normal * 4, 0.2f, GREEN);
                draw_game_rect(col.point + col.normal * 4, {1, 1}, {0.5f, 0.5f}, 0, GREEN * 0.9f);
            }
        }
        
        if (debug.draw_collision_grid) {
            // draw collision grid
            Collision_Grid grid = current_context->collision_grid;
            Vector2 player_position = player_entity ? player_entity->position : current_context->player_spawn_point;
            
            // Update_entity_collision_cells(&mouse_entity);.
            for (f32 row = -grid.size.y * 0.5f + grid.origin.y; row <= grid.size.y * 0.5f + grid.origin.y; row += grid.cell_size.y) {
                for (f32 column = -grid.size.x * 0.5f + grid.origin.x; column <= grid.size.x * 0.5f + grid.origin.x; column += grid.cell_size.x) {
                    Collision_Grid_Cell *cell = get_collision_cell_from_position({column, row});
                    
                    draw_game_rect_lines({column, row}, grid.cell_size, {0, 1}, 0.5f / current_context->cam.cam2D.zoom, (cell && (cell->dynamic_entities.count > 0 || cell->static_entities.count > 0)) ? GREEN : RED);
                }
            }
        }
    } EndMode2D();
    } EndTextureMode();
    
    drawing_state = CAMERA_DRAWING;
    if (debug.view_only_lightmaps) {
        BeginMode2D(current_context->cam.cam2D);
        for (i32 lightmap_index = 0; lightmap_index < current_context->lightmaps.count; lightmap_index++) {
            Lightmap_Data *lightmap_data = current_context->lightmaps.get(lightmap_index);
            
            Texture lightmap_texture = lightmap_data->lightmap_texture;
            
            if (current_context->lightmaps_render_textures_loaded) {
                lightmap_texture = lightmap_data->global_illumination_rt.texture;
            }
            
            draw_game_texture(lightmap_texture, lightmap_data->position, lightmap_data->game_size, {0.5f, 0.5f}, 0,  WHITE, false);
            draw_game_texture(lightmap_data->normal_rt.texture, lightmap_data->position, lightmap_data->game_size, {0.5f, 0.5f}, 0,  WHITE, true);
        }
        EndMode2D();
    } else if (debug.full_light) {
        draw_render_texture(render.main_render_texture.texture, {1, 1}, WHITE);
    } else {
        ClearBackground(Fade(BLACK, 0));
        BeginBlendMode(BLEND_ADDITIVE);
        BeginShaderMode(env_light_shader); {
            local_persist i32 gi_data_loc = get_shader_location(env_light_shader, "u_gi_data");
            set_shader_value_tex(env_light_shader, gi_data_loc, global_illumination_rt.texture);
            
            draw_render_texture(render.main_render_texture.texture, {1, 1}, WHITE);
        } EndShaderMode();
        EndBlendMode();
    }
} // new render end

void draw_game() {
    saved_cam = current_context->cam;

    apply_shake();
    
    with_shake_cam = current_context->cam;
    BeginDrawing();

    new_render();
        
    update_input_field();
    
    BeginMode2D(current_context->cam.cam2D); {
        if (editor_state == EDITOR || state_context.in_pause_editor) {
            draw_game_space_editor();
        }
        draw_immediate_stuff();
    } EndMode2D();
    
    draw_screen_space_editor();
    
    if (state_context.we_got_a_winner) {
        // Old::make_ui_text("Finale for now!\nNow you can try speedruns.\nOpen console with \"/\" (slash) button and type help.\ngame_speedrun for full game speedrun.\nlevel_speedrun for current level speedrun.\nfirst for loading first level\nnext for loading next level", {screen_width * 0.3f, screen_height * 0.2f}, 20, GREEN, "win_speedrun_text");
    }
    
    if (editor_state == GAME && player_data->dead_man && !state_context.we_got_a_winner) {
        f32 since_died = current_context->game_time - player_data->timers.died_time;
        
        f32 t = clamp01((since_died - 3.0f) / 2.0f);
        Old::make_ui_text("T - restart", {screen_width * 0.45f, screen_height * 0.45f}, 40, Fade(GREEN, t * t), "restart_text");
    }
    if (state_context.we_got_a_winner && editor_state == GAME) {
        Old::make_ui_text("V - next", {screen_width * 0.45f, screen_height * 0.45f}, 40, Fade(GREEN, .6f), "next_text");
    }

    
    draw_ui("");
    
    current_context->cam = saved_cam;
    
    if (editor_state == EDITOR || state_context.in_pause_editor) {
        make_lightmap_settings_panel();
    }
    
    f32 v_pos = 10;
    f32 font_size = 18;
    if (debug.info_fps) {
        draw_text(tprintf("FPS: %d", GetFPS()), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (editor_state == GAME && current_context->player_entity) {
        if (debug.info_blood_progress) {
            draw_text(tprintf("Blood progress: %.2f", player_data->blood_progress), 10, v_pos, font_size, RED);
            v_pos += font_size;
        }
    }
    
    if (debug.info_particle_count) {
        draw_text(tprintf("Particles count: %d", enabled_particles_count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (debug.info_player_speed) {
        draw_text(tprintf("Player speed: %.1f", magnitude(player_data->velocity)), 10, v_pos, font_size, RED);
        v_pos += font_size;
        draw_text(tprintf("Player Velocity: {%.1f, %.1f}", player_data->velocity.x, player_data->velocity.y), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    v_pos += font_size;
    draw_text(tprintf("Ammo: %d", player_data->ammo_count), 10, v_pos, font_size * 1.5f, VIOLET);
    v_pos += font_size * 1.5f;

    
    draw_console();
    
    // draw cursor
    if (editor_state == GAME) {
        draw_line(input.screen_mouse_position - Vector2_right * 10 - Vector2_up * 10, input.screen_mouse_position + Vector2_right * 10 + Vector2_up * 10, WHITE);
        draw_line(input.screen_mouse_position + Vector2_right * 10 - Vector2_up * 10, input.screen_mouse_position - Vector2_right * 10 + Vector2_up * 10, WHITE);
        draw_rect({input.screen_mouse_position.x - 2.5f, input.screen_mouse_position.y - 2.5f}, {5, 5}, GREEN);
    } else {
        draw_circle({input.screen_mouse_position.x, input.screen_mouse_position.y}, 20, Fade(RED, 0.1f));
        draw_rect({input.screen_mouse_position.x - 5, input.screen_mouse_position.y - 5}, {10, 10}, WHITE);
    }
    
    EndDrawing();
}

void setup_color_changer(Entity *entity) {
    entity->color_changer.start_color = entity->color;
    entity->color_changer.target_color = Fade(ColorBrightness(entity->color, 0.5f), 0.5f);
}

Entity *copy_and_add_entity(Entity *to_copy, Context *context_for_deep_copy, i32 id_to_insert) {
    // On calling copy_entity we're always doing a deep copy and adding entity to the entities array because we're cannot 
    // Init entity without it being inside a entity array because other entities might want to refer to it. And it's don't .
    // really makes sense to have dummy entity that creating things on level context.
    
    if (to_copy->runtime_only_flags & SHOULD_NOT_COPY) {
        return NULL;
    }
  
    i32 id_to_set = 0;
    Entity *e = NULL;
    if (id_to_insert > 0) {
        e = context_for_deep_copy->entities.insert({0}, id_to_insert - 1);
        id_to_set = id_to_insert;
    } else {
        e = context_for_deep_copy->entities.append({0}, &id_to_set);
        // Because append gives us index and entity id is index + 1 so id 0 is invalid.
        id_to_set += 1;
    }
    *e = *to_copy;
    // e->will_be_destroyed = false; // That could've shot when we're copying entity that will be destoryed for undo.
    // e->destroyed = false;
    e->color = to_copy->color_changer.start_color;
    e->id = id_to_set;
    
    e->context = context_for_deep_copy;        
    
    if (e->id <= 0) return e;
    
    assert(context_for_deep_copy && "Forgot to specify level context for deep copy.");      
                   
    if (e->flags & TEXTURE) {
        str_copy(e->texture_name, to_copy->texture_name);
    }
    
    e->color_changer = to_copy->color_changer;
    
    if (e->flags & NOTE) {
        e->note_index = add_note("");
        if (e->note_index != -1 && to_copy->note_index != -1) {
            *context_for_deep_copy->notes.get(e->note_index) = *to_copy->context->notes.get(to_copy->note_index);
        }
    }
    
    
    if (e->flags & DOOR) {
        e->door = to_copy->door;
    }
    
    if (to_copy->lights.count > 0) {
        // e->light_index = -1;
        e->lights = copy_array(&to_copy->lights);
        e->lights.clear();
        for_array (i, &to_copy->lights) {
            Light *copy_light = to_copy->context->lights.get(to_copy->lights.get_value(i));  
            copy_and_add_light_to_entity(e, copy_light);
        }
    }
    
    rotate_to(e, e->rotation);
    setup_color_changer(e);
    
    e->particle_emitters_indexes.clear(); // Because on init entities add emitters themselves.
    init_entity(e, true);
    
    // On init_entity we're adding all the types to arrays and now entity have a fresh pointer to fresh type (like PROPELLER, 
    // ENEMY etc.) and all the copying should happen after we're added new thing.
    if (e->flags & PROPELLER && to_copy->propeller) {
        assert(e->propeller);
        i32 my_index = e->propeller->index;
        *e->propeller = *to_copy->propeller;
        e->propeller->index = my_index;
    }
    
    // Copy sticky texture.
    if (e->flags & STICKY_TEXTURE && to_copy->sticky_texture) {
        assert(e->sticky_texture);
        i32 my_index = e->sticky_texture->index;
        *e->sticky_texture = *to_copy->sticky_texture;
        e->sticky_texture->index = my_index;
    }
    
    // Copy move sequence.
    if (e->flags & MOVE_SEQUENCE && to_copy->move_sequence) {
        assert(e->move_sequence);
        i32 my_index = e->move_sequence->index;
        *e->move_sequence = *to_copy->move_sequence;
        e->move_sequence->index = my_index;
        
        e->move_sequence->points = {0};
        e->move_sequence->points = copy_array(&to_copy->move_sequence->points);
    }
    
    // Copy bird enemy.
    if (e->flags & BIRD_ENEMY && to_copy->bird_enemy) {
        assert(e->bird_enemy);
        i32 my_index = e->bird_enemy->index;
        *e->bird_enemy = *to_copy->bird_enemy;
        e->bird_enemy->index = my_index;
    }
    
    // Copy turret.
    if (e->flags & TURRET && to_copy->turret) {
        assert(e->turret);
        i32 my_index = e->turret->index;
        *e->turret = *to_copy->turret;
        e->turret->index = my_index;
    }
    
    // Copy jump shooter.
    if (e->flags & JUMP_SHOOTER && to_copy->jump_shooter) {
        assert(e->jump_shooter);
        i32 my_index = e->jump_shooter->index;
        *e->jump_shooter = *to_copy->jump_shooter;
        e->jump_shooter->index = my_index;
        
        e->jump_shooter->move_points = {0};
        e->jump_shooter->move_points = copy_array(&to_copy->jump_shooter->move_points);
    }
    
    // Right now centipede in editor will be just it's head, so no need for strange segments copying.
    // And in case of copying level context it's should work fine out of a box.
    //
    // Copy centipede.
    if (e->flags & CENTIPEDE && to_copy->centipede) {
        assert(e->centipede);
        i32 my_index = e->centipede->index;
        Array <Entity *> my_segments = e->centipede->segments;
        *e->centipede = *to_copy->centipede;
               
        e->centipede->index = my_index;
        e->centipede->segments = my_segments;
    }
    // // Copy centipede segment.
    // if (e->flags & CENTIPEDE_SEGMENT && to_copy->centipede_segment) {
    //     assert(!(e->flags & CENTIPEDE));
    //     assert(e->centipede_segment);
    //     i32 my_index = e->centipede_segment->index;
    //     *e->centipede_segment = *to_copy->centipede_segment;
    //     e->centipede_segment->index = my_index;
    // }
    
    // Copy kill switch.
    if (e->flags & KILL_SWITCH && to_copy->kill_switch) {
        assert(e->kill_switch);
    
        i32 my_index = e->kill_switch->index;
        *e->kill_switch = *to_copy->kill_switch;
        e->kill_switch->index = my_index;
        
        
        e->kill_switch->connected = {0};
        e->kill_switch->connected = copy_array(&to_copy->kill_switch->connected);
    }
    
    // Copy trigger.
    if (e->flags & TRIGGER && to_copy->trigger) {
        assert(e->trigger);
        i32 my_index = e->trigger->index;
        *e->trigger = *to_copy->trigger;
        e->trigger->index = my_index;
        
        e->trigger->connected = {0};
        e->trigger->connected = copy_array(&to_copy->trigger->connected);
        e->trigger->tracking = {0};
        e->trigger->tracking =  copy_array(&to_copy->trigger->tracking);
        
        e->trigger->cam_rails_points = {0};
        e->trigger->cam_rails_points = copy_array(&to_copy->trigger->cam_rails_points);
    }
    
    // We actually could copy all enemy data above, but that's not that importnat because data should stay the same.
    // (Unless some type would make some manipulations on enemy data after copying, but we'll leave it be for now).
    // Copy enemy.
    if (e->flags & ENEMY && to_copy->union_enemy) {
        assert(e->union_enemy);
        
        i32 my_index = e->union_enemy->index;
        *e->union_enemy = *to_copy->union_enemy;
        e->union_enemy->index = my_index;
    }
    
    // Copy projectile.
    if (e->flags & PROJECTILE && to_copy->projectile) {
        assert(e->projectile);
        
        i32 my_index = e->projectile->index;
        *e->projectile = *to_copy->projectile;
        e->projectile->index = my_index;
    }
    
    return e;
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags) {
    i32 id = 0;
    Entity *e = current_context->entities.append({0}, &id);
    // Because append gives us index and entity id is index + 1 so id 0 is invalid.
    id += 1;
    *e = make_entity(pos, scale, pivot, rotation, flags);    
    e->id = id;
    
    e->context = current_context;
    
    init_entity(e);
    return e;
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Texture texture, FLAGS flags) {
    i32 id = 0;
    Entity *e = current_context->entities.append({0}, &id);
    // Because append gives us index and entity id is index + 1 so id 0 is invalid.
    id += 1;
    *e = make_entity(pos, scale, pivot, rotation, texture, flags);    
    e->id = id;
    e->context = current_context;
    
    init_entity(e);
    return e;
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags) {
    Entity *e = add_entity(pos, scale, pivot, rotation, flags);    
    e->color = color;
    setup_color_changer(e);
    return e;
}

inline Vector2 global_position(Entity *e, Vector2 local_pos) {
    return e->position + local_pos;
}

inline Vector2 global_position(Vector2 position, Vector2 local_pos) {
    return position + local_pos;
}

inline Vector2 local_position(Entity *e, Vector2 global_pos) {
    return global_pos - e->position;
}

inline Vector2 world_to_screen(Vector2 position) {
    Vector2 cam_pos = current_context->cam.position;

    Vector2 with_cam = subtract(position, cam_pos);
    Vector2 pixels   = multiply(with_cam, current_context->cam.unit_size);
    
    //Horizontal center and vertical bottom
    f32 width_add, height_add;
    
    width_add = current_context->cam.width * 0.5f;    
    height_add = current_context->cam.height * 0.5f;    
    Vector2 to_center = {pixels.x + width_add, height_add - pixels.y};

    return to_center;
}

//This gives us real screen pixel position
inline Vector2 world_to_screen_with_zoom(Vector2 position) {
    Vector2 cam_pos = current_context->cam.position;

    Vector2 with_cam = subtract(position, cam_pos);
    Vector2 pixels   = multiply(with_cam, current_context->cam.unit_size * current_context->cam.cam2D.zoom);
    //Horizontal center and vertical bottom
    
    f32 width_add, height_add;
    
    width_add = current_context->cam.width * 0.5f;    
    height_add = current_context->cam.height * 0.5f;    
    Vector2 to_center = {pixels.x + width_add, height_add - pixels.y};

    return to_center;
}

inline Vector2 get_texture_pixels_size(Texture texture, Vector2 game_scale) {
    Vector2 screen_texture_size_multiplier = transform_texture_scale(texture, game_scale);
    return multiply({(f32)texture.width, (f32)texture.height}, screen_texture_size_multiplier) * current_context->cam.cam2D.zoom;
}

inline Vector2 get_left_down_texture_screen_position(Texture texture, Vector2 world_position, Vector2 game_scale) {
    Vector2 pixels_size = get_texture_pixels_size(texture, game_scale);
    pixels_size.y *= -1;
    Vector2 texture_pos = world_to_screen_with_zoom(world_position) - pixels_size  * 0.5f;
    texture_pos.y = screen_height - texture_pos.y;
    
    return texture_pos;
}

inline Vector2 rect_screen_pos(Vector2 position, Vector2 scale, Vector2 pivot) {
    Vector2 pivot_add = multiply(pivot, scale);
    Vector2 with_pivot_pos = {position.x - pivot_add.x, position.y + pivot_add.y};
    Vector2 screen_pos = world_to_screen(with_pivot_pos);
    
    return screen_pos;
}

inline void draw_game_triangle(Vector2 a, Vector2 b, Vector2 c, Color color) {
    draw_triangle(world_to_screen(a), world_to_screen(b), world_to_screen(c), color);
}

inline void draw_game_circle(Vector2 position, f32 radius, Color color) {
    Vector2 screen_pos = world_to_screen(position);
    draw_circle(screen_pos, radius * current_context->cam.unit_size, color);
}

inline void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, Color color) {
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect(screen_pos, multiply(scale, current_context->cam.unit_size), color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color) {
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect_lines(screen_pos, scale * current_context->cam.unit_size, thick, color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, Color color) {
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect_lines(screen_pos, scale * current_context->cam.unit_size, color);
}

Static_Array <Vector2, 2048> screen_positions_buffer = Static_Array <Vector2, 2048>();

inline void draw_game_line_strip(Entity *entity, Color color) {
    screen_positions_buffer.clear();
    for (i32 i = 0; i < entity->vertices.count; i++) {
        screen_positions_buffer.append(world_to_screen(global_position(entity, entity->vertices.get_value(i))));
    }
    
    draw_line_strip(screen_positions_buffer.data, screen_positions_buffer.count, color);
}

inline void draw_game_line_strip(Vector2 *points, i32 count, Color color) {
    screen_positions_buffer.clear();
    for (i32 i = 0; i < count; i++) {
        screen_positions_buffer.append(world_to_screen(points[i]));
    }
    
    draw_line_strip(screen_positions_buffer.data, screen_positions_buffer.count, color);
}

inline void draw_game_line_strip(Vector2 position, Static_Array <Vector2, MAX_VERTICES> vertices, Color color) {
    local_persist Static_Array <Vector2, MAX_VERTICES> global_vertices_buffer = Static_Array <Vector2, MAX_VERTICES>();
    global_vertices_buffer.clear();
    
    for (i32 i = 0; i < vertices.count; i++) {
        global_vertices_buffer.append(vertices.get_value(i) + position);
    }
    draw_game_line_strip(global_vertices_buffer.data, global_vertices_buffer.count, color);
}

void draw_game_triangle_strip(Static_Array <Vector2, MAX_VERTICES> vertices, Vector2 position, Color color) {
    screen_positions_buffer.clear();
    for (i32 i = 0; i < vertices.count; i++) {
        screen_positions_buffer.append(world_to_screen(global_position(position, vertices.get_value(i))));
    }
    
    draw_triangle_strip(screen_positions_buffer.data, screen_positions_buffer.count, color);
}

inline void draw_game_triangle_strip(Entity *entity, Color color) {
    if (entity->hidden) {
        color = color_fade(entity->color, 0.2f);
    }
    draw_game_triangle_strip(entity->vertices, entity->position, color);
}

inline void draw_game_triangle_strip(Entity *entity) {
    draw_game_triangle_strip(entity, entity->color);
}

inline void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color) {
    Vector2 screen_pos = rect_screen_pos(position, scale, {0, 0});
    draw_rect(screen_pos, multiply(scale, current_context->cam.unit_size), pivot, rotation, color);
}

inline void draw_game_text(Vector2 position, const char *text, f32 size, Color color) {
    Vector2 screen_pos = world_to_screen(position);
    draw_text(text, screen_pos, size, color);
}

inline void draw_game_texture(Texture tex, Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color, b32 flip) {
    Vector2 screen_pos = world_to_screen(position);
    draw_texture(tex, screen_pos, transform_texture_scale(tex, scale), pivot, rotation, color, flip);
}

inline void draw_game_line(Vector2 start, Vector2 end, f32 thick, Color color) {
    draw_line(world_to_screen(start), world_to_screen(end), thick * current_context->cam.unit_size, color);
}

inline void draw_game_line(Vector2 start, Vector2 end, Color color) {
    draw_line(world_to_screen(start), world_to_screen(end), color);
}

inline void draw_game_ring_lines(Vector2 center, f32 inner_radius, f32 outer_radius, i32 segments, Color color, f32 start_angle, f32 end_angle) {
    draw_ring_lines(world_to_screen(center), inner_radius * current_context->cam.unit_size, outer_radius * current_context->cam.unit_size, segments, color);
}

inline void draw_game_triangle_lines(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
    draw_triangle_lines(world_to_screen(v1), world_to_screen(v2), world_to_screen(v3), color);
}

Vector2 get_left_up_no_rot(Entity *e) {
    return {e->position.x - e->pivot.x * e->bounds.size.x, e->position.y + e->pivot.y * e->bounds.size.y};
}

Vector2 get_left_up(Entity *e) {
    Vector2 lu = get_left_up_no_rot(e);
    
    rotate_around_point(&lu, e->position, e->rotation);
    return lu;
}

Vector2 get_right_down_no_rot(Entity *e) {
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x + e->bounds.size.x, lu.y - e->bounds.size.y};
}

Vector2 get_right_down(Entity *e) {
    Vector2 lu = get_left_up(e);
    Vector2 rd = lu + e->right * e->bounds.size.x;
    rd -= e->up * e->bounds.size.y;
    return rd;
}

Vector2 get_left_down_no_rot(Entity *e) {
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x, lu.y - e->bounds.size.y};
}

Vector2 get_left_down(Entity *e) {
    Vector2 rd = get_right_down(e);
    return rd - e->right * e->bounds.size.x;
}

Vector2 get_right_up_no_rot(Entity *e) {
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x + e->bounds.size.x, lu.y};
}

Vector2 get_right_up(Entity *e) {
    Vector2 lu = get_left_up(e);
    return lu + e->right * e->bounds.size.x;
}