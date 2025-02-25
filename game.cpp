#pragma once

// #define NO_EDITOR

//#define assert(a) (if (!a) (i32*)void*);
//#define assert(Expression) if(!(Expression)) {*(i32 *)0 = 0;}

#include "game.h"
#include "../my_libs/perlin.h"

// #define ForEntities(entityext_avaliable(table, 0);  xx < table.max_count; xx = table_next_avaliable(table, xx+0))

#define ForEntities(entity, flags) Entity *entity = NULL; for (i32 index = next_entity_avaliable(current_level_context, 0, &entity, flags); index < current_level_context->entities.max_count && entity; index = next_entity_avaliable(current_level_context, index+1, &entity, flags)) 
#define ForEntitiesInContext(context, entity, flags) Entity *entity = NULL; for (i32 index = next_entity_avaliable(context, 0, &entity, flags); index < current_level_context->entities.max_count && entity; index = next_entity_avaliable(context, index+1, &entity, flags)) 

#define ArrayOfStructsToDefaultValues(arr) for (i32 arr_index = 0; arr_index < arr.max_count; arr_index++){ (*arr.get_ptr(arr_index)) = {};}

//#define For(arr, type, value) for(i32 ii = 0; ii < arr.count; ii++){ type value = arr.get(ii);

global_variable Input input;
// global_variable Level current_level;
// global_variable Context context = {};
global_variable Level_Context editor_level_context = {};
global_variable Level_Context game_level_context = {};
global_variable Level_Context checkpoint_level_context = {};
global_variable Level_Context loaded_level_context = {};
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
// global_variable Context saved_level_context = {};
global_variable Console console = {};
global_variable Editor editor  = {}; 
global_variable Debug  debug  = {};
//global_variable Entity *player_entity;
global_variable b32 player_on_level;

global_variable const char *first_level_name = "new_basics1";

global_variable Array<Vector2, MAX_VERTICES> global_normals = Array<Vector2, MAX_VERTICES>();

global_variable Entity mouse_entity;

global_variable Entity *player_entity;
global_variable b32 need_destroy_player = false;

global_variable f32 frame_rnd;
global_variable Vector2 frame_on_circle_rnd;

global_variable b32 clicked_ui = false;

global_variable b32 enter_game_state_on_new_level = false;

global_variable Hash_Table_Int<Texture> textures_table = Hash_Table_Int<Texture>(512);
global_variable Hash_Table_Int<Sound_Handler> sounds_table = Hash_Table_Int<Sound_Handler>(128);

#include "../my_libs/random.hpp"
#include "particles.hpp"
#include "text_input.hpp"
#include "ui.hpp"

Player last_player_data = {};
Player death_player_data = {};

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
    
    if (e->flags & CENTIPEDE){
        // free centipede
        for (i32 i = 0; i < e->centipede.segments_ids.count; i++){
            Entity *segment = current_level_context->entities.get_by_key_ptr(e->centipede.segments_ids.get(i));
            segment->destroyed = true;
            segment->enabled = false;
        }
        
        e->centipede.segments_ids.clear();
    }
    
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
    
    // if (e->flags & MOVE_SEQUENCE){
    //     e->move_sequence.points.free_arr();
    // }
    
    // if (e->flags & JUMP_SHOOTER){
    //     e->jump_shooter.move_points.free_arr();
    // }
    
    // free light
    if (e->flags & LIGHT){
        free_entity_light(e);
    }
}

void add_rect_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    vertices->clear();
    vertices->add({pivot.x, pivot.y});
    vertices->add({-pivot.x, pivot.y});
    vertices->add({pivot.x, pivot.y - 1.0f});
    vertices->add({pivot.x - 1.0f, pivot.y - 1.0f});
}

void add_sword_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    vertices->clear();
    vertices->add({pivot.x * 0.3f, pivot.y});
    vertices->add({-pivot.x * 0.3f, pivot.y});
    vertices->add({pivot.x, pivot.y - 1.0f});
    vertices->add({pivot.x - 1.0f, pivot.y - 1.0f});
}

void add_upsidedown_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    vertices->clear();
    vertices->add({pivot.x, pivot.y});
    vertices->add({-pivot.x, pivot.y});
    vertices->add({pivot.x * 0.3f, -pivot.y});
    vertices->add({-pivot.x * 0.3f, -pivot.y});
}

void add_texture_vertices(Array<Vector2, MAX_VERTICES> *vertices, Texture texture, Vector2 pivot){
    vertices->clear();
    Vector2 scaled_size = {texture.width / session_context.cam.unit_size, texture.height / session_context.cam.unit_size};
    vertices->add({pivot.x * scaled_size.x, pivot.y * scaled_size.y});
    vertices->add({-pivot.x * scaled_size.x, pivot.y * scaled_size.y});
    vertices->add({pivot.x * scaled_size.x, (pivot.y - 1.0f) * scaled_size.y});
    vertices->add({(pivot.x - 1.0f) * scaled_size.x, (pivot.y - 1.0f) * scaled_size.y});
}

void pick_vertices(Entity *entity){
    // if (entity->flags & TEXTURE){
    //     add_texture_vertices(&entity->vertices, entity->texture, entity->pivot);
    //     add_texture_vertices(&entity->unscaled_vertices, entity->texture, entity->pivot);
    //     return;
    // }

    if (entity->flags & (SWORD | BIRD_ENEMY | CENTIPEDE | PROJECTILE)){
        add_sword_vertices(&entity->vertices, entity->pivot);
        add_sword_vertices(&entity->unscaled_vertices, entity->pivot);
    } else if (entity->flags & (JUMP_SHOOTER)){
        add_upsidedown_vertices(&entity->vertices, entity->pivot);
        add_upsidedown_vertices(&entity->unscaled_vertices, entity->pivot);
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
    scaling_multiplier = {texture.width / session_context.cam.unit_size, texture.height / session_context.cam.unit_size};
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
        scaling_multiplier = {texture.width / session_context.cam.unit_size, texture.height / session_context.cam.unit_size};
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
    
    // if (flags & PHYSICS_OBJECT){
    physics_object.rope_id = -1;
    physics_object.up_rope_point_id = -1;
    physics_object.down_rope_point_id = -1;
    // }
    
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
    
    if (flags & LIGHT){
        // if (!keep_id){
        light_index = -1;
        init_entity_light(this, copy_level_context->lights.get_ptr(copy->light_index));        
        // }
    }
    
    // if (flags & PARTICLE_EMITTER){
    //     for (i32 i = 0; i < copy->emitters.count; i++){
    //         emitters.add(copy->emitters.get(i));
    //     }
    // }
    
    
    if (should_init_entity){
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
    current_level_context = dest;
    
    Game_State original_game_state = game_state;
    game_state = EDITOR;
    
    for (i32 i = 0; i < src->entities.max_count; i++){
        Table_Data<Entity> data = {};
        
        data.key = src->entities.data[i].key;
        if (data.key != -1){
            // if (src->entities.get(i).flags & PLAYER){
            //     continue;
            // }
            data.value = Entity(&src->entities.data[i].value, true, src, should_init_entities);
            data.value.level_context = current_level_context;
        } else{
            data.value = {};
        }
        dest->entities.data[i] = data;
    }
    // mem_copy(dest->entities.data, src->entities.data, sizeof(Entity) * src->entities.max_count);
    dest->entities.max_count = src->entities.max_count;
    dest->entities.total_added_count = src->entities.total_added_count;
    dest->entities.last_added_key = src->entities.last_added_key;
    // dest->entities.max_count = src->entities.max_count;
    
    for (i32 i = 0; i < src->particles.count; i++){
        dest->particles.data[i] = src->particles.get(i);
    }
    dest->particles.count = src->particles.count;
    dest->particles.max_count = src->particles.max_count;
    
    for (i32 i = 0; i < src->emitters.count; i++){
        dest->emitters.data[i] = src->emitters.get(i);
    }
    dest->emitters.count = src->emitters.count;
    dest->emitters.max_count = src->emitters.max_count;

    // for (i32 i = 0; i < src->notes.count; i++){
    //     dest->notes.data[i] = src->notes.data[i];
    // }
    // dest->notes.count = src->notes.count;
    // dest->notes.max_count = src->notes.max_count;

    // for (i32 i = 0; i < src->lights.max_count; i++){
    //     copy_light(dest->lights.get_ptr(i), src->lights.get_ptr(i));
    //     // We init only entity lights, because we might want to free it and load render textures, 
    //     // but we don't free temp lights and they are loaded in memory in init_level_context.
    //     if (i >= session_context.entity_lights_start_index){
    //         init_light(dest->lights.get_ptr(i));
    //     }
    // }

    // for (i32 i = 0; i < src->lights.count; i++){
    //     dest->lights.data[i] = src->lights.get(i);
    // }
    
    current_level_context = original_level_context;
    game_state = original_game_state;
}

// void copy_player(Player *dest, Player *src){
//     *dest = *src;
//     dest->timers = {};
//     dest->connected_entities_ids = {};
// }

void clear_level_context(Level_Context *level_context){
    Level_Context *original_level_context = current_level_context;
    current_level_context = level_context;
    ForEntities(entity, 0){
        free_entity(entity);
        *entity = {};
    }

    level_context->entities.clear();
    level_context->particles.clear();
    // level_context->emitters.clear();
    
    ArrayOfStructsToDefaultValues(level_context->notes);
    
    for (i32 i = 0; i < level_context->lights.max_count; i++){
        level_context->lights.get_ptr(i)->exists = false;
        if (i >= session_context.entity_lights_start_index){
            free_light(level_context->lights.get_ptr(i));       
            *(level_context->lights.get_ptr(i)) = {};
        } else{ // So we in temp lights section
            // fill_light_by_temp_light_template(level_context->lights.get_ptr(i));
        }
    }
    
    // level_context->we_got_a_winner = false;
    player_data = {};
    
    current_level_context = original_level_context;
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
    
    fprintf(fptr, "player_spawn_point:{:%f:, :%f:} ", editor.player_spawn_point.x, editor.player_spawn_point.y);
    
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
        
        if (e->flags & LIGHT && e->light_index >= 0){
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
            fprintf(fptr, "trigger_locked_camera_position{:%f:, :%f:} ", e->trigger.locked_camera_position.x, e->trigger.locked_camera_position.y);
            
            fprintf(fptr, "trigger_load_level:%d: ", e->trigger.load_level);
            if (e->trigger.load_level){
                fprintf(fptr, "trigger_level_name:%s: ", e->trigger.level_name);
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
            if (e->enemy.gives_ammo){
                fprintf(fptr, "gives_full_ammo:%d: ", e->enemy.gives_full_ammo);
            }
            // fprintf(fptr, "max_hits_taken:%d: ", e->enemy.gives_full_ammo);
        }
        
        if (e->flags & EXPLOSIVE){
            fprintf(fptr, "explosive_radius_multiplier:%fs: ", e->enemy.explosive_radius_multiplier);
        }
        
        if (e->flags & PROPELLER){
            fprintf(fptr, "propeller_power:%f: ", e->propeller.power);
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
        if (!str_equal(session_context.current_level_name, name)){
            str_copy(session_context.previous_level_name, session_context.current_level_name);
        }
        str_copy(session_context.current_level_name, name);
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
    current_level_context = &loaded_level_context;
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
                    fill_vector2_from_string(&editor.player_spawn_point, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
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
                fill_i32_from_string(&entity_to_fill.flags, splitted_line.get(i+1).data);
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
            } else if (str_equal(splitted_line.get(i).data, "sword_kill_speed_modifier")){
                fill_f32_from_string(&entity_to_fill.enemy.sword_kill_speed_modifier, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "shoot_blocker_direction")){
                fill_vector2_from_string(&entity_to_fill.enemy.shoot_blocker_direction, splitted_line.get(i+1).data, splitted_line.get(i+2).data);
                i += 2;
            } else if (str_equal(splitted_line.get(i).data, "shoot_blocker_immortal")){
                fill_b32_from_string(&entity_to_fill.enemy.shoot_blocker_immortal, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "gives_full_ammo")){
                fill_b32_from_string(&entity_to_fill.enemy.gives_full_ammo, splitted_line.get(i+1).data);
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
            } else if (str_equal(splitted_line.get(i).data, "trigger_level_name")){
                str_copy(entity_to_fill.trigger.level_name, splitted_line.get(i+1).data);  
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
                i64 texture_hash = hash_str(entity_to_fill.texture_name);
                if (textures_table.has_key(texture_hash)){
                    entity_to_fill.texture = textures_table.get_by_key(texture_hash);
                } else{
                    print(text_format("WARNING: While loading entities could not find texture named %s.", entity_to_fill.texture_name));
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
        if (!str_equal(session_context.current_level_name, name)){
            str_copy(session_context.previous_level_name, session_context.current_level_name);
        }
        str_copy(session_context.current_level_name, name);
        print_to_console(text_format("Loaded level: %s", name));
        editor.last_autosave_time = core.time.app_time;
    }
    
    //free_string_array(&splitted_line);
    splitted_line.free_arr();
    unload_file(&file);
        
    loop_entities(init_loaded_entity);
    
    // We do that so editor has latest level in it.
    // current_level_context = editor_level-conte
    clear_level_context(&editor_level_context);
    clear_level_context(&game_level_context);
    copy_level_context(&editor_level_context, &loaded_level_context, true);
    // current_level_context = &editor_level_context;
    // game_state = EDITOR;
    
    if (enter_game_state_on_new_level || game_state == GAME){
        enter_game_state(&loaded_level_context, true);
        
        // ForEntities(entity, 0){
        //     update_editor_entity(entity);
        // }

        if (enter_game_state_on_new_level){
            player_data.blood_amount = last_player_data.blood_amount;
            player_data.blood_progress = last_player_data.blood_progress;
            player_data.ammo_count = last_player_data.ammo_count;
        }
        
        enter_game_state_on_new_level = false;
    }
    
    
    session_context.cam.position = editor.player_spawn_point;
    session_context.cam.target = editor.player_spawn_point;
    return true;
} // end load level end

global_variable Dynamic_Array<Collision> collisions_buffer        = Dynamic_Array<Collision>(256);
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

#define BIRD_ENEMY_COLLISION_FLAGS (GROUND | PLAYER | BIRD_ENEMY | CENTIPEDE_SEGMENT | BLOCKER | SHOOT_BLOCKER | SWORD_SIZE_REQUIRED)

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

void init_bird_emitters(Entity *entity){
    entity->emitters.clear();
    entity->bird_enemy.trail_emitter  = entity->emitters.add(entity->flags & EXPLOSIVE ? little_fire_emitter : air_dust_emitter);
    enable_emitter(entity->bird_enemy.trail_emitter);
    entity->bird_enemy.attack_emitter = entity->emitters.add(rifle_bullet_emitter);
    entity->bird_enemy.fire_emitter = entity->emitters.add(fire_emitter);
}

void init_bird_entity(Entity *entity){
    //entity->flags = ENEMY | BIRD_ENEMY | PARTICLE_EMITTER;
    assert(entity->flags > 0);
    entity->collision_flags = BIRD_ENEMY_COLLISION_FLAGS;//GROUND | PLAYER | BIRD_ENEMY;
    change_color(entity, entity->flags & EXPLOSIVE ? ORANGE * 0.9f : YELLOW * 0.9f);
    
    entity->enemy.gives_full_ammo = true;
    
    entity->enemy.sword_kill_speed_modifier = 4;
    
    init_bird_emitters(entity);
        
    //entity->emitter = entity->emitters.last_ptr();
    str_copy(entity->name, "enemy_bird"); 
    setup_color_changer(entity);
    
    entity->bird_enemy.attack_sound = sounds_table.get_by_key_ptr(hash_str("BirdAttack"));
    
    // if (entity->flags & EXPLOSIVE){
    // }        
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
    
    Entity platform_entity = Entity({0, 0}, {40, 2}, {0.5f, 0.5f}, 0, PLATFORM);
    platform_entity.color = Fade(ColorBrightness(BROWN, -0.1f), 0.1f);
    str_copy(platform_entity.name, "platform"); 
    setup_color_changer(&platform_entity);
    
    Spawn_Object platform_object;
    copy_entity(&platform_object.entity, &platform_entity);
    str_copy(platform_object.name, platform_entity.name);
    spawn_objects.add(platform_object);
    
    Entity enemy_base_entity = Entity({0, 0}, {3, 5}, {0.5f, 0.5f}, 0, ENEMY);
    enemy_base_entity.color = RED * 0.9f;
    str_copy(enemy_base_entity.name, "enemy_base"); 
    setup_color_changer(&enemy_base_entity);
    
    Spawn_Object enemy_base_object;
    copy_entity(&enemy_base_object.entity, &enemy_base_entity);
    str_copy(enemy_base_object.name, enemy_base_entity.name);
    spawn_objects.add(enemy_base_object);
    
    Entity bird_entity = Entity({0, 0}, {3, 5}, {0.5f, 0.5f}, 0, ENEMY | BIRD_ENEMY | PARTICLE_EMITTER);
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
    
    Entity enemy_trigger_entity = Entity({0, 0}, {5, 5}, {0.5f, 0.5f}, 0, ENEMY | TRIGGER);
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
    
    Entity centipede_segment_entity = Entity({0, 0}, {4, 4}, {0.5f, 0.5f}, 0, ENEMY | CENTIPEDE_SEGMENT | MOVE_SEQUENCE);
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

// manual textures 
Texture spiral_clockwise_texture;
Texture spiral_counterclockwise_texture;
Texture hitmark_small_texture;
Texture jump_shooter_bullet_hint_texture;
Texture big_sword_killable_texture;
Texture small_sword_killable_texture;

Texture get_texture(const char *name){
    Texture found_texture;
    found_texture = textures_table.get_by_key(hash_str(name));
    if (found_texture.width == 0){
        print(text_format("WARNING: Texture named %s cannot be found", name));
    }
    
    return found_texture;
}

void load_textures(){
    FilePathList textures = LoadDirectoryFiles("resources\\textures");
    for (i32 i = 0; i < textures.count; i++){
        char *name = textures.paths[i];
        
        if (!str_end_with(name, ".png")){
            continue;
        }
        
        Texture texture = LoadTexture(name);
        
        substring_after_line(name, "resources\\textures\\");
        // name = get_substring_before_symbol(name, '.');
        
        i64 hash = hash_str(name);
        //assert(!textures_table.has_key(hash));
        if (!textures_table.add(hash, texture)){
            printf("COULD NOT FIND HASH FOR TEXTURE: %s\n", name);
            continue;
        }
        
        if (str_contains(name, "_clockwise")){
            spiral_clockwise_texture = texture;
        }
        if (str_contains(name, "_counterclockwise")){
            spiral_counterclockwise_texture = texture;
        }
        if (str_contains(name, "hitmark_small")){
            hitmark_small_texture = texture;
        }
        
        add_spawn_object_from_texture(texture, name);
    }
    UnloadDirectoryFiles(textures);
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

i32 init_entity_light(Entity *entity, Light *light_copy, b32 free_light){
    Light *new_light = NULL;
    
    //Means we will copy ourselves, maybe someone changed size or any other shit
    if (!light_copy && entity->light_index > -1){
        light_copy = current_level_context->lights.get_ptr(entity->light_index);
    } else if (!light_copy){
        //lol
        // *light_copy = {};
    }
    
    // if (entity->light_index > -1 && current_level_context->lights.get(entity->light_index).exists){
    //     return entity->light_index;
    // }
        
    if (free_light){
        free_entity_light(entity);
    }
    
    for (i32 i = 0; i < current_level_context->lights.max_count; i++){
        if (!current_level_context->lights.get_ptr(i)->exists && i >= session_context.entity_lights_start_index){
            new_light = current_level_context->lights.get_ptr(i);
            entity->light_index = i;
            break;
        } else{
            // assert(current_level_context->lights.get_ptr(i)->connected_entity_id != entity->id);
        }
    }
    if (new_light){
        if (light_copy){
            // new_light->radius                 = light_copy->radius;            
            // new_light->make_shadows           = light_copy->make_shadows;
            // new_light->make_backshadows       = light_copy->make_backshadows;
            // new_light->bake_shadows           = light_copy->bake_shadows;
            // new_light->shadows_size_flags     = light_copy->shadows_size_flags;
            // new_light->backshadows_size_flags = light_copy->backshadows_size_flags;
            // *new_light = *light_copy;
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
    
    return new_light ? entity->light_index : -1;
}

void init_entity(Entity *entity){
    if (entity->flags & ENEMY){
        entity->enemy.original_scale = entity->scale;
        
        entity->enemy.explosion_sound = sounds_table.get_by_key_ptr(hash_str("Explosion"));
        entity->enemy.explosion_sound->base_volume = 0.3f;
        entity->enemy.big_explosion_sound = sounds_table.get_by_key_ptr(hash_str("BigExplosion"));
        entity->enemy.big_explosion_sound->base_volume = 0.5f;
    }
    
    if (entity->flags & BIRD_ENEMY){
        entity->enemy.max_hits_taken = 3;
        init_bird_entity(entity);
    }

    // init explosive
    if (entity->flags & EXPLOSIVE){
        entity->color_changer.change_time = 5.0f;
        entity->flags |= LIGHT;
        Light explosive_light = {};
        explosive_light.make_backshadows = true;
        explosive_light.make_shadows     = true;
        if (entity->light_index != -1){
            explosive_light = current_level_context->lights.get(entity->light_index);
        } 
        // explosive_light.make_backshadows = false; @WTF screen goes black in game mode with this shit. Should change the way lights stored and way we get access to them so don't bother, but wtf ebat (also render doc don't loading with this shit)
        if (entity->enemy.explosive_radius_multiplier >= 3){
            explosive_light.shadows_size_flags = BIG_LIGHT;
            explosive_light.backshadows_size_flags = BIG_LIGHT;
        } else if (entity->enemy.explosive_radius_multiplier > 1){
            explosive_light.shadows_size_flags = MEDIUM_LIGHT;
            explosive_light.backshadows_size_flags = MEDIUM_LIGHT;
        } else{
            explosive_light.make_shadows     = false;
            explosive_light.make_backshadows = false;
        }
        init_entity_light(entity, &explosive_light, true);
        Light *new_light = current_level_context->lights.get_ptr(entity->light_index);
        new_light->radius = 120;
        new_light->color = Fade(ColorBrightness(ORANGE, 0.2f), 0.9f);
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
    
    if (entity->flags & PROPELLER){
        entity->emitters.clear();
        entity->propeller.air_emitter = entity->emitters.add(air_emitter);
        enable_emitter(entity->propeller.air_emitter);
        
        entity->propeller.air_emitter->speed_multiplier    = entity->propeller.power / 50.0f;
        entity->propeller.air_emitter->count_multiplier    = entity->propeller.power / 50.0f;
        entity->propeller.air_emitter->lifetime_multiplier = (1.8f * (entity->scale.y / 120.0f)) / entity->propeller.air_emitter->speed_multiplier;
        // entity->propeller.air_emitter->spawn_radius        = entity->scale.x * 0.5f;
        entity->propeller.air_emitter->spawn_offset = entity->up * entity->scale.y * 0.125f;
        entity->propeller.air_emitter->spawn_area = {entity->scale.x, entity->scale.y * 0.25f};
        entity->propeller.air_emitter->direction           = entity->up;
    }        
    
    if (entity->flags & DOOR){
        entity->flags |= TRIGGER;
        entity->trigger.player_touch = false;
        //entity->door.open_sound = sounds_table.get_by_key_ptr(hash_str("OpenDoor"));
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

            segment->position = previous->position - previous->up * previous->scale.y * 1.2f;
            segment->move_sequence = entity->move_sequence;
            
            segment->hidden = entity->hidden;
            
            segment->flags = (entity->flags ^ CENTIPEDE) | CENTIPEDE_SEGMENT;
            segment->enemy = entity->enemy;
        }
        
        entity->flags ^= ENEMY;
    }
    
    if (entity->flags & JUMP_SHOOTER){
        // init jump shooter
        entity->enemy.max_hits_taken = 6;
        entity->emitters.clear();
        entity->jump_shooter.trail_emitter  = entity->emitters.add(air_dust_emitter);
        entity->jump_shooter.trail_emitter->follow_entity = false;
        enable_emitter(entity->jump_shooter.trail_emitter);
        entity->jump_shooter.flying_emitter = entity->emitters.add(rifle_bullet_emitter);
        entity->jump_shooter.flying_emitter->follow_entity = false;
        entity->enemy.gives_full_ammo = true;
        
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
}

inline void save_current_level(){
    save_level(session_context.current_level_name);
}

inline void autosave_level(){
    i32 max_autosaves = 5;
    i32 autosave_index = -1;    
    for (i32 i = 0; i < max_autosaves; i++){
        const char *path = text_format("levels/autosaves/AUTOSAVE_%d_%s.level", i, session_context.current_level_name);        
        if (!FileExists(path)){
            autosave_index = i;
            break;
        }
    }
    
    // Means we did not found vacant number so we'll see for oldest
    if (autosave_index == -1){
        i64 oldest_time = -1;
        
        for (i32 i = 0; i < max_autosaves; i++){
            const char *path = text_format("levels/autosaves/AUTOSAVE_%d_%s.level", i, session_context.current_level_name);        
            u64 modification_time = GetFileModTime(path);
            if (oldest_time == -1 || modification_time < oldest_time){
                oldest_time = modification_time;
                autosave_index = i;
            }
        }
    }
    
    assert(autosave_index != -1);
    
    save_level(text_format("autosaves/AUTOSAVE_%d_%s", autosave_index, session_context.current_level_name));
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
    load_level(session_context.current_level_name);       
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
    console.str += session_context.current_level_name;
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
        // current_level_context = &loaded_level_context;
        // clear_level_context(&loaded_level_context);
        clear_level_context(&editor_level_context);
        
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
    console.str += "\t>Ctrl+Shift+Space - Toggle Game/Editor\n";
    console.str += "\t>Ctrl+Shift+J - Save current level.\n";
    console.str += "\t>Alt - See and move vertices with mouse.\n";
    console.str += "\t>Alt+V - While moving vertex for snap it to closest.\n";
    console.str += "\t>Alt+1-5 - Fast entities creation.\n";
    console.str += "\t>Space - Create menu\n";
    console.str += "\t>P - Move player spawn point\n";
    console.str += "\t>Ctrl+Space - Pause in game\n";
    console.str += "\t>Shift+Space - Freecam in game\n";
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

void save_temp_replay(){
    const char *name = text_format("replays/%s.replay", text_format("TEMP_%s", get_substring_before_symbol(session_context.current_level_name, '.')));
    
    FILE *fptr;
    fptr = fopen(name, "wb");
    
    size_t write_result = fwrite(level_replay.input_record.data, sizeof(Replay_Frame_Data), level_replay.input_record.count, fptr);
    
    if (write_result != -1){
        console.str += text_format("\t>Temp replay named %s is saved\n", name);
    }
}

void load_temp_replay(){
    const char *name = text_format("replays/%s.replay", text_format("TEMP_%s", get_substring_before_symbol(session_context.current_level_name, '.')));
    FILE *fptr;
    fptr = fopen(name, "rb");

    size_t read_result = fread(level_replay.input_record.data, sizeof(Replay_Frame_Data), level_replay.input_record.max_count, fptr);
    level_replay.input_record.count = read_result;    
    
    if (read_result != -1){
        session_context.playing_replay = true;
        enter_editor_state();
        enter_game_state(current_level_context, true);
    
        console.str += text_format("\t>Temp replay named %s is loaded\n", name);
    }
}

void load_replay(const char *replay_name){
    
}

void save_replay(const char *replay_name){
    // char *name = 0;
    // if (replay_name){
    //     name = get_substring_before_symbol(replay_name, '.');
    // }
    // level_path += text_format("replays/%s.replay", name ? name : text_format("TEMP_%s", get_substring_before_symbol(session_context.current_level_name, '.')));
}

// void l

void debug_toggle_play_replay(){
    session_context.playing_replay = !session_context.playing_replay;
        
    if (session_context.playing_replay){
        enter_editor_state();
        enter_game_state(current_level_context, true);
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
    
    player_data.ammo_count = 0;
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
        add_player_ammo(100, true);
    }    
}

void debug_stop_game(){
    core.time.target_time_scale = 0;
    core.time.time_scale = 0;
    debug.drawing_stopped = true;
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
    
    console.commands.add(make_console_command("save",     save_current_level, save_level_by_name));
    console.commands.add(make_console_command("load",     NULL, load_level_by_name));
    console.commands.add(make_console_command("level",    print_current_level, load_level_by_name));
    console.commands.add(make_console_command("next",     try_load_next_level, NULL));
    console.commands.add(make_console_command("previous", try_load_previous_level, NULL));
    console.commands.add(make_console_command("reload",   reload_level, NULL));
    
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
    
    FilePathList sounds = LoadDirectoryFiles("resources\\audio");
    for (i32 i = 0; i < sounds.count; i++){
        char *name = sounds.paths[i];
        
        if (!str_end_with(name, ".ogg") && !str_end_with(name, ".wav")){
            continue;
        }
        
        Sound sound = LoadSound(name);
        substring_after_line(name, "resources\\audio\\");
        name = get_substring_before_symbol(name, '.');
        
        Sound_Handler handler;
        
        for (i32 s = 0; s < handler.buffer.max_count; s++){
            handler.buffer.add(LoadSoundAlias(sound));
        }
        
        i64 hash = hash_str(name);
        //UnloadSound(sound);
        
        if (!sounds_table.add(hash, handler)){
            printf("ERROR: COULD NOT FIND HASH FOR SOUND: %s\n", name);
            continue;
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

void play_sound(Sound_Handler *handler, Vector2 position, f32 volume_multiplier = 1){
    assert(handler->buffer.count > 0);
    
    Sound sound = handler->buffer.get(handler->current_index);
    handler->current_index = (handler->current_index + 1) % handler->buffer.max_count;
    
    //check vector to camera for volume and pan
    Vector2 to_position = position - session_context.cam.position;
    f32 len = magnitude(to_position);
    f32 max_len = 250;
    f32 distance_t = clamp01(len / max_len);
    
    f32 volume = lerp(handler->base_volume, 0.2f, distance_t * distance_t * distance_t);
    f32 pitch  = lerp(handler->base_pitch, 0.6f, distance_t * distance_t * distance_t);
    
    f32 on_right = normalized(to_position.x);
    f32 side_t = clamp01((to_position.x * on_right) / max_len);
    f32 pan_add = lerp(0.0f, 0.4f * on_right * -1, side_t * side_t);
    
    SetSoundVolume(sound, rnd(volume - handler->volume_variation, volume + handler->volume_variation) * volume_multiplier);
    SetSoundPitch (sound, rnd(pitch - handler->pitch_variation, pitch + handler->pitch_variation));
    SetSoundPan   (sound, 0.5f + pan_add);
    //pan    
    
    PlaySound(sound);
}

void play_sound(const char* name, Vector2 position, f32 volume_multiplier = 1){
    i64 hash = hash_str(name);    
    
    if (!sounds_table.has_key(hash)){
        printf("NO SOUND found %s\n", name);
        return;
    }
    
    play_sound(sounds_table.get_by_key_ptr(hash), position, volume_multiplier);
}

void play_sound(const char* name, f32 volume_multiplier = 1){
    play_sound(name, session_context.cam.position, volume_multiplier);
}

RenderTexture emitters_occluders_rt;
Shader emitters_occluders_shader;
RenderTexture voronoi_seed_rt;
Shader voronoi_seed_shader;
RenderTexture jump_flood_rt;
Shader jump_flood_shader;
RenderTexture distance_field_rt;
Shader distance_field_shader;
RenderTexture global_illumination_rt;
Shader global_illumination_shader;

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
}

void init_game(){
    str_copy(loaded_level_context.name, "loaded_level_context");
    str_copy(editor_level_context.name, "editor_level_context");
    str_copy(game_level_context.name, "game_level_context");
    str_copy(checkpoint_level_context.name, "checkpoint_level_context");
    
    // Now we need to init all level contexts once 
    init_level_context(&loaded_level_context);
    init_level_context(&editor_level_context);
    init_level_context(&game_level_context);
    init_level_context(&checkpoint_level_context);

    HideCursor();
    DisableCursor();
    
    game_state = EDITOR;

    // context = {};    
    
    session_context.entity_lights_start_index = session_context.temp_lights_count; 
    
    render = {};
    #ifndef NO_EDITOR
        str_copy(session_context.current_level_name, "test_level");
    #else
        str_copy(session_context.current_level_name, first_level_name);
    #endif
    
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
    
    global_illumination_shader = LoadShader(0, "./resources/shaders/global_illumination1.fs");
    
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
    
    load_sounds();
    
    mouse_entity = Entity(input.mouse_position, {1, 1}, {0.5f, 0.5f}, 0, 0);
    
    load_level(session_context.current_level_name);
    enter_editor_state();
    
    session_context.cam.position = Vector2_zero;
    session_context.cam.cam2D.target = world_to_screen({0, 0});
    session_context.cam.cam2D.offset = (Vector2){ screen_width/2.0f, (f32)screen_height * 0.5f };
    session_context.cam.cam2D.rotation = 0.0f;
    session_context.cam.cam2D.zoom = 0.4f;
    
    // enter_editor_state();
} // end init game end

void destroy_player(){
    assert(player_entity);

    player_entity->destroyed = true;
    player_entity->enabled   = false;
    
    assert(current_level_context->entities.has_key(player_data.connected_entities_ids.ground_checker_id));
    current_level_context->entities.get_by_key_ptr(player_data.connected_entities_ids.ground_checker_id)->destroyed = true;
    assert(current_level_context->entities.has_key(player_data.connected_entities_ids.sword_entity_id));
    current_level_context->entities.get_by_key_ptr(player_data.connected_entities_ids.sword_entity_id)->destroyed = true;
    
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
    // state_context.shoot_stopers_count = 0;
    // state_context.timers.last_bird_attack_time = -11111;
    // state_context.timers.last_jump_shooter_attack_time = -11111;
    // state_context.timers.last_collision_cells_clear_time = -2;
    
    // state_context.cam_state.locked = false;
    // state_context.cam_state.on_rails_horizontal = false;
    // state_context.cam_state.on_rails_vertical   = false;
    // state_context.cam_state.rails_trigger_id = -1;
    
    // state_context.death_instinct = {};
    checkpoint_trigger_id = -1;
    
    player_data = {};
    
    session_context.speedrun_timer.paused = false;
    if (!session_context.speedrun_timer.game_timer_active){
        session_context.speedrun_timer.time = 0;        
    }
    
    assign_selected_entity(NULL);
    editor.in_editor_time = 0;
    close_create_box();
    
    // if (player_entity){
    //     destroy_player();
    //     player_data.dead_man = false;
    // }
}

void enter_game_state(Level_Context *level_context, b32 should_init_entities){
    // if (game_state == GAME){
    //     return;
    // }
    
    clean_up_scene();
    clear_level_context(&game_level_context);
    
    state_context = {};
    session_context.just_entered_game_state = true;
    core.time.game_time = 0;
    core.time.hitstop = 0;
    core.time.previous_dt = 0;
    // core.time.app_time = 0;
    // core.time = {};
    
    HideCursor();
    DisableCursor();
    
    // clear_level_context(&game_level_context);
    current_level_context = &game_level_context;
    copy_level_context(&game_level_context, level_context, should_init_entities);
    
    Vector2 grid_target_pos = editor.player_spawn_point;
    session_context.collision_grid.origin = {(f32)((i32)grid_target_pos.x - ((i32)grid_target_pos.x % (i32)session_context.collision_grid.cell_size.x)), (f32)((i32)grid_target_pos.y - ((i32)grid_target_pos.y % (i32)session_context.collision_grid.cell_size.y))};

    state_context.timers.last_collision_cells_clear_time = core.time.app_time;
    for (i32 i = 0; i < session_context.collision_grid_cells_count; i++){        
        session_context.collision_grid.cells[i].entities_ids.clear();
    }
    
    // save_level(text_format("temp/TEMP_%s", session_context.current_level_name));

    game_state = GAME;
    
    session_context.cam.cam2D.zoom = 0.35f;
    session_context.cam.target_zoom = 0.35f;
    session_context.cam.position = editor.player_spawn_point;
    
    session_context.game_frame_count = 0;
    if (!session_context.playing_replay){
        level_replay.input_record.clear();
    }
    
    if (should_init_entities){
        player_entity = add_entity(editor.player_spawn_point, {1.0f, 2.0f}, {0.5f, 0.5f}, 0, RED, PLAYER | PARTICLE_EMITTER);
        player_entity->collision_flags = GROUND | ENEMY;
        player_entity->draw_order = 30;
        
        Entity *ground_checker = add_entity(player_entity->position - player_entity->up * player_entity->scale.y * 0.5f, {player_entity->scale.x * 0.9f, player_entity->scale.y * 1.5f}, {0.5f, 0.5f}, 0, 0); 
        ground_checker->collision_flags = GROUND;
        ground_checker->color = Fade(PURPLE, 0.8f);
        ground_checker->draw_order = 31;
        
        Entity *left_wall_checker = add_entity(player_entity->position - player_entity->right * player_entity->scale.x * 0.5f, {player_entity->scale.x * 0.9f, player_entity->scale.y * 0.9f}, {0.5f, 0.5f}, 0, 0); 
        left_wall_checker->collision_flags = GROUND;
        left_wall_checker->color = Fade(PURPLE, 0.8f);
        left_wall_checker->draw_order = 31;
        
        Entity *right_wall_checker = add_entity(player_entity->position + player_entity->right * player_entity->scale.x * 0.5f, {player_entity->scale.x * 0.9f, player_entity->scale.y * 0.9f}, {0.5f, 0.5f}, 0, 0); 
        right_wall_checker->collision_flags = GROUND;
        right_wall_checker->color = Fade(PURPLE, 0.8f);
        right_wall_checker->draw_order = 31;
        
        Entity *sword_entity = add_entity(editor.player_spawn_point, player_data.sword_start_scale, {0.5f, 1.0f}, 0, GRAY + RED * 0.1f, SWORD);
        sword_entity->collision_flags = ENEMY;
        sword_entity->color   = GRAY + RED * 0.1f;
        // sword_entity->color.a = 255;
        // sword_entity->color_changer.start_color = sword_entity->color;
        // sword_entity->color_changer.target_color = RED * 0.99f;
        // sword_entity->color_changer.interpolating = true;
        sword_entity->draw_order = 25;
        str_copy(sword_entity->name, "Player_Sword");
        
        player_data.connected_entities_ids.ground_checker_id = ground_checker->id;
        player_data.connected_entities_ids.left_wall_checker_id = left_wall_checker->id;
        player_data.connected_entities_ids.right_wall_checker_id = right_wall_checker->id;
        player_data.connected_entities_ids.sword_entity_id = sword_entity->id;
        player_data.dead_man = false;
        
        player_data.timers = {};
        
        player_data.stun_emitter        = player_entity->emitters.add(air_dust_emitter);
        player_data.rifle_trail_emitter = player_entity->emitters.add(gunpowder_emitter);
        player_data.rifle_trail_emitter->follow_entity = false;
        
        
        player_data.rifle_hit_sound = sounds_table.get_by_key_ptr(hash_str("RifleHit"));
        player_data.rifle_hit_sound->base_volume = 0.4f;
        player_data.player_death_sound = sounds_table.get_by_key_ptr(hash_str("PlayerTakeDamage"));
        player_data.sword_kill_sound = sounds_table.get_by_key_ptr(hash_str("SwordKill"));
        player_data.sword_kill_sound->base_volume = 0.4f;
        player_data.sword_kill_sound->base_pitch = 1.5f;
        player_data.sword_block_sound = sounds_table.get_by_key_ptr(hash_str("SwordBlock"));
        player_data.sword_block_sound->base_volume = 0.4f;
        player_data.sword_block_sound->base_pitch = 0.5f;
        player_data.sword_block_sound->pitch_variation = 0.1f;
        
        player_data.rifle_switch_sound = sounds_table.get_by_key_ptr(hash_str("RifleSwitch"));
        player_data.rifle_switch_sound->base_volume = 0.4f;
        player_data.rifle_switch_sound->base_pitch = 0.7f;
        player_data.rifle_switch_sound->pitch_variation = 0.1f;
        
        player_data.ammo_count = last_player_data.ammo_count;
        
        ForEntities(entity, 0){
            update_editor_entity(entity);
            init_entity(entity);
            update_entity_collision_cells(entity);
        }
    }
}

void kill_player(){
    if (debug.god_mode && !state_context.we_got_a_winner || player_data.dead_man){ 
        return;
    }
    
    death_player_data = player_data;

    emit_particles(big_blood_emitter, player_entity->position, player_entity->up, 1, 1);
    player_data.dead_man = true;
    player_data.timers.died_time = core.time.game_time;
    play_sound(player_data.player_death_sound, player_entity->position);
    state_context.timers.background_flash_time = core.time.app_time;
}

void enter_editor_state(){
    game_state = EDITOR;
    state_context = {};
    
    // current_level_context = &game_level_context;
    // clear_level_context(&game_level_context);
    
    clean_up_scene();
    
    current_level_context = &editor_level_context;
    // copy_context(&editor_level_context, 
    core.time.game_time = 0;
    core.time.hitstop = 0;
    core.time.previous_dt = 0;
    
    // const char *temp_level_name = text_format("temp/TEMP_%s", session_context.current_level_name);
    
    // if (!load_level(temp_level_name)){
    //     print("Could not load level on entering editor state");    
    //     return;
    // }
    
    // copy_context(&editor_level_context, &loaded
    
    SetMusicVolume(tires_theme, 0);
    SetMusicVolume(wind_theme, 0);

    // ForEntities(entity, 0){
    //     // init_entity(entity);
    //     update_editor_entity(entity);
    // }
}

Vector2 screen_to_world(Vector2 pos){
    f32 zoom = session_context.cam.cam2D.zoom;

    f32 width = session_context.cam.width   ;
    f32 height = session_context.cam.height ;

    Vector2 screen_pos = pos;
    Vector2 world_pos = {(screen_pos.x - width * 0.5f) / session_context.cam.unit_size, (height * 0.5f - screen_pos.y) / session_context.cam.unit_size};
    world_pos /= zoom;
    world_pos = world_pos + session_context.cam.position;
    
    return world_pos;
}

inline Vector2 game_mouse_pos(){
    return screen_to_world(input.screen_mouse_position);
}

void fixed_game_update(f32 dt){
    frame_rnd = perlin_noise3(core.time.game_time, core.time.app_time, 5) * 2 - 1.0f;
    frame_on_circle_rnd = get_perlin_in_circle(1.0f);

    if (game_state == GAME){
        if (!session_context.playing_replay){
            //record input
            if (level_replay.input_record.count >= MAX_INPUT_RECORDS - 1){
                // level_replay.input_record.remove_first_half();
            } else{
                input.rnd_state = rnd_state;
                level_replay.input_record.add({input});
            }
        } else{
            if (session_context.game_frame_count >= level_replay.input_record.count){
                // debug_set_time_scale(0);
                session_context.playing_replay = false;
            } else{
                input = level_replay.input_record.get(session_context.game_frame_count).frame_input;
                // core.time = level_replay.input_record.get(session_context.game_frame_count).frame_time_data;
                rnd_state = input.rnd_state;
            }
        }
    }

    update_entities(dt);
    update_emitters(dt);
    update_particles(dt);
    
    // update camera
    if (game_state == GAME && player_entity && !debug.free_cam && (!is_in_death_instinct() || !is_death_instinct_threat_active())){
        f32 time_since_death_instinct = core.time.app_time - state_context.death_instinct.stop_time;
        
        f32 locked_speed_t = clamp01(time_since_death_instinct);
        f32 locked_speed_multiplier = lerp(0.001f, 1.0f, locked_speed_t * locked_speed_t * locked_speed_t);
    
        if (!state_context.cam_state.locked){
            Vector2 player_velocity = player_data.velocity;
            f32 target_speed_multiplier = 1;
        
            f32 time_since_heavy_collision = core.time.game_time - player_data.heavy_collision_time;
            if (magnitude(player_data.velocity) < 80 && core.time.game_time > 5 && time_since_heavy_collision <= 1.0f){
                player_velocity = player_data.heavy_collision_velocity;
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
                    if (entity->flags == ENEMY || !entity->enemy.in_agro || entity->enemy.dead_man || entity->flags & PROJECTILE){
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
                    
                    Vector2 addition = dir_to_enemy * (((60.0f * 0.35f) / session_context.cam.cam2D.zoom) / enemies_count);
                    // If enemy really close or to far we want to reduce amount of displacement
                    f32 min_threshold = (SCREEN_WORLD_SIZE / session_context.cam.cam2D.zoom);
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
                    
                    f32 max_x = (SCREEN_WORLD_SIZE / session_context.cam.cam2D.zoom) * 0.25f;
                    f32 max_y = (SCREEN_WORLD_SIZE / aspect_ratio / session_context.cam.cam2D.zoom) * 0.2f;
                    clamp(&additional_position.x, -max_x, max_x);
                    clamp(&additional_position.y, -max_y, max_y);
                }
                
                target_position += additional_position;
                
                if (additional_position != Vector2_zero){
                    if (dot(player_data.velocity, additional_position) < 0){
                        target_position += target_position_velocity_addition * 0.3f;
                    } else{
                        target_position -= target_position_velocity_addition * 0.5f;
                    }
                }
            }
            
            
            Vector2 vec_to_target = target_position - session_context.cam.target;
            Vector2 vec_to_player = player_entity->position - session_context.cam.target;
            
            f32 target_dot = dot(vec_to_target, vec_to_player);
            
            f32 speed_t = clamp01(player_speed / 200.0f);
            
            f32 target_speed = lerp(3, 10, speed_t * speed_t);
            target_speed *= target_speed_multiplier;
            
            session_context.cam.target = lerp(session_context.cam.target, target_position, clamp01(dt * target_speed));
            
            f32 cam_speed = lerp(10.0f, 100.0f, speed_t * speed_t);
            
            session_context.cam.position = lerp(session_context.cam.position, session_context.cam.target, clamp01(dt * cam_speed * locked_speed_multiplier));
            
        // Locked camera
        } else if ((!is_in_death_instinct() || !is_death_instinct_threat_active()) || debug.free_cam){
            session_context.cam.position = lerp(session_context.cam.position, session_context.cam.target, clamp01(dt * 4 * locked_speed_multiplier));
            if (magnitude(session_context.cam.target - session_context.cam.position) <= EPSILON){
                session_context.cam.position = session_context.cam.target;
            }
        }
    }
    
    state_context.cam_state.trauma -= dt * state_context.cam_state.trauma_decrease_rate;
    state_context.cam_state.trauma = clamp01(state_context.cam_state.trauma);
    
    state_context.explosion_trauma = clamp01(state_context.explosion_trauma - dt * 20);

    if (!is_in_death_instinct() || !is_death_instinct_threat_active()){
        f32 zoom_speed = game_state == GAME ? 3 : 10;
        session_context.cam.cam2D.zoom = lerp(session_context.cam.cam2D.zoom, session_context.cam.target_zoom, dt * zoom_speed);
    }
    
    if (abs(session_context.cam.cam2D.zoom - session_context.cam.target_zoom) <= EPSILON){
        session_context.cam.cam2D.zoom = session_context.cam.target_zoom;
    }

    input.press_flags = 0;
    input.sum_mouse_delta = Vector2_zero;
    input.sum_mouse_wheel = 0;
    
    session_context.game_frame_count += 1;
}

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
    Cam cam = session_context.cam;
    cam.unit_size = width / SCREEN_WORLD_SIZE; 
    cam.cam2D.target = (Vector2){ width/2.0f, height/2.0f };
    cam.cam2D.offset = (Vector2){ width/2.0f, height/2.0f };
    cam.width = width;
    cam.height = height;
    
    return cam;
}

void update_game(){
    frame_rnd = rnd01();
    frame_on_circle_rnd = rnd_on_circle();
    
    //update input
    if (!session_context.playing_replay){    
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
    }
    //end update input
    
    if (screen_size_changed){
        session_context.cam.width = screen_width;
        session_context.cam.height = screen_height;
        session_context.cam.unit_size = screen_width / SCREEN_WORLD_SIZE; 
        session_context.cam.cam2D.target = (Vector2){ screen_width/2.0f, screen_height/2.0f };
        session_context.cam.cam2D.offset = (Vector2){ screen_width/2.0f, screen_height/2.0f };
        
        aspect_ratio = session_context.cam.width / session_context.cam.height;
        
        UnloadRenderTexture(render.main_render_texture);
        UnloadRenderTexture(global_illumination_rt);
        UnloadRenderTexture(light_geometry_rt);
        
        render.main_render_texture = LoadRenderTexture(screen_width, screen_height);
        global_illumination_rt = LoadRenderTexture(screen_width, screen_height);
        light_geometry_rt = LoadRenderTexture(screen_width, screen_height);
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_SPACE)){
        if (game_state == EDITOR){
            enter_game_state(&editor_level_context, true);
        } else if (game_state == GAME || game_state == PAUSE){
            enter_editor_state();
        }
    } 
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_SPACE) && IsKeyUp(KEY_LEFT_SHIFT)){
        if (game_state == GAME){
            game_state = PAUSE;
            editor.in_editor_time = 0;
        } else if (game_state == PAUSE){
            game_state = GAME;
        }
    }
    
    if (game_state == GAME && !console.is_open){
        if (IsKeyPressed(KEY_T)){
            if (session_context.speedrun_timer.game_timer_active && player_data.dead_man){
                restart_game();
                session_context.speedrun_timer.time = 0;
            } else if (session_context.speedrun_timer.level_timer_active){
                // enter_editor_state();
                enter_game_state(&editor_level_context, true);
                session_context.speedrun_timer.time = 0;
            } else if (player_data.dead_man){
                b32 is_have_checkpoint = checkpoint_trigger_id != -1;
            
                // enter_editor_state();
                if (is_have_checkpoint){
                    // ForEntities(entity, 0){
                    //     free_entity(entity);
                    //     *entity = {};
                    // }
    
                    enter_game_state(&checkpoint_level_context, false);
                    // clear_level_context(&game_level_context);
                    // copy_level_context(&game_level_context, &checkpoint_level_context);
                    player_entity = checkpoint_player_entity;
                    // player_entity->position = checkpoint_player_entity.position;
                    // checkpoint_player_data.connected_entities_ids = player_data.connected_entities_ids;
                    player_data = checkpoint_player_data;
                    core.time = checkpoint_time;
                    state_context = checkpoint_state_context;
                    
                    player_data.velocity = Vector2_zero;
                    session_context.speedrun_timer.time = 0;
                } else{
                    enter_game_state(&editor_level_context, true);
                }
            }
        }
    }
    
    core.time.app_time += GetFrameTime();
    core.time.real_dt = GetFrameTime();
    
    // update death instinct
    local_persist b32 was_in_death_instinct = false;
    if (is_in_death_instinct() && is_death_instinct_threat_active() && game_state == GAME){
        f32 time_since_death_instinct = core.time.app_time - state_context.death_instinct.start_time;
        
        Entity *threat_entity = get_entity_by_id(state_context.death_instinct.threat_entity_id);
        Vector2 cam_position = player_entity->position + (threat_entity->position - player_entity->position) * 0.5f;
        
        if (state_context.death_instinct.last_reason == ENEMY_ATTACKING){
            f32 distance_t = (1.0f - clamp01(magnitude(threat_entity->position - player_entity->position) / get_death_instinct_radius(threat_entity->scale)));
            
            f32 t = EaseOutQuint(distance_t);
            core.time.target_time_scale = lerp(0.6f, 0.02f, t * t);
            session_context.cam.position        = lerp(session_context.cam.position, lerp(session_context.cam.target, cam_position, t * t), core.time.real_dt * t * 5);
            session_context.cam.cam2D.zoom      = lerp(session_context.cam.cam2D.zoom, lerp(session_context.cam.target_zoom, 0.55f, t * t), core.time.real_dt * t * 5);;
        } else if (state_context.death_instinct.last_reason == SWORD_WILL_EXPLODE){
            f32 distance_t = (1.0f - clamp01(state_context.death_instinct.angle_till_explode / 150.0f));
            f32 t = EaseOutQuint(distance_t);
            core.time.target_time_scale = lerp(0.6f, 0.015f, t);
            session_context.cam.position        = lerp(session_context.cam.position, lerp(session_context.cam.target, cam_position, t), core.time.real_dt * t * 5);
            session_context.cam.cam2D.zoom      = lerp(session_context.cam.cam2D.zoom, lerp(session_context.cam.target_zoom, 0.55f, t), core.time.real_dt * t * 5);;
        } else{
            session_context.cam.position        = lerp(session_context.cam.position, cam_position, clamp01(core.time.real_dt * 5));
            session_context.cam.cam2D.zoom      = lerp(session_context.cam.cam2D.zoom, 0.55f, clamp01(core.time.real_dt * 5));
            core.time.target_time_scale = lerp(core.time.target_time_scale, 0.03f, clamp01(core.time.real_dt * 10));
        }
        
        f32 instinct_t = time_since_death_instinct / state_context.death_instinct.duration;
        make_line(player_entity->position, threat_entity->position, Fade(RED, instinct_t * instinct_t));
        f32 radius_multiplier = lerp(80.0f, 10.0f, sqrtf(instinct_t));
        Color ring_color = Fade(ColorBrightness(RED, abs(sinf(core.time.app_time * lerp(1.0f, 10.0f, instinct_t)) * 0.8f - 0.5f)), instinct_t * 0.4f);
        make_ring_lines(threat_entity->position, 1.0f * radius_multiplier, 2.0f * radius_multiplier, 14, ring_color);
        was_in_death_instinct = true;
        
        if (time_since_death_instinct >= 0.2f && !state_context.death_instinct.played_effects){
            play_sound("DeathInstinct", 2);
            state_context.death_instinct.played_effects = true;
        }
    } else if (was_in_death_instinct){
        stop_death_instinct();
        // core.time.target_time_scale = 1;
        was_in_death_instinct = false;
    } else if (game_state == GAME){
        core.time.target_time_scale = lerp(core.time.target_time_scale, 1.0f, clamp01(core.time.real_dt * 2));        
        if (1.0f - core.time.target_time_scale <= 0.01f){
            core.time.target_time_scale = 1;
        }
    }
    
    if (game_state == GAME){
        core.time.unscaled_dt = GetFrameTime();
        if (core.time.hitstop > 0){
            core.time.time_scale = fminf(core.time.time_scale, 0.1f);
            core.time.hitstop -= core.time.real_dt;
        } 
        
        if (core.time.hitstop <= 0){
            core.time.time_scale = core.time.target_time_scale;
        }
        
        if (core.time.debug_target_time_scale != 1 && (core.time.debug_target_time_scale < core.time.target_time_scale || core.time.hitstop <= 0)){
           core.time.time_scale = core.time.debug_target_time_scale; 
        }
        
        core.time.dt = GetFrameTime() * core.time.time_scale;
    } else if (game_state == EDITOR || game_state == PAUSE){
        core.time.unscaled_dt = 0;
        core.time.dt          = 0;
    }

    
    if (game_state == EDITOR || game_state == PAUSE){
        update_editor_ui();
        update_editor();
    }
    
    update_console();
    
    if (game_state == GAME){
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
        } else if (player_data.dead_man){
            color = RED;
            session_context.speedrun_timer.paused = true;
        }
        
        if (!session_context.speedrun_timer.paused){
            session_context.speedrun_timer.time += core.time.dt;
        }
        
        const char *title_and_time = text_format("%s\n%.4f", session_context.speedrun_timer.level_timer_active ? session_context.current_level_name : "Game speedrun", session_context.speedrun_timer.time);
        make_ui_text(title_and_time, {screen_width * 0.46f, 5}, "speedrun_timer", color, 22);
    }
    
    // update_entities();
    // update_emitters();
    // update_particles();
    
    if (IsKeyPressed(KEY_SPACE) && IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL)){
        debug.free_cam = !debug.free_cam;
        if (!debug.free_cam){
            session_context.cam.target_zoom = debug.last_zoom;
        } else{
            debug.last_zoom = session_context.cam.target_zoom;
        }
    }
    
    if (IsKeyPressed(KEY_L) && IsKeyDown(KEY_RIGHT_ALT)){
        debug_unlock_camera();
    }
    
    if (game_state == GAME && player_entity && !debug.free_cam){
    } else{
        f32 zoom = session_context.cam.target_zoom;

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            session_context.cam.position += ((Vector2){-input.mouse_delta.x / zoom, input.mouse_delta.y / zoom}) / (session_context.cam.unit_size);
        }
        if (input.mouse_wheel != 0 && !console.is_open && !editor.create_box_active){
            if (input.mouse_wheel > 0 && zoom < 5 || input.mouse_wheel < 0 && zoom > 0.1f){
                session_context.cam.target_zoom += input.mouse_wheel * 0.05f;
            }
        }
    }
    
    if (editor.update_cam_view_position){
        session_context.cam.view_position = session_context.cam.position;
    }
    
    draw_game();
    
    UpdateMusicStream(ambient_theme);
    UpdateMusicStream(wind_theme);
    UpdateMusicStream(tires_theme);
    
    session_context.just_entered_game_state = false;
    
    
    // We do this so lights don't bake all at one frame 
    session_context.baked_shadows_this_frame = false;
} // update game end

void update_color_changer(Entity *entity, f32 dt){
    Color_Changer *changer = &entity->color_changer;
    
    if (changer->changing){
        f32 t = abs(sinf(core.time.app_time * changer->change_time));
        entity->color = lerp(changer->start_color, changer->target_color, t);
    } else if (entity->flags & EXPLOSIVE){
        Color target_color = ColorBrightness(changer->start_color, 2);
        f32 t = abs(sinf(core.time.game_time * changer->change_time));
        entity->color = lerp(changer->start_color, target_color, t);
        
        if (entity->light_index > -1){
            current_level_context->lights.get_ptr(entity->light_index)->color = lerp(target_color, Fade(ColorBrightness(ORANGE, 0.3f), 0.9f), t);
            current_level_context->lights.get_ptr(entity->light_index)->radius = get_explosion_radius(entity) * 2 * lerp(0.9f, 1.3f, t);
        }
    } else if (changer->interpolating) {
        entity->color = lerp(changer->start_color, changer->target_color, changer->progress);
    }
    
    if (entity->flags & BLOCKER){
        entity->color = ColorTint(entity->color, RAYWHITE);
    }
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

void fill_arr_with_normals(Array<Vector2, MAX_VERTICES> *normals, Array<Vector2, MAX_VERTICES> vertices){
    //@INCOMPLETE now only for rects, need to find proper algorithm for calculating edge normals from vertices because 
    //we add vertices in triangle shape
    
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
    center += {(pivot.x - 0.5f) * bounds.size.x, (pivot.y - 0.5f) * bounds.size.y};
    
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
            
            if (!other || other->destroyed || !other->enabled || other->flags <= 0 || ((other->flags & include_flags) <= 0 && include_flags > 0) || (other->hidden && game_state == GAME) || other->id == my_id || added_collision_ids.contains(other->id)){
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

Entity *get_entity_by_index(i32 index){
    if (!current_level_context->entities.has_index(index)){
        //log error
        print("Attempt to get empty entity by index");
        return NULL;
    }
    
    return current_level_context->entities.get_ptr(index);
}

Entity *get_entity_by_id(i32 id){
    if (id == -1 || !current_level_context->entities.has_key(id)){
        return NULL;
    }
    
    return current_level_context->entities.get_by_key_ptr(id);
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
    editor.undo_actions.add(undo_action);
    
    if (editor.undo_actions.count >= MAX_UNDOS){
        editor.undo_actions.remove_first_half();
    }
    
    editor.max_undos_added = editor.undo_actions.count;
}

void undo_add_position(Entity *entity, Vector2 position_change){
    Undo_Action undo_action;
    undo_action.position_change = position_change;
    undo_action.entity_id = entity->id;
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
        Undo_Action undo_action;
        undo_action.entity_was_deleted = true;
        copy_entity(&undo_action.deleted_entity, editor.selected_entity);
        undo_action.entity_id = undo_action.deleted_entity.id;
        add_undo_action(undo_action);
    }
    entity->destroyed = true;
    editor.selected_entity = NULL;
    editor.dragging_entity = NULL;
    editor.cursor_entity   = NULL;
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

void update_editor_ui(){
    //inspector logic
    Entity *selected = editor.selected_entity;
    if (selected){
        Vector2 inspector_size = {screen_width * 0.2f, screen_height * 0.6f};
        Vector2 inspector_position = {screen_width - inspector_size.x - inspector_size.x * 0.1f, 0 + inspector_size.y * 0.05f};
        make_ui_image(inspector_position, inspector_size, {0, 0}, SKYBLUE * 0.7f, "inspector_window");
        f32 height_add = 30 * UI_SCALING;
        f32 v_pos = inspector_position.y + height_add + 40;
        
        make_ui_text(text_format("ID: %d", selected->id), {inspector_position.x + 100, inspector_position.y - 10}, 18, WHITE, "inspector_id"); 
        
        make_ui_text(text_format("Name: ", selected->id), {inspector_position.x, inspector_position.y + 10}, 24, BLACK, "inspector_id"); 
        if (make_input_field(text_format("%s", selected->name), {inspector_position.x + 65, inspector_position.y + 10}, {200, 25}, "inspector_name") ){
            str_copy(selected->name, focus_input_field.content);
        }
        
        make_ui_text("POSITION", {inspector_position.x + 100, inspector_position.y + 40}, 24, WHITE * 0.9f, "inspector_pos");
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
        
        make_ui_text("SCALE", {inspector_position.x + 100, inspector_position.y + 20 + v_pos - height_add}, 24, WHITE * 0.9f, "inspector_scale");
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
        if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Entity settings", "entity_settings")){
            editor.draw_entity_settings = !editor.draw_entity_settings;
        }
        v_pos += height_add;
        
        if (editor.draw_entity_settings){
            make_ui_text("Hidden: ", {inspector_position.x + 5, v_pos}, "text_entity_hidden");
            if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->hidden, "toggle_entity_hidden")){
                selected->hidden = !selected->hidden;
            }
            v_pos += height_add;
            
            make_ui_text("Spawn enemy when no ammo: ", {inspector_position.x + 5, v_pos}, "text_spawn_no_ammo");
            if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->spawn_enemy_when_no_ammo, "toggle_spawn_no_ammo")){
                selected->spawn_enemy_when_no_ammo = !selected->spawn_enemy_when_no_ammo;
            }
            v_pos += height_add;
            
            make_ui_text("Physics object: ", {inspector_position.x + 5, v_pos}, "physics_object");
            if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->flags & PHYSICS_OBJECT, "physics_object")){
                selected->flags ^= PHYSICS_OBJECT;
            }
            v_pos += height_add;
            
            if (selected->flags & PHYSICS_OBJECT){
                make_ui_text("Simulating: ", {inspector_position.x + 5, v_pos}, "physics_simulating");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->physics_object.simulating, "physics_simulating")){
                    selected->physics_object.simulating = !selected->physics_object.simulating;
                }
                v_pos += height_add;
                
                make_ui_text("On rope: ", {inspector_position.x + 5, v_pos}, "on_rope");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->physics_object.on_rope, "on_rope")){
                    selected->physics_object.on_rope = !selected->physics_object.on_rope;
                }
                v_pos += height_add;
                
                make_ui_text("Rotate by velocity: ", {inspector_position.x + 5, v_pos}, "physics_rotate_by_velocity");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->physics_object.rotate_by_velocity, "physics_rotate_by_velocity")){
                    selected->physics_object.rotate_by_velocity = !selected->physics_object.rotate_by_velocity;
                }
                v_pos += height_add;
                
                make_ui_text("Gravity multiplier: ", {inspector_position.x + 5, v_pos}, "physics_gravity_multiplier");
                if (make_input_field(text_format("%.1f", selected->physics_object.gravity_multiplier), {inspector_position.x + 250, v_pos}, {100, 25}, "physics_gravity_multiplier")){
                    selected->physics_object.gravity_multiplier = to_f32(focus_input_field.content);
                }
                v_pos += height_add;
                
                make_ui_text("Mass: ", {inspector_position.x + 5, v_pos}, "physics_mass");
                if (make_input_field(text_format("%.1f", selected->physics_object.mass), {inspector_position.x + 250, v_pos}, {100, 25}, "physics_mass")){
                    selected->physics_object.mass = clamp(to_f32(focus_input_field.content), 0.01f, 100000.0f);
                }
                v_pos += height_add;
            }
            
            make_ui_text("Move sequence: ", {inspector_position.x + 5, v_pos}, "text_entity_move_sequence");
            if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->flags & MOVE_SEQUENCE, "toggle_entity_move_sequence")){
                selected->flags ^= MOVE_SEQUENCE;
            }
            v_pos += height_add;
            
            if (selected->flags & MOVE_SEQUENCE){
                make_ui_text("Moving: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_moving");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->move_sequence.moving, "toggle_entity_move_sequence_moving")){
                    selected->move_sequence.moving = !selected->move_sequence.moving;
                }
                v_pos += height_add;
                make_ui_text("Loop: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_loop");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->move_sequence.loop, "toggle_entity_move_sequence_loop")){
                    selected->move_sequence.loop = !selected->move_sequence.loop;
                }
                v_pos += height_add;
                
                make_ui_text("Rotate: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_rotate");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->move_sequence.rotate, "toggle_entity_move_sequence_rotate")){
                    selected->move_sequence.rotate = !selected->move_sequence.rotate;
                }
                v_pos += height_add;
            
                make_ui_text("Speed: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_speed");
                if (make_input_field(text_format("%.1f", selected->move_sequence.speed), {inspector_position.x + 100, v_pos}, 100, "move_sequence_speed")){
                    selected->move_sequence.speed = to_f32(focus_input_field.content);
                }
                v_pos += height_add;
                
                make_ui_text("Speed related player distance: ", {inspector_position.x + 25, v_pos}, "move_sequence_speed_related_player_distance");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.8f, v_pos}, selected->move_sequence.speed_related_player_distance, "move_sequence_speed_related_player_distance")){
                    selected->move_sequence.speed_related_player_distance = !selected->move_sequence.speed_related_player_distance;
                }
                v_pos += height_add;
                if (selected->move_sequence.speed_related_player_distance){
                    make_ui_text("Min distance: ", {inspector_position.x + 25, v_pos}, "move_sequence_min_distance");
                    if (make_input_field(text_format("%.1f", selected->move_sequence.min_distance), {inspector_position.x + 170, v_pos}, 100, "move_sequence_min_distance")){
                        selected->move_sequence.min_distance = to_f32(focus_input_field.content);
                    }
                    v_pos += height_add;
                    make_ui_text("Max distance: ", {inspector_position.x + 25, v_pos}, "move_sequence_max_distance");
                    if (make_input_field(text_format("%.1f", selected->move_sequence.max_distance), {inspector_position.x + 170, v_pos}, 100, "move_sequence_max_distance")){
                        selected->move_sequence.max_distance = to_f32(focus_input_field.content);
                    }
                    v_pos += height_add;
                    make_ui_text("Max distance speed: ", {inspector_position.x + 25, v_pos}, "move_sequence_max_distance_speed");
                    if (make_input_field(text_format("%.1f", selected->move_sequence.max_distance_speed), {inspector_position.x + 170, v_pos}, 100, "move_sequence_max_distance_speed")){
                        selected->move_sequence.max_distance_speed = to_f32(focus_input_field.content);
                    }
                    v_pos += height_add;
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
        } // entity inspector end
        
        if (selected->flags & NOTE){
            make_ui_text("NOTE draw in game: ", {inspector_position.x + 5, v_pos}, "note_draw_in_game");
            Note *note = current_level_context->notes.get_ptr(selected->note_index);
            if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, note->draw_in_game, "note_draw_in_game")){
                note->draw_in_game = !note->draw_in_game;
            }
            v_pos += height_add;
            
            // make_color_picker(inspector_position, inspector_size, v_pos, &note->in_game_color);
            // v_pos += height_add;
        }
        
        // inspector light inspector
        if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Light settings", "light_settings")){
            editor.draw_light_settings = !editor.draw_light_settings;
        }
        v_pos += height_add;
        
        if (editor.draw_light_settings){
            make_ui_text("Make light: ", {inspector_position.x + 5, v_pos}, "make_light");
            if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->flags & LIGHT, "make_light")){
                selected->flags ^= LIGHT;
                if (selected->flags & LIGHT){
                    init_entity_light(selected);                    
                } else{
                    free_entity_light(selected);
                }
            }
            v_pos += height_add;
            
            if (selected->flags & LIGHT && selected->light_index >= 0){
                Light *light = current_level_context->lights.get_ptr(selected->light_index);
                
                make_color_picker(inspector_position, inspector_size, v_pos, &light->color);
                v_pos += height_add;
                
                make_ui_text("Light radius: ", {inspector_position.x + 5, v_pos}, "light_radius");
                if (make_input_field(text_format("%.2f", light->radius), {inspector_position.x + 100, v_pos}, {150, 20}, "light_radius") ){
                    light->radius = to_f32(focus_input_field.content);
                }
                v_pos += height_add;
                
                make_ui_text("Light opacity: ", {inspector_position.x + 5, v_pos}, "light_opacity");
                if (make_input_field(text_format("%.2f", light->opacity), {inspector_position.x + 100, v_pos}, {150, 20}, "light_opacity") ){
                    light->opacity = to_f32(focus_input_field.content);
                }
                v_pos += height_add;
                
                make_ui_text("Light power: ", {inspector_position.x + 5, v_pos}, "light_power");
                if (make_input_field(text_format("%.2f", light->power), {inspector_position.x + 100, v_pos}, {150, 20}, "light_power") ){
                    light->power = to_f32(focus_input_field.content);
                }
                v_pos += height_add;

                
                make_ui_text("Bake shadows: ", {inspector_position.x + 5, v_pos}, "light_bake_shadows", 17, ColorBrightness(light->bake_shadows ? GREEN : RED, 0.5f));
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, light->bake_shadows, "light_bake_shadows")){
                    light->bake_shadows = !light->bake_shadows;
                    init_entity_light(selected, light, true);
                }
                v_pos += height_add;
                
                make_ui_text("Make shadows (expensive): ", {inspector_position.x + 5, v_pos}, "light_make_shadows");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, light->make_shadows, "light_make_shadows")){
                    light->make_shadows = !light->make_shadows;
                    init_entity_light(selected, light, true);
                }
                v_pos += height_add;

                if (light->make_shadows){
                    make_ui_text("Shadows Size flags: ", {inspector_position.x + 5, v_pos}, "shadows_size_flags");
                    v_pos += height_add;
                    make_light_size_picker(inspector_position, inspector_size, v_pos, height_add, &light->shadows_size_flags, selected);
                    v_pos += height_add * 2;
                }
                
                make_ui_text("Make backshadows: ", {inspector_position.x + 5, v_pos}, "light_make_backshadows");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, light->make_backshadows, "light_make_backshadows")){
                    light->make_backshadows = !light->make_backshadows;
                    init_entity_light(selected, light, true);
                }
                v_pos += height_add;

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
            if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Trigger settings", "trigger_settings")){
                editor.draw_trigger_settings = !editor.draw_trigger_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_trigger_settings){
                make_ui_text("Activate on player: ", {inspector_position.x + 5, v_pos}, "text_player_touch");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.player_touch, "toggle_player_touch")){
                    selected->trigger.player_touch = !selected->trigger.player_touch;
                }
                v_pos += height_add;
                
                make_ui_text("Die after trigger: ", {inspector_position.x + 5, v_pos}, "trigger_die_after_trigger");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.die_after_trigger, "trigger_die_after_trigger")){
                    selected->trigger.die_after_trigger = !selected->trigger.die_after_trigger;
                }
                v_pos += height_add;
                
                make_ui_text("Kill player: ", {inspector_position.x + 5, v_pos}, "trigger_kill_player");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.kill_player, "trigger_kill_player")){
                    selected->trigger.kill_player = !selected->trigger.kill_player;
                }
                v_pos += height_add;
                
                make_ui_text("Kill enemies: ", {inspector_position.x + 5, v_pos}, "trigger_kill_enemies");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.kill_enemies, "trigger_kill_enemies")){
                    selected->trigger.kill_enemies = !selected->trigger.kill_enemies;
                }
                v_pos += height_add;
                
                make_ui_text("Open or close Doors: ", {inspector_position.x + 5, v_pos}, "trigger_open_doors");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.open_doors, "toggle_open_doors")){
                    selected->trigger.open_doors = !selected->trigger.open_doors;                 
                }
                v_pos += height_add;
                
                make_ui_text("Start physics: ", {inspector_position.x + 5, v_pos}, "trigger_start_physics_simulation");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.start_physics_simulation, "trigger_start_physics_simulation")){
                    selected->trigger.start_physics_simulation = !selected->trigger.start_physics_simulation;                 
                }
                v_pos += height_add;
                
                make_ui_text("Track enemies: ", {inspector_position.x + 5, v_pos}, "trigger_track_enemies");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.track_enemies, "toggle_track_enemies")){
                    selected->trigger.track_enemies = !selected->trigger.track_enemies;                 
                }
                v_pos += height_add;
                
                make_ui_text("Lines to tracked: ", {inspector_position.x + 5, v_pos}, "trigger_draw_lines_to_tracked");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.draw_lines_to_tracked, "trigger_draw_lines_to_tracked")){
                    selected->trigger.draw_lines_to_tracked = !selected->trigger.draw_lines_to_tracked;                 
                }
                v_pos += height_add;
                
                make_ui_text("Agro enemies: ", {inspector_position.x + 5, v_pos}, "text_trigger_agro_enemies");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.agro_enemies, "toggle_agro_enemies")){
                    selected->trigger.agro_enemies = !selected->trigger.agro_enemies;                 
                }
                v_pos += height_add;
                
                make_ui_text("Shows entities: ", {inspector_position.x + 5, v_pos}, "trigger_shows_entities", ColorBrightness(SKYBLUE, 0.5f));
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.shows_entities, "toggle_trigger_show_entity")){
                    selected->trigger.shows_entities = !selected->trigger.shows_entities;                 
                }
                v_pos += height_add;
                
                make_ui_text("Starts moving sequence: ", {inspector_position.x + 5, v_pos}, "trigger_starts_moving_sequence");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.starts_moving_sequence, "toggle_starts_moving_sequence")){
                    selected->trigger.starts_moving_sequence = !selected->trigger.starts_moving_sequence;                 
                }
                v_pos += height_add;
                
                Color cam_section_color = ColorBrightness(PINK, 0.4f);
                make_ui_text("Change zoom: ", {inspector_position.x + 5, v_pos}, "trigger_change_zoom_text", cam_section_color);
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.change_zoom, "toggle_change_zoom")){
                    selected->trigger.change_zoom = !selected->trigger.change_zoom;                 
                }
                v_pos += height_add;
                if (selected->trigger.change_zoom){
                    make_ui_text("Zoom: ", {inspector_position.x + 5, v_pos}, "trigger_change_zooom_text", cam_section_color);
                    if (make_input_field(text_format("%.2f", selected->trigger.zoom_value), {inspector_position.x + 100, v_pos}, {150, 20}, "trigger_zoom_name") ){
                        selected->trigger.zoom_value = to_f32(focus_input_field.content);
                    }
                    v_pos += height_add;
                }
                
                make_ui_text("Cam rails horizontal: ", {inspector_position.x + 5, v_pos}, "trigger_start_cam_rails_horizontal", cam_section_color);
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.start_cam_rails_horizontal, "trigger_start_cam_rails_horizontal")){
                    selected->trigger.start_cam_rails_horizontal = !selected->trigger.start_cam_rails_horizontal;                 
                    init_entity(selected);
                }
                v_pos += height_add;
                make_ui_text("Cam rails vertical: ", {inspector_position.x + 5, v_pos}, "trigger_start_cam_rails_vertical", cam_section_color);
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.start_cam_rails_vertical, "trigger_start_cam_rails_vertical")){
                    selected->trigger.start_cam_rails_vertical = !selected->trigger.start_cam_rails_vertical;                 
                    init_entity(selected);
                }
                v_pos += height_add;
                make_ui_text("Stop cam rails: ", {inspector_position.x + 5, v_pos}, "trigger_stop_cam_rails", cam_section_color);
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.stop_cam_rails, "trigger_stop_cam_rails")){
                    selected->trigger.stop_cam_rails = !selected->trigger.stop_cam_rails;                 
                    init_entity(selected);
                }
                v_pos += height_add;
                
                make_ui_text("Lock camera: ", {inspector_position.x + 5, v_pos}, "lock_camera_text", cam_section_color);
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.lock_camera, "lock_camera")){
                    selected->trigger.lock_camera = !selected->trigger.lock_camera;                 
                    if (selected->trigger.lock_camera && selected->trigger.locked_camera_position == Vector2_zero){
                        selected->trigger.locked_camera_position = selected->position;
                    }
                }
                v_pos += height_add;
                
                make_ui_text("Unlock camera: ", {inspector_position.x + 5, v_pos}, "unlock_camera_text", cam_section_color);
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.unlock_camera, "unlock_camera")){
                    selected->trigger.unlock_camera = !selected->trigger.unlock_camera;                 
                }
                v_pos += height_add;
                
                make_ui_text("Play sound: ", {inspector_position.x + 5, v_pos}, "trigger_play_sound");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.play_sound, "toggle_play_sound")){
                    selected->trigger.play_sound = !selected->trigger.play_sound;                 
                }
                v_pos += height_add;
                if (selected->trigger.play_sound){
                    make_ui_text("Sound name: ", {inspector_position.x + 5, v_pos}, "trigger_play_sound_name_text");
                    if (make_input_field(selected->trigger.sound_name, {inspector_position.x + 100, v_pos}, {150, 20}, "trigger_sound_name") ){
                        str_copy(selected->trigger.sound_name, focus_input_field.content);
                    }
                    v_pos += height_add;
                }

                
                make_ui_text("Load level: ", {inspector_position.x + 5, v_pos}, "trigger_load_level");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->trigger.load_level, "toggle_load_level")){
                    selected->trigger.load_level = !selected->trigger.load_level;                 
                }
                v_pos += height_add;
                
                if (selected->trigger.load_level){
                    make_ui_text("Level name: ", {inspector_position.x + 5, v_pos}, "trigger_load_level_name_text");
                    if (make_input_field(selected->trigger.level_name, {inspector_position.x + 100, v_pos}, {150, 20}, "trigger_load_level_name") ){
                        str_copy(selected->trigger.level_name, focus_input_field.content);
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
            if (selected->trigger.lock_camera){
                make_ui_text("Ctrl+R: Locked cam position", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "locked_cam_position");
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
        
        // enemy inspector
        if (selected->flags & ENEMY){
            if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Enemy settings", "enemy_settings")){
                editor.draw_enemy_settings = !editor.draw_enemy_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_enemy_settings){
                make_ui_text("Gives ammo: ", {inspector_position.x + 5, v_pos}, "enemy_gives_ammo");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->enemy.gives_ammo, "enemy_gives_ammo")){
                    selected->enemy.gives_ammo = !selected->enemy.gives_ammo;
                }
                v_pos += height_add;
                if (selected->enemy.gives_ammo){
                    make_ui_text("Gives full ammo: ", {inspector_position.x + 5, v_pos}, "gives_full_ammo");
                    if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->enemy.gives_full_ammo, "gives_full_ammo")){
                        selected->enemy.gives_full_ammo = !selected->enemy.gives_full_ammo;
                    }
                    v_pos += height_add;
                }
                
                make_ui_text("Explosive: ", {inspector_position.x + 5, v_pos}, "text_enemy_explosive");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->flags & EXPLOSIVE, "toggle_enemy_explosive")){
                    selected->flags ^= EXPLOSIVE;
                    if (!(selected->flags & EXPLOSIVE)){
                        free_entity_light(selected);
                    }
                    init_entity(selected);
                }
                v_pos += height_add;
                
                if (selected->flags & EXPLOSIVE){
                    make_ui_text("Explosion radius: ", {inspector_position.x + 5, v_pos}, "explosive_radius_multiplier");
                    if (make_input_field(text_format("%.2f", selected->enemy.explosive_radius_multiplier), {inspector_position.x + inspector_size.x * 0.5f, v_pos}, 100, "explosive_radius_multiplier")){
                        selected->enemy.explosive_radius_multiplier = fmax(to_f32(focus_input_field.content), 0);
                        init_entity(selected);
                    }
                    v_pos += height_add;
                }
                
                make_ui_text("Blocker: ", {inspector_position.x + 5, v_pos}, "text_enemy_blocker");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->flags & BLOCKER, "toggle_enemy_blocker")){
                    selected->flags ^= BLOCKER;
                    init_entity(selected);
                }
                v_pos += height_add;
                
                if (selected->flags & BLOCKER){
                    make_ui_text("Blocker immortal: ", {inspector_position.x + 5, v_pos}, "text_enemy_blocker_immortal");
                    if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->enemy.blocker_immortal, "toggle_enemy_blocker_immortal")){
                        selected->enemy.blocker_immortal = !selected->enemy.blocker_immortal;
                        init_entity(selected);
                    }
                    v_pos += height_add;

                    if (!selected->enemy.blocker_immortal){
                        make_ui_text("Blocker clockwise: ", {inspector_position.x + 5, v_pos}, "text_enemy_blocker_clocksize");
                        if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->enemy.blocker_clockwise, "toggle_enemy_blocker_clockwise")){
                            selected->enemy.blocker_clockwise = !selected->enemy.blocker_clockwise;
                            init_entity(selected);
                        }
                        v_pos += height_add;
                    }
                }
                
                make_ui_text("Shoot blocker: ", {inspector_position.x + 5, v_pos}, "text_enemy_shoot_blocker");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->flags & SHOOT_BLOCKER, "toggle_enemy_shoot_blocker")){
                    selected->flags ^= SHOOT_BLOCKER;
                    init_entity(selected);
                }
                v_pos += height_add;
                
                if (selected->flags & SHOOT_BLOCKER){
                    make_ui_text("Shoot blocker immortal: ", {inspector_position.x + 5, v_pos}, "text_enemy_shoot_blocker_immortal");
                    if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->enemy.shoot_blocker_immortal, "toggle_enemy_shoot_blocker_immortal")){
                        selected->enemy.shoot_blocker_immortal = !selected->enemy.shoot_blocker_immortal;
                        init_entity(selected);
                    }
                    v_pos += height_add;
                }
                
                make_ui_text("Sword size required: ", {inspector_position.x + 5, v_pos}, "enemy_sword_size_required");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->flags & SWORD_SIZE_REQUIRED, "enemy_sword_size_required")){
                    selected->flags ^= SWORD_SIZE_REQUIRED;
                    init_entity(selected);
                }
                v_pos += height_add;

                if (selected->flags & SWORD_SIZE_REQUIRED){
                    make_ui_text("Big (1) or small (0) killable: ", {inspector_position.x + 5, v_pos}, "enemy_big_or_small_killable");
                    if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->enemy.big_sword_killable, "enemy_big_or_small_killable")){
                        selected->enemy.big_sword_killable = !selected->enemy.big_sword_killable;
                        init_entity(selected);
                    }
                    v_pos += height_add;
                }
            }
        
            if (selected->flags & BLOCKER){
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
            if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Centipede settings", "centipede_settings")){
                editor.draw_centipede_settings = !editor.draw_centipede_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_centipede_settings){
                make_ui_text("Spikes on right: ", {inspector_position.x + 5, v_pos}, "spikes_on_right");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->centipede.spikes_on_right, "spikes_on_right")){
                    selected->centipede.spikes_on_right = !selected->centipede.spikes_on_right;
                }
                v_pos += height_add;
                make_ui_text("Spikes on left: ", {inspector_position.x + 5, v_pos}, "spikes_on_left");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->centipede.spikes_on_left, "spikes_on_left")){
                    selected->centipede.spikes_on_left = !selected->centipede.spikes_on_left;
                }
                v_pos += height_add;
                
                make_ui_text("Segments count: ", {inspector_position.x + 25, v_pos}, "segments_count");
                if (make_input_field(text_format("%d", selected->centipede.segments_count), {inspector_position.x + 100, v_pos}, 100, "segments_count")){
                    selected->centipede.segments_count = fmin(to_i32(focus_input_field.content), MAX_CENTIPEDE_SEGMENTS);
                }
                v_pos += height_add;
            }
        }
        
        // jumps shooter inspector
        if (selected->flags & JUMP_SHOOTER){
            if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Jump shooter settings", "jump_shooter_settings")){
                editor.draw_jump_shooter_settings = !editor.draw_jump_shooter_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_jump_shooter_settings){
                make_ui_text("Shots count: ", {inspector_position.x + 5, v_pos}, "jump_shooter_shots_count");
                if (make_input_field(text_format("%d", selected->jump_shooter.shots_count), {inspector_position.x + 100, v_pos}, 100, "jump_shooter_shots_count")){
                    selected->jump_shooter.shots_count = to_i32(focus_input_field.content);
                }
                v_pos += height_add;
                
                make_ui_text("Spread: ", {inspector_position.x + 5, v_pos}, "jump_shooter_spread");
                if (make_input_field(text_format("%.1f", selected->jump_shooter.spread), {inspector_position.x + 100, v_pos}, 100, "jump_shooter_spread")){
                    selected->jump_shooter.spread = clamp(to_f32(focus_input_field.content), 0.0f, 180.0f);
                }
                v_pos += height_add;
                
                make_ui_text("Explosive count: ", {inspector_position.x + 5, v_pos}, "jump_shooter_explosive_count");
                if (make_input_field(text_format("%d", selected->jump_shooter.explosive_count), {inspector_position.x + 100, v_pos}, 100, "jump_shooter_explosive_count")){
                    selected->jump_shooter.explosive_count = fmin(fmin(to_i32(focus_input_field.content), 64), selected->jump_shooter.shots_count);
                }
                v_pos += height_add;

                make_ui_text("Shoot sword blockers: ", {inspector_position.x + 5, v_pos}, "shoot_sword_blockers");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->jump_shooter.shoot_sword_blockers, "shoot_sword_blockers")){
                    selected->jump_shooter.shoot_sword_blockers = !selected->jump_shooter.shoot_sword_blockers;
                }
                v_pos += height_add;
                
                if (selected->jump_shooter.shoot_sword_blockers){
                    v_pos += height_add;
                    make_ui_text("Sword blockers immortal: ", {inspector_position.x + 5, v_pos}, "shoot_sword_blockers_immortal");
                    if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->jump_shooter.shoot_sword_blockers_immortal, "shoot_sword_blockers_immortal")){
                        selected->jump_shooter.shoot_sword_blockers_immortal = !selected->jump_shooter.shoot_sword_blockers_immortal;
                    }
                    v_pos += height_add;
                }
                
                make_ui_text("Shoot bullet blockers: ", {inspector_position.x + 5, v_pos}, "shoot_bullet_blockers");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->jump_shooter.shoot_bullet_blockers, "shoot_bullet_blockers")){
                    selected->jump_shooter.shoot_bullet_blockers = !selected->jump_shooter.shoot_bullet_blockers;
                }
                v_pos += height_add;
            }
        }

        if (selected->flags & DOOR){
            if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Door settings", "door_settings")){
                editor.draw_door_settings = !editor.draw_door_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_door_settings){
                make_ui_text("Open: ", {inspector_position.x + 5, v_pos}, "text_door_open");
                if (make_ui_toggle({inspector_position.x + inspector_size.x * 0.6f, v_pos}, selected->door.is_open, "toggle_door_open_closed")){
                    selected->door.is_open = !selected->door.is_open;
                    init_entity(selected);
                }
                v_pos += height_add;
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
            
            local_persist f32 create_box_scrolled = 0;
            create_box_scrolled += GetMouseWheelMove();
            Vector2 obj_position = field_position + Vector2_up * field_size.y * (fitting_count + 1) + Vector2_up * create_box_scrolled + Vector2_right * field_size.x * 0.2f;
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

void update_editor(){
    if (game_state == EDITOR){
        Vector2 grid_target_pos = editor.player_spawn_point;
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
    
    f32 zoom = session_context.cam.target_zoom;

    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        moving_editor_cam = true;
    }
    
    b32 need_move_vertices = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select;
    b32 need_snap_vertex = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_V);
    
    i32 selected_vertex_index;
    Vector2 closest_vertex_global;
    f32 distance_to_closest_vertex = INFINITY;
    
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
            editor_spawn_entity("enemy_base", input.mouse_position);
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
        
        //editor vertices
        for (i32 v = 0; v < e->vertices.count && (need_move_vertices || need_snap_vertex); v++){
            Vector2 *vertex = e->vertices.get_ptr(v);
            
            Vector2 vertex_global = global(e, *vertex);
            
            if (need_move_vertices && (!moving_vertex_entity_candidate || (editor.selected_entity && e->id == editor.selected_entity->id))){
                if (check_col_circles({input.mouse_position, 1}, {vertex_global, 0.5f * (0.4f / session_context.cam.cam2D.zoom)})){
                    moving_vertex_entity_candidate = e;
                    moving_vertex_candidate = v;
                }
            }
            
            if (editor.moving_vertex && need_snap_vertex && e->id != editor.moving_vertex_entity->id){
                f32 sqr_distance = sqr_magnitude(global(editor.moving_vertex_entity, *editor.moving_vertex) - vertex_global);
                if (sqr_distance < distance_to_closest_vertex){
                    distance_to_closest_vertex = sqr_distance;
                    closest_vertex_global = vertex_global;
                }
            }
        }
        
        //editor move sequence points        
        // We don't want move points if selected entity already is move sequence or if selected is trigger with cam rails.
        b32 cannot_move_points = editor.selected_entity && ((editor.selected_entity->flags & MOVE_SEQUENCE || (editor.selected_entity->flags & TRIGGER && editor.selected_entity->trigger.cam_rails_points.count > 0)) && editor.selected_entity->id != e->id);
        for (i32 p = 0; e->flags & MOVE_SEQUENCE && IsKeyDown(KEY_LEFT_ALT) && p < e->move_sequence.points.count && !cannot_move_points; p++){
            Vector2 *point = e->move_sequence.points.get_ptr(p);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && check_col_circles({input.mouse_position, 1}, {*point, 0.5f / session_context.cam.cam2D.zoom})){
                *point = input.mouse_position;
            }
        }
        
        //editor move cam rails points        
        for (i32 p = 0; e->flags & TRIGGER && (e->trigger.start_cam_rails_horizontal || e->trigger.start_cam_rails_vertical) && IsKeyDown(KEY_LEFT_ALT) && p < e->trigger.cam_rails_points.count && !cannot_move_points; p++){
            Vector2 *point = e->trigger.cam_rails_points.get_ptr(p);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && check_col_circles({input.mouse_position, 1}, {*point, 0.5f / session_context.cam.cam2D.zoom})){
                *point = input.mouse_position;
            }
        }
    }
    
    //assign move vertex
    if (need_move_vertices && moving_vertex_entity_candidate){
        assign_moving_vertex_entity (moving_vertex_entity_candidate, moving_vertex_candidate);
        undo_remember_vertices_start(moving_vertex_entity_candidate);
    }
    
    if (need_snap_vertex && editor.moving_vertex && editor.moving_vertex_entity){
        move_vertex(editor.moving_vertex_entity, closest_vertex_global, editor.moving_vertex_index);
    
        undo_apply_vertices_change(editor.selected_entity, &undo_action);
        something_in_undo = true;
        
        editor.moving_vertex = NULL;
        editor.moving_vertex_entity = NULL;
    }
    
    //This means we clicked all entities in one mouse position, so we want to cycle
    if (cursor_entities_count <= editor.place_cursor_entities.count){
        editor.place_cursor_entities.clear();
    }
    
    editor.cursor_entity = get_cursor_entity();
    
    b32 need_start_dragging = false;
    
    // mouse select
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select){
        if (editor.cursor_entity != NULL){ //selecting entity
            b32 is_same_selected_entity = editor.selected_entity != NULL && editor.selected_entity->id == editor.cursor_entity->id;
            need_start_dragging = is_same_selected_entity;
            if (!is_same_selected_entity){
                assign_selected_entity(editor.cursor_entity);
                editor.place_cursor_entities.add(editor.selected_entity);
                
                editor.selected_this_click = true;
            }
        }
    } 
    
    if (editor.dragging_entity == NULL && !editor.selected_this_click && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && editor.selected_entity != NULL && need_start_dragging && can_select){ // assign dragging entity
        if (editor.cursor_entity != NULL){
            if (editor.moving_vertex == NULL && editor.selected_entity->id == editor.cursor_entity->id){
                editor.dragging_entity = editor.selected_entity;
                editor.dragging_entity_id = editor.selected_entity->id;
                editor.dragging_start = editor.dragging_entity->position;
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
    
    //editor copy/paste
    if (editor.selected_entity && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)){
        copy_entity(&editor.copied_entity, editor.selected_entity);
        editor.is_copied = true;
    }
    
    if (editor.is_copied && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)){
        Entity *pasted_entity = add_entity(&editor.copied_entity);
        pasted_entity->position = input.mouse_position;
        assign_selected_entity(pasted_entity);
        
        Undo_Action undo_action;
        undo_action.spawned_entity = *pasted_entity;
        undo_action.entity_id = pasted_entity->id;
        undo_action.entity_was_spawned = true;
        add_undo_action(undo_action);
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
        Vector2 move_delta = ((Vector2){input.mouse_delta.x / zoom, -input.mouse_delta.y / zoom}) / (session_context.cam.unit_size);
        editor.dragging_entity->position += move_delta;
    }
    
    //editor Entity to mouse or go to entity
    if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.dragging_entity){
        editor.dragging_entity->position = input.mouse_position;
    } else if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.selected_entity){
        session_context.cam.position = editor.selected_entity->position;
    }
    
    //editor Entity rotation
    if (editor.selected_entity != NULL){
        if (can_control_with_single_button){
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
        }
        if (editor.is_rotating_entity && (IsKeyUp(KEY_E) && IsKeyUp(KEY_Q))){
            undo_add_rotation(editor.selected_entity, editor.selected_entity->rotation - editor.rotating_start);
            editor.is_rotating_entity = false;
        } 
    }
    if (editor.selected_entity && IsKeyDown(KEY_LEFT_ALT) && IsKeyUp(KEY_LEFT_SHIFT)){
        local_persist f32 holding_time = 0;
        if (IsKeyPressed(KEY_E)){
            undo_remember_vertices_start(editor.selected_entity);
            rotate(editor.selected_entity, 30);
            undo_add_rotation(editor.selected_entity, 30);
        }
        if (IsKeyPressed(KEY_Q)){
            undo_remember_vertices_start(editor.selected_entity);
            rotate(editor.selected_entity, -30);
            undo_add_rotation(editor.selected_entity, (-30));
        }
        
        if (IsKeyReleased(KEY_E) || IsKeyReleased(KEY_Q)){
            holding_time = 0;
        }
        
        if (IsKeyDown(KEY_E) || IsKeyDown(KEY_Q)){
            holding_time += dt;
            if (holding_time >= 0.2f){
                f32 direction = IsKeyDown(KEY_E) ? 30 : -30;
                undo_remember_vertices_start(editor.selected_entity);
                rotate(editor.selected_entity, direction);
                undo_add_rotation(editor.selected_entity, (direction));
                holding_time = 0;
            }
        }
    }
    
    //editor entity scaling
    if (editor.selected_entity != NULL){
        if (can_control_with_single_button){
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
        } 
        if (editor.is_scaling_entity && (IsKeyUp(KEY_W) && IsKeyUp(KEY_S) && IsKeyUp(KEY_A) && IsKeyUp(KEY_D))){
            Vector2 scale_change = editor.selected_entity->scale - editor.scaling_start;
            
            undo_add_scaling(editor.selected_entity, scale_change);
            editor.is_scaling_entity = false;
        } 
    }
    if (editor.selected_entity && IsKeyDown(KEY_LEFT_ALT) && IsKeyUp(KEY_LEFT_SHIFT)){
        local_persist f32 holding_time = 0;
        Vector2 scaling = Vector2_zero;
        
        if      (IsKeyPressed(KEY_W)) scaling.y += 50;
        else if (IsKeyPressed(KEY_S)) scaling.y -= 50;
        if      (IsKeyPressed(KEY_D)) scaling.x += 50;
        else if (IsKeyPressed(KEY_A)) scaling.x -= 50;


        if (scaling != Vector2_zero){
            undo_remember_vertices_start(editor.selected_entity);
            add_scale(editor.selected_entity, scaling);
            undo_add_scaling(editor.selected_entity, scaling);
        }
        
        if (IsKeyReleased(KEY_W) || IsKeyReleased(KEY_S) || IsKeyReleased(KEY_A) || IsKeyReleased(KEY_D)){
            holding_time = 0;
        }
        
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D)){
            holding_time += dt;
            if (holding_time >= 0.2f){
                if      (IsKeyDown(KEY_W)) scaling.y += 50;
                else if (IsKeyDown(KEY_S)) scaling.y -= 50;
                if      (IsKeyDown(KEY_D)) scaling.x += 50;
                else if (IsKeyDown(KEY_A)) scaling.x -= 50;
                
                undo_remember_vertices_start(editor.selected_entity);
                add_scale(editor.selected_entity, scaling);
                undo_add_scaling(editor.selected_entity, scaling);
                holding_time = 0;
            }
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
                    
                    if (check_col_circles({input.mouse_position, 1}, {point, 0.5f  * (0.4f / session_context.cam.cam2D.zoom)})){       
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
                    
                    if (check_col_circles({input.mouse_position, 1}, {point, 0.5f  * (0.4f / session_context.cam.cam2D.zoom)})){       
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
    
    if (editor.undo_actions.count > 0 && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z) && !IsKeyDown(KEY_LEFT_SHIFT)){
        Undo_Action *action = editor.undo_actions.pop_ptr();
        
        focus_input_field.in_focus = false;
        
        if (action->entity_was_deleted){
            Entity *restored_entity = add_entity(&action->deleted_entity, true);
            restored_entity->id = action->deleted_entity.id;
            action->entity_id = action->deleted_entity.id;
            
        } else if (action->entity_was_spawned){
            editor_delete_entity(action->entity_id, false);
        } else{
            assert(current_level_context->entities.has_key(action->entity_id));
            Entity *undo_entity = current_level_context->entities.get_by_key_ptr(action->entity_id);

            undo_entity->position   -= action->position_change;
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
    
    b32 need_make_redo = editor.max_undos_added > editor.undo_actions.count && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_Z);
    if (need_make_redo){
        editor.undo_actions.count++;        
        Undo_Action *action = editor.undo_actions.last_ptr();
        
        if (action->entity_was_deleted){ //so we need delete this again
            assert(current_level_context->entities.has_key(action->entity_id));
            editor_delete_entity(current_level_context->entities.get_by_key_ptr(action->entity_id), false);
        } else if (action->entity_was_spawned){ //so we need spawn this again
            Entity *restored_entity = add_entity(&action->spawned_entity, true);
            restored_entity->id = action->spawned_entity.id;
            action->entity_id = restored_entity->id;
        } else{
            assert(current_level_context->entities.has_key(action->entity_id));
            Entity *undo_entity = current_level_context->entities.get_by_key_ptr(action->entity_id);
            undo_entity->position   += action->position_change;
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
        editor.player_spawn_point = input.mouse_position;
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

void player_apply_friction(Entity *entity, f32 max_move_speed, f32 dt){
    f32 friction = player_data.friction;
    if (input.sum_direction.y < 0){
        friction *= 10;
    }
    
    if (abs(player_data.velocity.x) > max_move_speed){
        friction *= 2 + abs(player_data.velocity.x) / max_move_speed;
    }
    
    f32 friction_force = friction * -normalized (player_data.velocity.x) * dt;
    player_data.velocity.x += friction_force;
}

void player_accelerate(Entity *entity, Vector2 dir, f32 wish_speed, f32 acceleration, f32 dt){
    f32 speed_in_wish_direction = dot(player_data.velocity, dir);
    
    f32 speed_difference = wish_speed - speed_in_wish_direction;        
    
    //means we above max speed
    if (speed_difference <= 0){
        return;
    }
    
    f32 acceleration_speed = acceleration * speed_difference * dt;
    if (acceleration_speed > speed_difference){
        acceleration_speed = speed_difference;
    }
    
    player_data.velocity.x += dir.x * acceleration_speed;
}

void player_ground_move(Entity *entity, f32 dt){
    f32 max_move_speed = player_data.base_move_speed;
    
    player_apply_friction(entity, max_move_speed, dt);
    
    f32 acceleration = player_data.ground_acceleration;
    if (dot(player_data.velocity, input.sum_direction) <= 0){
        acceleration = player_data.ground_deceleration;
        if (input.sum_direction.y < 0){
            acceleration *= 0.3f;
        }
    }
    
    f32 wish_speed = sqr_magnitude(input.sum_direction) * max_move_speed;
    
    player_accelerate(entity, input.sum_direction, wish_speed, acceleration, dt);
}

void player_air_move(Entity *entity, f32 dt){
    f32 max_move_speed = player_data.base_move_speed;
    
    f32 acceleration = dot(player_data.velocity, input.sum_direction) > 0 ? player_data.air_acceleration : player_data.air_deceleration;
    
    f32 wish_speed = sqr_magnitude(input.sum_direction) * max_move_speed;
    
    player_accelerate(entity, input.sum_direction, wish_speed, acceleration, dt);
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
    player_data.sword_angular_velocity = value;
    player_data.sword_spin_direction = normalized(player_data.sword_angular_velocity);
    f32 sword_max_spin_speed = 5000;
    player_data.sword_spin_progress = clamp01(abs(player_data.sword_angular_velocity) / sword_max_spin_speed);
}

void add_player_ammo(i32 amount, b32 full_ammo){
    player_data.ammo_charges++;

    if (full_ammo){
        player_data.ammo_count += amount;
    } else if (player_data.ammo_charges >= player_data.ammo_charges_for_count){
        while (player_data.ammo_charges >= player_data.ammo_charges_for_count){
            player_data.ammo_count += 1;
            player_data.ammo_charges -= player_data.ammo_charges_for_count;
        }
        
    }
    
    player_data.ammo_count = clamp(player_data.ammo_count, 0, 3333);
}

inline b32 is_sword_can_damage(){
    return player_data.sword_spin_progress >= 0.12f;
}

inline b32 can_damage_blocker(Entity *blocker_entity){
    return is_sword_can_damage() && !blocker_entity->enemy.blocker_immortal && (blocker_entity->enemy.blocker_clockwise ? player_data.sword_spin_direction > 0 : player_data.sword_spin_direction < 0);
}

inline b32 can_damage_sword_size_required_enemy(Entity *enemy_entity){
    return is_sword_can_damage() && player_data.is_sword_big == enemy_entity->enemy.big_sword_killable;
}

b32 can_sword_damage_enemy(Entity *enemy_entity){
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
    Entity *sword = current_level_context->entities.get_by_key_ptr(player_data.connected_entities_ids.sword_entity_id);
    enemy_velocity->y = fmaxf(100.0f, 100.0f + enemy_velocity->y);
    enemy_velocity->x = player_data.sword_spin_direction * 50 + enemy_velocity->x;
    
    if (!enemy_entity->enemy.dead_man){
        add_hitstop(0.1f);
    }
    
    stun_enemy(enemy_entity, sword->position + sword->up * sword->scale.y * sword->pivot.y, sword_tip_emitter->direction, true);
}

b32 is_type(Entity *entity, FLAGS flags){
    return entity->flags & flags;
}

void try_sword_damage_enemy(Entity *enemy_entity, Vector2 hit_position){
    if (!can_sword_damage_enemy(enemy_entity)){
        return;
    }

    if (is_sword_can_damage() && !player_data.in_stun && is_enemy_can_take_damage(enemy_entity)){
        if (!(enemy_entity->flags & TRIGGER) && enemy_entity->enemy.gives_ammo && !enemy_entity->enemy.dead_man){
            add_player_ammo(1, enemy_entity->enemy.gives_full_ammo);
            add_blood_amount(&player_data, 10);
        }
    
        b32 was_alive_before_hit = !enemy_entity->enemy.dead_man;
        f32 hitstop_add = 0;
        
        if (enemy_entity->flags & BIRD_ENEMY){
            sword_kill_enemy(enemy_entity, &enemy_entity->bird_enemy.velocity);
        } else if (enemy_entity->flags & JUMP_SHOOTER){
            sword_kill_enemy(enemy_entity, &enemy_entity->jump_shooter.velocity);
        } else{
            kill_enemy(enemy_entity, hit_position, sword_tip_emitter->direction, lerp(1.0f, 4.0f, sqrtf(player_data.sword_spin_progress)));
        }
        
        f32 max_speed_boost = 6 * player_data.sword_spin_direction * enemy_entity->enemy.sword_kill_speed_modifier;
        f32 max_vertical_speed_boost = player_data.grounded ? 0 : 20;
        if (player_data.velocity.y > 0){
            max_vertical_speed_boost *= 0.3f;   
        }
        f32 spin_t = player_data.sword_spin_progress;
        player_data.velocity += Vector2_up   * lerp(0.0f, max_vertical_speed_boost, spin_t * spin_t)
                         + Vector2_right * lerp(0.0f, max_speed_boost, spin_t * spin_t); 
                         
        if (was_alive_before_hit){
            add_hitstop(0.01f + hitstop_add);
            shake_camera(0.1f);
        }
        
        play_sound("SwordKill", hit_position);
    }
}

void calculate_sword_collisions(Entity *sword, Entity *player_entity){
    fill_collisions(sword, &collisions_buffer, GROUND | ENEMY | WIN_BLOCK | CENTIPEDE_SEGMENT | PLATFORM | BLOCK_ROPE);
    
    Player *player = &player_data;
    
    for (i32 i = 0; i < collisions_buffer.count; i++){
        Collision col = collisions_buffer.get(i);
        Entity *other = col.other_entity;
        
        if ((other->flags & BLOCKER || other->flags & SWORD_SIZE_REQUIRED) && !player->in_stun){
            if (is_sword_can_damage() && !can_sword_damage_enemy(other)){
                player->velocity = player->velocity * -0.5f;
                emit_particles(rifle_bullet_emitter, col.point, col.normal, 3, 5);
                set_sword_velocity(normalized(-player->sword_angular_velocity) * 150);
                player_data.weak_recoil_stun_start_time = core.time.app_time;
                add_hitstop(0.1f);
                shake_camera(0.7f);
                play_sound(player_data.sword_block_sound, col.point);
                continue;
            }
        }
        
        if (other->flags & ENEMY){
            try_sword_damage_enemy(other, col.point);
        }
        
        if (other->flags & WIN_BLOCK && !player->in_stun){
            win_level();
        }
        
        if (other->flags & BLOCK_ROPE){
            // cut rope
            other->destroyed = true;
            emit_particles(rifle_bullet_emitter, col.point, other->up, 6, 50);
            play_sound("RopeCut", col.point);
        }
        
        if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT || other->flags & PLATFORM){
            player_data.sword_hit_ground = true;
        }
    }
}

void push_player_up(f32 power){
    if (player_data.velocity.y < 0){
        player_data.velocity.y = 0;
    }
    
    player_data.velocity.y += power;
    player_data.timers.since_jump_timer = 0;
    player_data.grounded = false;
}

void push_or_set_player_up(f32 power){
    if (player_data.velocity.y > power){
        power *= 0.25f;
    }

    player_data.velocity.y += power;
    
    player_data.timers.since_jump_timer = 0;
    player_data.grounded = false;
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

void update_player(Entity *entity, f32 dt){
    assert(entity->flags & PLAYER);

    if (player_data.dead_man){
        return;
    }
    
    Entity *ground_checker = current_level_context->entities.get_by_key_ptr(player_data.connected_entities_ids.ground_checker_id);
    Entity *left_wall_checker = current_level_context->entities.get_by_key_ptr(player_data.connected_entities_ids.left_wall_checker_id);
    Entity *right_wall_checker = current_level_context->entities.get_by_key_ptr(player_data.connected_entities_ids.right_wall_checker_id);
    Entity *sword          = current_level_context->entities.get_by_key_ptr(player_data.connected_entities_ids.sword_entity_id);
    
    ground_checker->position     = entity->position - entity->up * entity->scale.y * 0.5f;
    left_wall_checker->position  = entity->position - entity->right * entity->scale.x * 1.5f;
    right_wall_checker->position = entity->position + entity->right * entity->scale.x * 1.5f;
    sword->position = entity->position;
    
    // change sword size
    if (input.press_flags & SWORD_BIG){
        player_data.is_sword_big = !player_data.is_sword_big;
    }
    // player_data.is_sword_big = input.hold_flags & SWORD_BIG_DOWN;
    
    f32 max_strong_stun_time = 2.0f;
    f32 max_weak_stun_time = 0.3f;
    // f32 in_strong_stun_time = core.time.game_time - player_data.strong_recoil_stun_start_time;
    f32 in_weak_stun_time   = core.time.app_time - player_data.weak_recoil_stun_start_time;
    player_data.in_stun = (/*in_strong_stun_time <= max_strong_stun_time || */in_weak_stun_time <= max_weak_stun_time);
    // player_data.in_stun = false;
    
    Vector2 sword_target_size = player_data.sword_start_scale * (player_data.is_sword_big ? 6 : 1);
    
    change_scale(sword, lerp(sword->scale, sword_target_size, dt * 5));
    
    Vector2 sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
    
    Vector2 vec_to_mouse = input.mouse_position - entity->position;
    Vector2 dir_to_mouse = normalized(vec_to_mouse);
    
    if (input.press_flags & SPIN){
        player_data.rifle_active = false;
    }
    
    if (input.press_flags & SPIN){
        chainsaw_emitter->position = input.mouse_position;
        chainsaw_emitter->last_emitted_position = input.mouse_position;
        chainsaw_emitter->enabled = true;
    }
    if (input.press_flags & SPIN_RELEASED){
        chainsaw_emitter->enabled = false;
    }
    
    player_data.sword_angular_velocity *= 1.0f - (dt);
    
    b32 can_sword_spin = !player_data.rifle_active && !player_data.in_stun;
    
    f32 sword_max_spin_speed = 5000;
    if (can_sword_spin){
        f32 sword_spin_sense = player_data.is_sword_big ? 1 : 10; 
        if (can_sword_spin && input.hold_flags & SPIN_DOWN){
            player_data.sword_angular_velocity += input.sum_mouse_delta.x * sword_spin_sense;
            clamp(&player_data.sword_angular_velocity, -sword_max_spin_speed, sword_max_spin_speed);
        } else{
        }
    }
    
    f32 blood_progress_for_strong = 0.4f;
    b32 is_sword_filled = player_data.blood_progress >= blood_progress_for_strong;
    b32 is_rifle_charged = player_data.rifle_active && is_sword_filled;
    
    player_data.sword_spin_progress = clamp01(abs(player_data.sword_angular_velocity) / sword_max_spin_speed);
    
    b32 rifle_failed_hard = false;
    
    player_data.rifle_trail_emitter->position = sword_tip;
    player_data.rifle_trail_emitter->direction = sword->up;
    
    // @DO REdo this machinegun shit when we'll know for sure how we think this should work.
    i32 shoots_queued = 0;
    local_persist f32 shoot_press_time = -12;
    local_persist f32 rifle_in_machinegun_mode = false;
    if (input.press_flags & SHOOT && player_data.rifle_active){
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
    
    // player shoot
    b32 can_shoot_rifle = player_data.rifle_active && (player_data.ammo_count > 0 || debug.infinite_ammo) && state_context.shoot_stopers_count == 0;
    
    while (shoots_queued > 0){
        if (can_shoot_rifle){
            Vector2 shoot_direction = sword->up;
            
            if (rifle_in_machinegun_mode){
                f32 max_spread = 20;
                f32 spread_angle = rnd(-max_spread * 0.5f, max_spread * 0.5f);
                
                shoot_direction = get_rotated_vector(shoot_direction, spread_angle);
            }
            
            add_rifle_projectile(sword_tip, shoot_direction * player_data.rifle_strong_speed, STRONG);
            add_player_ammo(-1, true);
            
            add_explosion_light(sword_tip, 50, 0.03f, 0.05f, ColorBrightness(ORANGE, 0.3f));
            
            push_or_set_player_up(rifle_in_machinegun_mode ? 5 : 20);
            shake_camera(0.1f);
            play_sound("RifleShot", sword_tip, 0.3f);
            player_data.timers.rifle_shake_start_time = core.time.game_time;
            player_data.timers.rifle_shoot_time = core.time.game_time;
            
            enable_emitter(player_data.rifle_trail_emitter);
        } else if (input.press_flags & SHOOT){
            player_data.timers.rifle_shake_start_time = core.time.game_time;
            emit_particles(gunpowder_emitter, sword_tip, sword->up);
            
            // shoot blocker blocked
            if (state_context.shoot_stopers_count > 0){
                ForEntities(entity, SHOOT_STOPER){
                    if (entity->enemy.in_agro){
                        Entity *sticky_line = add_entity(player_entity->position, {1,1}, {0.5f,0.5f}, 0, STICKY_TEXTURE);
                        sticky_line->sticky_texture.draw_line = true;
                        sticky_line->sticky_texture.line_color = ColorBrightness(VIOLET, 0.1f);
                        sticky_line->sticky_texture.follow_id = entity->id;
                        sticky_line->sticky_texture.need_to_follow = true;
                        sticky_line->position = get_shoot_stoper_cross_position(entity);
                        sticky_line->sticky_texture.birth_time = core.time.game_time;
                        sticky_line->sticky_texture.max_distance = 0;
                        sticky_line->draw_order = 1;
                        shake_camera(0.5f);
                    }
                }
            }
        }
        shoots_queued -= 1;
    }
    
    if (player_data.rifle_active){
        change_up(sword, move_towards(sword->up, dir_to_mouse, 100, dt));        
    } else{
        player_data.rifle_perfect_shots_made = 0;
    }
    
    f32 time_since_shoot = core.time.game_time - player_data.timers.rifle_shoot_time;
    
    if (time_since_shoot >= 0.5f && core.time.game_time > 1){
        player_data.rifle_trail_emitter->enabled = false;
    } else{
    }
    
    //rifle activate
    b32 spin_enough_for_shoot = player_data.sword_spin_progress >= 0.1f;
    b32 can_activate_rifle = !player_data.rifle_active && !can_shoot_rifle && spin_enough_for_shoot;
    if (can_activate_rifle && input.press_flags & SHOOT){
        player_data.rifle_active = true;
        
        add_explosion_light(player_entity->position, 50, 0.02f, 0.06f, ColorBrightness(GREEN, 0.3f));
        
        player_data.sword_angular_velocity = 0;
        player_data.timers.rifle_activate_time = core.time.game_time;
        
        play_sound(player_data.rifle_switch_sound);
    } else if (!spin_enough_for_shoot && input.press_flags & SHOOT){
        // Failed to activate rifle.
        if (!player_data.rifle_active || (player_data.ammo_count <= 0 && !debug.infinite_ammo)){
            play_sound("FailedRifleActivation", 0.4f);
        }
        player_data.timers.rifle_shake_start_time = core.time.game_time;
        emit_particles(gunpowder_emitter, sword_tip, sword->up);
    }
    
    // sword->color_changer.progress = can_activate_rifle ? 1 : 0;
    sword->color = player_data.is_sword_big ? ColorBrightness(RED, 0.1f) : ColorBrightness(SKYBLUE, 0.3f);
    
    sword_tip_emitter->position = sword_tip;
    sword_tip_ground_emitter->position = sword_tip;
    chainsaw_emitter->position = input.mouse_position;
    
    {
        f32 spin_t = player_data.sword_spin_progress;
        f32 blood_t = player_data.blood_progress;
    
        chainsaw_emitter->lifetime_multiplier = 1.0f + spin_t * spin_t * 2; 
        chainsaw_emitter->speed_multiplier    = 1.0f + spin_t * spin_t * 2; 
        
        chainsaw_emitter->count_multiplier = player_data.is_sword_big ? 0.1f : 1;
        chainsaw_emitter->size_multiplier  = player_data.is_sword_big ? 5 : 1;
        chainsaw_emitter->color            = player_data.is_sword_big ? ColorBrightness(ORANGE, 0.2f) : YELLOW;
        
        sword_tip_emitter->lifetime_multiplier = 1.0f + blood_t * blood_t * 3.0f;
        sword_tip_emitter->speed_multiplier    = 1.0f + blood_t * blood_t * 5.0f;
        sword_tip_emitter->count_multiplier    = blood_t * blood_t * 2.0f;
              
        f32 blood_decease = 5;
              
        add_blood_amount(&player_data, -blood_decease * dt);
    }
    
    f32 sword_min_rotation_amount = 5;
    f32 need_to_rotate = player_data.sword_angular_velocity * dt;
    
    player_data.sword_spin_direction = normalized(player_data.sword_angular_velocity);
    
    if (abs(player_data.sword_angular_velocity) > 10){ 
        // Someone could enter sword on previous frame after this update so we'll check for that.
        calculate_sword_collisions(sword, entity);
        while(need_to_rotate > sword_min_rotation_amount){
            rotate(sword, sword_min_rotation_amount);
            calculate_sword_collisions(sword, entity);
            need_to_rotate -= sword_min_rotation_amount;
        }
        rotate(sword, need_to_rotate);
        calculate_sword_collisions(sword, entity);
    }
    
    // @OPTIMIZATION We actually can skip this shit when death instinct is on cooldown, but if that sword check affects performance in 
    // any way - i want to know about it, so we'll keep it that way. Because it would create a thing that we have more fps 
    // when insinct on cooldown. But we may do that at some point, not a big deal.
    b32 found_explosive = false;
    if (is_sword_can_damage()){ // sword death instinct
        f32 previous_rotation = sword->rotation;
        Vector2 previous_scale = sword->scale;
        f32 instinct_check_angle = 150;
        f32 instinct_step = 10;
        f32 checked = 0;
        Vector2 instinct_additional_scale = {1.0f, 1.0f};
        Vector2 sword_base_scale = player_data.is_sword_big ? sword_target_size : sword->scale;
        // change_scale(sword, {sword_base_scale.x * instinct_additional_scale.x, sword_base_scale.y * instinct_additional_scale.y});
        while (checked <= instinct_check_angle){
            checked += instinct_step;
            rotate(sword, instinct_step * player_data.sword_spin_direction);
            fill_collisions(sword, &collisions_buffer, EXPLOSIVE);
            for (i32 i = 0; i < collisions_buffer.count; i++){
                Collision col = collisions_buffer.get(i);
                Entity *other = col.other_entity;
                
                if (!can_sword_damage_enemy(other)){
                    continue;
                }
                
                Vector2 vec_to_other = other->position - player_entity->position;
                Vector2 dir_to_other = normalized(vec_to_other);
                f32 distance_to_other = magnitude(vec_to_other);
                Collision ray_collision = raycast(player_entity->position, dir_to_other, distance_to_other - 2, GROUND | CENTIPEDE_SEGMENT | CENTIPEDE);
                // This means explosion won't kill player so we move on.
                // Maybe should keep flags that block explosion for player as separate define.
                if (ray_collision.collided){
                    continue;
                }
                if (start_death_instinct(collisions_buffer.get(0).other_entity, SWORD_WILL_EXPLODE)){
                    // core.time.time_scale = 0.2f;
                    // player_data.sword_angular_velocity *= 0.5f;
                }
                
                state_context.death_instinct.angle_till_explode = checked - instinct_step;
                found_explosive = true;
                break;
            }
            
            if (found_explosive){
                break;
            }
        }
        rotate_to(sword, previous_rotation);
        change_scale(sword, previous_scale);
    }
    
    player_data.is_sword_will_hit_explosive = found_explosive;
    
    player_data.timers.since_jump_timer += dt;
    
    if (!player_data.in_stun){
        player_data.stun_emitter->enabled = false;
    } else{
        enable_emitter(player_data.stun_emitter);
    }
    
    /*
    local_persist f32 last_climb_spin_direction = 0;
    local_persist b32 in_climbing_state = false;
    local_persist Vector2 last_climbing_normal = Vector2_one;
    local_persist f32 climbing_in_one_direction_time = 0;
    
    if (in_climbing_state && !(input.hold_flags & DOWN)){
        in_climbing_state = false;
    }
    
    if (in_climbing_state){
        f32 max_climbing_speed = 300;
        
        Collision ground_collision = get_nearest_ground_collision(entity->position, entity->scale.y * 0.6f);
        
        if (!ground_collision.collided){
            Vector2 dir = normalized(player_data.velocity) * -1.0f - last_climbing_normal;
            ground_collision = raycast(player_entity->position, dir, entity->scale.y * 4, GROUND, 1.0f, entity->id);
            if (ground_collision.collided){
                entity->position += normalized(ground_collision.point - entity->position);
            }
        }
        
        if (ground_collision.collided){
            Vector2 move_plane = get_rotated_vector_90(ground_collision.normal, -1) * player_data.sword_spin_direction;
            
            if (player_data.sword_spin_direction != last_climb_spin_direction){
                last_climb_spin_direction = player_data.sword_spin_direction;
                climbing_in_one_direction_time = fminf(climbing_in_one_direction_time, 0.5f);
            }
            climbing_in_one_direction_time += dt;
            
            player_data.velocity = lerp(Vector2_zero, move_plane * max_climbing_speed * (player_data.sword_spin_progress * player_data.sword_spin_progress), clamp01(climbing_in_one_direction_time / 1.0f));
            
            clamp_magnitude(&player_data.velocity, max_climbing_speed);
            entity->position += normalized(ground_collision.point - entity->position) * dt;
            last_climbing_normal = ground_collision.normal;
            
        } else{
            in_climbing_state = false;
        }
        
        if (input.press_flags & JUMP){
            in_climbing_state = false;
            player_data.velocity *= 1.2f;
            player_data.velocity += last_climbing_normal * 20;
        }                
    } else{
        last_climbing_normal = Vector2_one;
        climbing_in_one_direction_time = 0;
        last_climb_spin_direction = 0;
    }
    */
    //player movement
    if (player_data.grounded && !player_data.in_stun/* && !in_climbing_state*/){
        // if (input.hold_flags & DOWN){
        //     in_climbing_state = true;
        // }
    
        player_ground_move(entity, dt);
        
        player_data.plane_vector = get_rotated_vector_90(player_data.ground_normal, -normalized(player_data.velocity.x));
        player_data.velocity = player_data.plane_vector * magnitude(player_data.velocity);
        
        entity->position.y -= dt;
        player_data.velocity -= player_data.ground_normal * dt;
        
        if (player_data.sword_spin_progress > 0.3f){
            Vector2 plane = get_rotated_vector_90(player_data.ground_normal, -player_data.sword_spin_direction);
            
            f32 spin_t = player_data.sword_spin_progress;
            f32 blood_t = player_data.blood_progress;
            
            f32 spin_acceleration = 400;
            if (IsKeyDown(KEY_LEFT_SHIFT)){
                spin_acceleration *= 3;
            }
            player_data.velocity += plane * lerp(0.0f, spin_acceleration, spin_t * spin_t) * dt;
        }
        
        player_data.timers.since_airborn_timer = 0;
    } else/* if (!in_climbing_state)*/{
        if (player_data.velocity.y > 0 && player_data.timers.since_jump_timer <= 0.3f){ //so we make jump gravity
            f32 max_height_jump_time = 0.2f;
            f32 jump_t = clamp01(player_data.timers.since_jump_timer / max_height_jump_time);
            player_data.gravity_mult = lerp(3.0f, 1.0f, jump_t * jump_t * jump_t);
        } else{
            if (input.sum_direction.y < 0 && !player_data.in_stun){
                player_data.gravity_mult = 5;
            } else{
                player_data.gravity_mult = lerp(1.0f, 0.5f, player_data.sword_spin_progress * player_data.sword_spin_progress);
                if (player_data.velocity.y > 0){
                    f32 up_velocity_t = clamp01(player_data.velocity.y / 200.0f);
                    f32 additional_gravity = lerp(0.0f, 2.0f, up_velocity_t * up_velocity_t);
                    player_data.gravity_mult += additional_gravity;
                }
            }
        }
        
        if (!player_data.in_stun){
            player_air_move(entity, dt);
            
        }
        player_data.velocity.y -= player_data.gravity * player_data.gravity_mult * dt;
        
        clamp(&player_data.velocity.y, -500, 500);
        
        player_data.timers.since_airborn_timer += dt;
        
        if (player_data.sword_spin_progress > 0.3f){
            f32 spin_t = player_data.sword_spin_progress;
            f32 blood_t = player_data.blood_progress;
            
            f32 max_spin_acceleration = 150;
            f32 min_spin_acceleration = 150;
            f32 spin_acceleration = lerp(min_spin_acceleration, max_spin_acceleration, blood_t * blood_t);
        
            f32 airborn_reduce_spin_acceleration_time = 0.5f;
            f32 t = clamp01(spin_t - clamp01(airborn_reduce_spin_acceleration_time - player_data.timers.since_airborn_timer));
            player_data.velocity.x += lerp(0.0f, spin_acceleration, t * t) * dt * player_data.sword_spin_direction;
        }
        
    }
    
    if (input.press_flags & JUMP){
        player_data.timers.jump_press_time = core.time.game_time;
    }
    
    f32 time_since_jump_press = core.time.game_time - player_data.timers.jump_press_time;
    
    b32 need_jump = (input.press_flags & JUMP && player_data.grounded)
                 || (player_data.grounded && time_since_jump_press <= player_data.jump_buffer_time) 
                 || (input.press_flags & JUMP && player_data.timers.since_airborn_timer <= player_data.coyote_time && player_data.timers.since_jump_timer > player_data.coyote_time);
    
    if (need_jump){
        push_player_up(player_data.jump_force);
    }
    
    Vector2 next_pos = {entity->position.x + player_data.velocity.x * dt, entity->position.y + player_data.velocity.y * dt};
    
    entity->position = next_pos;
    
    f32 found_ground = false;
    f32 just_grounded = false;
    
    f32 wall_acceleration = 400;
    
    f32 sword_ground_particles_speed = 1;
    
    // player collisions
    
    f32 time_since_wall_jump = core.time.game_time - player_data.timers.wall_jump_time;
    f32 player_speed = magnitude(player_data.velocity);
    
    // Collision ceiling_collision = raycast(entity->position, Vector2_up, 4, GROUND, entity->id);
    
    if (1/* || !ceiling_collision.collided*/){
        // player left wall
        fill_collisions(left_wall_checker, &collisions_buffer, GROUND | CENTIPEDE_SEGMENT | PLATFORM | BLOCKER | SHOOT_BLOCKER);
        for (i32 i = 0; i < collisions_buffer.count && !player_data.in_stun; i++){
            Collision col = collisions_buffer.get(i);
            Entity *other = col.other_entity;
            assert(col.collided);
            
            if (time_since_jump_press <= player_data.wall_jump_buffer_time && time_since_wall_jump > 0.4f){
                player_data.velocity += col.normal * player_data.jump_force;
                player_data.timers.wall_jump_time = core.time.game_time;
            }
            
            if (player_data.sword_spin_direction > 0){
                break;
            }
            
            if (other->flags & PLATFORM && dot(player_data.velocity, other->up) > 0){
                continue;
            }
            
            Vector2 plane = get_rotated_vector(col.normal, -player_data.sword_spin_direction * -95);
            f32 spin_t = player_data.sword_spin_progress;
            
            f32 acceleration = lerp(0.0f, wall_acceleration, spin_t * spin_t);
            if (player_speed <= 5){
                acceleration *= 10;
            }
            
            if (other->flags & PHYSICS_OBJECT){
                other->physics_object.velocity -= (plane * acceleration * dt) / other->physics_object.mass;
            }
            
            if (dot(plane, player_data.velocity) < 0){
                acceleration *= 4;
            }
            
            player_data.velocity += plane * acceleration * dt;
            sword_ground_particles_speed += 2;
        }
        
        // player right wall
        fill_collisions(right_wall_checker, &collisions_buffer, GROUND | CENTIPEDE_SEGMENT | PLATFORM | BLOCKER | SHOOT_BLOCKER);
        for (i32 i = 0; i < collisions_buffer.count && !player_data.in_stun; i++){
            Collision col = collisions_buffer.get(i);
            Entity *other = col.other_entity;
            assert(col.collided);
            
            if (time_since_jump_press <= player_data.wall_jump_buffer_time && time_since_wall_jump > 0.4f){
                player_data.velocity += col.normal * player_data.jump_force;
                player_data.timers.wall_jump_time = core.time.game_time;
            }
            
            if (player_data.sword_spin_direction < 0){
                break;
            }
            
            if (other->flags & PLATFORM && dot(player_data.velocity, other->up) > 0){
                continue;
            }
            
            Vector2 plane = get_rotated_vector(col.normal, -player_data.sword_spin_direction * -95);
            f32 spin_t = player_data.sword_spin_progress;
            
            f32 acceleration = lerp(0.0f, wall_acceleration, spin_t * spin_t);
            if (player_speed <= 5){
                acceleration *= 10;
            }
            
            if (other->flags & PHYSICS_OBJECT){
                other->physics_object.velocity -= (plane * acceleration * dt) / other->physics_object.mass;
            }
            
            if (dot(plane, player_data.velocity) < 0){
                acceleration *= 4;
            }
    
            player_data.velocity += plane * acceleration * dt;
            sword_ground_particles_speed += 2;
        }
        
    }
    
    b32 moving_object_detected = false;
    // player ground checker
    fill_collisions(ground_checker, &collisions_buffer, GROUND | BLOCKER | SHOOT_BLOCKER | SWORD_SIZE_REQUIRED | PLATFORM | CENTIPEDE_SEGMENT);
    b32 is_huge_collision_speed = false;
    for (i32 i = 0; i < collisions_buffer.count && !player_data.in_stun; i++){
        Collision col = collisions_buffer.get(i);
        Entity *other = col.other_entity;
        assert(col.collided);
        
        f32 dot_velocity = dot(col.normal, player_data.velocity);
        if (dot_velocity >= 0){
            continue;
        }
        
        if (other->flags & PLATFORM && dot(player_data.velocity, other->up) > 0){
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
            try_sword_damage_enemy(other, col.point);
            continue;
        }
        
        if (other->flags & CENTIPEDE_SEGMENT){
            if (other->centipede_head->centipede.spikes_on_right && other->centipede_head->centipede.spikes_on_left){
                kill_player();
                return;
            } else if (!other->centipede_head->centipede.spikes_on_right && !other->centipede_head->centipede.spikes_on_left){
                
            } else{
                Vector2 side = other->centipede_head->centipede.spikes_on_right ? other->right : (other->right * -1.0f);
                f32 side_dot = dot(side, entity->position - other->position);
                // so we on side of the centipede segments where are SPIKES
                if (side_dot > 0){
                    kill_player();
                    return;
                }
            }
        }
        
        entity->position.y += col.overlap;
        
        Vector2 velocity_direction = normalized(player_data.velocity);
        f32 before_speed = magnitude(player_data.velocity);
        
        if (before_speed > 200){
            emit_particles(air_dust_emitter, col.point, col.normal, 0.2f, 1);
            
            tires_volume = lerp(tires_volume, 0.5f, core.time.real_dt * 2.0f);
            SetMusicVolume(tires_theme, tires_volume);
            is_huge_collision_speed = true;
        }
    
        f32 collision_force_multiplier = 1;
        
        if (other->flags & PHYSICS_OBJECT){             // force
            f32 direction_normal_dot = dot(velocity_direction, col.normal);
            other->physics_object.velocity += ((player_data.velocity * PLAYER_MASS) / other->physics_object.mass) * direction_normal_dot * -1;
            collision_force_multiplier = other->physics_object.mass / PLAYER_MASS;
            entity->position += other->physics_object.velocity * dt;
            player_data.on_moving_object = true;
            player_data.moving_object_velocity = other->physics_object.velocity;
            moving_object_detected = true;
        }
        
        if (dot(((Vector2){0, 1}), col.normal) > 0.5f){
            player_data.velocity -= col.normal * dot(player_data.velocity, col.normal);
        }
        
        if (other->flags & MOVE_SEQUENCE && other->move_sequence.moving){
            entity->position += other->move_sequence.moved_last_frame;
        }
        
        f32 angle = fangle(col.normal, entity->up);
        
        if (angle <= player_data.max_ground_angle){
            found_ground = true;
            player_data.ground_normal = col.normal;
            
            if (!player_data.grounded && !just_grounded){
                player_data.plane_vector = get_rotated_vector_90(player_data.ground_normal, -normalized(player_data.velocity.x));
                player_data.velocity = player_data.plane_vector * magnitude(player_data.velocity);
                just_grounded = true;
                
                //heavy landing
                if (before_speed > 200 && magnitude(player_data.velocity) < 100){
                    player_data.heavy_collision_time = core.time.game_time;
                    player_data.heavy_collision_velocity = player_data.velocity;
                    emit_particles(ground_splash_emitter, col.point, col.normal, 1, 1.5f);
                    shake_camera(0.7f);
                    
                    play_sound("HeavyLanding", col.point, 1.5f);
                }
            }
        }
    }
    
    if (!moving_object_detected && player_data.on_moving_object){
        if (dot(player_data.moving_object_velocity, player_data.velocity) > magnitude(player_data.velocity)){
        } else if (dot(player_data.moving_object_velocity, player_data.velocity) > 0){
            player_data.velocity += player_data.moving_object_velocity;   
        }
        player_data.on_moving_object = false;
    }
    
    if (!is_huge_collision_speed){
        tires_volume = lerp(tires_volume, 0.0f, core.time.real_dt * 5.0f);
        SetMusicVolume(tires_theme, tires_volume);
    }
    
    
    // player body collision
    fill_collisions(entity, &collisions_buffer, GROUND | BLOCKER | SHOOT_BLOCKER | SWORD_SIZE_REQUIRED | PROPELLER | CENTIPEDE_SEGMENT | PLATFORM);
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
        
        //triggers
        if (other->flags & PROPELLER){
            // update propeller
            if (player_data.sword_spin_progress > EPSILON){
                Vector2 acceleration_dir = other->up;
                Vector2 deceleration_plane = other->right;
                
                f32 power_t = player_data.sword_spin_progress;
                
                Vector2 to_player = player_entity->position - other->position;
                
                f32 deceleration_power = lerp(0.0f, 300.0f, power_t * power_t);
                f32 acceleration_power = lerp(0.0f, other->propeller.power, sqrtf(power_t));
                
                f32 deceleration_sign = dot(to_player, deceleration_plane) > 0 ? -1 : 1;
                
                f32 damping_factor = lerp(0.0f, 10.0f, sqrtf(power_t));
                
                player_data.velocity += deceleration_plane * deceleration_power * deceleration_sign * dt;
                player_data.velocity *= 1.0f - (damping_factor * dt);
                player_data.velocity += acceleration_dir * acceleration_power * dt;
                
                f32 new_dot = dot(deceleration_plane, player_data.velocity);
                if (new_dot * deceleration_sign < 0){
                    player_data.velocity += deceleration_plane * deceleration_sign * -1.0f * new_dot * dt;
                }
            }
            continue; 
        }

        
        f32 dot_velocity = dot(col.normal, player_data.velocity);
        if (dot_velocity >= 0){
            continue;
        }
        
        if (other->flags & PLATFORM && dot(player_data.velocity, other->up) > 0){
            continue;
        }
        
        if (other->flags & ENEMY && can_sword_damage_enemy(other) && !(other->flags & CENTIPEDE_SEGMENT)){
            try_sword_damage_enemy(other, col.point);
            continue;
        }
        
        if (other->flags & CENTIPEDE_SEGMENT){
            if (other->centipede_head->centipede.spikes_on_right && other->centipede_head->centipede.spikes_on_left){
                kill_player();
                return;
            } else if (!other->centipede_head->centipede.spikes_on_right && !other->centipede_head->centipede.spikes_on_left){
                
            } else{
                Vector2 side = other->centipede_head->centipede.spikes_on_right ? other->right : (other->right * -1.0f);
                f32 side_dot = dot(side, entity->position - other->position);
                // so we on side of the centipede segments where are SPIKES
                if (side_dot > 0){
                    kill_player();
                    return;
                }
            }
        }
        
        resolve_collision(entity, col);
        
        Vector2 velocity_direction = normalized(player_data.velocity);
        
        f32 before_speed = magnitude(player_data.velocity);
        
        if (before_speed > 200){
            emit_particles(air_dust_emitter, col.point, col.normal, 0.5f, 1.0f);
        }
        
        if (player_data.in_stun){
            player_data.velocity = reflected_vector(player_data.velocity * 0.5f, col.normal);
            shake_camera(0.2f);
            continue;
        }

        f32 collision_force_multiplier = 1;
        
        if (other->flags & PHYSICS_OBJECT){             // force
            f32 direction_normal_dot = dot(velocity_direction, col.normal);
            other->physics_object.velocity += ((player_data.velocity * PLAYER_MASS) / other->physics_object.mass) * direction_normal_dot * -1;
            collision_force_multiplier = other->physics_object.mass / PLAYER_MASS;
        }
        
        clamp(&collision_force_multiplier, 0, 1.0f);
        
        player_data.velocity -= col.normal * dot(player_data.velocity, col.normal) * collision_force_multiplier;
        
        //heavy collision
        if (before_speed > 200 && magnitude(player_data.velocity) < 100){
            player_data.heavy_collision_time = core.time.game_time;
            player_data.heavy_collision_velocity = player_data.velocity;
            emit_particles(ground_splash_emitter, col.point, col.normal, 1, 1.5f);
            shake_camera(0.7f);
            play_sound("HeavyLanding", col.point, 1.5f);
        }
    } // end player body collisions
    
    player_data.grounded = found_ground;
    
    if (player_data.sword_hit_ground){
        enable_emitter(sword_tip_ground_emitter);
        sword_tip_ground_emitter->speed_multiplier = sword_ground_particles_speed;
        sword_tip_ground_emitter->count_multiplier = fmax(1.0f, sword_ground_particles_speed * 0.5f);
    } else{
        sword_tip_ground_emitter->enabled = false;
    }
    
    player_data.sword_hit_ground = false;

    
    f32 wind_t = clamp01(magnitude(player_data.velocity) / 300.0f);
    SetMusicVolume(wind_theme, lerp(0.0f, 1.0f, wind_t * wind_t));
    
    ground_checker->position     = entity->position - entity->up * entity->scale.y * 0.5f;
    left_wall_checker->position  = entity->position - entity->right * entity->scale.x * 1.5f;
    right_wall_checker->position = entity->position + entity->right * entity->scale.x * 1.5f;
    sword->position = entity->position;
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
            kill_enemy(entity, col.point, direction, lerp(1, 5, speed_t * speed_t));
        } else{
            physics_object->velocity = reflected_vector(physics_object->velocity * 0.5f, col.normal);
        }
        
        if (is_high_velocity){
            emit_particles(rifle_bullet_emitter, col.point, direction, lerp(0.5f, 2.0f, speed_t * speed_t), lerp(5, 20, speed_t * speed_t));
            play_sound("BirdToGround", col.point, 0.5f);
        }
        
        if (physics_object->rotate_by_velocity && col.normal.y >= 0){
            change_up(entity, move_towards(entity->up, col.normal, speed, core.time.fixed_dt));            
        }
    }        
    
    if (other->flags & PLAYER){
        resolve_collision(entity, col);
        f32 force = dot(((physics_object->velocity - player_data.velocity)* physics_object->mass) / PLAYER_MASS, col.normal * -1);   
        // if (physics_object->mass >= PLAYER_MASS && force > 1000){
            // kill_player();
        resolve_physics_collision(&physics_object->velocity, physics_object->mass, &player_data.velocity, PLAYER_MASS, col.normal);
        // player_data.velocity += (physics_object->velocity - player_data.velocity) * 1.1f;
        // } //else{
        // player_data.velocity += physics_object->velocity - player_data.velocity;
        // }
    } else if (other->flags & BIRD_ENEMY){
        f32 force = dot(((physics_object->velocity - other->bird_enemy.velocity)* physics_object->mass) / 5, col.normal * -1);   
        if (physics_object->mass >= 5 && force > 1000){
            kill_enemy(other, col.point, direction, force / 200);
            play_sound("SwordKill", col.point);
        } else{
            resolve_collision(entity, col);
            // other->bird_enemy.velocity += direction * force / 100;
        }
        resolve_physics_collision(&physics_object->velocity, physics_object->mass, &other->bird_enemy.velocity, 5, col.normal);
    } else if (other->flags & ENEMY && (!(other->flags & JUMP_SHOOTER))){
        f32 force = dot(((physics_object->velocity) * physics_object->mass) / 5, col.normal * -1);   
        if (physics_object->mass >= 5 && force > 1000){
            kill_enemy(other, col.point, direction, force / 200);
            play_sound("SwordKill", col.point);
        } 
        Vector2 fictional_velocity = Vector2_zero;
        resolve_physics_collision(&physics_object->velocity, physics_object->mass, &fictional_velocity, 0.1f, col.normal);
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
            emit_particles(fire_emitter, shooter_entity->position, col.normal, 4, 3);
            play_sound("Explosion", shooter_entity->position, shooter_entity->volume_multiplier);
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
            emit_particles(ground_splash_emitter, col.point, col.normal, 6, 2.5f);
            
            shooter->flying_emitter->enabled = false;
        } else if (!shooter->states.standing){
            shooter->velocity = reflected_vector(shooter->velocity * 0.7f, col.normal);
            emit_particles(ground_splash_emitter, col.point, col.normal, 1, 0.5f);
            
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
    if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT || other->flags & BLOCKER || other->flags & SHOOT_BLOCKER){
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
                emit_particles(fire_emitter, bird_entity->position, col.normal, 2, 3);
                play_sound("Explosion", bird_entity->position, bird_entity->volume_multiplier);
                
                add_explosion_light(bird_entity->position, rnd(75.0f, 200.0f), 0.1f, 0.3f, ColorBrightness(ORANGE, 0.5f));
                
                bird_entity->destroyed = true;
                bird_entity->enabled = false;
                shake_camera(0.6f);
                return;
            }
            
            bird->velocity = reflected_vector(bird->velocity * 0.9f, col.normal);
            if (bird->attacking){
                bird->attacking = false;
                bird->attack_emitter->enabled = false;
                bird->roaming = true;
                bird->attacked_time = core.time.game_time;
                bird->roam_start_time = core.time.game_time;
            }
        }
        
        emit_particles(rifle_bullet_emitter, col.point, normalized(bird->velocity), lerp(0.5f, 2.0f, bird_speed_t * bird_speed_t), lerp(5, 20, bird_speed_t * bird_speed_t));
        
        if (is_high_velocity){
            play_sound("BirdToGround", col.point, 0.5f);
        }
    }
    
    if (other->flags & BIRD_ENEMY && !bird->attacking){
        resolve_collision(bird_entity, col);
        
        if (enemy->dead_man){
            emit_particles(fire_emitter, bird_entity->position, col.normal, 2, 3);
            stun_enemy(other, other->position, col.normal);
        }
        
        bird->velocity              = reflected_vector(bird->velocity * 0.8f, col.normal);
        other->bird_enemy.velocity += reflected_vector(bird->velocity * 0.3f, col.normal * -1);
        
        emit_particles(rifle_bullet_emitter, col.point, col.normal, 0.5f, 1);
        
        if (is_high_velocity){
            play_sound("BirdToBird", col.point, 0.5f);
        }
    }
    
    if (other->flags & PLAYER && !player_data.dead_man && bird->attacking && !enemy->dead_man){
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
        bird->velocity.y -= player_data.gravity * dt;
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
        rotate(entity, bird->velocity.x);
        bird_clear_formation(bird);
        
        f32 since_died_time = core.time.game_time - enemy->died_time;
        if (since_died_time >= 15 && sqr_magnitude(entity->position - player_entity->position) >= 50000){
            destroy_enemy(entity);
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
        bird->attack_emitter->enabled = false;
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
                
                emit_particles(sparks_emitter, entity->position, entity->up, 2, 3);
                enable_emitter(bird->attack_emitter);
                play_sound(bird->attack_sound, entity->position);
            }
        } 
    }
    
    if (bird->attacking){
        f32 attacking_time = core.time.game_time - bird->attack_start_time;
        
        if (attacking_time >= bird->max_attack_time){
            bird->attacking = false;
            bird->roaming = true;
            bird->roam_start_time = core.time.game_time;
            bird->attack_emitter->enabled = false;
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
    
    bird->trail_emitter->direction = entity->up * -1;
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

void kill_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, f32 particles_speed_modifier){
    assert(enemy_entity->flags & ENEMY);
    
    f32 hitmark_scale = 1;
    Color hitmark_color = WHITE;
    
    if (!enemy_entity->enemy.dead_man){
        enemy_entity->enemy.stun_start_time = core.time.game_time;
        enemy_entity->enemy.last_hit_time   = core.time.game_time;
        f32 count = player_data.is_sword_big ? 3 : 1;
        f32 area = player_data.is_sword_big ? 3 : 1;
        emit_particles(*blood_emitter, kill_position, kill_direction, count, particles_speed_modifier, area);
    
        enemy_entity->enemy.dead_man = true;
        enemy_entity->enemy.died_time = core.time.game_time;
        if (!(enemy_entity->flags & (TRIGGER | CENTIPEDE_SEGMENT))){
            enemy_entity->enabled = false;
            destroy_enemy(enemy_entity);
        }
        
        if (enemy_entity->flags & MOVE_SEQUENCE && !(enemy_entity->flags & CENTIPEDE_SEGMENT)){
            enemy_entity->move_sequence.moving = false;
        }
        
        if (enemy_entity->flags & EXPLOSIVE){
            hitmark_scale += 4;
            hitmark_color = Fade(ColorBrightness(ORANGE, 0.3f), 0.8f);
            f32 explosion_radius = get_explosion_radius(enemy_entity);
            emit_particles(explosion_emitter, enemy_entity->position, Vector2_up, explosion_radius / 40.0f);
            play_sound(enemy_entity->enemy.big_explosion_sound, enemy_entity->position);
            
            i32 light_size_flag = SMALL_LIGHT;
            if (enemy_entity->light_index != -1) light_size_flag = current_level_context->lights.get(enemy_entity->light_index).shadows_size_flags;
            add_explosion_light(enemy_entity->position, explosion_radius * rnd(3.0f, 6.0f), 0.15f, fminf(enemy_entity->enemy.explosive_radius_multiplier, 3.0f), ColorBrightness(ORANGE, 0.3f), light_size_flag);
            
            f32 explosion_add_speed = 80;
            ForEntities(other_entity, 0){
                Vector2 vec_to_other = other_entity->position - enemy_entity->position;
                f32 distance_to_other = magnitude(vec_to_other);
                
                if (distance_to_other > explosion_radius){
                    continue;
                }
                
                Vector2 dir_to_other = normalized(vec_to_other);
                u64 additional_flags = 0;
                if (other_entity->flags & PLAYER) additional_flags |= CENTIPEDE_SEGMENT | CENTIPEDE;
                
                Collision raycast_collision = raycast(enemy_entity->position, dir_to_other, distance_to_other, GROUND | additional_flags, 2, enemy_entity->id);
                if (raycast_collision.collided){
                    emit_particles(ground_splash_emitter, raycast_collision.point, raycast_collision.normal, 4, 5.5f);
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
                }
                
                if (other_entity->flags & PHYSICS_OBJECT){
                    other_entity->physics_object.velocity += (dir_to_other * explosion_add_speed * (explosion_radius * 0.1f)) / other_entity->physics_object.mass;
                }
                
                if (other_entity->flags & PLAYER && !player_data.dead_man && distance_to_other < explosion_radius * 0.75f){
                    kill_player();
                }
            }
            
            add_hitstop(0.1f * fmaxf(1.0f, enemy_entity->enemy.explosive_radius_multiplier * 0.5f), true);
            shake_camera(0.5f * fmaxf(1.0f, enemy_entity->enemy.explosive_radius_multiplier * 0.5f));
        }
        
        b32 is_hitmark_follow = false;
        
        if (enemy_entity->flags & (CENTIPEDE_SEGMENT | TRIGGER)){
            is_hitmark_follow = true;
        }
        
        add_hitmark(enemy_entity, is_hitmark_follow, hitmark_scale, hitmark_color); 
    }
}

inline b32 is_enemy_can_take_damage(Entity *enemy_entity, b32 check_for_last_hit_time){
    assert(enemy_entity->flags & ENEMY);

    if (enemy_entity->flags & CENTIPEDE_SEGMENT && enemy_entity->enemy.dead_man){
        return false;
    }
    
    if (!check_for_last_hit_time){
        return true;
    }
    b32 recently_got_hit = (core.time.game_time - enemy_entity->enemy.last_hit_time) <= 0.2f;
    return !recently_got_hit;
}

void agro_enemy(Entity *entity){
    if (entity->enemy.in_agro || entity->enemy.dead_man){
        return;
    }

    entity->enemy.in_agro = true;
    
    add_explosion_light(entity->position, (entity->scale.y + entity->scale.x) * 10, 0.1f, 2.2f, Fade(ColorBrightness(RED, 0.5f), 0.5f), SMALL_LIGHT, entity->id);
    
    if (entity->flags & SHOOT_STOPER){
        state_context.shoot_stopers_count++;
    }
}

void destroy_enemy(Entity *entity){
    entity->destroyed = true;
    
    if (entity->flags & SHOOT_STOPER){
        // assert(state_context.shoot_stopers_count >= 0);
        if (state_context.shoot_stopers_count > 0){
            state_context.shoot_stopers_count--;
        } else{
            print("WARNING: Shoot stopers count could go below zero. That may be because we skipped trigger and kill it, so no assertion, just warning");            
        }
    }
}

void add_fire_light_to_entity(Entity *entity){
    i32 new_light_index = init_entity_light(entity, NULL, true);
    if (new_light_index != -1){
        Light *new_fire_light = current_level_context->lights.get_ptr(new_light_index);
        new_fire_light->make_shadows = false;
        new_fire_light->make_backshadows = false;
        new_fire_light->shadows_size_flags = MEDIUM;
        new_fire_light->backshadows_size_flags = MEDIUM;
        new_fire_light->color = ColorBrightness(ORANGE, 0.4f);
        new_fire_light->fire_effect = true;
        entity->flags |= LIGHT;
    }
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
        enemy->last_hit_time = core.time.game_time;
        enemy->hits_taken++;
        b32 should_die_in_one_hit = enemy_entity->flags & BIRD_ENEMY && enemy_entity->bird_enemy.attacking;
        
        if ((enemy->hits_taken >= enemy->max_hits_taken || serious || should_die_in_one_hit) && !enemy->dead_man){
            f32 area_multiplier = serious ? 3 : 1;
            f32 count = serious ? 3 : 1;
            f32 speed = serious ? 3 : 1;
            emit_particles(*blood_emitter, kill_position, kill_direction, count, speed, area_multiplier);
        
            enemy->dead_man = true;
            enemy->died_time = core.time.game_time;
            
            if (enemy_entity->flags & BIRD_ENEMY){
                // birds handle dead state by themselves
                enable_emitter(enemy_entity->bird_enemy.fire_emitter);
                add_fire_light_to_entity(enemy_entity);
            } else if (enemy_entity->flags & JUMP_SHOOTER){
                Particle_Emitter *dead_fire_emitter = enemy_entity->emitters.add(fire_emitter);
                dead_fire_emitter->position = enemy_entity->position;
                enable_emitter(dead_fire_emitter);
                add_fire_light_to_entity(enemy_entity);
            } else if (enemy_entity->flags & CENTIPEDE_SEGMENT){
                
            } else{
                destroy_enemy(enemy_entity);
            }
        } else{
            enemy->stun_start_time = core.time.game_time;
            enemy->last_hit_time   = core.time.game_time;
        }
        add_hitmark(enemy_entity, true); 
    }
}

void add_rifle_projectile(Vector2 start_position, Vector2 velocity, Projectile_Type type){
    Entity *projectile_entity = add_entity(start_position, {2, 2}, {0.5f, 0.5f}, 0, PINK, PROJECTILE | PARTICLE_EMITTER);
    projectile_entity->projectile.flags = PLAYER_RIFLE;
    projectile_entity->projectile.type  = type;
    projectile_entity->projectile.birth_time = core.time.game_time;
    projectile_entity->projectile.velocity = velocity;
    projectile_entity->projectile.max_lifetime = 7;
    
    Particle_Emitter *bullet_emitter = projectile_entity->emitters.add(rifle_bullet_emitter);
    bullet_emitter->position = start_position;
    enable_emitter(bullet_emitter);
}

inline Vector2 transform_texture_scale(Texture texture, Vector2 wish_scale){
    Vector2 real_scale = {(f32)texture.width / session_context.cam.unit_size, (f32)texture.height / session_context.cam.unit_size};
    
    return {wish_scale.x / real_scale.x, wish_scale.y / real_scale.y};
}

void add_hitmark(Entity *entity, b32 need_to_follow, f32 scale_multiplier, Color tint){
    Entity *hitmark = add_entity(entity->position, transform_texture_scale(hitmark_small_texture, {10 * scale_multiplier, 10 * scale_multiplier}), {0.5f, 0.5f}, rnd(-90.0f, 90.0f), hitmark_small_texture, TEXTURE | STICKY_TEXTURE);
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
}

Vector2 get_entity_velocity(Entity *entity){
    if (entity->flags & PLAYER){
        return player_data.velocity;
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
    // return fmaxf(fminf((18.0f / session_context.cam.cam2D.zoom), 60), 40) * fmaxf(magnitude(velocity) / 200.0f, 1.0f);
    return 40 + (threat_scale.x + threat_scale.y) - 8.0f;
}

inline b32 is_death_instinct_threat_active(){
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
                return player_data.is_sword_will_hit_explosive;     
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
        is_threat_status_gives_cooldown = (!threat_entity || threat_entity->enemy.dead_man || is_enemy_should_trigger_death_instinct(threat_entity, get_entity_velocity(threat_entity), normalized(player_entity->position - threat_entity->position), magnitude(player_entity->position - threat_entity->position), true));
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
    b32 flying_towards = true;
    // @TODO: We definetely want to better check if enemy is flying towards. For example we can just simulate enemy some steps forward.
    if (check_if_flying_towards){
        flying_towards = distance_to_player < get_death_instinct_radius(entity->scale) && dot(dir_to_player, normalized(velocity)) >= 0.9f;
    }
    
    if (!flying_towards){
        return false;
    }
    
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
    if (is_in_death_instinct() || is_death_instinct_in_cooldown()){
        return false;
    }
    
    state_context.death_instinct.start_time = core.time.app_time;
    state_context.death_instinct.threat_entity_id = threat_entity->id;
    state_context.death_instinct.last_reason = reason;
    return true;
}

inline b32 should_kill_player(Entity *entity){
    return !can_sword_damage_enemy(entity);
}

void calculate_projectile_collisions(Entity *entity){
    Projectile *projectile = &entity->projectile;
    
    if (projectile->flags & PLAYER_RIFLE){
        fill_collisions(entity, &collisions_buffer, GROUND | ENEMY | WIN_BLOCK | ROPE_POINT);
        
        Player *player = &player_data;
        
        b32 damaged_enemy = false;
        
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            Entity *other = col.other_entity;
            
            if (projectile->already_hit_ids.contains(other->id)){
                continue;
            }
            
            
            b32 need_bounce = false;
            
            Vector2 velocity_dir = normalized(projectile->velocity);
            f32 sparks_speed = 1;
            f32 sparks_count = 1;
            f32 hitstop_add = 0;
            
            if (other->flags & ENEMY && is_enemy_can_take_damage(other, false) && (projectile->type != WEAK || !projectile->dying)){
                projectile->already_hit_ids.add(other->id);
                b32 killed = false;
                b32 can_damage = true;
                
                if (other->flags & SHOOT_BLOCKER){
                    Vector2 shoot_blocker_direction = get_rotated_vector(other->enemy.shoot_blocker_direction, other->rotation);
                    f32 velocity_dot_direction = dot(velocity_dir, shoot_blocker_direction);    
                        
                    can_damage = !other->enemy.shoot_blocker_immortal && (compare_difference(velocity_dot_direction, 1, 0.1f) || compare_difference(velocity_dot_direction, -1, 0.1f));
                    sparks_speed += 2;
                    sparks_count += 2;
                    
                    if (!can_damage){
                        need_bounce = true;
                        other->enemy.last_hit_time = core.time.game_time;
                        play_sound("ShootBlock", col.point);
                    }
                }
                
                if (other->flags & WIN_BLOCK && can_damage){
                    win_level();
                } else if (other->flags & BIRD_ENEMY && can_damage){
                    other->bird_enemy.velocity += projectile->velocity * 0.05f;
                    projectile->velocity = reflected_vector(projectile->velocity * 0.6f, col.normal);
                    projectile->type = WEAK;
                    projectile->birth_time = core.time.game_time;
                    stun_enemy(other, entity->position, col.normal);    
                    sparks_speed += 1;
                } else if (other->flags & JUMP_SHOOTER && can_damage){
                    other->jump_shooter.velocity += projectile->velocity * 0.05f;
                    projectile->velocity = reflected_vector(projectile->velocity * 0.6f, col.normal);
                    projectile->type = WEAK;
                    projectile->birth_time = core.time.game_time;
                    stun_enemy(other, entity->position, col.normal);    
                    sparks_speed += 1;
                } else if (can_damage){
                    kill_enemy(other, entity->position, col.normal);
                    killed = true;
                }
                
                if (other->flags & TRIGGER && can_damage){
                    sparks_count += 20;
                    hitstop_add = 0.1f;
                }
                
                emit_particles(big_sparks_emitter, col.point, velocity_dir, sparks_count, sparks_speed);
                add_hitstop(0.03f + hitstop_add);
                shake_camera(0.1f);
                if (can_damage){
                    play_sound(player_data.rifle_hit_sound, col.point);
                }
            
                damaged_enemy = can_damage;
            } else if (other->flags & ENEMY && projectile->type == WEAK){
                need_bounce = true;
            }
            
            if (other->flags & PHYSICS_OBJECT){
                apply_physics_force(projectile->velocity, 0.5f, &other->physics_object, col.normal);
                emit_particles(big_sparks_emitter, col.point, velocity_dir, sparks_count, sparks_speed);
                entity->destroyed = true;
            }
            
            if (other->flags & GROUND){
                entity->destroyed = true;
                emit_particles(big_sparks_emitter, col.point, velocity_dir, sparks_count, sparks_speed);
            }
            
            if (other->flags & ROPE_POINT){
                // cut rope point
                other->destroyed = true;
                emit_particles(rifle_bullet_emitter, col.point, col.normal, 6, 10);
                emit_particles(big_sparks_emitter, col.point, velocity_dir, sparks_count, sparks_speed);
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
    } else if (projectile->flags & JUMP_SHOOTER_PROJECTILE){
        fill_collisions(entity, &collisions_buffer, GROUND | PLAYER | CENTIPEDE_SEGMENT);
        
        Enemy *enemy = &entity->enemy;
        
        for (i32 i = 0; i < collisions_buffer.count; i++){
            Collision col = collisions_buffer.get(i);
            Entity *other = col.other_entity;
            
            if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT){
                kill_enemy(entity, col.point, col.normal, 1);
                emit_particles(big_sparks_emitter, col.point, col.normal * -1, 1);
            }
            
            if (other->flags & PLAYER && !player_data.dead_man && !enemy->dead_man){
                // It's a good thing that we don't kill player when projectile is blocker or explosive, 
                // but of course we need to better tell player what exactly will kill him on touch. 
                // While projectiles are flying - they're leave particle trail and all flying projectiles 
                // will kill us. So when projectile stopped we should still produce particles for 
                // base projectiles so player can know what he need to know.
                // That also works for enemies - they're kill player on touch when they're producing certain particles.
                b32 can_kill_player = !projectile->dying || !(entity->flags & (BLOCKER | EXPLOSIVE));
                if (can_kill_player){
                    if (can_sword_damage_enemy(entity)){
                        kill_enemy(entity, col.point, col.normal, 1);
                    } else{
                        kill_player();
                    }
                }
            }
        }
    }
}

void update_projectile(Entity *entity, f32 dt){
    assert(entity->flags & PROJECTILE);
    
    Projectile *projectile = &entity->projectile;
    f32 lifetime = core.time.game_time - projectile->birth_time;
    
    if (projectile->max_lifetime > 0 && lifetime> projectile->max_lifetime){
        entity->destroyed = true;    
        return;
    }
    
    if (projectile->flags & PLAYER_RIFLE){
        f32 distance_to_player = magnitude(entity->position - player_entity->position);
        
        if (distance_to_player > 1000){
            entity->destroyed = true;
            return;
        }
        
        if (projectile->type == WEAK){
            f32 weak_threshold = 0.1f;
            if (lifetime > weak_threshold){
                f32 life_overshoot = lifetime - weak_threshold;
                projectile->dying = true;
                
                clamp_magnitude(&projectile->velocity, 60);
                projectile->velocity.y -= player_data.gravity * dt;
            }
        }
    }
    
    if (projectile->flags & JUMP_SHOOTER_PROJECTILE){
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
        } else{
        }
    }
    
    Vector2 move = projectile->velocity * dt;
    Vector2 move_dir = normalized(move);
    f32 move_len = magnitude(move);
    f32 max_move_len = entity->scale.y * 0.5f;
    
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
        //So if entiity was somehow destoyed, annighilated
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
            printf("WARNING: Entity with flag LIGHT don't have corresponding light index (id - %d)\n", e->id);
        } else{
            Light *light = current_level_context->lights.get_ptr(e->light_index);
            light->position = e->position;
        }
    }
    
    if (e->flags & DOOR){
        e->door.closed_position = e->door.is_open ? e->position - e->up * e->scale.y : e->position;
        e->door.open_position   = e->door.is_open ? e->position : e->position + e->up * e->scale.y;
    }
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
}

i32 update_trigger(Entity *e){
    assert(e->flags & TRIGGER);
    
    b32 trigger_now = false;
    
    if (e->flags & ENEMY && e->enemy.dead_man){
        trigger_now = true;
        e->enemy.dead_man = false;
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
        if (str_contains(e->name, "checkpoint") && checkpoint_trigger_id != e->id){
            copy_level_context(&checkpoint_level_context, current_level_context, false);
            checkpoint_player_entity = player_entity;
            checkpoint_player_data = player_data;
            checkpoint_time = core.time;
            checkpoint_state_context = state_context;
            
            checkpoint_trigger_id = e->id;
        }
    
        if (e->trigger.load_level){
            b32 we_on_last_level = str_equal(e->trigger.level_name, "LAST_LEVEL_MARK");
            if (we_on_last_level || session_context.speedrun_timer.level_timer_active){
                win_level();
            } else{
                enter_game_state_on_new_level = true;
                last_player_data = player_data;
                load_level(e->trigger.level_name);
                return TRIGGER_LEVEL_LOAD;
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
            session_context.cam.target_zoom = e->trigger.zoom_value;
        }
        
        if (e->trigger.unlock_camera){
            state_context.cam_state.locked = false;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical = false;
        } else if (e->trigger.lock_camera){
            state_context.cam_state.locked = true;
            state_context.cam_state.on_rails_horizontal = false;
            state_context.cam_state.on_rails_vertical = false;
            session_context.cam.target = e->trigger.locked_camera_position;
        }
    
        if (e->flags & DOOR){
            trigger_entity(e, e);
        }
    
        if (e->trigger.kill_player && !debug.god_mode){
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

inline b32 update_entity(Entity *e, f32 dt){
    update_color_changer(e, dt);            
    
    if (e->flags & PHYSICS_OBJECT){
        // update rope stuff
         if (e->physics_object.on_rope){
            Entity *rope_entity = NULL;
            if (e->physics_object.rope_id == -1){
                rope_entity = add_entity(e->position, {1, 10}, {0.5f, 1.0f}, 0, BLACK, BLOCK_ROPE);
                rope_entity->need_to_save = false;
                e->physics_object.rope_id = rope_entity->id;
            } else{
                rope_entity = get_entity_by_id(e->physics_object.rope_id);
            }
            
            // spawn rope point and check
            Entity *up_rope_point_entity = NULL;
            if (e->physics_object.up_rope_point_id == -1){
                up_rope_point_entity = add_entity(e->physics_object.rope_point, {5, 5}, {0.5f, 0.5f}, 0, GREEN, ROPE_POINT);
                up_rope_point_entity->draw_order = e->draw_order - 1;
                up_rope_point_entity->need_to_save = false;
                e->physics_object.up_rope_point_id = up_rope_point_entity->id;
            } else{
                up_rope_point_entity = get_entity_by_id(e->physics_object.up_rope_point_id);
            }
            
            Entity *down_rope_point_entity = NULL;
            if (e->physics_object.down_rope_point_id == -1){
                down_rope_point_entity = add_entity(e->physics_object.rope_point, {5, 5}, {0.5f, 0.5f}, 0, GREEN, ROPE_POINT);
                down_rope_point_entity->draw_order = e->draw_order - 1;
                down_rope_point_entity->need_to_save = false;
                e->physics_object.down_rope_point_id = down_rope_point_entity->id;
            } else{
                down_rope_point_entity =  get_entity_by_id(e->physics_object.down_rope_point_id);
            }
            
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
            print("WARNING: Entity with flag LIGHT don't have corresponding light index");
        }
    }
    
    session_context.collision_grid.origin = {(f32)((i32)player_entity->position.x - ((i32)player_entity->position.x % (i32)session_context.collision_grid.cell_size.x)), (f32)((i32)player_entity->position.y - ((i32)player_entity->position.y % (i32)session_context.collision_grid.cell_size.y))};
    
    if (e->flags & PLAYER){
        if (IsKeyDown(KEY_K) && !console.is_open){
            e->position = input.mouse_position;
        } else{
            update_player(e, dt);
        }
    }
      
    if (e->flags & BIRD_ENEMY && debug.enemy_ai){
        update_bird_enemy(e, dt);
    }
    
    if (e->flags & PROJECTILE){
        update_projectile(e, dt);
    }
    
    for (i32 em = 0; em < e->emitters.count; em++){
        if (e->emitters.get_ptr(em)->follow_entity){
            e->emitters.get_ptr(em)->position = e->position;
        }
        update_emitter(e->emitters.get_ptr(em), dt);
    }
    
    if (e->flags & STICKY_TEXTURE){
        update_sticky_texture(e, dt);
    }
    
    if (e->flags & TRIGGER){
        i32 trigger_action_flags = update_trigger(e);
        if (trigger_action_flags & TRIGGER_LEVEL_LOAD){
            return false;
        }
    }
    
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
            e->enemy.dead_man = true;
            e->enemy.died_time = core.time.game_time;
            e->flags = ENEMY | BIRD_ENEMY | (e->flags & LIGHT);
            Vector2 rnd = rnd_in_circle();// e->move_sequence.moved_last_frame;
            e->bird_enemy.velocity = {e->move_sequence.velocity.x * rnd.x, e->move_sequence.velocity.y * rnd.y};

            e->move_sequence.moving = false;
            e->collision_flags = GROUND;
            init_bird_emitters(e);
            add_fire_light_to_entity(e);
            
            for (i32 i = 0; i < centipede->segments_count; i++){
                Entity *segment = current_level_context->entities.get_by_key_ptr(centipede->segments_ids.get(i));
                
                segment->volume_multiplier = 0.3f;
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
                    emit_particles(ground_splash_emitter, e->position - e->up * e->scale.y * 0.5f, e->up, 4, 1.5f);
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
                    projectile_entity->projectile.flags = JUMP_SHOOTER_PROJECTILE;
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
                    
                    Particle_Emitter *trail_emitter = projectile_entity->emitters.add(rifle_bullet_emitter);                     
                    trail_emitter->position = projectile_entity->position;
                    enable_emitter(trail_emitter);
                    
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
            
            shooter->trail_emitter->direction = e->up * -1;
            shooter->trail_emitter->count_multiplier = lerp(1.0f, 10.0f, sqrtf(picking_point_t));
            shooter->trail_emitter->speed_multiplier = lerp(1.0f, 10.0f, sqrtf(picking_point_t));
            shooter->trail_emitter->over_time = 50;
            
            // jump shooter fly to next
            if (picking_point_time >= shooter->max_picking_point_time){
                shooter->states.picking_point = false;
                shooter->states.flying_to_point = true;
                shooter->states.flying_start_time = core.time.game_time;
                
                shooter->velocity = dir * 400;
                shooter->flying_emitter->position = e->position - e->up * e->scale.y * 0.5f;
                enable_emitter(shooter->flying_emitter);
                
                shooter->trail_emitter->count_multiplier = 1;
                shooter->trail_emitter->speed_multiplier = 1;
                shooter->trail_emitter->over_time = 10;
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
        
        shooter->trail_emitter->position = e->position - e->up * e->scale.y * 0.5f;
        if (!shooter->states.picking_point && shooter->velocity != Vector2_zero){
            shooter->trail_emitter->direction = normalized(shooter->velocity * -1);;
        }
        
        if (shooter->states.flying_to_point){
            shooter->flying_emitter->position  = e->position - e->up * e->scale.y * 0.5f;
            if (shooter->velocity != Vector2_zero){
                shooter->flying_emitter->direction = normalized(shooter->velocity * -1);;
            }
        }                
    } // end update jump shooter
    
    if (e->flags & DOOR){
        update_door(e);
    }
    
    return true;
} //update entity end

void update_entities(f32 dt){
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
        
        if (e->enabled && game_state == GAME && e->spawn_enemy_when_no_ammo && player_data.ammo_count <= 0 && (!current_level_context->entities.has_key(e->spawned_enemy_id) || e->spawned_enemy_id == -1)){
            Entity *spawned = spawn_object_by_name("enemy_base", e->position);
            spawned->enemy.gives_full_ammo = true;
            e->spawned_enemy_id = spawned->id;
        }
        
        if (!e->enabled || (e->hidden && game_state == GAME)){
            continue;
        }
        
        if (game_state == EDITOR || game_state == PAUSE){
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

void draw_player(Entity *entity){
    assert(entity->flags & PLAYER);
    
    if (player_data.dead_man){
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

void draw_sword(Entity *entity){
    assert(entity->flags & SWORD);
    
    Entity visual_entity = *entity;
    
    f32 time_since_shake = core.time.game_time - player_data.timers.rifle_shake_start_time;
    
    if (0 && player_data.rifle_active){
        f32 activated_time = core.time.game_time - player_data.timers.rifle_activate_time;
        f32 activate_t = clamp01(activated_time / player_data.rifle_max_active_time);
        
        Vector2 perlin_rnd = {perlin_noise3(core.time.game_time * 30, 1, 2), perlin_noise3(1, core.time.game_time * 30, 3)};
        
        //visual_entity.position += rnd_in_circle() * lerp(0.2f, 1.3f, activate_t * activate_t);
        visual_entity.position += perlin_rnd * lerp(0.2f, 1.3f, activate_t * activate_t);
    } else if (time_since_shake <= 0.2f){
        Vector2 perlin_rnd = {perlin_noise3(core.time.game_time * 30, 1, 2), perlin_noise3(1, core.time.game_time * 30, 3)};
        visual_entity.position += perlin_rnd * 1.8f;
    }
    
    if (player_data.rifle_active){
        visual_entity.color = ColorBrightness(GREEN, 0.3f);
    }
    
    draw_game_triangle_strip(&visual_entity);
    
    if (player_data.rifle_active){
        draw_game_line_strip(&visual_entity, WHITE);
    }
}

void draw_enemy(Entity *entity){
    assert(entity->flags & ENEMY);
    
    draw_game_triangle_strip(entity);
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
    cam_bounds.size /= session_context.cam.unit_size;
    
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

void fill_entities_draw_queue(){
    session_context.entities_draw_queue.clear();
    
    // That also acts entities loop on draw update call. For example we use it for some immediate stuff that should
    // work on occluded entities.
    ForEntities(entity, 0){
        if (!entity->enabled){
            continue;
        }
        
        if (entity->hidden && game_state == GAME/* && !should_draw_entity_anyway(&e)*/){
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
                    draw_game_circle(point, 1  * (0.4f / session_context.cam.cam2D.zoom), SKYBLUE);
                    draw_game_text(point - Vector2_up, text_format("%d", ii), 18 / session_context.cam.cam2D.zoom, RED);
                    
                    if (entity->flags & JUMP_SHOOTER){
                        Collision nearest_ground = get_nearest_ground_collision(point, 20);
                        if (nearest_ground.collided){
                            Collision ray_collision = raycast(point, normalized(nearest_ground.point - point), magnitude(nearest_ground.point - point), GROUND, 1);
                            if (ray_collision.collided){
                                make_line(ray_collision.point, ray_collision.point + ray_collision.normal * 5, GREEN);
                            }
                        } else{
                            draw_game_circle(point, 1 * (0.4f / session_context.cam.cam2D.zoom), RED);
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
            if (should_draw_editor_hints()){
                // draw cam zoom trigger draw trigger zoom draw trigger cam
                if (entity->trigger.change_zoom){
                    Bounds cam_bounds = get_cam_bounds(session_context.cam, entity->trigger.zoom_value);
                    Vector2 position = entity->position;
                    if (entity->trigger.lock_camera){
                    }
                    draw_game_circle(entity->trigger.locked_camera_position, 2, PINK);
                    
                    Color cam_border_color = Fade(PINK, 0.15f);
                    if (editor.selected_entity && editor.selected_entity->id == entity->id){
                        cam_border_color = Fade(ColorBrightness(PINK, 0.3f), 0.45f);
                    }
                    position = entity->trigger.locked_camera_position;
                    make_rect_lines(position + cam_bounds.offset, cam_bounds.size, {0.5f, 0.5f}, 2.0f / (session_context.cam.cam2D.zoom), cam_border_color);
                    draw_game_text((position + cam_bounds.offset) - cam_bounds.size * 0.5f, text_format("%.2f", entity->trigger.zoom_value), 18.0f / session_context.cam.cam2D.zoom, ColorBrightness(color_fade(cam_border_color, 1.5f), 0.5f));
                }
                
                if (entity->trigger.lock_camera){
                }
                
                if (entity->trigger.start_cam_rails_horizontal || entity->trigger.start_cam_rails_vertical){
                    for (i32 ii = 0; ii < entity->trigger.cam_rails_points.count; ii++){
                        Vector2 point = entity->trigger.cam_rails_points.get(ii);
                        
                        Color color = editor.selected_entity && editor.selected_entity->id == entity->id ? ColorBrightness(WHITE, 0.2f) : ColorBrightness(Fade(WHITE, 0.1f), 0.05f);
                        
                        if (IsKeyDown(KEY_LEFT_ALT)){
                            draw_game_circle(point, 1  * (0.4f / session_context.cam.cam2D.zoom), SKYBLUE);
                            draw_game_text(point - Vector2_up, text_format("%d", ii), 18 / session_context.cam.cam2D.zoom, RED);
                        }
                        if (ii < entity->trigger.cam_rails_points.count - 1){
                            make_line(point, entity->trigger.cam_rails_points.get(ii+1), color);
                        } 
                    }
                }
            }
            
            // @SHIT Wtf, why we do that here. That should be in trigger drawing and should happen only in editor. 
            // But if we remove connected removal right now - we destroy ourselves because we should track that separetely in 
            // game trigger update.
            // Maybe that was made here because i wanted to draw stuff like lines and we did not have immediate drawing at that time, 
            // but still that too stupid to be true.
            b32 is_trigger_selected = editor.selected_entity && editor.selected_entity->id == entity->id;
            for (i32 ii = 0; ii < entity->trigger.connected.count; ii++){
                i32 id = entity->trigger.connected.get(ii);
                Entity *connected_entity = current_level_context->entities.get_by_key_ptr(id);
                
                if (!connected_entity){
                    continue;
                }
                
                if (connected_entity->flags & DOOR && ((entity->flags ^ TRIGGER) > 0 || game_state != GAME)){
                    Color color = connected_entity->door.is_open == entity->trigger.open_doors ? SKYBLUE : ORANGE;
                    f32 width = connected_entity->door.is_open == entity->trigger.open_doors ? 1.0f : 0.2f;
                    make_line(entity->position, connected_entity->position, width, Fade(ColorBrightness(color, 0.2f), 0.3f));
                } else if (is_trigger_selected && should_draw_editor_hints()){
                    make_line(entity->position, connected_entity->position, RED);
                }
            }
            for (i32 ii = 0; ii < entity->trigger.tracking.count; ii++){
                i32 id = entity->trigger.tracking.get(ii);
                Entity *tracked_entity = get_entity_by_id(id);
                if (!tracked_entity){
                    continue;
                }
                                
                if (is_trigger_selected && should_draw_editor_hints()){
                    make_line(entity->position, tracked_entity->position, GREEN);
                } else if (entity->trigger.draw_lines_to_tracked && game_state != EDITOR){
                    if ((tracked_entity->flags & ENEMY | CENTIPEDE) && !tracked_entity->enemy.dead_man){
                        make_line(entity->position, tracked_entity->position, 1.0f, Fade(PINK, 0.3f));
                    }
                }
            }
        }
        
        //always draw sticky texture
        if (entity->flags & STICKY_TEXTURE){
            Sticky_Texture *st = &entity->sticky_texture;
            Entity *follow_entity = get_entity_by_id(entity->sticky_texture.follow_id);
            if (follow_entity){
                if (follow_entity->flags & BLOCKER && st->should_draw_texture){
                    make_light(follow_entity->position, 75, 1, 1.0f, WHITE);
                }
                
                if (follow_entity->flags & SWORD_SIZE_REQUIRED && st->should_draw_texture){
                    make_light(follow_entity->position, 75, 1.5, 0.7f, follow_entity->enemy.big_sword_killable ? ColorBrightness(RED, 0.4f) : BLUE);
                }
            }
            
            f32 lifetime = core.time.game_time - entity->sticky_texture.birth_time;
            f32 lifetime_t = 0.5f;
            if (entity->sticky_texture.max_lifetime > EPSILON){
                lifetime_t = lifetime / entity->sticky_texture.max_lifetime;
            }
        
            if (entity->sticky_texture.draw_line && entity->sticky_texture.need_to_follow && player_entity){
                Entity *follow_entity = current_level_context->entities.get_by_key_ptr(entity->sticky_texture.follow_id);
                Color line_color = entity->sticky_texture.line_color;
                if (follow_entity && follow_entity->flags & ENEMY && entity->sticky_texture.max_lifetime > 0 && !(follow_entity->flags & SHOOT_STOPER)){
                    line_color = follow_entity->enemy.dead_man ? color_fade(SKYBLUE, 0.3f) : color_fade(RED, 0.3f);
                }
    
                Vector2 vec_to_follow = entity->position - player_entity->position;
                f32 len = magnitude(vec_to_follow);
                if (len <= entity->sticky_texture.max_distance || entity->sticky_texture.max_distance <= 0){
                    make_line(player_entity->position, entity->position, lerp(line_color, color_fade(line_color, 0), lifetime_t * lifetime_t));
                }
            }
        }
        
        // This checks for occlusion.
        if (should_not_draw_entity(entity, session_context.cam)){
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
    }
    
    draw_game_triangle_strip(e, Fade(e->color, 0.1f));
    draw_game_line_strip(line_strip_points.data, line_strip_points.count, RED);
}

inline Vector2 get_shoot_stoper_cross_position(Entity *entity){
    return entity->position + entity->up * entity->scale.y * 0.85f;
}

inline b32 should_draw_editor_hints(){
    return (game_state == EDITOR || game_state == PAUSE || debug.draw_areas_in_game);
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
                    e->scale = (e->sticky_texture.base_size) / fminf(session_context.cam.cam2D.zoom, 0.35f); 
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
        if (game_state == EDITOR || game_state == PAUSE){
            make_texture(e->texture, e->position, e->scale, e->pivot, e->rotation, e->color);
            // draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
            if (editor.selected_entity && editor.selected_entity->id == e->id || IsKeyDown(KEY_LEFT_SHIFT) || focus_input_field.in_focus && str_contains(focus_input_field.tag, text_format("%d", e->id))){
                Vector2 note_size = {screen_width * 0.2f, screen_height * 0.2f};
                i32 content_count = str_len(note->content);
                f32 chars_scaling_treshold = 200 * UI_SCALING;
                if (content_count > chars_scaling_treshold){
                    note_size *= lerp(1.0f, 2.5f, clamp01(((f32)content_count - chars_scaling_treshold) / (chars_scaling_treshold * 4)));
                }
                if (make_input_field(note->content, world_to_screen_with_zoom(e->position + ((Vector2){e->scale.x * 0.5f, e->scale.y * -0.5f})), note_size, text_format("note%d", e->id))){
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
        if (game_state == EDITOR || game_state == PAUSE){
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
                draw_game_circle(e->position + e->right * e->scale.x * 0.5f, e->scale.y * 0.4f, GREEN);
            }
        }
        if (e->centipede_head->centipede.spikes_on_left){
            draw_spikes(e, e->up, e->right * -1.0f, e->scale.y, e->scale.x);
        } else{
            if (!e->enemy.dead_man){
                draw_game_circle(e->position - e->right * e->scale.x * 0.5f, e->scale.y * 0.4f, GREEN);
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
                scale /= session_context.cam.cam2D.zoom;
                Texture blocker_texture = e->jump_shooter.blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
                make_texture(blocker_texture, bullet_hint_position + e->up * scale.y * 0.65f, scale, {0.5f, 0.5f}, 0, WHITE);
            }
        }
    } else if (e->flags & ENEMY){
        draw_enemy(e);
    }
    
    if (e->flags & WIN_BLOCK){
    }
    
    if (e-> flags & DRAW_TEXT){
        draw_game_text(e->position, e->text_drawer.text, e->text_drawer.size, RED);
    }
    
    if (e->flags & SPIKES && (!e->hidden || game_state != GAME)){
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
    
    if (e->flags & PROPELLER && (game_state == EDITOR || game_state == PAUSE || debug.draw_areas_in_game)){
        draw_game_line_strip(e, e->color);
        draw_game_triangle_strip(e, e->color * 0.1f);
    }
    
    if (e->flags & BLOCKER && (game_state == EDITOR) && !e->enemy.blocker_immortal){
        Texture texture = e->enemy.blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
        
        draw_game_texture(texture, e->position, {10.0f, 10.0f}, {0.5f, 0.5f}, 0, WHITE);
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
    
    if (debug.draw_bounds || editor.selected_entity && (game_state == EDITOR || game_state == PAUSE) && e->id == editor.selected_entity->id){
        make_rect_lines(e->position + e->bounds.offset, e->bounds.size, e->pivot, 1.0f / session_context.cam.cam2D.zoom, GREEN);
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
                rotate(e, player_data.sword_angular_velocity * core.time.not_updated_accumulated_dt);
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
        
        draw_game_circle(editor.player_spawn_point, 3, BLUE);
        
        b32 draw_circles_on_vertices = IsKeyDown(KEY_LEFT_ALT);
        if (draw_circles_on_vertices){
            for (i32 v = 0; v < e->vertices.count; v++){
                draw_game_circle(global(e, e->vertices.get(v)), 1.0f * (0.4f / session_context.cam.cam2D.zoom), PINK);
                //draw unscaled vertices
                if (IsKeyDown(KEY_LEFT_SHIFT)){    
                    draw_game_circle(global(e, e->unscaled_vertices.get(v)), 1.0f * 0.4f, PURPLE);
                }
            }
        }
        
        if (debug.draw_position){
            draw_game_text(e->position + ((Vector2){0, -3}), text_format("POS:   {%.2f, %.2f}", e->position.x, e->position.y), 20, RED);
        }
        
        if (debug.draw_rotation){
            draw_game_text(e->position, text_format("%d", (i32)e->rotation), 20, RED);
        }
        
        if (debug.draw_scale){
            draw_game_text(e->position + ((Vector2){0, -6}), text_format("SCALE:   {%.2f, %.2f}", e->scale.x, e->scale.y), 20, RED);
        }
        
        if (debug.draw_directions){
            draw_game_text(e->position + ((Vector2){0, -6}), text_format("UP:    {%.2f, %.2f}", e->up.x, e->up.y), 20, RED);
            draw_game_text(e->position + ((Vector2){0, -9}), text_format("RIGHT: {%.2f, %.2f}", e->right.x, e->right.y), 20, RED);
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
        
        draw_game_text(editor.ruler_start_position + (vec_to_mouse * 0.5f), text_format("%.2f", length), 24.0f / session_context.cam.cam2D.zoom, RED);
        draw_game_text(input.mouse_position + Vector2_up, text_format("{%.2f, %.2f}", input.mouse_position.x, input.mouse_position.y), 26.0f / session_context.cam.cam2D.zoom, GREEN); 
        
    }
}

void draw_particles(){
    for (i32 i = 0; i < current_level_context->particles.max_count; i++){
        Particle particle = current_level_context->particles.get(i);
        if (!particle.enabled){
            continue;   
        }
        
        draw_game_rect(particle.position, particle.scale, {0.5f, 0.5f}, 0, particle.color);
    }
}

void draw_ui(const char *tag){
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
    return drawing_state == CAMERA_DRAWING;
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
    
    if (!debug.drawing_stopped){
        render.lines_to_draw.clear();
        render.ring_lines_to_draw.clear();
        render.rect_lines_to_draw.clear();
        render.textures_to_draw.clear();
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
    
    session_context.cam.position += ((Vector2){x_offset, y_offset}) * state_context.cam_state.trauma * state_context.cam_state.trauma;
}

Cam saved_cam;
Cam with_shake_cam;

inline f32 get_light_zoom(f32 radius){
    return SCREEN_WORLD_SIZE / radius;
}

inline void add_light_to_draw_queue(Light light){
    render.lights_draw_queue.add(light);
}

void draw_game(){
    saved_cam = session_context.cam;

    apply_shake();
    
    with_shake_cam = session_context.cam;

    local_persist Shader smooth_edges_shader = LoadShader(0, "./resources/shaders/smooth_edges.fs");
    
    
    BeginDrawing();
    BeginTextureMode(render.main_render_texture);
    BeginMode2D(session_context.cam.cam2D);
    
    ClearBackground(is_explosion_trauma_active() ? (player_data.dead_man ? RED : WHITE) : GRAY);
    
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
    
    if (game_state == EDITOR || game_state == PAUSE){
        draw_editor();
    }
    
    if (debug.draw_collision_grid){
        // draw collision grid
        Collision_Grid grid = session_context.collision_grid;
        Vector2 player_position = player_entity ? player_entity->position : editor.player_spawn_point;
        
        update_entity_collision_cells(&mouse_entity);
        for (f32 row = -grid.size.y * 0.5f + grid.origin.y; row <= grid.size.y * 0.5f + grid.origin.y; row += grid.cell_size.y){
            for (f32 column = -grid.size.x * 0.5f + grid.origin.x; column <= grid.size.x * 0.5f + grid.origin.x; column += grid.cell_size.x){
                Collision_Grid_Cell *cell = get_collision_cell_from_position({column, row});
                
                draw_game_rect_lines({column, row}, grid.cell_size, {0, 1}, 0.5f / session_context.cam.cam2D.zoom, (cell && cell->entities_ids.count > 0) ? GREEN : RED);
            }
        }
    }
    
    EndMode2D();
    EndTextureMode();

    drawing_state = LIGHTING_DRAWING;
    //Light render pass
    BeginTextureMode(global_illumination_rt);{
        if (!debug.full_light){
            ClearBackground(Fade(ColorBrightness(SKYBLUE, -0.4f), 0));
        } else{
            ClearBackground(WHITE);
        }
    } EndTextureMode();

    BeginTextureMode(light_geometry_rt);{
        ClearBackground(Fade(BLACK, 0));
        BeginMode2D(session_context.cam.cam2D);
        BeginShaderMode(gaussian_blur_shader);
            i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
            set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / (screen_width), (1.0f) / (screen_height)});

            // ForEntities(entity, GROUND | ENEMY | PLAYER | PLATFORM | SWORD){
            for (i32 i = 0; i < session_context.entities_draw_queue.count; i++){
                Entity *entity = session_context.entities_draw_queue.get_ptr(i);
                if (entity->hidden || should_not_draw_entity(entity, session_context.cam)){
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
        
        if (!light_ptr->exists){
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
        if (!should_calculate_light_anyway && (!check_bounds_collision(session_context.cam.view_position, light.position, get_cam_bounds(session_context.cam, session_context.cam.cam2D.zoom), lightmap_bounds) || (connected_entity && connected_entity->hidden && game_state == GAME)) || debug.full_light){
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
                session_context.cam = get_cam_for_resolution(shadows_texture_size.x, shadows_texture_size.y);
                session_context.cam.position = light.position;
                session_context.cam.view_position = light.position;
                session_context.cam.cam2D.zoom = get_light_zoom(light.radius);
                BeginMode2D(session_context.cam.cam2D);
                ForEntities(entity, GROUND | light.additional_shadows_flags){
                    if (entity->hidden || entity->id == light.connected_entity_id || should_not_draw_entity(entity, session_context.cam)){
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
                session_context.cam = with_shake_cam;
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
                session_context.cam = get_cam_for_resolution(backshadows_texture_size.x, backshadows_texture_size.y);
                session_context.cam.position = light.position;
                session_context.cam.view_position = light.position;
                session_context.cam.cam2D.zoom = get_light_zoom(light.radius);
                BeginMode2D(session_context.cam.cam2D);
                ForEntities(entity, ENEMY | BLOCK_ROPE | SPIKES | PLAYER | PLATFORM | SWORD){
                    if (entity->hidden || entity->id == light.connected_entity_id || should_not_draw_entity(entity, session_context.cam)){
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
                session_context.cam = with_shake_cam;
            }; EndTextureMode();
        }
        
        // if (light.make_shadows || light.make_backshadows){
        //     BeginTextureMode(light.geometry_rt);{
        //         ClearBackground(Fade(BLACK, 0));
        //         session_context.cam = get_cam_for_resolution(light.geometry_size, light.geometry_size);
        //         session_context.cam.position = light_position;
        //         session_context.cam.view_position = light_position;
        //         session_context.cam.cam2D.zoom = get_light_zoom(light.radius);
        //         BeginMode2D(session_context.cam.cam2D);
        //         BeginShaderMode(gaussian_blur_shader);
        //             i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
        //             set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / (light.geometry_size), (1.0f) / (light.geometry_size)});
    
        //             ForEntities(entity, GROUND | ENEMY | PLAYER | PLATFORM | SWORD){
        //                 if (entity->hidden || should_not_draw_entity(entity, session_context.cam)){
        //                     continue;
        //                 }
        //                 Color prev_color = entity->color;
        //                 entity->color = BLACK;
        //                 draw_entity(entity);
        //                 entity->color = prev_color;
        //             }
        //             //draw_particles();
        //         EndShaderMode();
        //         EndMode2D();
        //         session_context.cam = with_shake_cam;
        //     }EndTextureMode();
        // }
            
        add_light_to_draw_queue(light);
    }
    
    BeginTextureMode(global_illumination_rt);{
    BeginShaderMode(smooth_edges_shader);{
    for (i32 i = 0; i <  render.lights_draw_queue.count; i++){
        Light light = render.lights_draw_queue.get(i);
        Vector2 lightmap_game_scale = {light.radius, light.radius};
            Texture shadowmask_texture = light.make_shadows ? light.shadowmask_rt.texture : white_transparent_pixel_texture;
        
            Vector2 lightmap_texture_pos = get_left_down_texture_screen_position(shadowmask_texture, light.position, lightmap_game_scale);
            BeginMode2D(session_context.cam.cam2D);{
                    local_persist i32 light_power_loc         = get_shader_location(smooth_edges_shader, "light_power");
                    local_persist i32 light_color_loc         = get_shader_location(smooth_edges_shader, "light_color");
                    local_persist i32 my_pos_loc              = get_shader_location(smooth_edges_shader, "my_pos");
                    local_persist i32 my_size_loc             = get_shader_location(smooth_edges_shader, "my_size");
                    local_persist i32 gi_size_loc             = get_shader_location(smooth_edges_shader, "gi_size");
                    local_persist i32 gi_texture_loc          = get_shader_location(smooth_edges_shader, "gi_texture");
                    local_persist i32 geometry_texture_loc    = get_shader_location(smooth_edges_shader, "geometry_texture");
                    local_persist i32 light_texture_loc       = get_shader_location(smooth_edges_shader, "light_texture");
                    local_persist i32 backshadows_texture_loc = get_shader_location(smooth_edges_shader, "backshadows_texture");
                    
                    set_shader_value_color(smooth_edges_shader, light_color_loc, ColorNormalize(Fade(light.color, light.opacity)));
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
            session_context.cam = with_shake_cam;
    }
    } EndShaderMode();
    } EndTextureMode();
    
    render.lights_draw_queue.clear();
    
    //blur pass
    i32 iterations = 1;
    for (i32 i = 0; i < iterations; i++){
        BeginTextureMode(global_illumination_rt);{
            BeginShaderMode(gaussian_blur_shader);{
                i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
                set_shader_value(gaussian_blur_shader, u_pixel_loc, {(2.0f) / screen_width, (2.0f) / screen_height});
                draw_render_texture(global_illumination_rt.texture, {1.0f, 1.0f}, WHITE);
            } EndShaderMode();
        } EndTextureMode();
    }
    
    drawing_state = CAMERA_DRAWING;
    BeginShaderMode(env_light_shader);{
        local_persist i32 gi_data_loc = get_shader_location(env_light_shader, "u_gi_data");
        set_shader_value_tex(env_light_shader, gi_data_loc, global_illumination_rt.texture);
        
        draw_render_texture(render.main_render_texture.texture, {1, 1}, WHITE);
    } EndShaderMode();
    
    update_input_field();
    
    BeginMode2D(session_context.cam.cam2D);{
        draw_immediate_stuff();
    } EndMode2D();
    
    if (state_context.we_got_a_winner){
        make_ui_text("Finale for now!\nNow you can try speedruns.\nOpen console with \"/\" (slash) button and type help.\ngame_speedrun for full game speedrun.\nlevel_speedrun for current level speedrun.\nfirst for loading first level\nnext for loading next level", {screen_width * 0.3f, screen_height * 0.2f}, 20, GREEN, "win_speedrun_text");
    }
    
    if (game_state == GAME && player_data.dead_man && !state_context.we_got_a_winner){
        f32 since_died = core.time.game_time - player_data.timers.died_time;
        
        f32 t = clamp01((since_died - 3.0f) / 2.0f);
        make_ui_text("T - restart", {screen_width * 0.45f, screen_height * 0.45f}, 40, Fade(GREEN, t * t), "restart_text");
    }
    
    draw_ui("");
    
    session_context.cam = saved_cam;
    
    f32 v_pos = 10;
    f32 font_size = 18;
    if (debug.info_fps){
        draw_text(text_format("FPS: %d", GetFPS()), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (game_state == GAME && player_entity){            
        if (debug.info_spin_progress){
            draw_text(text_format("Spin progress: %.2f", player_data.sword_spin_progress), 10, v_pos, font_size, RED);
            v_pos += font_size;
        }
        
        if (debug.info_blood_progress){
            draw_text(text_format("Blood progress: %.2f", player_data.blood_progress), 10, v_pos, font_size, RED);
            v_pos += font_size;
        }
    }
    
    if (debug.info_particle_count){
        draw_text(text_format("Particles count: %d", enabled_particles_count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (debug.info_emitters_count){
        draw_text(text_format("Emitters count: %d", current_level_context->emitters.count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (debug.info_player_speed){
        draw_text(text_format("Player speed: %.1f", magnitude(player_data.velocity)), 10, v_pos, font_size, RED);
        v_pos += font_size;
        draw_text(text_format("Player Velocity: {%.1f, %.1f}", player_data.velocity.x, player_data.velocity.y), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    v_pos += font_size;
    draw_text(text_format("Ammo: %d", player_data.ammo_count), 10, v_pos, font_size * 1.5f, VIOLET);
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
        if (player_data.rifle_active){
            draw_line(input.screen_mouse_position - Vector2_right * 10 - Vector2_up * 10, input.screen_mouse_position + Vector2_right * 10 + Vector2_up * 10, WHITE);
            draw_line(input.screen_mouse_position + Vector2_right * 10 - Vector2_up * 10, input.screen_mouse_position - Vector2_right * 10 + Vector2_up * 10, WHITE);
            draw_rect({input.screen_mouse_position.x - 2.5f, input.screen_mouse_position.y - 2.5f}, {5, 5}, GREEN);
        } else{
            draw_rect({input.screen_mouse_position.x - 5, input.screen_mouse_position.y - 5}, {10, 10}, RED);
        }
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

Entity* add_entity(Entity *copy, b32 keep_id){
    Entity e = Entity(copy, keep_id);
    
    if (!keep_id && game_state == EDITOR){
        ForEntities(table_entity, 0){
            if (table_entity->flags & TRIGGER && table_entity->trigger.connected.contains(copy->id)){
                table_entity->trigger.connected.add(e.id);
            }
        }
    }
    e.level_context = current_level_context;
    current_level_context->entities.add(e.id, e);
    return current_level_context->entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    Entity e = Entity(pos, scale, pivot, rotation, flags);    
    e.id = current_level_context->entities.total_added_count + core.time.app_time * 10000 + 100;
    check_avaliable_ids_and_set_if_found(&e.id);
    e.level_context = current_level_context;
    // assert(e.id != 1);  
    // if (flags & PLAYER){
    //     e.id = 1;
    // }
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

Particle_Emitter* add_emitter(){
    Particle_Emitter e = Particle_Emitter();
    current_level_context->emitters.add(e);    
    return current_level_context->emitters.last_ptr();
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

Vector2 world_to_screen(Vector2 position){
    Vector2 cam_pos = session_context.cam.position;

    Vector2 with_cam = subtract(position, cam_pos);
    Vector2 pixels   = multiply(with_cam, session_context.cam.unit_size);
    
    //Horizontal center and vertical bottom
    f32 width_add, height_add;
    
    width_add = session_context.cam.width * 0.5f;    
    height_add = session_context.cam.height * 0.5f;    
    Vector2 to_center = {pixels.x + width_add, height_add - pixels.y};

    return to_center;
}

//This gives us real screen pixel position
Vector2 world_to_screen_with_zoom(Vector2 position){
    Vector2 cam_pos = session_context.cam.position;

    Vector2 with_cam = subtract(position, cam_pos);
    Vector2 pixels   = multiply(with_cam, session_context.cam.unit_size * session_context.cam.cam2D.zoom);
    //Horizontal center and vertical bottom
    
    f32 width_add, height_add;
    
    width_add = session_context.cam.width * 0.5f;    
    height_add = session_context.cam.height * 0.5f;    
    Vector2 to_center = {pixels.x + width_add, height_add - pixels.y};

    return to_center;
}

Vector2 get_texture_pixels_size(Texture texture, Vector2 game_scale){
    Vector2 screen_texture_size_multiplier = transform_texture_scale(texture, game_scale);
    return multiply({(f32)texture.width, (f32)texture.height}, screen_texture_size_multiplier) * session_context.cam.cam2D.zoom;
}

Vector2 get_left_down_texture_screen_position(Texture texture, Vector2 world_position, Vector2 game_scale){
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

void draw_game_circle(Vector2 position, f32 radius, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_circle(screen_pos, radius * session_context.cam.unit_size, color);
}

void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect(screen_pos, multiply(scale, session_context.cam.unit_size), color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect_lines(screen_pos, scale * session_context.cam.unit_size, thick, color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect_lines(screen_pos, scale * session_context.cam.unit_size, color);
}

void draw_game_line_strip(Entity *entity, Color color){
    Vector2 screen_positions[entity->vertices.count];
    
    for (i32 i = 0; i < entity->vertices.count; i++){
        screen_positions[i] = world_to_screen(global(entity, entity->vertices.get(i)));
    }
    
    draw_line_strip(screen_positions, entity->vertices.count, color);
}

void draw_game_line_strip(Vector2 *points, i32 count, Color color){
    Vector2 screen_positions[count];
    
    for (i32 i = 0; i < count; i++){
        screen_positions[i] = world_to_screen(points[i]);
    }
    
    draw_line_strip(screen_positions, count, color);
}

void draw_game_triangle_strip(Array<Vector2, MAX_VERTICES> vertices, Vector2 position, Color color){
    Vector2 screen_positions[vertices.count];
    
    for (i32 i = 0; i < vertices.count; i++){
        screen_positions[i] = world_to_screen(global(position, vertices.get(i)));
    }
    
    draw_triangle_strip(screen_positions, vertices.count, color);
}

inline void draw_game_triangle_strip(Entity *entity, Color color){
    draw_game_triangle_strip(entity->vertices, entity->position, color);
}

inline void draw_game_triangle_strip(Entity *entity){
    draw_game_triangle_strip(entity, entity->color);
}

void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, {0, 0});
    draw_rect(screen_pos, multiply(scale, session_context.cam.unit_size), pivot, rotation, color);
}

void draw_game_text(Vector2 position, const char *text, f32 size, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_text(text, screen_pos, size, color);
}

void draw_game_texture(Texture tex, Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color, b32 flip){
    Vector2 screen_pos = world_to_screen(position);
    draw_texture(tex, screen_pos, transform_texture_scale(tex, scale), pivot, rotation, color, flip);
}

void draw_game_texture_full(Texture tex, Vector2 position, Vector2 pivot, f32 rotation, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_texture(tex, screen_pos, {6.0f, 6.0f}, pivot, rotation, color, true);
}

void draw_game_line(Vector2 start, Vector2 end, f32 thick, Color color){
    draw_line(world_to_screen(start), world_to_screen(end), thick * session_context.cam.unit_size, color);
}

void draw_game_line(Vector2 start, Vector2 end, Color color){
    draw_line(world_to_screen(start), world_to_screen(end), color);
}

void draw_game_ring_lines(Vector2 center, f32 inner_radius, f32 outer_radius, i32 segments, Color color, f32 start_angle, f32 end_angle){
    draw_ring_lines(world_to_screen(center), inner_radius * session_context.cam.unit_size, outer_radius * session_context.cam.unit_size, segments, color);
}

void draw_game_triangle_lines(Vector2 v1, Vector2 v2, Vector2 v3, Color color){
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