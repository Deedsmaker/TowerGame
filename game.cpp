#pragma once

//#define assert(a) (if (!a) (int*)void*);
//#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

#include "game.h"
#include "../my_libs/perlin.h"

#define ForTable(table, xx) for(int xx = table_next_avaliable(table, 0);  xx < table.max_count; xx = table_next_avaliable(table, xx+1))
#define ForEntities(entity, flags) Entity *entity = NULL; for (int index = next_entity_avaliable(0, &entity, flags); index < context.entities.max_count && entity; index = next_entity_avaliable(index+1, &entity, flags)) 
//#define For(arr, type, value) for(int ii = 0; ii < arr.count; ii++){ type value = arr.get(ii);

global_variable Input input;
global_variable Level current_level;
global_variable Context context = {};
global_variable Render render = {};
global_variable Context saved_level_context = {};
global_variable Console console = {};
global_variable Editor editor  = {};
global_variable Debug  debug  = {};
//global_variable Entity *player_entity;
global_variable b32 player_on_level;

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

void free_entity(Entity *e){
    if (e->flags & TRIGGER){
        if (e->trigger.connected.max_count > 0){
            e->trigger.connected.free_arr();
        }
    }
    
    if (e->flags & BIRD_ENEMY){
        bird_clear_formation(&e->bird_enemy);
    }
    
    if (e->flags & CENTIPEDE){
        // free centipede
        for (int i = 0; i < e->centipede.segments_ids.count; i++){
            Entity *segment = context.entities.get_by_key_ptr(e->centipede.segments_ids.get(i));
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
    Vector2 scaled_size = {texture.width / UNIT_SIZE, texture.height / UNIT_SIZE};
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
    scaling_multiplier = {texture.width / UNIT_SIZE, texture.height / UNIT_SIZE};
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

Entity::Entity(Entity *copy){
    *this = *copy;

    id = copy->id;
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
        scaling_multiplier = {texture.width / UNIT_SIZE, texture.height / UNIT_SIZE};
        scale = {texture.width / 10.0f, texture.height / 10.0f};
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
        for (int i = 0; i < copy->trigger.connected.count; i++){
            trigger.connected.add(copy->trigger.connected.get(i));
        }
        trigger.tracking = Dynamic_Array<int>();
        for (int i = 0; i < copy->trigger.tracking.count; i++){
            trigger.tracking.add(copy->trigger.tracking.get(i));
        }
    }
    
    if (flags & MOVE_SEQUENCE){
        move_sequence = copy->move_sequence;
        move_sequence.points = Dynamic_Array<Vector2>();
        for (int i = 0; i < copy->move_sequence.points.count; i++){
            move_sequence.points.add(copy->move_sequence.points.get(i));
        }
    }
    
    if (flags & PROPELLER){
        propeller = copy->propeller;
    }
    
    if (flags & DOOR){
        door = copy->door;
    }
    
    // if (flags & PARTICLE_EMITTER){
    //     for (int i = 0; i < copy->emitters.count; i++){
    //         emitters.add(copy->emitters.get(i));
    //     }
    // }
    
    rotate_to(this, rotation);
    
    setup_color_changer(this);
    
    init_entity(this);
    calculate_bounds(this);
}

void parse_line(char *line, char *result, int *index){ 
    assert(line[*index] == ':');
    
    int i;
    int added_count = 0;
    for (i = *index + 1; line[i] != NULL && line[i] != ':'; i++){
        result[added_count] = line[i];
        added_count++;
    }
    
    *index = i;
}

// void copy_context(Context *dest, Context *src){
//     //*dest = *src;

//     copy_array(&dest->entities, &src->entities);
//     //copy_array(&dest->particles, &src->particles);
//     copy_array(&dest->emitters, &src->emitters);
// }

void clear_context(Context *c){
    ForEntities(entity, 0){
        free_entity(entity);
        *entity = {};
    }

    c->entities.clear();
    c->particles.clear();
    c->emitters.clear();
    
    c->we_got_a_winner = false;
    player_data = {};
}

int save_level(const char *level_name){
    char *name;
    name = get_substring_before_symbol(level_name, '.');

    String level_path = init_string();
    level_path += "levels/";
    level_path += name;
    level_path += ".level";
    FILE *fptr;
    fptr = fopen(level_path.data, "w");
    
    if (fptr == NULL){
        return 0;
    }
    
    fprintf(fptr, "Setup Data:\n");
    
    fprintf(fptr, "player_spawn_point:{:%f:, :%f:} ", editor.player_spawn_point.x, editor.player_spawn_point.y);
    
    fprintf(fptr, ";\n");
    
    fprintf(fptr, "Entities:\n");
    for (int i = 0; i < context.entities.max_count; i++){        
        if (!context.entities.has_index(i)){
            continue;
        }
    
        Entity *e = context.entities.get_ptr(i);
        
        if (!e->need_to_save){
            continue;
        }
        
        Color color = e->color_changer.start_color;
        fprintf(fptr, "name:%s: id:%d: pos{:%f:, :%f:} scale{:%f:, :%f:} pivot{:%f:, :%f:} rotation:%f: color{:%d:, :%d:, :%d:, :%d:}, flags:%llu:, draw_order:%d: ", e->name, e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags, e->draw_order);
        
        fprintf(fptr, "vertices [ ");
        for (int v = 0; v < e->vertices.count; v++){
            fprintf(fptr, "{:%f:, :%f:} ", e->vertices.get(v).x, e->vertices.get(v).y); 
        }
        fprintf(fptr, "] "); 
        
        fprintf(fptr, "unscaled_vertices [ ");
        for (int v = 0; v < e->unscaled_vertices.count; v++){
            fprintf(fptr, "{:%f:, :%f:} ", e->unscaled_vertices.get(v).x, e->unscaled_vertices.get(v).y); 
        }
        fprintf(fptr, "] "); 
        
        fprintf(fptr, "hidden:%d: ", e->hidden);
        fprintf(fptr, "spawn_enemy_when_no_ammo:%d: ", e->spawn_enemy_when_no_ammo);
        
        if (e->flags & TRIGGER){
            if (e->trigger.connected.count > 0){
                fprintf(fptr, "trigger_connected [ ");
                for (int v = 0; v < e->trigger.connected.count; v++){
                    fprintf(fptr, ":%d: ", e->trigger.connected.get(v)); 
                }
                fprintf(fptr, "] "); 
            }
            
            if (e->trigger.tracking.count > 0){
                fprintf(fptr, "trigger_tracking [ ");
                for (int v = 0; v < e->trigger.tracking.count; v++){
                    fprintf(fptr, ":%d: ", e->trigger.tracking.get(v)); 
                }
                fprintf(fptr, "] "); 
            }
            
            fprintf(fptr, "trigger_kill_player:%d: ", e->trigger.kill_player);
            fprintf(fptr, "trigger_open_doors:%d: ", e->trigger.open_doors);
            fprintf(fptr, "trigger_track_enemies:%d: ", e->trigger.track_enemies);
            fprintf(fptr, "trigger_agro_enemies:%d: ", e->trigger.agro_enemies);
            fprintf(fptr, "trigger_player_touch:%d: ", e->trigger.player_touch);
            fprintf(fptr, "trigger_shows_entities:%d: ", e->trigger.shows_entities);
            fprintf(fptr, "trigger_starts_moving_sequence:%d: ", e->trigger.starts_moving_sequence);
            fprintf(fptr, "trigger_lock_camera:%d: ", e->trigger.lock_camera);
            fprintf(fptr, "trigger_unlock_camera:%d: ", e->trigger.unlock_camera);
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
        }
        
        if (e->flags & MOVE_SEQUENCE){
            if (e->move_sequence.points.count > 0){
                fprintf(fptr, "move_sequence_points [ ");
                for (int v = 0; v < e->move_sequence.points.count; v++){
                    fprintf(fptr, "{:%f:, :%f:} ", e->move_sequence.points.get(v).x, e->move_sequence.points.get(v).y); 
                }
                fprintf(fptr, "] "); 
            }
            
            fprintf(fptr, "move_sequence_moving:%d: ", e->move_sequence.moving);
            fprintf(fptr, "move_sequence_speed:%f: ", e->move_sequence.speed);
            fprintf(fptr, "move_sequence_loop:%d: ", e->move_sequence.loop);
            fprintf(fptr, "move_sequence_rotate:%d: ", e->move_sequence.rotate);
        }
        
        if (e->flags & CENTIPEDE){
            fprintf(fptr, "spikes_on_right:%d: ", e->centipede.spikes_on_right);
            fprintf(fptr, "segments_count:%d: ", e->centipede.segments_count);
        }
        
        if (e->flags & PHYSICS_OBJECT){
            fprintf(fptr, "on_rope:%d: ", e->physics_object.on_rope);
            fprintf(fptr, "physics_rotate_by_velocity:%d: ", e->physics_object.rotate_by_velocity);
            fprintf(fptr, "physics_gravity_multiplier:%f: ", e->physics_object.gravity_multiplier);
            fprintf(fptr, "physics_mass:%f: ", e->physics_object.mass);
        }
        
        if (e->flags & DOOR){
            fprintf(fptr, "door_open:%d: ", e->door.is_open);
        }
        
        if (e->flags & BLOCKER){
            fprintf(fptr, "blocker_clockwise:%d: ", e->enemy.blocker_clockwise);
            fprintf(fptr, "blocker_immortal:%d: ", e->enemy.blocker_immortal);
        }
        
        if (e->flags & ENEMY && e->enemy.sword_kill_speed_modifier != 1){
            fprintf(fptr, "sword_kill_speed_modifier:%.1f: ", e->enemy.sword_kill_speed_modifier);
        }
        
        if (e->flags & ENEMY){
            fprintf(fptr, "gives_full_ammo:%d: ", e->enemy.gives_full_ammo);
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
        
        fprintf(fptr, ";\n"); 
    }
    
    fclose(fptr);
    
    b32 is_temp_level = str_start_with_const(name, "TEMP_");
    b32 is_autosave   = str_start_with_const(name, "AUTOSAVE_");
    if (!is_temp_level && !is_autosave){
        str_copy(context.current_level_name, name);
        reload_level_files();
        console.str += "\t>Level saved: ";
        console.str += name;
        console.str += "\n";
        printf("level saved: %s\n", level_path.data);
    }
    
    if (is_temp_level){
        // console.str += "\t>Temp level saved: ";
        // console.str += name;
        // console.str += "\n";
        printf("Temp level saved: %s\n", level_path.data);
    }
    
    if (is_autosave){
        // console.str += "\t>Autosaved: ";
        // console.str += name;
        // console.str += "\n";
        printf("Temp level saved: %s\n", level_path.data);
    }
    
    
    level_path.free_str();

    
    return 1;
}

inline void save_level_by_name(char *name){
    save_level(name);
}

b32 is_digit_or_minus(char ch){
    return ch == '-' || is_digit(ch);
}

void fill_int_from_string(int *int_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *int_ptr = atoi(str_data);
}    

void fill_int_from_string(u64 *int_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *int_ptr = atoi(str_data);
}    

void fill_b32_from_string(b32 *b32_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *b32_ptr = atoi(str_data);
}    

void fill_float_from_string(float *float_ptr, char *str_data){
    assert(is_digit_or_minus(*str_data));
    *float_ptr = atof(str_data);
}    

void fill_vector2_from_string(Vector2 *vec_ptr, char *x_str, char *y_str){
    assert(is_digit_or_minus(*x_str));
    assert(is_digit_or_minus(*y_str));
    
    vec_ptr->x = atof(x_str);
    vec_ptr->y = atof(y_str);
}

void fill_vector4_from_string(Color *vec_ptr, char *x_str, char *y_str, char *z_str, char *w_str){
    assert(is_digit_or_minus(*x_str));
    assert(is_digit_or_minus(*y_str));
    assert(is_digit_or_minus(*z_str));
    assert(is_digit_or_minus(*w_str));
    
    vec_ptr->r = atof(x_str);
    vec_ptr->g = atof(y_str);
    vec_ptr->b = atof(z_str);
    vec_ptr->a = atof(w_str);
}

void fill_vertices_array_from_string(Array<Vector2, MAX_VERTICES> *vertices, Dynamic_Array<Medium_Str> line_arr, int *index_ptr){
    assert(line_arr.get(*index_ptr + 1).data[0] == '[');
    assert(is_digit_or_minus(line_arr.get(*index_ptr + 2).data[0]));
    
    *index_ptr += 2;
    
    for (; *index_ptr < line_arr.count - 1 && line_arr.get(*index_ptr).data[0] != ']'; *index_ptr += 2){
        Medium_Str current = line_arr.get((*index_ptr));
        Medium_Str next    = line_arr.get((*index_ptr) + 1);
        
        fill_vector2_from_string(vertices->get_ptr(vertices->count), current.data, next.data);
        vertices->count++;
    }
}

void fill_vector2_array_from_string(Dynamic_Array<Vector2> *points, Dynamic_Array<Medium_Str> line_arr, int *index_ptr){
    assert(line_arr.get(*index_ptr + 1).data[0] == '[');
    assert(is_digit_or_minus(line_arr.get(*index_ptr + 2).data[0]));
    
    *index_ptr += 2;
    
    for (; *index_ptr < line_arr.count - 1 && line_arr.get(*index_ptr).data[0] != ']'; *index_ptr += 2){
        Medium_Str current = line_arr.get((*index_ptr));
        Medium_Str next    = line_arr.get((*index_ptr) + 1);
        
        points->add({});
        fill_vector2_from_string(points->last_ptr(), current.data, next.data);
    }
}

void fill_int_array_from_string(Dynamic_Array<int> *arr, Dynamic_Array<Medium_Str> line_arr, int *index_ptr){
    assert(line_arr.get(*index_ptr + 1).data[0] == '[');
    //assert(is_digit_or_minus(line_arr.get(*index_ptr + 2).data[0]));
    
    *index_ptr += 2;
    
    for (; *index_ptr < line_arr.count - 1 && line_arr.get(*index_ptr).data[0] != ']'; *index_ptr += 1){
        Medium_Str current = line_arr.get((*index_ptr));
        //Medium_Str next    = line_arr.get((*index_ptr) + 1);
        int value = -1;
        fill_int_from_string(&value, current.data);  
        arr->add(value);
        //fill_vector2_from_string(arr->get_ptr(arr->count), current.data, next.data);
        //arr->count++;
    }
}

int load_level(const char *level_name){
    game_state = EDITOR;

    char *name;
    name = get_substring_before_symbol(level_name, '.');

    String level_path = init_string();
    level_path += "levels/";
    level_path += name;
    level_path += ".level";
    
    File file = load_file(level_path.data, "r");
    
    if (!file.loaded){
        console.str += "Could not load level: ";
        console.str += name;
        console.str += "\n";
        level_path.free_str();
        return 0;
    }
    
    
    clean_up_scene();
    clear_context(&context);
    setup_particles();
    
    Dynamic_Array<Medium_Str> splitted_line = Dynamic_Array<Medium_Str>(64);
    
    b32 parsing_setup_data = false;
    b32 parsing_entities   = false;
    
    for (int line_index = 0; line_index < file.lines.count; line_index++){
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
        
        for (int i = 0; i < splitted_line.count; i++){
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

            if (str_equal(splitted_line.get(i).data, "name")){
                str_copy(entity_to_fill.name, splitted_line.get(i+1).data);  
                i++;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "id")){
                fill_int_from_string(&entity_to_fill.id, splitted_line.get(i+1).data);
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
                fill_float_from_string(&entity_to_fill.rotation, splitted_line.get(i+1).data);
                i += 1;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "color")){
                fill_vector4_from_string(&entity_to_fill.color, splitted_line.get(i+1).data, splitted_line.get(i+2).data, splitted_line.get(i+3).data, splitted_line.get(i+4).data);
                i += 4;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "flags")){
                fill_int_from_string(&entity_to_fill.flags, splitted_line.get(i+1).data);
                i++;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "vertices")){
                // fill_int_from_string(&entity_to_fill.rotation);
                fill_vertices_array_from_string(&entity_to_fill.vertices, splitted_line, &i);
                // i--;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "unscaled_vertices")){
                // fill_int_from_string(&entity_to_fill.rotation);
                fill_vertices_array_from_string(&entity_to_fill.unscaled_vertices, splitted_line, &i);
                //i--;
                continue;
            } else if (str_equal(splitted_line.get(i).data, "texture_name")){
                str_copy(entity_to_fill.texture_name, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "draw_order")){
                fill_int_from_string(&entity_to_fill.draw_order, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_connected")){
                fill_int_array_from_string(&entity_to_fill.trigger.connected, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "trigger_tracking")){
                fill_int_array_from_string(&entity_to_fill.trigger.tracking, splitted_line, &i);
            } else if (str_equal(splitted_line.get(i).data, "blocker_clockwise")){
                fill_b32_from_string(&entity_to_fill.enemy.blocker_clockwise, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "blocker_immortal")){
                fill_b32_from_string(&entity_to_fill.enemy.blocker_immortal, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "propeller_power")){
                fill_float_from_string(&entity_to_fill.propeller.power, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "sword_kill_speed_modifier")){
                fill_float_from_string(&entity_to_fill.enemy.sword_kill_speed_modifier, splitted_line.get(i+1).data);
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
            } else if (str_equal(splitted_line.get(i).data, "trigger_kill_player")){
                fill_b32_from_string(&entity_to_fill.trigger.kill_player, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_open_doors")){
                fill_b32_from_string(&entity_to_fill.trigger.open_doors, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_track_enemies")){
                fill_b32_from_string(&entity_to_fill.trigger.track_enemies, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_agro_enemies")){
                fill_b32_from_string(&entity_to_fill.trigger.agro_enemies, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "trigger_player_touch")){
                fill_b32_from_string(&entity_to_fill.trigger.player_touch, splitted_line.get(i+1).data);
                i++;
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
            } else if (str_equal(splitted_line.get(i).data, "on_rope")){
                fill_b32_from_string(&entity_to_fill.physics_object.on_rope, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "physics_rotate_by_velocity")){
                fill_b32_from_string(&entity_to_fill.physics_object.rotate_by_velocity, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "physics_gravity_multiplier")){
                fill_float_from_string(&entity_to_fill.physics_object.gravity_multiplier, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "physics_mass")){
                fill_float_from_string(&entity_to_fill.physics_object.mass, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "segments_count")){
                fill_int_from_string(&entity_to_fill.centipede.segments_count, splitted_line.get(i+1).data);
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
                fill_float_from_string(&entity_to_fill.trigger.zoom_value, splitted_line.get(i+1).data);
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
            } else if (str_equal(splitted_line.get(i).data, "hidden")){
                fill_b32_from_string(&entity_to_fill.hidden, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "spawn_enemy_when_no_ammo")){
                fill_b32_from_string(&entity_to_fill.spawn_enemy_when_no_ammo, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_speed")){
                fill_float_from_string(&entity_to_fill.move_sequence.speed, splitted_line.get(i+1).data);
                i++;
            } else if (str_equal(splitted_line.get(i).data, "move_sequence_points")){
                fill_vector2_array_from_string(&entity_to_fill.move_sequence.points, splitted_line, &i);
                //i++;
            } else{
                //assert(false);
                print("Something unknown during level load");
            }
        }
        
        if (parsing_entities){
            if (entity_to_fill.flags & TEXTURE){
                i64 texture_hash = hash_str(entity_to_fill.texture_name);
                assert(textures_table.has_key(texture_hash));
                entity_to_fill.texture = textures_table.get_by_key(texture_hash);
            }
            
            setup_color_changer(&entity_to_fill);
            Entity *added_entity = add_entity(&entity_to_fill, true);
            //rotate_to(added_entity, added_entity->rotation);
            
            calculate_bounds(added_entity);
        }
    }
    
    b32 is_temp_level = str_start_with_const(name, "TEMP_");
    b32 is_autosave   = str_start_with_const(name, "AUTOSAVE_");
    if (!is_temp_level && !is_autosave){
        str_copy(context.current_level_name, name);
        console.str += "\t>Loaded level: ";
        console.str += name;
        console.str += "\n";
    }
    
    //free_string_array(&splitted_line);
    splitted_line.free_arr();
    unload_file(&file);
        
    loop_entities(init_loaded_entity);
    
    level_path.free_str();
    
    if (enter_game_state_on_new_level){
        ForEntities(entity, 0){
            update_editor_entity(entity);
        }

        enter_game_state();
        enter_game_state_on_new_level = false;
        player_data.blood_amount = last_player_data.blood_amount;
        player_data.blood_progress = last_player_data.blood_progress;
        player_data.ammo_count = last_player_data.ammo_count;
    }
    
    editor.last_autosave_time = core.time.app_time;
    context.cam.position = editor.player_spawn_point;
    context.cam.target = editor.player_spawn_point;
    return 1;
}

global_variable Array<Collision, MAX_COLLISIONS> collisions_data = Array<Collision, MAX_COLLISIONS>();

#define MAX_SPAWN_OBJECTS 128

global_variable Array<Spawn_Object, MAX_SPAWN_OBJECTS> spawn_objects = Array<Spawn_Object, MAX_SPAWN_OBJECTS>();

Texture cat_texture;

#define BIRD_ENEMY_COLLISION_FLAGS (GROUND | PLAYER | BIRD_ENEMY)

Entity *spawn_object_by_name(const char* name, Vector2 position){
    for (int i = 0; i < spawn_objects.count; i++){
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
        context.bird_slots[bird->slot_index].occupied = false;
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
    
    init_bird_emitters(entity);
        
    //entity->emitter = entity->emitters.last_ptr();
    str_copy(entity->name, "enemy_bird"); 
    setup_color_changer(entity);
    
    entity->bird_enemy.attack_sound = sounds_table.get_by_key_ptr(hash_str("BirdAttack"));
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
    
    Entity dummy_entity = Entity({0, 0}, {10, 5}, {0.5f, 0.5f}, 0, DUMMY);
    dummy_entity.color  = Fade(GREEN, 0.5f);
    dummy_entity.hidden = true;
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
    
    Entity centipede_entity = Entity({0, 0}, {7, 5}, {0.5f, 0.5f}, 0, CENTIPEDE | MOVE_SEQUENCE);
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
    
    Entity jump_shooter_entity = Entity({0, 0}, {6, 8}, {0.5f, 0.5f}, 0, ENEMY | JUMP_SHOOTER | MOVE_SEQUENCE);
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

void load_textures(){
    FilePathList textures = LoadDirectoryFiles("resources\\textures");
    for (int i = 0; i < textures.count; i++){
        char *name = textures.paths[i];
        
        if (!str_end_with(name, ".png")){
            continue;
        }
        
        Texture texture = LoadTexture(name);
        
        substring_after_line(name, "resources\\textures\\");
        
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
    for (int i = 0; i < context.entities.max_count; i++){
        if (!context.entities.has_index(i)){
            continue;
        }
    
        Entity *e = context.entities.get_ptr(i);
        if (!e->enabled){
            continue;
        }
        
        func(e);
    }
}

void print_to_console(char *text){
    console.str += "\t>";
    console.str += text;
    console.str += "\n";
}

void print_to_console(const char *text){
    console.str += "\t>";
    console.str += text;
    console.str += "\n";
}

inline void init_loaded_entity(Entity *entity){
    if (entity->flags & BIRD_ENEMY){
        init_bird_entity(entity);
    }
}


inline int table_next_avaliable(Hash_Table_Int<Entity> table, int index, FLAGS flags){
    for (int i = index; i < table.max_count; i++){
        if (table.has_index(i) && (flags == 0 || table.get_ptr(i)->flags & flags)){
            return i;
        }
    }
    
    return table.max_count;
}

inline int next_entity_avaliable(int index, Entity **entity, FLAGS flags){
    for (int i = index; i < context.entities.max_count; i++){
        if (context.entities.has_index(i) && (flags == 0 || context.entities.get_ptr(i)->flags & flags)){
            *entity = context.entities.get_ptr(i);
            return i;
        }
    }
    
    *entity = NULL;
    return context.entities.max_count;
}

// inline void assign_texture(Entity *entity, Texture texture, const char *texture_name){
//     entity->texture = texture;
//     str_copy(entity->texture_name, texture_name);
// }

void init_entity(Entity *entity){
    if (entity->flags & ENEMY){
        entity->enemy.original_scale = entity->scale;
        
        entity->enemy.explosion_sound = sounds_table.get_by_key_ptr(hash_str("Explosion"));
        entity->enemy.explosion_sound->base_volume = 0.3f;
        entity->enemy.big_explosion_sound = sounds_table.get_by_key_ptr(hash_str("BigExplosion"));
        entity->enemy.big_explosion_sound->base_volume = 0.5f;
    }
    
    if (entity->flags & BIRD_ENEMY){
        init_bird_entity(entity);
    }

    if (entity->flags & EXPLOSIVE){
        entity->color_changer.change_time = 5.0f;
    }
    
    if (entity->flags & BLOCKER && game_state == GAME){
        // init blocker
        if (entity->enemy.blocker_sticky_id != -1 && context.entities.has_key(entity->enemy.blocker_sticky_id)){
            context.entities.get_by_key_ptr(entity->enemy.blocker_sticky_id)->destroyed = true;
        }
        
        if (!entity->enemy.blocker_immortal){
            Texture texture = entity->enemy.blocker_clockwise ? spiral_clockwise_texture : spiral_counterclockwise_texture;
            Entity *sticky_entity = add_entity(entity->position, {10, 10}, {0.5f, 0.5f}, 0, texture, TEXTURE | STICKY_TEXTURE);
            init_entity(sticky_entity);
            str_copy(sticky_entity->name, "blocker_attack_mark");
            sticky_entity->need_to_save = false;
            //sticky_entity->texture = texture;
            sticky_entity->draw_order = 1;
            sticky_entity->sticky_texture.texture_position = entity->position;
            sticky_entity->sticky_texture.max_lifetime = 0;
            sticky_entity->sticky_texture.line_color = ORANGE;
            sticky_entity->sticky_texture.need_to_follow = true;
            sticky_entity->sticky_texture.follow_id = entity->id;
            sticky_entity->sticky_texture.birth_time = core.time.game_time;
            
            entity->enemy.blocker_sticky_id = sticky_entity->id;
        }
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
        entity->door.closed_position = entity->door.is_open ? entity->position - entity->up * entity->scale.y : entity->position;
        entity->door.open_position   = entity->door.is_open ? entity->position : entity->position + entity->up * entity->scale.y;
        
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
        for (int i = 0; i < centipede->segments_count; i++){
            Entity* segment = spawn_object_by_name("centipede_segment", entity->position);
            segment->centipede_head = entity;
            change_up(segment, entity->up);
            segment->draw_order = entity->draw_order + 1;
            centipede->segments_ids.add(segment->id);
            Entity *previous;
            if (i > 0){
                previous = context.entities.get_by_key_ptr(centipede->segments_ids.get(i-1));
            } else{
                previous = entity;
            }

            segment->position = previous->position - previous->up * previous->scale.y * 1.2f;
            segment->move_sequence = entity->move_sequence;
        }
    }
    
    if (entity->flags & PHYSICS_OBJECT || entity->collision_flags == 0){
        entity->collision_flags |= GROUND | ENEMY | PLAYER;
    }
    
    setup_color_changer(entity);
}

inline void save_current_level(){
    save_level(context.current_level_name);
}

inline void autosave_level(){
    String temp_level_name = init_string();
    temp_level_name += "AUTOSAVE_";
    temp_level_name += context.current_level_name;
    
    save_level(temp_level_name.data);
    temp_level_name.free_str();
}

inline void load_level_by_name(char *name){
    if (load_level(name)){
    } else{
    }
}

Console_Command make_console_command(const char *name, void (func)() = NULL, void (func_arg)(char*) = NULL){
    Console_Command command;
    str_copy(command.name, name);
    command.func = func;
    command.func_arg = func_arg;
    return command;
}

void print_current_level(){
    console.str += "\t>";
    console.str += context.current_level_name;
    console.str += "\n";
}

void create_level(char *level_name){
    String path = init_string();
    path += "levels/";
    char *name;
    name = get_substring_before_symbol(level_name, '.');
    path += name;
    path += ".level";
    
    FILE *fptr = fopen(path.data, "r");
    
    if (fptr != NULL){
         console.str += "this level already exists\n";
    } else{
        clean_up_scene();
        clear_context(&context);
        
        save_level(level_name);
        enter_editor_state();
        console.str += "Level successfuly created";
    }
    
    path.free_str();
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
    for (int i = 0; i < levels.count; i++){
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
    console.str += "\t>Ctrl+Shift+J - Save current level\n";
    console.str += "\t>Alt - See and move vertices\n";
    console.str += "\t>Alt+V - While moving vertex for snap it to closest\n";
    console.str += "\t>Space - Create menu\n";
    console.str += "\t>P - Move player spawn point\n";
    console.str += "\t>Ctrl+Space - Pause in game\n\n";
    console.str += "\t>Shift+Space - Freecam in game\n\n";
    console.str += "\t>Right_Alt+L - Unlock camera\n\n";
    
    console.str += "Commands:\n\t>debug - debug commands info\n";
    console.str += "\t>save <level> - save current level or specify level where to save\n";
    console.str += "\t>level <level> - get current level name or load level if provided\n";
    console.str += "\t>load <level> - load level\n";
    console.str += "\t>create/new_level <level> - create empty level\n";
}

void debug_unlock_camera(){
    context.cam.locked = false;
}

void print_debug_commands_to_console(){
    console.str += "\t\t>Debug Functions:\n";
    console.str += "\t>infinite_ammo\n";
    console.str += "\t>enemy_ai\n";
    console.str += "\t>god_mode\n";
    console.str += "\t>unlock_camera\n";
    
    console.str += "\t\t>Debug Info:\n";
    console.str += "\t>player_speed\n";
    console.str += "\t>entities_count\n";
}

inline void debug_toggle_player_speed(){
    debug.info_player_speed = !debug.info_player_speed;
}

inline void debug_print_entities_count(){
    i32 count = 0;
    ForTable(context.entities, i){
        count++;
    }
    console.str += TextFormat("\t>Entities count: %d\n", count);
}

inline void debug_infinite_ammo(){
    debug.infinite_ammo = !debug.infinite_ammo;
    console.str += TextFormat("\t>Infinite ammo %s\n", debug.infinite_ammo ? "enabled" : "disabled");
}

inline void debug_enemy_ai(){
    debug.enemy_ai = !debug.enemy_ai;
    console.str += TextFormat("\t>Enemy ai %s\n", debug.enemy_ai ? "enabled" : "disabled");
}

inline void debug_god_mode(){
    debug.god_mode = !debug.god_mode;
    console.str += TextFormat("\t>God mode %s\n", debug.god_mode ? "enabled" : "disabled");
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
    console.commands.add(make_console_command("enemy_ai",       debug_enemy_ai));
    console.commands.add(make_console_command("god_mode",       debug_god_mode));
    console.commands.add(make_console_command("unlock_camera",  debug_unlock_camera));
    
    console.commands.add(make_console_command("save",    save_current_level, save_level_by_name));
    console.commands.add(make_console_command("load",    NULL, load_level_by_name));
    console.commands.add(make_console_command("level",   print_current_level, load_level_by_name));
    
    console.commands.add(make_console_command("create",    print_create_level_hint, create_level));
    console.commands.add(make_console_command("new_level", print_create_level_hint, create_level));
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
    for (int i = 0; i < sounds.count; i++){
        char *name = sounds.paths[i];
        
        if (!str_end_with(name, ".ogg") && !str_end_with(name, ".wav")){
            continue;
        }
        
        Sound sound = LoadSound(name);
        substring_after_line(name, "resources\\audio\\");
        name = get_substring_before_symbol(name, '.');
        
        Sound_Handler handler;
        
        for (int s = 0; s < handler.buffer.max_count; s++){
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
    Vector2 to_position = position - context.cam.position;
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
    play_sound(name, context.cam.position, volume_multiplier);
}

void init_game(){
    HideCursor();
    DisableCursor();
    
    game_state = EDITOR;

    context = {};    
    render = {};
    str_copy(context.current_level_name, "test_level");
    
    render.main_render_texture = LoadRenderTexture(screen_width, screen_height);
    render.test_shader = LoadShader(0, "../test_shader.fs");

    input = {};
    init_console();

    current_level = {};
    //current_level.context = (Context*)malloc(sizeof(Context));
    //context = *current_level.context;
    
    init_spawn_objects();
    load_textures();
    load_sounds();
    
    //mouse_entity = add_entity(input.mouse_position, {1, 1}, {0.5f, 0.5f}, 0, -1);
    mouse_entity = Entity(input.mouse_position, {1, 1}, {0.5f, 0.5f}, 0, 0);
    
    load_level(context.current_level_name);
    
    // ForTable(context.entities, i){
    //     init_entity(context.entities.get_ptr(i));
    // }
    
    Context *c = &context;
    
    c->unit_screen_size = {screen_width / UNIT_SIZE, screen_height / UNIT_SIZE};
    
    c->cam.position = Vector2_zero;
    c->cam.cam2D.target = world_to_screen({0, 0});
    c->cam.cam2D.offset = (Vector2){ screen_width/2.0f, (f32)screen_height * 0.5f };
    c->cam.cam2D.rotation = 0.0f;
    c->cam.cam2D.zoom = 0.4f;
}

void destroy_player(){
    assert(player_entity);

    player_entity->destroyed                 = true;
    
    assert(context.entities.has_key(player_data.ground_checker_id));
    context.entities.get_by_key_ptr(player_data.ground_checker_id)->destroyed = true;
    assert(context.entities.has_key(player_data.sword_entity_id));
    context.entities.get_by_key_ptr(player_data.sword_entity_id)->destroyed = true;
    
    player_entity = NULL;
}

void clean_up_scene(){
    ForTable(context.entities, i){
        Entity *e = context.entities.get_ptr(i);
        e->color = e->color_changer.start_color;
    }
    
    for (int i = 0; i < MAX_BIRD_POSITIONS; i++){
        context.bird_slots[i].occupied = false;
    }

    context.shoot_stopers_count = 0;
    context.last_bird_attack_time = -11111;
    context.cam.locked = false;
    
    assign_selected_entity(NULL);
    editor.in_editor_time = 0;
    close_create_box();
    
    if (player_entity){
        destroy_player();
        player_data.dead_man = false;
    }
}

void enter_game_state(){
    game_state = GAME;
    core.time.game_time = 0;
    
    HideCursor();
    DisableCursor();
    
    clean_up_scene();
    
    ForTable(context.entities, i){
        init_entity(context.entities.get_ptr(i));
    }
    
    context.cam.cam2D.zoom = 0.35f;
    context.cam.target_zoom = 0.35f;
    
    String temp_level_name = init_string();
    temp_level_name += "TEMP_";
    temp_level_name += context.current_level_name;
    
    save_level(temp_level_name.data);
    temp_level_name.free_str();
    
    //copy_context(&saved_level_context, &context);
    
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
    sword_entity->color.a = 255;
    sword_entity->color_changer.start_color = sword_entity->color;
    sword_entity->color_changer.target_color = RED * 0.99f;
    sword_entity->color_changer.interpolating = true;
    sword_entity->draw_order = 25;
    str_copy(sword_entity->name, "Player_Sword");
    
    //sword_entity->index = sword_entity->id % MAX_ENTITIES;
    
    player_data.ground_checker_id = ground_checker->id;
    player_data.left_wall_checker_id = left_wall_checker->id;
    player_data.right_wall_checker_id = right_wall_checker->id;
    player_data.sword_entity_id = sword_entity->id;
    player_data.dead_man = false;
    
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
}

void kill_player(){
    if (debug.god_mode && !context.we_got_a_winner || player_data.dead_man){ 
        return;
    }
    
    death_player_data = player_data;

    emit_particles(big_blood_emitter, player_entity->position, player_entity->up, 1, 1);
    player_data.dead_man = true;
    play_sound(player_data.player_death_sound, player_entity->position);
}

void enter_editor_state(){
    game_state = EDITOR;
    
    // ShowCursor();
    // EnableCursor();
    
    // SetMousePosition(mouse_position.x, mouse_position.y);
    
    String temp_level_name = init_string();
    temp_level_name += "TEMP_";
    temp_level_name += context.current_level_name;
    
    load_level(temp_level_name.data);
    temp_level_name.free_str();
    
    SetMusicVolume(tires_theme, 0);
    SetMusicVolume(wind_theme, 0);

    ForEntities(entity, 0){
        update_editor_entity(entity);
    }
    //copy_context(&context, &saved_level_context);
}

Vector2 screen_to_world(Vector2 pos){
    f32 zoom = context.cam.cam2D.zoom;

    f32 width = screen_width   ;
    f32 height = screen_height ;

    Vector2 screen_pos = pos;
    Vector2 world_pos = {(screen_pos.x - width * 0.5f) / UNIT_SIZE, (height * 0.5f - screen_pos.y) / UNIT_SIZE};
    world_pos /= zoom;
    world_pos = world_pos + context.cam.position;// + ( (Vector2){0, -context.unit_screen_size.y * 0.5f});
    
    return world_pos;
}

Vector2 game_mouse_pos(){
    return screen_to_world(mouse_position);
}

void fixed_game_update(f32 dt){
    update_entities(dt);
    input.press_flags = 0;
    input.sum_mouse_delta = Vector2_zero;
    input.sum_mouse_wheel = 0;
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
            make_next_input_field_in_focus();
        }
    }
            
    if (console.is_open && can_control_console){
        f32 time_since_open = core.time.app_time - console.opened_time;
        console.open_progress = clamp01(time_since_open / 0.3f);
        
        Color color = lerp(WHITE * 0, GRAY, console.open_progress * console.open_progress);
        
        console.args.clear();
        split_str(focus_input_field.content, " ", &console.args);
        
        b32 content_changed = false;
        for (int i = 0; i < console.commands.count && console.args.count == 1; i++){
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
            for (int i = 0; i < console.level_files.count; i++){
                if (str_contains(console.level_files.get(i).data, console.args.get(1).data)){
                    String new_console_content = init_string();
                    new_console_content += console.args.get(0).data;
                    new_console_content += " ";
                    new_console_content += console.level_files.get(i).data;
                    
                    make_ui_text(new_console_content.data, {3.0f, (f32)screen_height * 0.5f + focus_input_field.font_size}, focus_input_field.font_size, color * 0.7f, "console_hint_text");
                    
                    if (IsKeyPressed(KEY_TAB)){
                        set_focus_input_field(new_console_content.data);
                        content_changed = true;
                    }
                    new_console_content.free_str();
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
            
            for (int i = 0; i < console.commands.count && console.args.count > 0; i++){
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

void update_game(){
    frame_rnd = rnd01();
    frame_on_circle_rnd = rnd_on_circle();
    
    //update input
    input.mouse_delta = GetMouseDelta();
    mouse_position += input.mouse_delta;
    input.sum_mouse_delta += input.mouse_delta;
    clamp(&mouse_position.x, 0, screen_width);
    clamp(&mouse_position.y, 0, screen_height);
    
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
    
    //end update input
    
    if (screen_size_changed){
        context.unit_screen_size = {screen_width / UNIT_SIZE, screen_height / UNIT_SIZE};
        context.cam.cam2D.target = (Vector2){ screen_width/2.0f, screen_height/2.0f };
        context.cam.cam2D.offset = (Vector2){ screen_width/2.0f, screen_height/2.0f };
        
        UnloadRenderTexture(render.main_render_texture);
        
        render.main_render_texture = LoadRenderTexture(screen_width, screen_height);
        // UnloadRenderTexture(context.up_render_target);
        // UnloadRenderTexture(context.down_render_target);
        // UnloadRenderTexture(context.up_render_target);
        // UnloadRenderTexture(render.ray_collision_render_texture);
        
        // render.ray_collision_render_texture = LoadRenderTexture(screen_width, screen_height);
        // context.up_render_target = LoadRenderTexture(context.up_screen_size.x, context.up_screen_size.y);
        // context.down_render_target = LoadRenderTexture(context.down_screen_size.x, context.down_screen_size.y);
        // context.right_render_target = LoadRenderTexture(context.right_screen_size.x, context.right_screen_size.y);
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_SPACE)){
        if (game_state == EDITOR){
            enter_game_state();
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
    
    if (player_data.dead_man && game_state == GAME){
        if (IsKeyPressed(KEY_T)){
            enter_editor_state();
            enter_game_state();
        }
    }
    
    core.time.app_time += GetFrameTime();
    core.time.real_dt = GetFrameTime();
    
    if (game_state == GAME){
        core.time.unscaled_dt = GetFrameTime();
        if (core.time.hitstop > 0){
            core.time.time_scale = 0.1f;
            core.time.hitstop -= core.time.real_dt;
        } else{
            core.time.time_scale = 1;
        }
        
        core.time.dt = GetFrameTime() * core.time.time_scale;
        
        core.time.game_time += core.time.dt;
    } else if (game_state == EDITOR || game_state == PAUSE){
        core.time.unscaled_dt = 0;
        core.time.dt          = 0;
    }

    
    update_input_field();
    update_console();
    
    if (game_state == EDITOR || game_state == PAUSE){
        update_editor_ui();
        update_editor();
    }
    
    //zoom_entity->text_drawer.text = TextFormat("%f", context.cam.cam2D.zoom);
    
    if (game_state == GAME){
        float full_delta = core.time.dt + core.time.previous_dt;
        core.time.previous_dt = 0;
        
        full_delta = Clamp(full_delta, 0, 0.5f);
        b32 updated_today = false;
        while (full_delta >= TARGET_FRAME_TIME){
            core.time.fixed_dt = TARGET_FRAME_TIME;
            fixed_game_update(core.time.fixed_dt);
            updated_today = true;
            full_delta -= TARGET_FRAME_TIME;
        }
        
        if (updated_today){
            input.hold_flags = 0;
            input.sum_direction = Vector2_zero;
        }
        
        core.time.previous_dt = full_delta;
    } else{
        fixed_game_update(core.time.real_dt);
    }
    
    // update_entities();
    update_emitters();
    update_particles();
    
    if (IsKeyPressed(KEY_SPACE) && IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL)){
        debug.free_cam = !debug.free_cam;
        if (!debug.free_cam){
            context.cam.target_zoom = debug.last_zoom;
        } else{
            debug.last_zoom = context.cam.target_zoom;
        }
    }
    
    if (IsKeyPressed(KEY_L) && IsKeyDown(KEY_RIGHT_ALT)){
        debug_unlock_camera();
    }
    
    if (game_state == GAME && player_entity && !debug.free_cam){
        // camera logic
        if (!context.cam.locked){
            Vector2 player_velocity = player_data.velocity;
            f32 target_speed_multiplier = 1;
        
            f32 time_since_heavy_collision = core.time.game_time - player_data.heavy_collision_time;
            if (magnitude(player_data.velocity) < 80 && core.time.game_time > 5 && time_since_heavy_collision <= 1.0f){
                player_velocity = player_data.heavy_collision_velocity;
                target_speed_multiplier = 0.05f;
            }
            
            f32 player_speed = magnitude(player_velocity);
        
            Vector2 target_position = player_entity->position + Vector2_up * 10 + player_velocity * 0.25f;
            
            Vector2 vec_to_target = target_position - context.cam.target;
            Vector2 vec_to_player = player_entity->position - context.cam.target;
            
            f32 target_dot = dot(vec_to_target, vec_to_player);
            
            f32 speed_t = clamp01(player_speed / 200.0f);
            
            f32 target_speed = lerp(3, 10, speed_t * speed_t);
            target_speed *= target_speed_multiplier;
            
            context.cam.target = lerp(context.cam.target, target_position, clamp01(core.time.dt * target_speed));
            
            f32 cam_speed = lerp(10.0f, 100.0f, speed_t * speed_t);
            
            context.cam.position = lerp(context.cam.position, context.cam.target, clamp01(core.time.dt * cam_speed));
            //context.cam.position = move_by_velocity(context.cam.position, target_position, &context.cam.move_settings, clamp01(core.time.dt));
        } else{
            context.cam.position = lerp(context.cam.position, context.cam.target, clamp01(core.time.dt * 4));
            if (magnitude(context.cam.target - context.cam.position) <= EPSILON){
                context.cam.position = context.cam.target;
            }
        }
    } else{
        f32 zoom = context.cam.target_zoom;

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            context.cam.position += ((Vector2){-input.mouse_delta.x / zoom, input.mouse_delta.y / zoom}) / (UNIT_SIZE);
        }
        if (input.mouse_wheel != 0 && !console.is_open && !editor.create_box_active){
            if (input.mouse_wheel > 0 && zoom < 5 || input.mouse_wheel < 0 && zoom > 0.1f){
                context.cam.target_zoom += input.mouse_wheel * 0.05f;
            }
        }
    }
    
    if (editor.update_cam_view_position){
        context.cam.view_position = context.cam.position;
    }
    
    draw_game();
    
    UpdateMusicStream(ambient_theme);
    UpdateMusicStream(wind_theme);
    UpdateMusicStream(tires_theme);
    // ForTable(sounds_table, i){
    //     Sound_Handler *handler = sounds_table.get_by_key_ptr(i);
        
    //     for (int h = 0; h < handler->buffer.count; h++){
            
    //     }
    // }
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
    } else if (changer->interpolating) {
        entity->color = lerp(changer->start_color, changer->target_color, changer->progress);
    }
    
    if (entity->flags & BLOCKER){
        entity->color = ColorTint(entity->color, RAYWHITE);
        //entity->color = ColorBrightness(entity->color, 1.1f);
    }
}

b32 check_col_point_rec(Vector2 point, Entity *e){
    Vector2 l_u = get_left_up_no_rot(e);
    Vector2 r_d = get_right_down_no_rot(e);

    return ((point.x >= l_u.x) && (point.x <= r_d.x) && (point.y >= r_d.y) && (point.y <= l_u.y));
}

b32 check_col_circles(Circle a, Circle b){
    f32 distance = sqr_magnitude(a.position - b.position);
    
    return distance < a.radius * a.radius + b.radius * b.radius;
}

Vector2 get_rotated_vector_90(Vector2 v, f32 clockwise){
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

b32 check_rectangles_collision(Vector2 pos1, Vector2 scale1, Vector2 pos2, Vector2 scale2){
    b32 solution = pos1.x + scale1.x * 0.5f > pos2.x - scale2.x * 0.5f &&
                   pos1.x - scale1.x * 0.5f < pos2.x + scale2.x * 0.5f &&
                   pos1.y + scale1.y * 0.5f > pos2.y - scale2.y * 0.5f &&
                   pos1.y - scale1.y * 0.5f < pos2.y + scale2.y * 0.5f;
                   
    return solution;
}

// inline b32 check_bounds_collision(Vector2 position1, Bounds bounds1, Vector2 position2, Bounds bounds2){
//     return check_rectangles_collision(position1 + bounds1.offset, bounds1.size, position2 + bounds2.offset, bounds2.size);
// }

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
    // Vector2 position2 = entity2->position + ((Vector2){(entity2->pivot.x - 0.5f) * entity2->scale.x, (entity2->pivot.y - 0.5f) * entity2->scale.y});
    Vector2 pivot_add2 = multiply(entity2->pivot, entity2->bounds.size);
    Vector2 position2 = entity2->position + entity2->bounds.offset;
    //firstly for left up
    Vector2 with_pivot_pos2 = {position2.x - pivot_add2.x, position2.y + pivot_add2.y};
    Vector2 final2 = {with_pivot_pos2.x + entity2->bounds.size.x * 0.5f, with_pivot_pos2.y - entity2->bounds.size.y * 0.5f};
    
    return check_rectangles_collision(position1 + bounds1.offset, bounds1.size, final2, entity2->bounds.size);
}

Collision check_collision(Vector2 position1, Vector2 position2, Array<Vector2, MAX_VERTICES> vertices1, Array<Vector2, MAX_VERTICES> vertices2, Vector2 pivot1 = {0.5f, 0.5f}, Vector2 pivot2 = {0.5f, 0.5f}){
    Collision result = {};
    // result.other_entity = entity2;
    
    Bounds bounds1 = get_bounds(vertices1, pivot1);
    Bounds bounds2 = get_bounds(vertices2, pivot2);
    if (!check_bounds_collision(position1, position2, bounds1, bounds2, pivot1, pivot2)){
        return result;
    }

    //Array<Vector2> normals = Array<Vector2>(entity1->vertices.count + entity2->vertices.count);
    global_normals.count = 0;
    fill_arr_with_normals(&global_normals, vertices1);
    fill_arr_with_normals(&global_normals, vertices2);
    
    f32 overlap = INFINITY;
    Vector2 min_overlap_axis = Vector2_zero;
    
    Vector2 min_overlap_projection = {};

    for (int i = 0; i < global_normals.count; i++){
        Vector2 projections[2];
        //x - min, y - max
        projections[0].x =  INFINITY;
        projections[1].x =  INFINITY;
        projections[0].y = -INFINITY;
        projections[1].y = -INFINITY;
        
        Vector2 axis = global_normals.get(i);

        for (int shape = 0; shape < 2; shape++){
            Array<Vector2, MAX_VERTICES> vertices;
            // Entity *entity;
            Vector2 position;
            if (shape == 0) {
                vertices = vertices1;
                position = position1;
            } else{
                vertices = vertices2;
                position = position2;
            }
            
            for (int j = 0; j < vertices.count; j++){            
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
            //normals.free_arr();
            return result;
        }
    }
    
    Vector2 vec_to_first = position1 - position2;
    
    result.collided = true;
    //if (entity1->flags > 0){
        result.overlap = overlap;
        //result.normal = dir_to_first;
        result.dir_to_first = normalized(vec_to_first);
        result.normal = dot(result.dir_to_first, min_overlap_axis) > 0 ? min_overlap_axis : min_overlap_axis * -1.0f;
        //result.point = entity1->position - dir_to_first * overlap;
        result.point = position1 - result.normal * ((min_overlap_projection.y - min_overlap_projection.x) * 0.5f);
    //}
    
    //normals.free_arr();
    return result;
}

Collision check_entities_collision(Entity *entity1, Entity *entity2){
    Collision result = check_collision(entity1->position, entity2->position, entity1->vertices, entity2->vertices, entity1->pivot, entity2->pivot);
    result.other_entity = entity2;
    
    return result;
}

void resolve_collision(Entity *entity, Collision col){
    if (!col.collided){
        return;
    }

    //entity->position += col.dir_to_first * col.overlap;
    entity->position += col.normal * col.overlap;
}

void fill_collisions(Entity *entity, Array<Collision, MAX_COLLISIONS> *result, FLAGS include_flags){
    result->count = 0;

    if (entity->destroyed || !entity->enabled){
        return;
    }
    
    for (int i = 0; i < context.entities.max_count; i++){
        if (!context.entities.has_index(i)){
            continue;
        }
    
        Entity *other = context.entities.get_ptr(i);
        
        if (other->destroyed || !other->enabled || other == entity || other->flags <= 0 || (other->flags & include_flags) <= 0 || (other->hidden && game_state == GAME)){
            continue;
        }
        
        Collision col = check_entities_collision(entity, other);
        
        if (col.collided){
            result->add(col);
        }
    }
}

Entity *get_entity_by_index(i32 index){
    if (!context.entities.has_index(index)){
        //log error
        print("Attempt to get empty entity by index");
        return NULL;
    }
    
    return context.entities.get_ptr(index);
}

Entity *get_entity_by_id(i32 id){
    if (id == -1 || !context.entities.has_key(id)){
        return NULL;
    }
    
    return context.entities.get_by_key_ptr(id);
}

Collision raycast(Vector2 start_position, Vector2 direction, f32 len, FLAGS include_flags, f32 step = 4, i32 my_id = -1){
    f32 current_len = 0;
    Array<Vector2, MAX_VERTICES> ray_vertices = Array<Vector2, MAX_VERTICES>();
    
    b32 found = false;
    Collision result = {};
    while (current_len < len){
        current_len += step;
        Vector2 east_direction = get_rotated_vector_90(direction, -1);
        ray_vertices.clear();
        ray_vertices.add(direction * current_len + east_direction * 0.5f);
        ray_vertices.add(direction * current_len - east_direction * 0.5f);
        ray_vertices.add(east_direction * 0.5f);
        ray_vertices.add(east_direction * -0.5f);
    
        ForEntities(entity, include_flags){
            if (my_id != -1 && my_id == entity->id){
                continue;
            }
            result = check_collision(start_position, entity->position, ray_vertices, entity->vertices, {0.5f, 1.0f}, entity->pivot);
            if (result.collided){
                result.other_entity = entity;
                found = true;
                result.point = start_position + direction * current_len - direction * result.overlap;
                break;
            }
        }
        
        if (found){
            break;
        }
    }
    return result;
}

void assign_moving_vertex_entity(Entity *e, int vertex_index){
    Vector2 *vertex = e->vertices.get_ptr(vertex_index);

    assign_selected_entity(e);
    editor.moving_vertex = vertex;
    editor.moving_vertex_index = vertex_index;
    editor.moving_vertex_entity = e;
    editor.moving_vertex_entity_id = e->id;
    
    editor.dragging_entity = NULL;
}

void move_vertex(Entity *entity, Vector2 target_position, int vertex_index){
    Vector2 *vertex = entity->vertices.get_ptr(vertex_index);
    Vector2 *unscaled_vertex = entity->unscaled_vertices.get_ptr(vertex_index);
    
    Vector2 local_target = local(entity, target_position);

    // Vector2 displacement = local_target - *vertex;

    // Vector2 unscaled_displacement = {displacement.x / (entity->scale.x ), displacement.y / (entity->scale.y )};
    
    *vertex = local_target;
    //Vector2 modified_unscaled = *unscaled_vertex + unscaled_displacement;
    //*unscaled_vertex = normalized(*vertex) * magnitude(modified_unscaled);
    //*unscaled_vertex = modified_unscaled;
    *unscaled_vertex = {vertex->x / entity->scale.x, vertex->y / entity->scale.y};
    // *unscaled_vertex = *vertex / magnitude(entity->scale);
    
    calculate_bounds(entity);
}

void validate_editor_pointers(){
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

void editor_delete_entity(int entity_id, b32 add_undo){
    assert(context.entities.has_key(entity_id));
    editor_delete_entity(context.entities.get_by_key_ptr(entity_id), add_undo);
}

void undo_apply_vertices_change(Entity *entity, Undo_Action *undo_action){
    for (int i = 0; i < entity->vertices.count; i++){
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
    for (int i = 0; i < entity->vertices.count; i++){
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

void update_editor_ui(){
    //inspector logic
    Entity *selected = editor.selected_entity;
    if (selected){
        Vector2 inspector_size = {screen_width * 0.2f, screen_height * 0.6f};
        Vector2 inspector_position = {screen_width - inspector_size.x - inspector_size.x * 0.1f, 0 + inspector_size.y * 0.05f};
        make_ui_image(inspector_position, inspector_size, {0, 0}, SKYBLUE * 0.7f, "inspector_window");
        f32 height_add = 30;
        f32 v_pos = inspector_position.y + height_add + 40;
        
        make_ui_text(TextFormat("ID: %d", selected->id), {inspector_position.x + 100, inspector_position.y - 10}, 18, WHITE, "inspector_id"); 
        
        make_ui_text(TextFormat("Name: ", selected->id), {inspector_position.x, inspector_position.y + 10}, 24, BLACK, "inspector_id"); 
        if (make_input_field(TextFormat("%s", selected->name), {inspector_position.x + 65, inspector_position.y + 10}, {200, 25}, "inspector_name") ){
            str_copy(selected->name, focus_input_field.content);
        }
        
        make_ui_text("POSITION", {inspector_position.x + 100, inspector_position.y + 40}, 24, WHITE * 0.9f, "inspector_pos");
        make_ui_text("X:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_pos_x");
        make_ui_text("Y:", {inspector_position.x + 5 + 35 + 100, v_pos}, 22, BLACK * 0.9f, "inspector_pos_y");
        if (make_input_field(TextFormat("%.3f", selected->position.x), {inspector_position.x + 30, v_pos}, {100, 25}, "inspector_pos_x")
            || make_input_field(TextFormat("%.3f", selected->position.y), {inspector_position.x + 30 + 100 + 35, v_pos}, {100, 25}, "inspector_pos_y")
            ){
            Vector2 old_position = selected->position;
            if (str_equal(focus_input_field.tag, "inspector_pos_x")){
                selected->position.x = atof(focus_input_field.content);
            } else if (str_equal(focus_input_field.tag, "inspector_pos_y")){
                selected->position.y = atof(focus_input_field.content);
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
        if (make_input_field(TextFormat("%.3f", editor.selected_entity->scale.x), {inspector_position.x + 30, v_pos}, {100, 25}, "inspector_scale_x")
            || make_input_field(TextFormat("%.3f", editor.selected_entity->scale.y), {inspector_position.x + 30 + 100 + 35, v_pos}, {100, 25}, "inspector_scale_y")
            ){
            Vector2 old_scale = editor.selected_entity->scale;
            Vector2 new_scale = old_scale;
            undo_remember_vertices_start(editor.selected_entity);
            
            if (str_equal(focus_input_field.tag, "inspector_scale_x")){
                new_scale.x = atof(focus_input_field.content);
            } else if (str_equal(focus_input_field.tag, "inspector_scale_y")){
                new_scale.y = atof(focus_input_field.content);
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
        if (make_input_field(TextFormat("%.2f", editor.selected_entity->rotation), {inspector_position.x + 150, v_pos}, {75, 25}, "inspector_rotation")
            ){
            f32 old_rotation = editor.selected_entity->rotation;
            f32 new_rotation = old_rotation;
            
            undo_remember_vertices_start(editor.selected_entity);
            
            if (str_equal(focus_input_field.tag, "inspector_rotation")){
                new_rotation = atof(focus_input_field.content);
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
        if (make_input_field(TextFormat("%d", editor.selected_entity->draw_order), {inspector_position.x + 150, v_pos}, {75, 25}, "inspector_draw_order")
            ){
            i32 old_draw_order = editor.selected_entity->draw_order;
            i32 new_draw_order = old_draw_order;
            
            //undo_remember_vertices_start(editor.selected_entity);
            
            if (str_equal(focus_input_field.tag, "inspector_draw_order")){
                new_draw_order = atoi(focus_input_field.content);
            } else{
                assert(false);
            }
            
            i32 draw_order_add = new_draw_order - old_draw_order;
            if (draw_order_add != 0){
                //rotate(editor.selected_entity, draw_order_add);
                editor.selected_entity->draw_order += draw_order_add;
            }
            
            undo_add_draw_order(editor.selected_entity, draw_order_add);
        }
        v_pos += height_add;
        
        height_add = 16;
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
            if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->hidden, "toggle_entity_hidden")){
                selected->hidden = !selected->hidden;
            }
            v_pos += height_add;
            
            make_ui_text("Spawn enemy when no ammo: ", {inspector_position.x + 5, v_pos}, "text_spawn_no_ammo");
            if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->spawn_enemy_when_no_ammo, "toggle_spawn_no_ammo")){
                selected->spawn_enemy_when_no_ammo = !selected->spawn_enemy_when_no_ammo;
            }
            v_pos += height_add;
            
            make_ui_text("Physics object: ", {inspector_position.x + 5, v_pos}, "physics_object");
            if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->flags & PHYSICS_OBJECT, "physics_object")){
                selected->flags ^= PHYSICS_OBJECT;
            }
            v_pos += height_add;
            
            if (selected->flags & PHYSICS_OBJECT){
                make_ui_text("On rope: ", {inspector_position.x + 5, v_pos}, "on_rope");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->physics_object.on_rope, "on_rope")){
                    selected->physics_object.on_rope = !selected->physics_object.on_rope;
                }
                v_pos += height_add;
                
                make_ui_text("Rotate by velocity: ", {inspector_position.x + 5, v_pos}, "physics_rotate_by_velocity");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->physics_object.rotate_by_velocity, "physics_rotate_by_velocity")){
                    selected->physics_object.rotate_by_velocity = !selected->physics_object.rotate_by_velocity;
                }
                v_pos += height_add;
                
                make_ui_text("Gravity multiplier: ", {inspector_position.x + 5, v_pos}, "physics_gravity_multiplier");
                if (make_input_field(TextFormat("%.1f", selected->physics_object.gravity_multiplier), {inspector_position.x + 250, v_pos}, {100, 25}, "physics_gravity_multiplier")){
                    selected->physics_object.gravity_multiplier = atof(focus_input_field.content);
                }
                v_pos += height_add;
                
                make_ui_text("Mass: ", {inspector_position.x + 5, v_pos}, "physics_mass");
                if (make_input_field(TextFormat("%.1f", selected->physics_object.mass), {inspector_position.x + 250, v_pos}, {100, 25}, "physics_mass")){
                    selected->physics_object.mass = clamp(atof(focus_input_field.content), 0.01f, 100000.0f);
                }
                v_pos += height_add;
            }
            
            make_ui_text("Move sequence: ", {inspector_position.x + 5, v_pos}, "text_entity_move_sequence");
            if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->flags & MOVE_SEQUENCE, "toggle_entity_move_sequence")){
                selected->flags ^= MOVE_SEQUENCE;
            }
            v_pos += height_add;
            
            if (selected->flags & MOVE_SEQUENCE){
                make_ui_text("Moving: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_moving");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->move_sequence.moving, "toggle_entity_move_sequence_moving")){
                    selected->move_sequence.moving = !selected->move_sequence.moving;
                }
                v_pos += height_add;
                make_ui_text("Loop: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_loop");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->move_sequence.loop, "toggle_entity_move_sequence_loop")){
                    selected->move_sequence.loop = !selected->move_sequence.loop;
                }
                v_pos += height_add;
                
                make_ui_text("Rotate: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_rotate");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->move_sequence.rotate, "toggle_entity_move_sequence_rotate")){
                    selected->move_sequence.rotate = !selected->move_sequence.rotate;
                }
                v_pos += height_add;
            
                make_ui_text("Speed: ", {inspector_position.x + 25, v_pos}, "text_move_sequence_speed");
                if (make_input_field(TextFormat("%.2f", selected->move_sequence.speed), {inspector_position.x + 100, v_pos}, 100, "move_sequence_speed")){
                    selected->move_sequence.speed = atof(focus_input_field.content);
                }
                v_pos += height_add;
                
                make_ui_text(TextFormat("Points count: %d", selected->move_sequence.points.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_count");
                type_info_v_pos += type_font_size;
                make_ui_text("Alt+C clear points", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_clear");
                type_info_v_pos += type_font_size;
                make_ui_text("Alt+M Remove point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_remove");
                type_info_v_pos += type_font_size;
                make_ui_text("Alt+N Add point", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "move_sequence_add_point");
                type_info_v_pos += type_font_size;
                
                make_ui_text("Move sequence settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "move_sequence_settings");
            type_info_v_pos += type_font_size;

            }
        }
        
        if (selected->flags & TRIGGER){
            if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Trigger settings", "trigger_settings")){
                editor.draw_trigger_settings = !editor.draw_trigger_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_trigger_settings){
                make_ui_text("Activate on player: ", {inspector_position.x + 5, v_pos}, "text_player_touch");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.player_touch, "toggle_player_touch")){
                    selected->trigger.player_touch = !selected->trigger.player_touch;
                }
                v_pos += height_add;
                
                make_ui_text("Kill player: ", {inspector_position.x + 5, v_pos}, "text_kill_player");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.kill_player, "toggle_kill_player")){
                    selected->trigger.kill_player = !selected->trigger.kill_player;
                }
                v_pos += height_add;
                
                make_ui_text("Open or close Doors: ", {inspector_position.x + 5, v_pos}, "trigger_open_doors");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.open_doors, "toggle_open_doors")){
                    selected->trigger.open_doors = !selected->trigger.open_doors;                 
                }
                v_pos += height_add;
                
                make_ui_text("Track enemies: ", {inspector_position.x + 5, v_pos}, "trigger_track_enemies");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.track_enemies, "toggle_track_enemies")){
                    selected->trigger.track_enemies = !selected->trigger.track_enemies;                 
                }
                v_pos += height_add;
                
                make_ui_text("Agro enemies: ", {inspector_position.x + 5, v_pos}, "text_trigger_agro_enemies");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.agro_enemies, "toggle_agro_enemies")){
                    selected->trigger.agro_enemies = !selected->trigger.agro_enemies;                 
                }
                v_pos += height_add;
                
                make_ui_text("Shows entities: ", {inspector_position.x + 5, v_pos}, "trigger_shows_entities");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.shows_entities, "toggle_trigger_show_entity")){
                    selected->trigger.shows_entities = !selected->trigger.shows_entities;                 
                }
                v_pos += height_add;
                
                make_ui_text("Starts moving sequence: ", {inspector_position.x + 5, v_pos}, "trigger_starts_moving_sequence");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.starts_moving_sequence, "toggle_starts_moving_sequence")){
                    selected->trigger.starts_moving_sequence = !selected->trigger.starts_moving_sequence;                 
                }
                v_pos += height_add;
                
                make_ui_text("Change zoom: ", {inspector_position.x + 5, v_pos}, "trigger_change_zoom_text");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.change_zoom, "toggle_change_zoom")){
                    selected->trigger.change_zoom = !selected->trigger.change_zoom;                 
                }
                v_pos += height_add;
                if (selected->trigger.change_zoom){
                    make_ui_text("Zoom: ", {inspector_position.x + 5, v_pos}, "trigger_change_zooom_text");
                    if (make_input_field(TextFormat("%.2f", selected->trigger.zoom_value), {inspector_position.x + 100, v_pos}, {150, 20}, "trigger_zoom_name") ){
                        selected->trigger.zoom_value = atof(focus_input_field.content);
                    }
                    v_pos += height_add;
                }
                
                make_ui_text("Lock camera: ", {inspector_position.x + 5, v_pos}, "lock_camera_text");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.lock_camera, "lock_camera")){
                    selected->trigger.lock_camera = !selected->trigger.lock_camera;                 
                    if (selected->trigger.lock_camera && selected->trigger.locked_camera_position == Vector2_zero){
                        selected->trigger.locked_camera_position = selected->position;
                    }
                }
                v_pos += height_add;
                
                make_ui_text("Unlock camera: ", {inspector_position.x + 5, v_pos}, "unlock_camera_text");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.unlock_camera, "unlock_camera")){
                    selected->trigger.unlock_camera = !selected->trigger.unlock_camera;                 
                }
                v_pos += height_add;
                
                make_ui_text("Play sound: ", {inspector_position.x + 5, v_pos}, "trigger_play_sound");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.play_sound, "toggle_play_sound")){
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
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->trigger.load_level, "toggle_load_level")){
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
        
            if (selected->trigger.lock_camera){
                make_ui_text("Alt+R: Locked cam position", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "locked_cam_position");
                type_info_v_pos += type_font_size;
            }
            make_ui_text("Clear ALL Connected: Alt+L", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_clear");
            type_info_v_pos += type_font_size;
            make_ui_text("Remove selected: Alt+D", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_remove");
            type_info_v_pos += type_font_size;
            make_ui_text("Assign New: Alt+A", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_assign");
            type_info_v_pos += type_font_size;
            make_ui_text("Assign tracking enemy: Alt+V", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_assign");
            type_info_v_pos += type_font_size;
            make_ui_text(TextFormat("Connected count: %d", selected->trigger.connected.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_connected_count");
            type_info_v_pos += type_font_size;
            make_ui_text(TextFormat("Tracking count: %d", selected->trigger.tracking.count), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "trigger_tracking_count");
            type_info_v_pos += type_font_size;
            make_ui_text("Trigger settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "trigger_settings");
            type_info_v_pos += type_font_size;
        }
        
        if (selected->flags & ENEMY){
            if (make_button({inspector_position.x + 5, v_pos}, {200, 18}, "Enemy settings", "enemy_settings")){
                editor.draw_enemy_settings = !editor.draw_enemy_settings;
            }
            v_pos += height_add;
            
            if (editor.draw_enemy_settings){
                make_ui_text("Gives full ammo: ", {inspector_position.x + 5, v_pos}, "gives_full_ammo");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->enemy.gives_full_ammo, "gives_full_ammo")){
                    selected->enemy.gives_full_ammo = !selected->enemy.gives_full_ammo;
                }
                v_pos += height_add;
                
                make_ui_text("Explosive: ", {inspector_position.x + 5, v_pos}, "text_enemy_explosive");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->flags & EXPLOSIVE, "toggle_enemy_explosive")){
                    selected->flags ^= EXPLOSIVE;
                    init_entity(selected);
                }
                v_pos += height_add;
                
                make_ui_text("Blocker: ", {inspector_position.x + 5, v_pos}, "text_enemy_blocker");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->flags & BLOCKER, "toggle_enemy_blocker")){
                    selected->flags ^= BLOCKER;
                    init_entity(selected);
                }
                v_pos += height_add;
                
                if (selected->flags & BLOCKER){
                    make_ui_text("Blocker immortal: ", {inspector_position.x + 5, v_pos}, "text_enemy_blocker_immortal");
                    if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->enemy.blocker_immortal, "toggle_enemy_blocker_immortal")){
                        selected->enemy.blocker_immortal = !selected->enemy.blocker_immortal;
                        init_entity(selected);
                    }
                    v_pos += height_add;

                    if (!selected->enemy.blocker_immortal){
                        make_ui_text("Blocker clockwise: ", {inspector_position.x + 5, v_pos}, "text_enemy_blocker_clocksize");
                        if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->enemy.blocker_clockwise, "toggle_enemy_blocker_clockwise")){
                            selected->enemy.blocker_clockwise = !selected->enemy.blocker_clockwise;
                            init_entity(selected);
                        }
                        v_pos += height_add;
                    }
                }
                
                make_ui_text("Shoot blocker: ", {inspector_position.x + 5, v_pos}, "text_enemy_shoot_blocker");
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->flags & SHOOT_BLOCKER, "toggle_enemy_shoot_blocker")){
                    selected->flags ^= SHOOT_BLOCKER;
                    init_entity(selected);
                }
                v_pos += height_add;
                
                if (selected->flags & SHOOT_BLOCKER){
                    make_ui_text("Shoot blocker immortal: ", {inspector_position.x + 5, v_pos}, "text_enemy_shoot_blocker_immortal");
                    if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->enemy.shoot_blocker_immortal, "toggle_enemy_shoot_blocker_immortal")){
                        selected->enemy.shoot_blocker_immortal = !selected->enemy.shoot_blocker_immortal;
                        init_entity(selected);
                    }
                    v_pos += height_add;
                }
            }
        
            if (selected->flags & BLOCKER){
            }
            
            make_ui_text(TextFormat("Alt+O/P Sword kill speed: %.1f", selected->enemy.sword_kill_speed_modifier), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "sword_kill_speed_modifier_change");
            type_info_v_pos += type_font_size;
            
            if (selected->flags & SHOOT_BLOCKER){
                if (!selected->enemy.shoot_blocker_immortal){
                    make_ui_text(TextFormat("Alt+F/G Shoot Block Vector: {%.2f, %.2f}", selected->enemy.shoot_blocker_direction.x, selected->enemy.shoot_blocker_direction.y), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "shoot_blocker_direction");
                    type_info_v_pos += type_font_size;
                }
            }

            make_ui_text("Enemy settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "enemy_settings");
            type_info_v_pos += type_font_size;
        }
        
        if (selected->flags & PROPELLER){
            make_ui_text(TextFormat("Alt+Q/E Power change: %.0f", selected->propeller.power), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "propeller_power");
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
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->centipede.spikes_on_right, "spikes_on_right")){
                    selected->centipede.spikes_on_right = !selected->centipede.spikes_on_right;
                }
                v_pos += height_add;
                
                make_ui_text("Segments count: ", {inspector_position.x + 25, v_pos}, "segments_count");
                if (make_input_field(TextFormat("%d", selected->centipede.segments_count), {inspector_position.x + 100, v_pos}, 100, "segments_count")){
                    selected->centipede.segments_count = fmin(atoi(focus_input_field.content), MAX_CENTIPEDE_SEGMENTS);
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
                if (make_ui_toggle({inspector_position.x + 250, v_pos}, selected->door.is_open, "toggle_door_open_closed")){
                    selected->door.is_open = !selected->door.is_open;
                    init_entity(selected);
                }
                v_pos += height_add;
            }
        
            make_ui_text(TextFormat("Alt+T Trigger: %s", selected->door.is_open ? "Open" : "Close"), {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, ColorBrightness(RED, -0.2f), "door_trigger");
            type_info_v_pos += type_font_size;
            
            make_ui_text("Door settings:", {inspector_position.x - 150, (f32)screen_height - type_info_v_pos}, type_font_size, SKYBLUE * 0.9f, "door_settings");
            type_info_v_pos += type_font_size;
        }
        
        //type info background
        make_ui_image({inspector_position.x - 160, (f32)screen_height - type_info_v_pos}, {(f32)screen_width * 0.5f, type_info_v_pos}, {0, 0}, SKYBLUE * 0.7f, "inspector_type_info_background");
    }
    
    //create box
    b32 can_control_create_box = !console.is_open;
    b32 need_close_create_box = false;
    
    if (can_control_create_box && IsKeyPressed(KEY_SPACE) && editor.in_editor_time > 0.05f){
        if (editor.create_box_active && !editor.create_box_closing){
            need_close_create_box = true;
        } else{ //open create box
            editor.create_box_open_mouse_position = input.mouse_position;
            
            editor.create_box_active = true;
            editor.create_box_closing = false;
            editor.create_box_lifetime = 0;
            make_next_input_field_in_focus();
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
        
        //auto fitting_objects = Array<Spawn_Object, MAX_SPAWN_OBJECTS>();
        int input_len = str_len(focus_input_field.content);
        int fitting_count = 0;
        
        for (int i = 0; i < spawn_objects.count; i++){
            Spawn_Object obj = spawn_objects.get(i);
            if (input_len > 0 && !str_contains(obj.name, focus_input_field.content)){
                continue;
            }
            
            Vector2 obj_position = field_position + Vector2_up * field_size.y * (fitting_count + 1) + Vector2_right * field_size.x * 0.2f;
            Vector2 obj_size = {field_size.x * 0.6f, field_size.y};
            
            b32 this_object_selected = editor.create_box_selected_index == fitting_count;
            
            Color button_color = lerp(BLACK * 0, BLACK * 0.9f, clamp01(create_t * 2));
            Color text_color   = lerp(WHITE * 0, WHITE * 0.9f, clamp01(create_t * 2));
            
            if (make_button(obj_position, obj_size, {0, 0}, obj.name, 24, "create_box", button_color, text_color) || (this_object_selected && IsKeyPressed(KEY_ENTER))){
                Entity *entity = add_entity(&obj.entity);
                entity->position = editor.create_box_open_mouse_position;
                need_close_create_box = true;
                
                Undo_Action undo_action;
                undo_action.spawned_entity = entity;
                //undo_action.entity = entity;
                undo_action.entity_id = entity->id;
                undo_action.entity_was_spawned = true;
                add_undo_action(undo_action);
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

    ForTable(context.entities, i){
        Entity *e = context.entities.get_ptr(i);
        if (!e->enabled/* || e->flags == -1*/){
            continue;
        }
        
        if ((check_entities_collision(&mouse_entity, e)).collided){
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
    }        
    
    return cursor_entity_candidate;
}

void update_editor(){
    Undo_Action undo_action;
    b32 something_in_undo = false;
    b32 can_control_with_single_button = !focus_input_field.in_focus && !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT);
    b32 can_select = !clicked_ui;
    
    f32 dt = core.time.real_dt;
    
    editor.in_editor_time += dt;

    if (editor.need_validate_entity_pointers){
        validate_editor_pointers();
        editor.need_validate_entity_pointers = false;
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_L)){
        editor.update_cam_view_position = !editor.update_cam_view_position;
    }
    
    b32 moving_editor_cam = false;
    
    f32 zoom = context.cam.target_zoom;

    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        // context.cam.position += ((Vector2){-input.mouse_delta.x / zoom, input.mouse_delta.y / zoom}) / (UNIT_SIZE);
        moving_editor_cam = true;
    }
    
    // if (input.mouse_wheel != 0 && !console.is_open && !editor.create_box_active){
    //     if (input.mouse_wheel > 0 && zoom < 5 || input.mouse_wheel < 0 && zoom > 0.1f){
    //         context.cam.target_zoom += input.mouse_wheel * 0.05f;
            
    //         // UnloadRenderTexture(render.ray_collision_render_texture);
        
    //         // render.ray_collision_render_texture = LoadRenderTexture(screen_width / context.cam.cam2D.zoom, screen_height / context.cam.cam2D.zoom);
    //     }
    // }
    
    b32 need_move_vertices = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select;
    b32 need_snap_vertex = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_V);
    
    int selected_vertex_index;
    Vector2 closest_vertex_global;
    f32 distance_to_closest_vertex = INFINITY;
    
    mouse_entity.position = input.mouse_position;
    
    //Entity *cursor_entity_candidate = NULL;
    
    int cursor_entities_count = 0;
    
    Entity *moving_vertex_entity_candidate = NULL;
    int moving_vertex_candidate = -1;
    
    //editor entities loop
    for (int i = 0; i < context.entities.max_count; i++){        
        Entity *e = context.entities.get_ptr(i);
        
        if (!e->enabled/* || e->flags == -1*/){
            continue;
        }
        
        if ((check_entities_collision(&mouse_entity, e)).collided){
            cursor_entities_count++;
        }
        
        //editor vertices
        for (int v = 0; v < e->vertices.count && (need_move_vertices || need_snap_vertex); v++){
            Vector2 *vertex = e->vertices.get_ptr(v);
            
            Vector2 vertex_global = global(e, *vertex);
            
            if (need_move_vertices && (!moving_vertex_entity_candidate || (editor.selected_entity && e->id == editor.selected_entity->id))){
                if (check_col_circles({input.mouse_position, 1}, {vertex_global, 0.5f * (0.4f / context.cam.cam2D.zoom)})){
                    moving_vertex_entity_candidate = e;
                    moving_vertex_candidate = v;
                    // assign_moving_vertex_entity(e, v);
                    // undo_remember_vertices_start(e);
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
        for (int p = 0; e->flags & MOVE_SEQUENCE && IsKeyDown(KEY_LEFT_ALT) && p < e->move_sequence.points.count; p++){
            Vector2 *point = e->move_sequence.points.get_ptr(p);
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && check_col_circles({input.mouse_position, 1}, {*point, 0.5f})){
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
            //undo_action.entity = editor.moving_vertex_entity;
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
            //undo_action.entity = editor.selected_entity;
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
        //undo_action.entity = pasted_entity;
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
        Vector2 move_delta = ((Vector2){input.mouse_delta.x / zoom, -input.mouse_delta.y / zoom}) / (UNIT_SIZE);
        editor.dragging_entity->position += move_delta;
    }
    
    //editor Entity to mouse or go to entity
    if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.dragging_entity){
        editor.dragging_entity->position = input.mouse_position;
    } else if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.selected_entity){
        context.cam.position = editor.selected_entity->position;
    }
    
    //editor Entity rotation
    if (can_control_with_single_button && editor.selected_entity != NULL){
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
        
        if (rotation != 0){
            rotate(editor.selected_entity, rotation);
        }
        
        if (editor.is_rotating_entity && (IsKeyUp(KEY_E) && IsKeyUp(KEY_Q))){
            undo_add_rotation(editor.selected_entity, editor.selected_entity->rotation - editor.rotating_start);
            editor.is_rotating_entity = false;
        } 
    }
    
    //editor entity scaling
    if (can_control_with_single_button && editor.selected_entity != NULL){
        Vector2 scaling = {};
        f32 speed = 80;
        
        if (!editor.is_scaling_entity && (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_A))){
            editor.scaling_start = editor.selected_entity->scale;
            undo_remember_vertices_start(editor.selected_entity);
            editor.is_scaling_entity = true;
        }
        if (IsKeyDown(KEY_W)){
            scaling.y += speed * dt;
        } else if (IsKeyDown(KEY_S)){
            scaling.y -= speed * dt;
        }
        if (IsKeyDown(KEY_D)){
            scaling.x += speed * dt;
        } else if (IsKeyDown(KEY_A)){
            scaling.x -= speed * dt;
        }
        
        if (scaling != Vector2_zero){
            add_scale(editor.selected_entity, scaling);
        }
        
        if (editor.is_scaling_entity && (IsKeyUp(KEY_W) && IsKeyUp(KEY_S) && IsKeyUp(KEY_A) && IsKeyUp(KEY_D))){
            //something_in_undo = true;
            Vector2 scale_change = editor.selected_entity->scale - editor.scaling_start;
            
            undo_add_scaling(editor.selected_entity, scale_change);
            editor.is_scaling_entity = false;
        } 
    }
    
    //editor components management
    if (editor.selected_entity){
        Entity *selected = editor.selected_entity;
        if (selected->flags & TRIGGER){
            b32 wanna_assign = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_A);
            b32 wanna_assign_tracking_enemy = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_V);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_D);
            b32 wanna_change_locked_camera_position = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_R);
            //trigger assign or remove
            if (wanna_assign || wanna_remove){
                fill_collisions(&mouse_entity, &collisions_data, DOOR | ENEMY | SPIKES | GROUND | PLATFORM | MOVE_SEQUENCE | TRIGGER);
                
                for (int i = 0; i < collisions_data.count; i++){
                    Collision col = collisions_data.get(i);
                    
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
                fill_collisions(&mouse_entity, &collisions_data, ENEMY | CENTIPEDE);
                for (int i = 0; i < collisions_data.count; i++){
                    Collision col = collisions_data.get(i);
                    
                    if (!selected->trigger.tracking.contains(col.other_entity->id)){
                        selected->trigger.tracking.add(col.other_entity->id);
                    }
                }
            }
            
            //trigger clear
            if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_L)){
                selected->trigger.connected.clear();
            }
        }
        
        //enemy components
        if (selected->flags & ENEMY){
            b32 wanna_increase_sword_kill_speed  = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_P);
            b32 wanna_decrease_sword_kill_speed  = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_O);
            
            b32 wanna_rotate_shoot_blocker_direction   = IsKeyDown(KEY_LEFT_ALT) && (IsKeyDown(KEY_F) || IsKeyDown(KEY_G));
            
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
            b32 wanna_increase_power = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_E);
            b32 wanna_decrease_power = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_Q);
            
            if (wanna_increase_power || wanna_decrease_power){
                f32 power_change = wanna_increase_power ? 100 : -100;
                selected->propeller.power += power_change;
            }
        }
        
        // door settings
        if (selected->flags & DOOR){
            b32 wanna_trigger = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_T);
            
            if (wanna_trigger){
                activate_door(selected, !selected->door.is_open);
            }
        }
        
        // move sequence settings
        if (selected->flags & MOVE_SEQUENCE){
            b32 wanna_clear    = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_C);
            b32 wanna_add    = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_N);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_M);
            
            if (wanna_remove){
                for (int i = 0; i < selected->move_sequence.points.count; i++){
                    Vector2 point = selected->move_sequence.points.get(i);   
                    
                    if (check_col_circles({input.mouse_position, 1}, {point, 0.5f  * (0.4f / context.cam.cam2D.zoom)})){       
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
            //action->entity = restored_entity;
            action->entity_id = action->deleted_entity.id;
            
            editor.need_validate_entity_pointers = true;
        } else if (action->entity_was_spawned){
            editor_delete_entity(action->entity_id, false);
            editor.need_validate_entity_pointers = true;
        } else{
            assert(context.entities.has_key(action->entity_id));
            Entity *undo_entity = context.entities.get_by_key_ptr(action->entity_id);

            undo_entity->position   -= action->position_change;
            undo_entity->scale      -= action->scale_change;
            undo_entity->rotation   -= action->rotation_change;
            undo_entity->draw_order -= action->draw_order_change;
            
            for (int i = 0; i < action->vertices_change.count; i++){
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
            assert(context.entities.has_key(action->entity_id));
            editor_delete_entity(context.entities.get_by_key_ptr(action->entity_id), false);
            editor.need_validate_entity_pointers = true;
        } else if (action->entity_was_spawned){ //so we need spawn this again
            Entity *restored_entity = add_entity(&action->spawned_entity, true);
            restored_entity->id = action->spawned_entity.id;
            action->entity_id = restored_entity->id;
            
            editor.need_validate_entity_pointers = true;
        } else{
            assert(context.entities.has_key(action->entity_id));
            Entity *undo_entity = context.entities.get_by_key_ptr(action->entity_id);
            undo_entity->position   += action->position_change;
            undo_entity->scale      += action->scale_change;
            undo_entity->rotation   += action->rotation_change;
            undo_entity->draw_order += action->draw_order_change;
            
            for (int i = 0; i < action->vertices_change.count; i++){
                *undo_entity->vertices.get_ptr(i)          += action->vertices_change.get(i);
                *undo_entity->unscaled_vertices.get_ptr(i) += action->unscaled_vertices_change.get(i);
            }
            rotate(undo_entity, 0);
            
            calculate_bounds(undo_entity);
        }
    }
    
    //editor Save level
    if (IsKeyPressed(KEY_J) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL)){
        //save_level("test_level");
        save_current_level();
    }
    
    f32 time_since_autosave = core.time.app_time - editor.last_autosave_time;
    if (time_since_autosave > 20){
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
    // entity->color_changer.start_color  = new_color;
    // entity->color_changer.target_color = new_color * 1.5f;
    setup_color_changer(entity);
}

Bounds get_bounds(Array<Vector2, MAX_VERTICES> vertices, Vector2 pivot){
    f32 top_vertex    = -INFINITY;
    f32 bottom_vertex =  INFINITY;
    f32 right_vertex  = -INFINITY;
    f32 left_vertex   =  INFINITY;
    
    Vector2 middle_position;
    
    for (int i = 0; i < vertices.count; i++){
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
    for (int i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        Vector2 unscaled_vertex = entity->unscaled_vertices.get(i);
        
        //Vector2 dir_to_vertex = normalized(unscaled_vertex - entity->position);
        
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
    
    for (int i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        rotate_around_point(vertex, {0, 0}, entity->rotation - old_rotation);
        rotate_around_point(entity->unscaled_vertices.get_ptr(i), {0, 0}, entity->rotation - old_rotation);
    }
    
    // calculate_vertices(entity);
    
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

void add_hitstop(f32 added){
    if (core.time.hitstop < 0){
        core.time.hitstop = 0;
    }

    core.time.hitstop += added;
    
    clamp(&core.time.hitstop, 0, 0.1f);
}

void shake_camera(f32 trauma){
    context.cam.trauma += trauma;
    context.cam.trauma = clamp01(context.cam.trauma);
}

void add_blood_amount(Player *player, Entity *sword, f32 added){
    player->blood_amount += added;
    clamp(&player->blood_amount, 0, player->max_blood_amount);
    player->blood_progress = player->blood_amount / player->max_blood_amount;
    
    Vector2 sword_target_scale = player->sword_start_scale * 2;
    // change_scale(sword, lerp(player->sword_start_scale, sword_target_scale, player->blood_progress * player->blood_progress));
}

void win_level(){
    if (!context.we_got_a_winner){    
        context.we_got_a_winner = true;
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
    
    player_data.ammo_count = clamp(player_data.ammo_count, 0, 5000);
}

inline b32 is_sword_can_damage(){
    return player_data.sword_spin_progress >= 0.12f;
}

inline b32 can_damage_blocker(Entity *blocker_entity){
    return is_sword_can_damage() && !blocker_entity->enemy.blocker_immortal && (blocker_entity->enemy.blocker_clockwise ? player_data.sword_spin_direction > 0 : player_data.sword_spin_direction < 0);
}

void sword_kill_bird(Entity *bird_entity){
    Entity *sword = context.entities.get_by_key_ptr(player_data.sword_entity_id);
    bird_entity->bird_enemy.velocity = sword_tip_emitter->direction * sqrtf(player_data.sword_spin_progress) * 100;
    stun_enemy(bird_entity, sword->position + sword->up * sword->scale.y * sword->pivot.y, sword_tip_emitter->direction, true);
    add_hitstop(0.1f);
}

void calculate_sword_collisions(Entity *sword, Entity *player_entity){
    fill_collisions(sword, &player_data.collisions, GROUND | ENEMY | WIN_BLOCK | CENTIPEDE_SEGMENT | PLATFORM | BLOCK_ROPE);
    
    Player *player = &player_data;
    
    for (int i = 0; i < player->collisions.count; i++){
        Collision col = player->collisions.get(i);
        Entity *other = col.other_entity;
        
        if (other->flags & BLOCKER && !player->in_stun){
            if (is_sword_can_damage() && !can_damage_blocker(other)){
                player->velocity = player->velocity * -0.5f;
                emit_particles(rifle_bullet_emitter, col.point, col.normal, 3, 5);
                set_sword_velocity(-player->sword_angular_velocity * 0.1f);
                player_data.weak_recoil_stun_start_time = core.time.game_time;
                add_hitstop(0.1f);
                shake_camera(0.7f);
                play_sound(player_data.sword_block_sound, col.point);
                continue;
            }
        }
        
        if (other->flags & ENEMY && is_sword_can_damage() && !other->enemy.dead_man && !player->in_stun && is_enemy_can_take_damage(other)){
            f32 hitstop_add = 0;
            if (other->flags & BIRD_ENEMY){
                sword_kill_bird(other);
            } else{
                kill_enemy(other, sword->position + sword->up * sword->scale.y * sword->pivot.y, sword_tip_emitter->direction, lerp(1.0f, 4.0f, sqrtf(player_data.sword_spin_progress)));
            }
            
            f32 max_speed_boost = 6 * player->sword_spin_direction * other->enemy.sword_kill_speed_modifier;
            if (!player->grounded){
                //max_speed_boost *= -1;
            }
            f32 max_vertical_speed_boost = player->grounded ? 0 : 20;
            if (player_data.velocity.y > 0){
                max_vertical_speed_boost *= 0.3f;   
            }
            f32 spin_t = player->sword_spin_progress;
            player->velocity += Vector2_up   * lerp(0.0f, max_vertical_speed_boost, spin_t * spin_t)
                             + Vector2_right * lerp(0.0f, max_speed_boost, spin_t * spin_t); 
                             
            add_blood_amount(player, sword, 10);
            add_hitstop(0.01f + hitstop_add);
            shake_camera(0.1f);
            play_sound(player_data.sword_kill_sound, col.point);
            
            if (!(other->flags & TRIGGER)){
                add_player_ammo(1, other->enemy.gives_full_ammo);
            }
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
    player_data.since_jump_timer = 0;
    player_data.grounded = false;
}

void push_or_set_player_up(f32 power){
    if (player_data.velocity.y > power){
        power *= 0.25f;
    }

    player_data.velocity.y += power;
    
    player_data.since_jump_timer = 0;
    player_data.grounded = false;
}

f32 apply_physics_force(Vector2 velocity, f32 mass, Physics_Object *to_whom, Vector2 normal){
    Vector2 velocity_direction = normalized(velocity);
    if (normal == Vector2_zero){
        normal = velocity_direction * -1.0f;
    }
    f32 collision_force_multiplier = 1;

    // to_whom->velocity += (velocity * mass) / to_whom->mass;
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
    
    //Player *p player_data;
    Entity *ground_checker = context.entities.get_by_key_ptr(player_data.ground_checker_id);
    Entity *left_wall_checker = context.entities.get_by_key_ptr(player_data.left_wall_checker_id);
    Entity *right_wall_checker = context.entities.get_by_key_ptr(player_data.right_wall_checker_id);
    Entity *sword          = context.entities.get_by_key_ptr(player_data.sword_entity_id);
    
    ground_checker->position     = entity->position - entity->up * entity->scale.y * 0.5f;
    left_wall_checker->position  = entity->position - entity->right * entity->scale.x * 1.5f;
    right_wall_checker->position = entity->position + entity->right * entity->scale.x * 1.5f;
    sword->position = entity->position;
    
    // change sword size
    if (input.press_flags & SWORD_BIG){
        player_data.is_sword_big = !player_data.is_sword_big;
    }
    
    Vector2 sword_target_size = player_data.sword_start_scale * (player_data.is_sword_big ? 6 : 1);
    
    change_scale(sword, lerp(sword->scale, sword_target_size, dt * 5));
    
    Vector2 sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
    
    Vector2 vec_to_mouse = input.mouse_position - entity->position;
    Vector2 dir_to_mouse = normalized(vec_to_mouse);
    //Vector2 vec_tip_to_mouse = input.mouse_position - sword_tip;
    
    if (input.press_flags & SPIN){
        chainsaw_emitter->position = input.mouse_position;
        chainsaw_emitter->last_emitted_position = input.mouse_position;
        chainsaw_emitter->enabled = true;
    }
    if (input.press_flags & SPIN_RELEASED){
        chainsaw_emitter->enabled = false;
    }
    
    player_data.sword_angular_velocity *= 1.0f - (dt);
    
    if (input.hold_flags & SPIN_DOWN){
        player_data.rifle_active = false;
    }
    
    b32 can_sword_spin = !player_data.rifle_active;
    
    f32 sword_max_spin_speed = 5000;
    if (can_sword_spin){
        f32 sword_spin_sense = player_data.is_sword_big ? 1 : 10; 
        if (can_sword_spin && input.hold_flags & SPIN_DOWN){
            player_data.sword_angular_velocity += input.sum_mouse_delta.x * sword_spin_sense;
            clamp(&player_data.sword_angular_velocity, -sword_max_spin_speed, sword_max_spin_speed);
        } else{
            //chainsaw_emitter->last_emitted_position = input.mouse_position;
        }
    }
    
    f32 blood_progress_for_strong = 0.4f;
    b32 is_sword_filled = player_data.blood_progress >= blood_progress_for_strong;
    b32 is_rifle_charged = player_data.rifle_active && is_sword_filled;
    
    player_data.sword_spin_progress = clamp01(abs(player_data.sword_angular_velocity) / sword_max_spin_speed);
    
    b32 rifle_failed_hard = false;
    
    player_data.rifle_trail_emitter->position = sword_tip;
    player_data.rifle_trail_emitter->direction = sword->up;
    
    // player shoot
    b32 can_shoot_rifle = player_data.rifle_active && (player_data.ammo_count > 0 || debug.infinite_ammo) && context.shoot_stopers_count == 0;
    
    if (can_shoot_rifle && input.press_flags & SHOOT){
        add_rifle_projectile(sword_tip, sword->up * player_data.rifle_strong_speed, STRONG);
        add_player_ammo(-1, true);
        
        //push_player_up(60);
        push_or_set_player_up(20);
        shake_camera(0.1f);
        play_sound("RifleShot", sword_tip, 0.3f);
        player_data.rifle_shake_start_time = core.time.game_time;
        player_data.rifle_shoot_time = core.time.game_time;
        
        enable_emitter(player_data.rifle_trail_emitter);
    } else if (input.press_flags & SHOOT){
        player_data.rifle_shake_start_time = core.time.game_time;
        emit_particles(gunpowder_emitter, sword_tip, sword->up);
        
        // shoot blocker blocked
        if (context.shoot_stopers_count > 0){
            ForTable(context.entities, i){
                Entity *e = context.entities.get_ptr(i);
                if (e->flags & SHOOT_STOPER){
                    Entity *sticky_line = add_entity(player_entity->position, {1,1}, {0.5f,0.5f}, 0, STICKY_TEXTURE);
                    sticky_line->sticky_texture.line_color = ColorBrightness(VIOLET, 0.1f);
                    sticky_line->sticky_texture.follow_id = e->id;
                    sticky_line->sticky_texture.need_to_follow = true;
                    sticky_line->sticky_texture.texture_position = get_shoot_stoper_cross_position(e);
                    sticky_line->sticky_texture.birth_time = core.time.game_time;
                    sticky_line->sticky_texture.max_distance = 0;
                    sticky_line->draw_order = 1;
                    shake_camera(0.5f);
                }
            }
        }
    }
    
    if (player_data.rifle_active){
        change_up(sword, move_towards(sword->up, dir_to_mouse, 100, dt));        
    } else{
        player_data.rifle_perfect_shots_made = 0;
    }
    
    f32 time_since_shoot = core.time.game_time - player_data.rifle_shoot_time;
    
    if (time_since_shoot >= 0.5f && core.time.game_time > 1){
        player_data.rifle_trail_emitter->enabled = false;
    } else{
    }
    
    //rifle activate
    b32 spin_enough_for_shoot = player_data.sword_spin_progress >= 0.1f;
    b32 can_activate_rifle = !player_data.rifle_active && !can_shoot_rifle && spin_enough_for_shoot;
    if (can_activate_rifle && input.press_flags & SHOOT){
        player_data.rifle_active = true;
        
        //player_data.rifle_current_power += player_data.sword_spin_progress * 50;
        //player_data.sword_spin_progress *= 0.1f;
        player_data.sword_angular_velocity = 0;
        player_data.rifle_activate_time = core.time.game_time;
        
        play_sound(player_data.rifle_switch_sound);
    } else if (!spin_enough_for_shoot && input.press_flags & SHOOT){
        player_data.rifle_shake_start_time = core.time.game_time;
        emit_particles(gunpowder_emitter, sword_tip, sword->up);
    }
    
    sword->color_changer.progress = can_activate_rifle ? 1 : 0;//player_data.blood_progress * player_data.blood_progress;//player_data.sword_spin_progress * player_data.sword_spin_progress;
    
    sword_tip_emitter->position = sword_tip;
    sword_tip_ground_emitter->position = sword_tip;
    chainsaw_emitter->position = input.mouse_position;
    
    //if (!player_data.rifle_active){ 
    {
        f32 spin_t = player_data.sword_spin_progress;
        f32 blood_t = player_data.blood_progress;
    
        chainsaw_emitter->lifetime_multiplier = 1.0f + spin_t * spin_t * 2; //@VISUAL: change color
        chainsaw_emitter->speed_multiplier    = 1.0f + spin_t * spin_t * 2; //@VISUAL: change color
        
        chainsaw_emitter->count_multiplier = player_data.is_sword_big ? 0.1f : 1;
        chainsaw_emitter->size_multiplier  = player_data.is_sword_big ? 5 : 1;
        chainsaw_emitter->color            = player_data.is_sword_big ? ColorBrightness(ORANGE, 0.2f) : YELLOW;
        
        sword_tip_emitter->lifetime_multiplier = 1.0f + blood_t * blood_t * 3.0f;
        sword_tip_emitter->speed_multiplier    = 1.0f + blood_t * blood_t * 5.0f;
        sword_tip_emitter->count_multiplier    = blood_t * blood_t * 2.0f;
              
        f32 blood_decease = 5;// + blood_t * 10;
              
        add_blood_amount(&player_data, sword, -blood_decease * dt);
    }
    
    f32 sword_min_rotation_amount = 20;
    f32 need_to_rotate = player_data.sword_angular_velocity * dt;
    
    player_data.sword_spin_direction = normalized(player_data.sword_angular_velocity);
    
    if (abs(player_data.sword_angular_velocity) > 10){ 
        while(need_to_rotate > sword_min_rotation_amount){
            rotate(sword, sword_min_rotation_amount);
            calculate_sword_collisions(sword, entity);
            need_to_rotate -= sword_min_rotation_amount;
        }
        rotate(sword, need_to_rotate);
        calculate_sword_collisions(sword, entity);
    }
    
    player_data.since_jump_timer += dt;
    
    f32 max_strong_stun_time = 2.0f;
    f32 max_weak_stun_time = 0.8f;
    f32 in_strong_stun_time = core.time.game_time - player_data.strong_recoil_stun_start_time;
    f32 in_weak_stun_time   = core.time.game_time - player_data.weak_recoil_stun_start_time;
    player_data.in_stun = core.time.game_time > 3 && (in_strong_stun_time <= max_strong_stun_time || in_weak_stun_time <= max_weak_stun_time);
    
    if (!player_data.in_stun){
        player_data.stun_emitter->enabled = false;
    } else{
        enable_emitter(player_data.stun_emitter);
    }
    
    //player movement
    if (player_data.grounded && !player_data.in_stun){
        if (1 /*!sword_attacking*/){
            player_ground_move(entity, dt);
            
            player_data.plane_vector = get_rotated_vector_90(player_data.ground_normal, -normalized(player_data.velocity.x));
            player_data.velocity = player_data.plane_vector * magnitude(player_data.velocity);
            
            entity->position.y -= dt;
            player_data.velocity -= player_data.ground_normal * dt;
        }
        
        if (player_data.sword_spin_progress > 0.3f){
            Vector2 plane = get_rotated_vector_90(player_data.ground_normal, -player_data.sword_spin_direction);
            
            f32 spin_t = player_data.sword_spin_progress;
            f32 blood_t = player_data.blood_progress;
            
            // f32 max_spin_acceleration = 500;
            // f32 min_spin_acceleration = 200;
            f32 max_spin_acceleration = 400;
            f32 min_spin_acceleration = 400;
            f32 spin_acceleration = lerp(min_spin_acceleration, max_spin_acceleration, blood_t * blood_t);
            player_data.velocity += plane * lerp(0.0f, spin_acceleration, spin_t * spin_t) * dt;
        }
        
        player_data.since_airborn_timer = 0;
    } else{
        if (player_data.velocity.y > 0 && player_data.since_jump_timer <= 0.3f){ //so we make jump gravity
            f32 max_height_jump_time = 0.2f;
            f32 jump_t = clamp01(player_data.since_jump_timer / max_height_jump_time);
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
        
        player_data.since_airborn_timer += dt;
        
        if (player_data.sword_spin_progress > 0.3f){
            f32 spin_t = player_data.sword_spin_progress;
            f32 blood_t = player_data.blood_progress;
            
            // f32 max_spin_acceleration = 200;
            // f32 min_spin_acceleration = 50;
            f32 max_spin_acceleration = 150;
            f32 min_spin_acceleration = 150;
            f32 spin_acceleration = lerp(min_spin_acceleration, max_spin_acceleration, blood_t * blood_t);
        
            f32 airborn_reduce_spin_acceleration_time = 0.5f;
            f32 t = clamp01(spin_t - clamp01(airborn_reduce_spin_acceleration_time - player_data.since_airborn_timer));
            player_data.velocity.x += lerp(0.0f, spin_acceleration, t * t) * dt * player_data.sword_spin_direction;
        }
        
    }
    
    if (player_data.grounded && input.press_flags & JUMP){
        push_player_up(player_data.jump_force);
    }
    
    Vector2 next_pos = {entity->position.x + player_data.velocity.x * dt, entity->position.y + player_data.velocity.y * dt};
    
    entity->position = next_pos;
    
    f32 found_ground = false;
    f32 just_grounded = false;
    
    f32 wall_acceleration = 400;
    
    f32 sword_ground_particles_speed = 1;
    
    //player collisions
    
    // player left wall
    fill_collisions(left_wall_checker, &player_data.collisions, GROUND | CENTIPEDE_SEGMENT | PLATFORM);
    for (int i = 0; i < player_data.collisions.count && !player_data.in_stun; i++){
        Collision col = player_data.collisions.get(i);
        Entity *other = col.other_entity;
        assert(col.collided);
        
        if (input.press_flags & JUMP){
            player_data.velocity += col.normal * player_data.jump_force;
        }

        
        if (player_data.sword_spin_direction > 0){
            break;
        }
        
        if (other->flags & PLATFORM && dot(player_data.velocity, other->up) > 0){
            continue;
        }
        
        Vector2 plane = get_rotated_vector_90(col.normal, -player_data.sword_spin_direction);
        f32 spin_t = player_data.sword_spin_progress;
        
        f32 acceleration = lerp(0.0f, wall_acceleration, spin_t * spin_t);
        
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
    fill_collisions(right_wall_checker, &player_data.collisions, GROUND | CENTIPEDE_SEGMENT | PLATFORM);
    for (int i = 0; i < player_data.collisions.count && !player_data.in_stun; i++){
        Collision col = player_data.collisions.get(i);
        Entity *other = col.other_entity;
        assert(col.collided);
        
        if (input.press_flags & JUMP){
            player_data.velocity += col.normal * player_data.jump_force;
        }

        
        if (player_data.sword_spin_direction < 0){
            break;
        }
        
        if (other->flags & PLATFORM && dot(player_data.velocity, other->up) > 0){
            continue;
        }
        
        Vector2 plane = get_rotated_vector_90(col.normal, -player_data.sword_spin_direction);
        f32 spin_t = player_data.sword_spin_progress;
        
        f32 acceleration = lerp(0.0f, wall_acceleration, spin_t * spin_t);
        
        if (other->flags & PHYSICS_OBJECT){
            other->physics_object.velocity -= (plane * acceleration * dt) / other->physics_object.mass;
        }
        
        if (dot(plane, player_data.velocity) < 0){
            acceleration *= 4;
        }

        player_data.velocity += plane * acceleration * dt;
        sword_ground_particles_speed += 2;
    }
    
    b32 moving_object_detected = false;
    // player ground checker
    fill_collisions(ground_checker, &player_data.collisions, GROUND | BLOCKER | PLATFORM | CENTIPEDE_SEGMENT);
    b32 is_huge_collision_speed = false;
    for (int i = 0; i < player_data.collisions.count && !player_data.in_stun; i++){
        Collision col = player_data.collisions.get(i);
        Entity *other = col.other_entity;
        assert(col.collided);
        
        f32 dot_velocity = dot(col.normal, player_data.velocity);
        if (dot_velocity >= 0){
            continue;
        }
        
        if (other->flags & PLATFORM && dot(player_data.velocity, other->up) > 0){
            continue;
        }
        
        if (other->flags & CENTIPEDE_SEGMENT){
            Vector2 side = other->centipede_head->centipede.spikes_on_right ? other->right : (other->right * -1.0f);
            f32 side_dot = dot(side, entity->position - other->position);
            // so we on right side of the centipede segments where are SPIKES
            if (side_dot > 0){
                kill_player();
                return;
            }
        }
        
        entity->position.y += col.overlap;
        
        Vector2 velocity_direction = normalized(player_data.velocity);
        f32 before_speed = magnitude(player_data.velocity);
        
        if (before_speed > 200){
            // player_data.heavy_collision_time = core.time.game_time;
            // player_data.heavy_collision_velocity = player_data.velocity;
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
            // player_data.on_moving_object = true;
            // player_data.moving_object_velocity = other->move_sequence.moved_last_frame / dt;
            // moving_object_detected = true;
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
                    emit_particles(air_dust_emitter, col.point, col.normal, 4, 3);
                    shake_camera(0.7f);
                    
                    play_sound("HeavyLanding", col.point, 1.5f);
                }
            }
        }
    }
    
    if (!moving_object_detected && player_data.on_moving_object){
        if (dot(player_data.moving_object_velocity, player_data.velocity) > magnitude(player_data.velocity)){
            // player_data.velocity += player_data.moving_object_velocity - player_data.velocity;
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
    fill_collisions(entity, &player_data.collisions, GROUND | BLOCKER | PROPELLER | CENTIPEDE_SEGMENT | PLATFORM);
    for (int i = 0; i < player_data.collisions.count; i++){
        Collision col = player_data.collisions.get(i);
        Entity *other = col.other_entity;
        assert(col.collided);
        
        //triggers
        if (other->flags & PROPELLER){
            if (player_data.sword_spin_progress > EPSILON){
                Vector2 acceleration_dir = other->up;
                Vector2 deceleration_plane = other->right;//get_rotated_vector_90(acceleration_dir, 1);
                
                f32 power_t = player_data.sword_spin_progress;
                
                Vector2 to_player = player_entity->position - other->position;
                
                f32 deceleration_power = lerp(0.0f, 300.0f, power_t * power_t);
                f32 acceleration_power = lerp(0.0f, other->propeller.power, sqrtf(power_t));
                
                //f32 deceleration_sign = dot(deceleration_plane, player_data.velocity) > 0 ? -1 : 1;
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
        
        
        if (other->flags & CENTIPEDE_SEGMENT){
            Vector2 side = other->centipede_head->centipede.spikes_on_right ? other->right : (other->right * -1.0f);
            f32 side_dot = dot(side, entity->position - other->position);
            // so we on right side of the centipede segments where are SPIKES
            if (side_dot > 0){
                kill_player();
                return;
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
            // entity->position += other->physics_object.velocity * dt;
        }
        
        clamp(&collision_force_multiplier, 0, 1.0f);
        
        player_data.velocity -= col.normal * dot(player_data.velocity, col.normal) * collision_force_multiplier;
        
        //heavy collision
        if (before_speed > 200 && magnitude(player_data.velocity) < 100){
            player_data.heavy_collision_time = core.time.game_time;
            player_data.heavy_collision_velocity = player_data.velocity;
            emit_particles(air_dust_emitter, col.point, col.normal, 10, 2);
            shake_camera(0.7f);
            play_sound("HeavyLanding", col.point, 1.5f);
        }
    }
    
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
    fill_collisions(entity, &collisions_data, entity->collision_flags);
    
    for (int i = 0; i < collisions_data.count; i++){
        Collision col = collisions_data.get(i);
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
    } else if (other->flags & ENEMY){
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
    if (other->flags & GROUND){
        resolve_collision(shooter_entity, col);
        
        
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
    if (other->flags & GROUND){
        resolve_collision(bird_entity, col);
        
        if (other->flags & PHYSICS_OBJECT){
            f32 collision_force = apply_physics_force(bird->velocity, 5, &other->physics_object, col.normal);
            
            if (collision_force <= 0.5f){
                should_respond = false;
            }
        }
        
        if (should_respond){
            if (enemy->dead_man){
                emit_particles(fire_emitter, bird_entity->position, col.normal, 2, 3);
                play_sound("Explosion", bird_entity->position, bird_entity->volume_multiplier);
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
            // bird_entity->destroyed = true;
            // bird_entity->enabled = false;
            
            stun_enemy(other, other->position, col.normal);
            //return;
        }
        
        bird->velocity = reflected_vector(bird->velocity * 0.8f, col.normal);
        
        other->bird_enemy.velocity += reflected_vector(bird->velocity * 0.3f, col.normal * -1);
        
        emit_particles(rifle_bullet_emitter, col.point, col.normal, 0.5f, 1);
        
        if (is_high_velocity){
            play_sound("BirdToBird", col.point, 0.5f);
        }
    }
    
    if (other->flags & PLAYER && !player_data.dead_man && bird->attacking && !enemy->dead_man){
        b32 should_kill_player = !is_sword_can_damage();
        if (bird_entity->flags & BLOCKER){
            should_kill_player = !can_damage_blocker(bird_entity);
        }
    
        if (should_kill_player){
            kill_player();
        } else{
            //kill_enemy(bird_entity, bird_entity->position, bird->velocity);
            sword_kill_bird(bird_entity);
        }
    }
}

// inline void calculate_bird_collisions(Entity *bird_entity){
//     calculate_collisions(&respond_bird_collision, bird_entity, GROUND | BIRD_ENEMY | PLAYER);
// }    

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
    //respond_collision_func(entity);
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
        //bird->roam_timer += dt;
        f32 roam_time = core.time.game_time - bird->roam_start_time;
        
        f32 max_roam_time = bird->max_roam_time;
        
        if (bird->slot_index != -1){
            max_roam_time *= 0.5f;
        }
        
        if (roam_time >= max_roam_time){
            bird->roaming = false;
            bird->charging = true;
            bird->charging_start_time = core.time.game_time;
            //bird->velocity = Vector2_zero;
        }
    }
    
    if (bird->charging){
        f32 charging_time = core.time.game_time - bird->charging_start_time;
        if (charging_time >= bird->max_charging_time){
            f32 time_since_last_bird_attacked = core.time.game_time - context.last_bird_attack_time;
            
            if (time_since_last_bird_attacked >= 0.2f){
                //bird start attack
                context.last_bird_attack_time = core.time.game_time;
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
        f32 attack_time = core.time.game_time - bird->attack_start_time;
        
        if (attack_time >= bird->max_attack_time){
            bird->attacking = false;
            bird->roaming = true;
            bird->roam_start_time = core.time.game_time;
            bird->attack_emitter->enabled = false;
        } 
    }
    
    f32 bird_speed = magnitude(bird->velocity);
    
    f32 time_since_birth = core.time.game_time - enemy->birth_time;
    
    //update bird
    if (bird->roaming){
        f32 roam_time = core.time.game_time - bird->roam_start_time;
        f32 roam_t = roam_time / bird->max_roam_time;
    
        if (roam_t <= 0.2f && time_since_birth > 5){
            rotate(entity, bird_speed * 0.2f * normalized(bird->velocity.x));
            bird->velocity = move_towards(bird->velocity, Vector2_zero, bird_speed * 0.8f, dt);
        } else{
            f32 distance_t = clamp01(distance_to_player / 300.0f);
            f32 acceleration = lerp(bird->roam_acceleration * 0.5f, bird->roam_acceleration, distance_t * distance_t);
            f32 max_speed = lerp(bird->max_roam_speed * 0.5f, bird->max_roam_speed, distance_t);
        
            Vector2 target_position = player_entity->position + Vector2_up * 120;        
            if (bird->slot_index == -1){
                for (int i = 0; i < MAX_BIRD_POSITIONS; i++){
                    Bird_Slot *slot = &context.bird_slots[i];
                    
                    if (!slot->occupied){
                        slot->occupied = true;
                        bird->slot_index = i;
                        break;
                    }
                }
            }
            
            if (bird->slot_index != -1){
                target_position = player_entity->position + bird_formation_positions[bird->slot_index];
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
        bird_clear_formation(bird);
        
        f32 charging_time = core.time.game_time - bird->charging_start_time;
        f32 t = charging_time / bird->max_charging_time;
        
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
        bird_clear_formation(bird);
    
        f32 speed = magnitude(bird->velocity);
        change_up(entity, move_towards(entity->up, dir_to_player, 2, dt));
        bird->velocity = entity->up * speed;
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
    } else{
        assert(false);
        //what a state
    }
    
    bird->trail_emitter->direction = entity->up * -1;
}

inline f32 get_explosion_radius(Entity *entity, f32 base_radius = 40){
    f32 scale_sum = (entity->scale.x * 0.5f + entity->scale.y * 0.5f);
    f32 scale_progress = clamp01(scale_sum / 200.0f);
    
    return lerp(base_radius, base_radius * 6, scale_progress);
}

void kill_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, f32 particles_speed_modifier){
    assert(enemy_entity->flags & ENEMY);
    
    f32 hitmark_scale = 1;
    Color hitmark_color = WHITE;
    
    if (!enemy_entity->enemy.dead_man){
        enemy_entity->enemy.stun_start_time = core.time.game_time;
        f32 count = player_data.is_sword_big ? 3 : 1;
        f32 area = player_data.is_sword_big ? 3 : 1;
        emit_particles(*blood_emitter, kill_position, kill_direction, count, particles_speed_modifier, area);
    
        enemy_entity->enemy.dead_man = true;
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
            emit_particles(explosion_emitter, enemy_entity->position, Vector2_up, get_explosion_radius(enemy_entity) / 40.0f);
            play_sound(enemy_entity->enemy.big_explosion_sound, enemy_entity->position);
            
            f32 explosion_add_speed = 80;
            ForTable(context.entities, i){
                Entity *other_entity = context.entities.get_ptr(i);
                Vector2 vec_to_other = other_entity->position - enemy_entity->position;
                f32 distance_to_other = magnitude(vec_to_other);
                
                if (distance_to_other > explosion_radius){
                    continue;
                }
                
                Vector2 dir_to_other = normalized(vec_to_other);
                
                if (other_entity->flags & ENEMY){
                    if (!other_entity->enemy.dead_man){
                        stun_enemy(other_entity, other_entity->position, dir_to_other, true);
                    }
                    
                    if (other_entity->flags & BIRD_ENEMY){
                        other_entity->bird_enemy.velocity += dir_to_other * explosion_add_speed;
                    }
                }
                
                if (other_entity->flags & PHYSICS_OBJECT){
                    other_entity->physics_object.velocity += (dir_to_other * explosion_add_speed * (explosion_radius * 0.1f)) / other_entity->physics_object.mass;
                }
                
                if (other_entity->flags & PLAYER && !player_data.dead_man && distance_to_other < explosion_radius * 0.75f){
                    kill_player();
                }
            }
            
            add_hitstop(0.1f);
            shake_camera(0.5f);
        }
        
        b32 is_hitmark_follow = false;
        
        if (enemy_entity->flags & (CENTIPEDE_SEGMENT | TRIGGER)){
            is_hitmark_follow = true;
        }
        
        add_hitmark(enemy_entity, is_hitmark_follow, hitmark_scale, hitmark_color); 
    }
}

inline b32 is_enemy_can_take_damage(Entity *enemy_entity){
    assert(enemy_entity->flags & ENEMY);

    if (enemy_entity->flags & CENTIPEDE_SEGMENT && enemy_entity->enemy.dead_man){
        return false;
    }
    
    b32 recently_got_hit = core.time.game_time - enemy_entity->enemy.stun_start_time <= 0.05f;
    return !recently_got_hit;
}

void agro_enemy(Entity *entity){
    if (entity->enemy.in_agro){
        return;
    }

    entity->enemy.in_agro = true;
    
    if (entity->flags & SHOOT_STOPER){
        context.shoot_stopers_count++;
    }
}

void destroy_enemy(Entity *entity){
    entity->destroyed = true;
    
    if (entity->flags & SHOOT_STOPER){
        context.shoot_stopers_count--;
        assert(context.shoot_stopers_count >= 0);
    }
}

void stun_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction, b32 serious){
    assert(enemy_entity->flags & ENEMY);
    
    Enemy *enemy = &enemy_entity->enemy;
    
    if (enemy_entity->flags & EXPLOSIVE){
        kill_enemy(enemy_entity, kill_position, kill_direction);
        return;
    }
    
    if (is_enemy_can_take_damage(enemy_entity)){
        if (enemy_entity->flags & MOVE_SEQUENCE && !(enemy_entity->flags & CENTIPEDE_SEGMENT)){
            enemy_entity->move_sequence.moving = false;
        }
        agro_enemy(enemy_entity);
    
        enemy->stun_start_time = core.time.game_time;
        enemy->hits_taken++;
        b32 should_die_in_one_hit = enemy_entity->flags & BIRD_ENEMY && enemy_entity->bird_enemy.attacking;
        if (enemy->hits_taken >= enemy->max_hits_taken || serious || should_die_in_one_hit){
            f32 area_multiplier = serious ? 3 : 1;
            f32 count = serious ? 3 : 1;
            f32 speed = serious ? 3 : 1;
            emit_particles(*blood_emitter, kill_position, kill_direction, count, speed, area_multiplier);
        
            enemy->dead_man = true;
            
            if (enemy_entity->flags & BIRD_ENEMY){
                enable_emitter(enemy_entity->bird_enemy.fire_emitter);
            } else if (enemy_entity->flags & CENTIPEDE_SEGMENT){
                
            } else{
                destroy_enemy(enemy_entity);
            }
        } else{
            enemy->stun_start_time = core.time.game_time;
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
    Vector2 real_scale = {(f32)texture.width / UNIT_SIZE, (f32)texture.height / UNIT_SIZE};
    
    return {wish_scale.x / real_scale.x, wish_scale.y / real_scale.y};
}

void add_hitmark(Entity *entity, b32 need_to_follow, f32 scale_multiplier, Color tint){
    Entity *hitmark = add_entity(entity->position, transform_texture_scale(hitmark_small_texture, {10 * scale_multiplier, 10 * scale_multiplier}), {0.5f, 0.5f}, rnd(-90.0f, 90.0f), hitmark_small_texture, TEXTURE | STICKY_TEXTURE);
    hitmark->need_to_save = false;
    //hitmark->color = WHITE;
    init_entity(hitmark);    
    change_color(hitmark, tint);
    hitmark->draw_order = 1;
    str_copy(hitmark->name, "hitmark_small");
    //hitmark->texture = hitmark_small_texture;
    
    hitmark->sticky_texture.texture_position = entity->position;
    hitmark->sticky_texture.need_to_follow = need_to_follow;
    hitmark->sticky_texture.follow_id = entity->id;
    hitmark->sticky_texture.birth_time = core.time.game_time;
    hitmark->sticky_texture.max_distance = 1000;
}

inline b32 compare_difference(f32 first, f32 second, f32 allowed_difference = EPSILON){
    return abs(first - second) <= allowed_difference;
}

void calculate_projectile_collisions(Entity *entity){
    Projectile *projectile = &entity->projectile;
    
    if (projectile->flags & PLAYER_RIFLE){
        fill_collisions(entity, &player_data.collisions, GROUND | ENEMY | WIN_BLOCK | ROPE_POINT);
        
        Player *player = &player_data;
        
        for (int i = 0; i < player->collisions.count; i++){
            Collision col = player->collisions.get(i);
            Entity *other = col.other_entity;
            
            b32 need_bounce = false;
            
            Vector2 velocity_dir = normalized(projectile->velocity);
            f32 sparks_speed = 1;
            f32 sparks_count = 1;
            f32 hitstop_add = 0;
            
            if (other->flags & ENEMY && is_enemy_can_take_damage(other) && (projectile->type != WEAK || !projectile->dying)){
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
                        other->enemy.stun_start_time = core.time.game_time;
                        play_sound("ShootBlock", col.point);
                    }
                }
                
                if (other->flags & WIN_BLOCK && can_damage){
                    win_level();
                } else if (other->flags & BIRD_ENEMY && can_damage){
                    other->bird_enemy.velocity += projectile->velocity * 0.05f;
                    //other->bird_enemy.velocity = projectile->velocity * 0.05f;
                    projectile->velocity = reflected_vector(projectile->velocity * 0.3f, col.normal);
                    projectile->type = WEAK;
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
            
            
            } else if (other->flags & ENEMY && projectile->type == WEAK){
                need_bounce = true;
            }
            
            if (other->flags & PHYSICS_OBJECT){
                // other->physics_object.velocity += (projectile->velocity * 0.5f * dot(normalized(projectile->velocity), col.normal * -1.0f)) / other->physics_object.mass;
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
        }
    } else if (projectile->flags & JUMP_SHOOTER_PROJECTILE){
        fill_collisions(entity, &collisions_data, GROUND | PLAYER);
        
        for (int i = 0; i < collisions_data.count; i++){
            Collision col = collisions_data.get(i);
            Entity *other = col.other_entity;
            
            
        }
    }
}

void update_projectile(Entity *entity, f32 dt){
    assert(entity->flags & PROJECTILE);
    
    Projectile *projectile = &entity->projectile;
    
    //projectile->lifetime += dt;
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
                
                //clamp_magnitude(&projectile->velocity, lerp(player_data.rifle_weak_speed, 60.0f, clamp01(sqrtf(life_overshoot))));
                clamp_magnitude(&projectile->velocity, 60);
                projectile->velocity.y -= player_data.gravity * dt;
            }
        }
    }
    
    if (projectile->flags & JUMP_SHOOTER_PROJECTILE){
        if (lifetime >= projectile->max_lifetime * 0.6f){
            projectile->velocity *= 1.0f - (dt * 4);
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

void update_sticky_texture(Entity *real_entity, f32 dt){
    Sticky_Texture *st = &real_entity->sticky_texture;
    
    b32 need_to_follow = st->need_to_follow && context.entities.has_key(st->follow_id) && context.entities.get_by_key_ptr(st->follow_id)->enabled;
    f32 lifetime = core.time.game_time - st->birth_time;
    f32 lifetime_t = 0;
    if (st->max_lifetime > EPSILON){
        lifetime_t = lifetime / st->max_lifetime;
        if (lifetime >= st->max_lifetime){
            real_entity->destroyed = true;
        } else{
            real_entity->color = lerp(real_entity->color_changer.start_color, Fade(WHITE, 0), EaseOutExpo(lifetime_t));
        }
    }
    
    if (need_to_follow){
        Entity *follow_entity = context.entities.get_by_key_ptr(st->follow_id);
        // real_entity->position = lerp(real_entity->position, follow_entity->position, dt * 40);
        Vector2 target_position = follow_entity->position;
        if (follow_entity->flags & SHOOT_STOPER){
            target_position = get_shoot_stoper_cross_position(follow_entity);
        }
        real_entity->sticky_texture.texture_position = lerp(real_entity->sticky_texture.texture_position, target_position, dt * 40);
    } else if (st->max_lifetime <= EPSILON){
        real_entity->destroyed = true;
    }
    real_entity->position = player_entity->position;
    st->need_to_follow = need_to_follow;
}

void trigger_verify_connected(Entity *e){
    for (int i = 0; i < e->trigger.connected.count; i++){   
        //So if entiity was somehow destoyed, annighilated
        if (!context.entities.has_key(e->trigger.connected.get(i))){
            e->trigger.connected.remove(i);
            i--;
            continue;
        }
    }
}

void update_editor_entity(Entity *e){
    if (e->flags & TRIGGER){
        trigger_verify_connected(e);
    }
    
    if (e->flags & PHYSICS_OBJECT){
        if (e->physics_object.on_rope && core.time.app_time - e->physics_object.last_pick_rope_point_time > 0.5f){
            Collision ray_col = raycast(e->position + e->up * e->scale.y * 0.5f, e->up, 300, GROUND, 4, e->id);
            if (ray_col.collided){
                e->physics_object.rope_point = ray_col.point;
            }
            e->physics_object.last_pick_rope_point_time = core.time.app_time;
        }
    }
}

void trigger_entity(Entity *trigger_entity, Entity *connected){
    connected->hidden = !trigger_entity->trigger.shows_entities;

    if (connected->flags & ENEMY && debug.enemy_ai && trigger_entity->trigger.agro_enemies){
        agro_enemy(connected);
    }
    
    if (connected->flags & DOOR && connected->door.is_open != trigger_entity->trigger.open_doors){
        activate_door(connected, trigger_entity->trigger.open_doors);
    }
    
    if (connected->flags & MOVE_SEQUENCE){
        connected->move_sequence.moving = trigger_entity->trigger.starts_moving_sequence;
    }
}

void update_trigger(Entity *e){
    assert(e->flags & TRIGGER);
    
    b32 trigger_now = false;
    
    if (e->flags & ENEMY && e->enemy.dead_man){
        trigger_now = true;
        e->enemy.dead_man = false;
    }
    
    if (e->trigger.track_enemies){
        b32 found_enemy = false;
        for (int i = 0; i < e->trigger.tracking.count; i++){
            i32 id = e->trigger.tracking.get(i);
            if (!context.entities.has_key(id)){
                e->trigger.tracking.remove(i);
                i--;
                continue;
            }
            
            Entity *tracking_entity = context.entities.get_by_key_ptr(id);

            if (!tracking_entity->enemy.dead_man){
                found_enemy = true;
                break;
            }
        }
        
        // if (e->flags & ENEMY && !e->enemy.dead_man){
        //     found_enemy = true;
        // } else if (e->flags & ENEMY && e->enemy.dead_man){
        //     e->enemy.dead_man = false;
        // }
        
        if (!found_enemy){
            trigger_now = true;            
        }
    }
    
    if (trigger_now || e->trigger.player_touch && check_entities_collision(e, player_entity).collided){
        if (e->trigger.load_level){
            enter_game_state_on_new_level = true;
            last_player_data = player_data;
            load_level_by_name(e->trigger.level_name);
            return;
        }
        
        if (e->trigger.play_sound && !e->trigger.triggered){
            //enter_game_state_on_new_level = true;
            //load_level_by_name(e->trigger.level_name);
            play_sound(e->trigger.sound_name);
        }
        
        if (e->trigger.change_zoom){
            context.cam.target_zoom = e->trigger.zoom_value;
        }
        
        if (e->trigger.unlock_camera){
            context.cam.locked = false;
        } else if (e->trigger.lock_camera){
            context.cam.locked = true;
            context.cam.target = e->trigger.locked_camera_position;
        }
    
        if (e->flags & DOOR){
            trigger_entity(e, e);
        }
    
        if (e->trigger.kill_player && !debug.god_mode){
            kill_player();
        }
        
        for (int i = 0; i < e->trigger.connected.count; i++){
            i32 id = e->trigger.connected.get(i);
            if (!context.entities.has_key(id)){
                e->trigger.connected.remove(i);
                i--;
                continue;
            }
            
            Entity *connected_entity = context.entities.get_by_key_ptr(id);
                        
            trigger_entity(e, connected_entity);
        }
        
        e->trigger.triggered = true;
    }
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
    add_rect_vertices(&vertices, {0.5f, 0.5f});    
    for (int i = 0; i < vertices.count; i++){
        *vertices.get_ptr(i) *= 2.0f * radius;
    }
    
    ForEntities(entity, GROUND){
        Collision col = check_collision(point, entity->position, vertices, entity->vertices, {0.5f, 0.5f}, entity->pivot);
        if (col.collided){
            col.other_entity = entity;
            return col;
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
    // Vector2 current = sequence->points.get(sequence->current_index);
    
    if (sequence->current_index >= sequence->points.count-1 && !sequence->loop){
        target = sequence->points.last();
    }
    
    if (sequence->just_born){
        if (entity->flags & JUMP_SHOOTER){
            Jump_Shooter *shooter = &entity->jump_shooter;
            shooter->move_points.clear();
            for (int i = 0; i < sequence->points.count; i++){
                Vector2 point = sequence->points.get(i);
                Collision nearest_ground = get_nearest_ground_collision(point, 20);
                            
                if (nearest_ground.collided){
                    Vector2 point_to_collision = nearest_ground.point - point;
                    Vector2 dir = normalized(point_to_collision);
                    f32 len = magnitude(point_to_collision);
                    Collision ray_collision = raycast(point, dir, len, GROUND, 1); 
                    
                    if (ray_collision.collided){
                        shooter->move_points.add({ray_collision.point, ray_collision.normal});
                        print(ray_collision.point);
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
            sequence->velocity = normalized(target - entity->position) * sequence->speed;
            sequence->wish_position = entity->position;
        }
        
        sequence->just_born = false;
    }
        
    if (entity->flags & JUMP_SHOOTER){
        Jump_Shooter *shooter = &entity->jump_shooter;   
        // f32 time_since_jump = core.time.game_time - shooter->last_jump_time;
        
        
    } else{
        Vector2 previous_position = entity->position;
        sequence->wish_position = move_towards(sequence->wish_position, target, sequence->speed, dt);
        
        if (!sequence->loop && sequence->current_index >= sequence->points.count - 2){
            entity->position = move_towards(entity->position, sequence->wish_position, sequence->speed, dt);
        } else{
            Vector2 wish_vec = sequence->wish_position - entity->position;
            f32 wish_len = magnitude(wish_vec);
            if (wish_len > 0){
                sequence->wish_velocity = (wish_vec / wish_len) * sequence->speed;
                sequence->velocity = move_towards(sequence->velocity, sequence->wish_velocity, sequence->speed * 4, dt);
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

void update_entities(f32 dt){
    Context *c = &context;
    Hash_Table_Int<Entity> *entities = &c->entities;
    
    for (int entity_index = 0; entity_index < entities->max_count; entity_index++){
        if (!entities->has_index(entity_index)){
            continue;
        }
    
        Entity *e = entities->get_ptr(entity_index);
        
        //assert(e->flags > 0);
        
        if (e->flags & PLAYER){
            if (need_destroy_player){
                destroy_player();   
                need_destroy_player = false;
            }
        }
        
        if (e->destroyed){
            //@TODO properly destroy different entities
            //e->enabled = false;
            
            if (e->flags & PLAYER){
                
            }
            
            free_entity(e);
            entities->remove_index(entity_index);    
            //i--;
            
            if (game_state == EDITOR || game_state == PAUSE){
                editor.need_validate_entity_pointers = true;
            }
            continue;
        }
        
        if (e->enabled && game_state == GAME && e->spawn_enemy_when_no_ammo && player_data.ammo_count <= 0 && (!context.entities.has_key(e->spawned_enemy_id) || e->spawned_enemy_id == -1)){
            Entity *spawned = spawn_object_by_name("enemy_base", e->position);
            spawned->enemy.gives_full_ammo = true;
            e->spawned_enemy_id = spawned->id;
        }
        
        if (!e->enabled || (e->hidden && game_state == GAME)){
            continue;
        }
        
        update_color_changer(e, dt);            
        
        if (e->flags & PHYSICS_OBJECT){
             if (e->physics_object.on_rope){
                // spawn rope and update it
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
        
        if (game_state == EDITOR || game_state == PAUSE){
            if (game_state == EDITOR){
                update_editor_entity(e);
            }
            continue;
        }
        
        if (e->flags & PLAYER){
            if (IsKeyDown(KEY_G)){
                e->position = input.mouse_position;
            } else{
                update_player(e, dt);
            }
            // player_data.stun_emitter.position = e->position;
            // update_emitter(&player_data.stun_emitter);
        }
          
        if (e->flags & BIRD_ENEMY && debug.enemy_ai){
            update_bird_enemy(e, dt);
        }
        
        if (e->flags & PROJECTILE){
            update_projectile(e, dt);
        }
        
        //if (e->flags & PARTICLE_EMITTER){
        for (int em = 0; em < e->emitters.count; em++){
            if (e->emitters.get_ptr(em)->follow_entity){
                e->emitters.get_ptr(em)->position = e->position;
            }
            update_emitter(e->emitters.get_ptr(em));
        }
        //}
        
        if (e->flags & STICKY_TEXTURE){
            update_sticky_texture(e, dt);
        }
        
        if (e->flags & TRIGGER){
            update_trigger(e);
            // if (e->id == -1){ // this means that trigger loads level and destroyed all that we cared about
            //     continue;
            // }
        }
        
        if (e->flags & MOVE_SEQUENCE){
            update_move_sequence(e, dt);
        }
        
        if (e->flags & PHYSICS_OBJECT){
            //update physics object 
            
            // if (e->physics_object.on_rope && 
            
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
        }
        
        if (e->flags & CENTIPEDE && debug.enemy_ai && !e->enemy.dead_man){
            // update centipede
            Centipede *centipede = &e->centipede;
            
            i32 alive_count = 0;
            for (int i = 0; i < centipede->segments_count; i++){
                Entity *segment = context.entities.get_by_key_ptr(centipede->segments_ids.get(i));
                
                if (!segment->enemy.dead_man){
                    alive_count++;
                }
            }
            
            if (alive_count == 0){
                e->enemy.dead_man = true;
                e->flags = ENEMY | BIRD_ENEMY;
                Vector2 rnd = rnd_in_circle();
                e->bird_enemy.velocity = {e->move_sequence.velocity.x * rnd.x, e->move_sequence.velocity.y * rnd.y};

                e->move_sequence.moving = false;
                e->collision_flags = GROUND;
                init_bird_emitters(e);
                
                for (int i = 0; i < centipede->segments_count; i++){
                    Entity *segment = context.entities.get_by_key_ptr(centipede->segments_ids.get(i));
                    
                    segment->volume_multiplier = 0.3f;
                    segment->flags = ENEMY | BIRD_ENEMY;
                    segment->move_sequence.moving = false;
                    segment->collision_flags = GROUND;
                    Vector2 rnd = rnd_in_circle();
                    segment->bird_enemy.velocity = {segment->move_sequence.velocity.x * rnd.x, segment->move_sequence.velocity.y * rnd.y};
                    init_bird_emitters(segment);
                }
            }
            // // end update centipede end
        }

        if (e->flags & JUMP_SHOOTER && /*e->enemy.in_agro && */debug.enemy_ai){
            // update jump shooter
            Jump_Shooter *shooter = &e->jump_shooter;
            
            Vector2 vec_to_player = player_entity->position - e->position;
            Vector2 dir_to_player = normalized(vec_to_player);
            
            if (shooter->standing){
                f32 standing_time = core.time.game_time - shooter->standing_start_time;
                f32 max_standing_time = 2.0f;
                
                // squizing animation
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
                    shooter->standing = false;
                    shooter->jumping = true;
                    shooter->jump_start_time = core.time.game_time;
                    
                    shooter->velocity = e->up * 200;
                }
            }
            
            if (shooter->jumping){
                f32 jumping_time = core.time.game_time - shooter->jump_start_time;
                f32 max_jumping_time = 1.5f;
                f32 jump_t = clamp01((jumping_time / max_jumping_time));
                
                // salto here
                
                f32 gravity_multiplier = e->up.y > 0 ? lerp(3.0f, 2.0f, jump_t * jump_t) : lerp(-2.0f, 0.0f, jump_t * jump_t);
                shooter->velocity.y -= GRAVITY * gravity_multiplier * dt;
                shooter->velocity.x = lerp(shooter->velocity.x, 0.0f, jump_t * dt * 6);
                
                if (jumping_time >= max_jumping_time || (jumping_time >= max_jumping_time * 0.5f && shooter->velocity.y < 40)){
                    shooter->jumping = false;
                    shooter->charging = true;
                    shooter->charging_start_time = core.time.game_time;
                }
            }
            
            if (shooter->charging){
                f32 charging_time = core.time.game_time - shooter->charging_start_time;
                f32 charging_t = clamp01(charging_time / shooter->max_charging_time);
                
                // shooter->velockity *= 1.0f - (lerp(0.0f, 4.0f * dt, charging_t * charging_t));
                move_vec_towards(&shooter->velocity, Vector2_zero, lerp(0.0f, 100.0f, sqrtf(charging_t)), dt);
                shooter->velocity.x = lerp(shooter->velocity.x, 0.0f, charging_t * dt * 5);
                
                f32 look_speed = lerp(0.0f, 10.0f, charging_t * charging_t);
                change_right(e, move_towards(e->right, dir_to_player.x > 0 ? dir_to_player : dir_to_player * -1, look_speed, dt));
                
                // jump shooter shoot
                if (charging_time >= shooter->max_charging_time){
                    f32 spread = 45;
                    f32 angle = -spread * 0.5f;
                    f32 angle_step = spread / shooter->shots_count;
                    
                    for (int i = 0; i < shooter->shots_count; i++){
                        Vector2 direction = get_rotated_vector(dir_to_player, angle);
                        angle += angle_step;
                        f32 speed = 100;
                        
                        Entity *projectile_entity = add_entity(e->position, {2, 4}, {0.5f, 0.5f}, 0, PROJECTILE | ENEMY);
                        projectile_entity->projectile.birth_time = core.time.game_time;
                        projectile_entity->projectile.flags = JUMP_SHOOTER_PROJECTILE;
                        projectile_entity->projectile.velocity = direction * speed;
                    }
                    
                    shooter->velocity = dir_to_player * -30 + Vector2_up * 100;
                    
                    shooter->charging = false;
                    shooter->in_recoil = true;
                    shooter->recoil_start_time = core.time.game_time;
                }
            }
            
            if (shooter->in_recoil){
                f32 in_recoil_time = core.time.game_time - shooter->recoil_start_time;
                f32 max_recoil_time = 1.0f;
                
                //rotate here
                f32 gravity_multiplier = shooter->velocity.y > 0 ? 1.5f : 0.7f;
                shooter->velocity.y -= GRAVITY * gravity_multiplier * dt;
                
                if (in_recoil_time >= max_recoil_time){
                    shooter->in_recoil = false;
                    shooter->picking_point = true;
                    shooter->picking_point_start_time = core.time.game_time;
                }
            }
            
            if (shooter->picking_point){
                f32 picking_point_time = core.time.game_time - shooter->picking_point_start_time;
                f32 max_picking_point_time = 1.5f;
                f32 picking_point_t = clamp01(picking_point_time / max_picking_point_time);
                
                Move_Point next_point = shooter->move_points.get((shooter->current_index + 1) % shooter->move_points.count);
                print(next_point.position);
                
                Vector2 vec_to_point = next_point.position - e->position;
                Vector2 dir = normalized(vec_to_point);
                
                // look at next target here (and shake on drawing like birdies)
                move_vec_towards(&shooter->velocity, Vector2_zero, lerp(0.0f, 100.0f, sqrtf(picking_point_t)), dt);
                
                f32 look_speed = lerp(0.0f, 10.0f, picking_point_t * picking_point_t);
                change_up(e, move_towards(e->up, dir, look_speed, dt));
                
                // jump shooter fly to next
                if (picking_point_time >= max_picking_point_time){
                    shooter->picking_point = false;
                    shooter->flying_to_point = true;
                    shooter->flying_start_time = core.time.game_time;
                    
                    shooter->velocity = dir * 200;
                }
            }
            
            if (shooter->flying_to_point){
                // when we fly we just wait for ground collision to change state and if it took too long - i think we sould die
                
                // rotate by normal of point
                Move_Point target_point = shooter->move_points.get((shooter->current_index + 1) % shooter->move_points.count);
                
                Vector2 vec_to_point = target_point.position - e->position;
                Vector2 dir = normalized(vec_to_point);
            }
            
            move_by_velocity_with_collisions(e, shooter->velocity, e->scale.x * 0.5f + e->scale.y * 0.5f, &respond_jump_shooter_collision, dt);
        } // end update jump shooter
        
        if (e->flags & DOOR){
            update_door(e);
        }
    }
}

Vector2 move_by_velocity(Vector2 position, Vector2 target, Velocity_Move* settings, f32 dt){
    Vector2 vec = target - position;
    Vector2 dir = normalized(vec);
    f32     len = magnitude(vec);
    
    f32 velocity_dot = dot(dir, settings->velocity);
    
    f32 acceleration = velocity_dot >= 0 ? settings->acceleration : settings->deceleration;
    f32 damping      = velocity_dot >= 0 ? settings->accel_damping : settings->decel_damping;
    
    settings->velocity += dir * acceleration * dt;
    //settings->velocity *= (1.0f - damping * dt);
    clamp_magnitude(&settings->velocity, settings->max_speed);
    
    return position + settings->velocity * dt;
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
}

inline Vector2 get_perlin_in_circle(f32 speed){
    return {perlin_noise3_seed(core.time.game_time * speed, 1, 2, rnd(0, 10000)), perlin_noise3_seed(1, core.time.game_time * speed, 3, rnd(0, 10000))};
}

void draw_sword(Entity *entity){
    assert(entity->flags & SWORD);
    
    Entity visual_entity = *entity;
    
    f32 time_since_shake = core.time.game_time - player_data.rifle_shake_start_time;
    
    if (0 && player_data.rifle_active){
        f32 activated_time = core.time.game_time - player_data.rifle_activate_time;
        f32 activate_t = clamp01(activated_time / player_data.rifle_max_active_time);
        
        Vector2 perlin_rnd = {perlin_noise3(core.time.game_time * 30, 1, 2), perlin_noise3(1, core.time.game_time * 30, 3)};
        
        //visual_entity.position += rnd_in_circle() * lerp(0.2f, 1.3f, activate_t * activate_t);
        visual_entity.position += perlin_rnd * lerp(0.2f, 1.3f, activate_t * activate_t);
    } else if (time_since_shake <= 0.2f){
        //Failed chainsaw start sound!!!!!
        Vector2 perlin_rnd = {perlin_noise3(core.time.game_time * 30, 1, 2), perlin_noise3(1, core.time.game_time * 30, 3)};
        //visual_entity.position += rnd_on_circle() * 0.8f;
        visual_entity.position += perlin_rnd * 1.8f;
    }
    
    if (player_data.rifle_active){
        visual_entity.color = ColorBrightness(GREEN, 0.3f);
    }
    
    draw_game_triangle_strip(&visual_entity);
}

void draw_enemy(Entity *entity){
    assert(entity->flags & ENEMY);
    
    draw_game_triangle_strip(entity);
}

void draw_bird_enemy(Entity *entity){
    assert(entity->flags & BIRD_ENEMY);
    
    Entity visual_entity = *entity;
    if (entity->bird_enemy.charging){
        f32 charge_time = core.time.game_time - entity->bird_enemy.charging_start_time;
        f32 charging_progress = charge_time / entity->bird_enemy.max_charging_time;
        visual_entity.position += get_perlin_in_circle(30) * lerp(0.0f, 1.0f, charging_progress * charging_progress);
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
    cam_bounds.size /= UNIT_SIZE;
    
    cam_bounds.offset = {0, 0};
    
    return cam_bounds;
}

void fill_entities_draw_queue(){
    context.entities_draw_queue.clear();
    
    ForTable(context.entities, i){
        Entity *e_ptr = context.entities.get_ptr(i);
        Entity e = context.entities.get(i);
        
        if (!e_ptr || !e.enabled){
            continue;
        }
        
        //now we want entities that have external lines
        b32 is_should_draw_anyway = e.flags & (TRIGGER | MOVE_SEQUENCE);
        
        if (e.hidden && game_state == GAME && !is_should_draw_anyway){
            continue;
        }
        
        Bounds cam_bounds = get_cam_bounds(context.cam, context.cam.cam2D.zoom);
        if (!is_should_draw_anyway && !check_bounds_collision(context.cam.view_position, cam_bounds, &e)){
            e_ptr->visible = false;
            continue;
        } else{
            e_ptr->visible = true;
        }
        
        context.entities_draw_queue.add(e);
    }
    
    qsort(context.entities_draw_queue.data, context.entities_draw_queue.count, sizeof(Entity), compare_entities_draw_order);
}

#define MAX_LINE_STRIP_POINTS 1024

Array<Vector2, MAX_LINE_STRIP_POINTS> line_strip_points;

void draw_spikes(Entity *e, Vector2 side_direction, Vector2 up_direction, f32 width, f32 height){
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

void draw_entities(){
    fill_entities_draw_queue();

    //Hash_Table_Int<Entity> *entities = &context.entities;
    Dynamic_Array<Entity> *entities = &context.entities_draw_queue;
    
    for (int entity_index = 0; entity_index < entities->count; entity_index++){
        // if (!entities->has_index(i)){
        //     continue;
        // }
        
        Entity *e = entities->get_ptr(entity_index);
        if (!e || !context.entities.has_key(e->id)){
            continue;
        }
    
        if (!e->enabled/* || e->flags == -1*/){
            continue;
        }
        
        if (e->flags & TEXTURE){
            // draw texture
            Vector2 position = e->position;
            if (e->flags & STICKY_TEXTURE){
                position = e->sticky_texture.texture_position;
                e->scale = ((Vector2){3, 3}) / context.cam.cam2D.zoom; 
            }
            draw_game_texture(e->texture, position, e->scale, e->pivot, e->rotation, e->color);
        }
        
        if (e->flags & GROUND || e->flags & PLATFORM || e->flags == 0 || e->flags & PROJECTILE){
            // draw ground
            if (e->vertices.count > 0){
                draw_game_triangle_strip(e);
            } else{
                draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
            }
        }
        
        if (e->flags & PHYSICS_OBJECT){
            // draw physics object
            if (e->physics_object.on_rope && game_state == EDITOR){
                Vector2 start_point = e->position + e->up * e->scale.y * 0.5f;
                draw_game_line(start_point, e->physics_object.rope_point, 1, BLACK);
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
            draw_game_triangle_strip(e);
            draw_game_line_strip(e, SKYBLUE);
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
            Vector2 spikes_direction = e->centipede_head->centipede.spikes_on_right ? e->right : (e->right * -1.0f);
            draw_spikes(e, e->up, spikes_direction, e->scale.y, e->scale.x);
            if (!e->enemy.dead_man){
                draw_game_circle(e->position - spikes_direction * e->scale.x * 0.5f, e->scale.y * 0.4f, GREEN);
            }
        } else if (e->flags & CENTIPEDE){
            // draw centipede
            draw_game_triangle_strip(e);
        } else if (e->flags & JUMP_SHOOTER){
            // draw jump shooter
            
            Entity visual_entity = *e;
            if (e->jump_shooter.charging){
                f32 charging_time = core.time.game_time - e->jump_shooter.charging_start_time;
                f32 charging_t = charging_time / e->jump_shooter.max_charging_time;
                visual_entity.position += get_perlin_in_circle(30) * lerp(0.0f, 1.0f, charging_t * charging_t);
            }
    
            draw_game_triangle_strip(&visual_entity);
            
            for (int i = 0; i < e->jump_shooter.move_points.count; i++){
                draw_game_circle(e->jump_shooter.move_points.get(i).position, 3, PURPLE);
            }
        } else if (e->flags & ENEMY){
            draw_enemy(e);
        }
        
        if (e->flags & WIN_BLOCK){
            // draw_game_triangle_strip(e);
            // Vector2 line_from = e->position - e->up * e->win_block.kill_direction.y * e->scale.y * 0.5f - e->right * e->win_block.kill_direction.x * e->scale.x * 0.5f;
            // Vector2 line_to   = e->position + e->up * e->win_block.kill_direction.y * e->scale.y * 0.5f + e->right * e->win_block.kill_direction.x * e->scale.x * 0.5f;
            // draw_game_line(line_from, line_to, 0.8f, PURPLE * 0.9f);
        }
        
        if (e-> flags & DRAW_TEXT){
            draw_game_text(e->position, e->text_drawer.text, e->text_drawer.size, RED);
        }
        
        b32 should_draw_editor_hints = (game_state == EDITOR || game_state == PAUSE || debug.draw_areas_in_game);
        
        if (e->flags & TRIGGER){
            // draw trigger
            if (should_draw_editor_hints){
                draw_game_line_strip(e, e->color);
                draw_game_triangle_strip(e, Fade(e->color, 0.1f));
                
                // draw cam zoom trigger draw trigger zoom draw trigger cam
                if (e->trigger.change_zoom){
                    Bounds cam_bounds = get_cam_bounds(context.cam, e->trigger.zoom_value);
                    Vector2 position = e->position;
                    if (e->trigger.lock_camera){
                        position = e->trigger.locked_camera_position;
                    }
                    draw_game_rect_lines(position + cam_bounds.offset, cam_bounds.size, {0.5f, 0.5f}, 2.0f / (context.cam.cam2D.zoom), Fade(PINK, 0.4f));
                }
                
                if (e->trigger.lock_camera){
                    draw_game_circle(e->trigger.locked_camera_position, 2, PINK);
                }
            }
            
            b32 is_trigger_selected = editor.selected_entity && editor.selected_entity->id == e->id;
            for (int ii = 0; ii < e->trigger.connected.count; ii++){
                int id = e->trigger.connected.get(ii);
                if (!context.entities.has_key(id)){
                    e->trigger.connected.remove(ii);
                    ii--;
                    continue;
                }
                Entity *connected_entity = context.entities.get_by_key_ptr(id);
                
                if (connected_entity->flags & DOOR && ((e->flags ^ TRIGGER) > 0 || game_state != GAME)){
                    Color color = connected_entity->door.is_open == e->trigger.open_doors ? SKYBLUE : ORANGE;
                    f32 width = connected_entity->door.is_open == e->trigger.open_doors ? 1.0f : 0.2f;
                    draw_game_line(e->position, connected_entity->position, width, Fade(ColorBrightness(color, 0.2f), 0.6f));
                } else if (is_trigger_selected && should_draw_editor_hints){
                    draw_game_line(e->position, connected_entity->position, RED);
                }
            }
            for (int ii = 0; ii < e->trigger.tracking.count; ii++){
                int id = e->trigger.tracking.get(ii);
                if (!context.entities.has_key(id)){
                    e->trigger.tracking.remove(ii);
                    ii--;
                    continue;
                }
                Entity *connected_entity = context.entities.get_by_key_ptr(id);
                
                if (is_trigger_selected && should_draw_editor_hints){
                    draw_game_line(e->position, connected_entity->position, GREEN);
                }
            }
        }
        
        if (e->flags & MOVE_SEQUENCE && (game_state == EDITOR || game_state == PAUSE || debug.draw_areas_in_game)){
            //draw_game_line_strip(e->move_sequence.points.data, e->move_sequence.points.count, BLUE);
            for (int ii = 0; ii < e->move_sequence.points.count; ii++){
                Vector2 point = e->move_sequence.points.get(ii);
                
                Color color = editor.selected_entity && editor.selected_entity->id == e->id ? ColorBrightness(GREEN, 0.2f) : Fade(BLUE, 0.8f);
                
                if (IsKeyDown(KEY_LEFT_ALT)){
                    draw_game_circle(point, 1  * (0.4f / context.cam.cam2D.zoom), SKYBLUE);
                    draw_game_text(point - Vector2_up, TextFormat("%d", ii), 18 / context.cam.cam2D.zoom, RED);
                    
                    if (e->flags & JUMP_SHOOTER){
                        Collision nearest_ground = get_nearest_ground_collision(point, 20);
                        if (nearest_ground.collided){
                            Collision ray_collision = raycast(point, normalized(nearest_ground.point - point), magnitude(nearest_ground.point - point), GROUND, 1);
                            if (ray_collision.collided){
                                draw_game_line(ray_collision.point, ray_collision.point + ray_collision.normal * 5, GREEN);
                            }
                        } else{
                            draw_game_circle(point, 1 * (0.4f / context.cam.cam2D.zoom), RED);
                        }
                    }
                }
                if (ii < e->move_sequence.points.count - 1){
                    draw_game_line(point, e->move_sequence.points.get(ii+1), color);
                } else if (e->move_sequence.loop){
                    draw_game_line(point, e->move_sequence.points.get(0), color);
                }
            }
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
        
        if (e->flags & EXPLOSIVE){
            draw_game_circle(e->position, get_explosion_radius(e), Fade(ORANGE, 0.1f));
        }
        
        if (e->flags & PROPELLER && (game_state == EDITOR || game_state == PAUSE || debug.draw_areas_in_game)){
            draw_game_line_strip(e, e->color);
            draw_game_triangle_strip(e, e->color * 0.1f);
            
            // e->propeller.air_emitter->speed_multiplier = e->propeller.power / 50.0f;
            // e->propeller.air_emitter->spawn_offset = e->up * e->scale.y * 0.25;
            // e->propeller.air_emitter->spawn_area = {e->scale.x, e->scale.y * 0.5f};
            // e->propeller.air_emitter->direction        = e->up;
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
        
        if (e->flags & SHOOT_BLOCKER){
            if (e->enemy.shoot_blocker_immortal){
                draw_game_ring_lines(e->position, 3, 6, 8, WHITE);                
            } else{
                Vector2 direction = get_rotated_vector(e->enemy.shoot_blocker_direction, e->rotation);
                Vector2 start_position = e->position - direction * e->scale.x * 0.6f;
                Vector2 end_position   = e->position + direction * e->scale.x * 0.6f;
                draw_game_line(start_position, end_position, 1.5f, VIOLET);
            }
        }
        
        if (e->flags & STICKY_TEXTURE){
            // draw sticky texture
            f32 lifetime = core.time.game_time - e->sticky_texture.birth_time;
            f32 lifetime_t = 0.5f;
            if (e->sticky_texture.max_lifetime > EPSILON){
                lifetime_t = lifetime / e->sticky_texture.max_lifetime;
            }
        
            if (e->sticky_texture.need_to_follow && player_entity){
                Entity *follow_entity = context.entities.get_by_key_ptr(e->sticky_texture.follow_id);
                Color line_color = e->sticky_texture.line_color;
                if (follow_entity && follow_entity->flags & ENEMY && e->sticky_texture.max_lifetime > 0 && !(follow_entity->flags & SHOOT_STOPER)){
                    line_color = follow_entity->enemy.dead_man ? SKYBLUE : RED;
                }

                Vector2 vec_to_follow = e->sticky_texture.texture_position - player_entity->position;
                f32 len = magnitude(vec_to_follow);
                if (len <= e->sticky_texture.max_distance || e->sticky_texture.max_distance <= 0){
                    draw_game_line(player_entity->position, e->sticky_texture.texture_position, lerp(Fade(line_color, 0.8f), Fade(line_color, 0), lifetime_t * lifetime_t));
                }
            }
        }
        
        if (e->flags & TEST){
        }
        
        if (game_state == EDITOR || debug.draw_up_right){
            draw_game_line(e->position, e->position + e->right * 3, RED);
            draw_game_line(e->position, e->position + e->up    * 3, GREEN);
        }
        
        if (debug.draw_bounds || editor.selected_entity && (game_state == EDITOR || game_state == PAUSE) && e->id == editor.selected_entity->id){
            draw_game_rect_lines(e->position + e->bounds.offset, e->bounds.size, e->pivot, 2, GREEN);
            //draw_game_text(e->position, TextFormat("{%.2f, %.2f}", e->bounds.offset.x, e->bounds.offset.y), 22, PURPLE);
        }
    }
}

void draw_editor(){
    f32 closest_len = 1000000;
    Entity *closest = NULL;

    Hash_Table_Int<Entity> *entities = &context.entities;

    for (int i = 0; i < entities->max_count; i++){
        Entity *e = entities->get_ptr(i);
        
        if (!context.entities.has_index(i)){
            continue;
        }
    
        if (!e->enabled || e->flags == -1){
            continue;
        }
        
        draw_game_circle(editor.player_spawn_point, 3, BLUE);
        
        b32 draw_circles_on_vertices = IsKeyDown(KEY_LEFT_ALT);
        if (draw_circles_on_vertices){
            for (int v = 0; v < e->vertices.count; v++){
                draw_game_circle(global(e, e->vertices.get(v)), 1.0f * (0.4f / context.cam.cam2D.zoom), PINK);
                //draw unscaled vertices
                if (IsKeyDown(KEY_LEFT_SHIFT)){    
                    draw_game_circle(global(e, e->unscaled_vertices.get(v)), 1.0f * 0.4f, PURPLE);
                }
            }
        }
        
        if (debug.draw_position){
            draw_game_text(e->position + ((Vector2){0, -3}), TextFormat("POS:   {%.2f, %.2f}", e->position.x, e->position.y), 20, RED);
        }
        
        if (debug.draw_rotation){
            draw_game_text(e->position, TextFormat("%d", (i32)e->rotation), 20, RED);
        }
        
        if (debug.draw_scale){
            draw_game_text(e->position + ((Vector2){0, -6}), TextFormat("SCALE:   {%.2f, %.2f}", e->scale.x, e->scale.y), 20, RED);
        }
        
        if (debug.draw_directions){
            draw_game_text(e->position + ((Vector2){0, -6}), TextFormat("UP:    {%.2f, %.2f}", e->up.x, e->up.y), 20, RED);
            draw_game_text(e->position + ((Vector2){0, -9}), TextFormat("RIGHT: {%.2f, %.2f}", e->right.x, e->right.y), 20, RED);
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
            //Array<Vector2> normals = Array<Vector2>(e->vertices.count + 4);
            fill_arr_with_normals(&global_normals, e->vertices);
            
            for (int n = 0; n < global_normals.count; n++){
                Vector2 start = e->position + global_normals.get(n) * 4; 
                Vector2 end   = e->position + global_normals.get(n) * 8; 
                draw_game_line(start, end, 0.5f, PURPLE);
                draw_game_rect(end, {1, 1}, {0.5f, 0.5f}, PURPLE * 0.9f);
            }
            //global_normals.free_arr();
        }
    }
    
    if (editor.dragging_entity != NULL && closest){
        draw_game_line(editor.dragging_entity->position, closest->position, 0.1f, PINK);
    }
    
    //editor ruler drawing
    if (editor.ruler_active){
        draw_game_line(editor.ruler_start_position, input.mouse_position, 0.3f, BLUE * 0.9f);
        Vector2 vec_to_mouse = input.mouse_position - editor.ruler_start_position;
        f32 length = magnitude(vec_to_mouse);
        
        draw_game_text(editor.ruler_start_position + (vec_to_mouse * 0.5f), TextFormat("%.2f", length), 24, RED);
        draw_game_text(input.mouse_position + Vector2_up, TextFormat("{%.2f, %.2f}", input.mouse_position.x, input.mouse_position.y), 26, GREEN); 
        
    }
}

void draw_particles(){
    for (int i = 0; i < context.particles.max_count; i++){
        Particle particle = context.particles.get(i);
        if (!particle.enabled){
            continue;   
        }
        
        draw_game_rect(particle.position, particle.scale, {0.5f, 0.5f}, 0, particle.color);
    }
}

void draw_ui(const char *tag){
    int tag_len = str_len(tag);

    for (int i = 0; i < ui_context.elements.count; i++){
        Ui_Element element = ui_context.elements.get(i);
        
        if (tag_len > 0 && !str_equal(element.tag, tag)){
            continue;
        }
        
        if (element.ui_flags & UI_IMAGE){
            Ui_Image ui_image = element.ui_image;
            draw_rect(element.position, element.size, element.pivot, 0, element.color);
        }
    }
    for (int i = 0; i < ui_context.elements.count; i++){
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
        }
        
        if (element.ui_flags & UI_TEXT){
            Ui_Text ui_text = element.text;
            draw_text(ui_text.content, element.position, ui_text.font_size, ui_text.text_color);
        }
    }

    for (int i = 0; i < input_fields.count; i++){
        Input_Field input_field = input_fields.get(i);
        
        if (tag_len > 0 && !str_equal(input_field.tag, tag)){
            continue;
        }
        
        Color background_color = input_field.color;
        if (input_field.in_focus){
            background_color = Fade(ColorTint(background_color, ColorBrightness(SKYBLUE, 0.2f)), 1.5f);
            // f32 blink_speed = 4.0f;
            // color_multiplier = lerp(0.5f, 0.8f, (sinf(core.time.app_time * blink_speed) + 1) * 0.5f);
        }
        draw_rect(input_field.position, input_field.size, {0, 0}, 0, background_color);
        
        if (input_field.in_focus){
            draw_text(TextFormat("%s_", input_field.content), input_field.position + Vector2_right * 3, input_field.font_size, WHITE * 0.9f);
        } else{
            draw_text(input_field.content, input_field.position + Vector2_right * 3, input_field.font_size, WHITE * 0.9f);
        }
    }
    
    if (tag_len == 0){
        ui_context.elements.clear();
        input_fields.clear();
    }
}

void apply_shake(){
    if (context.cam.trauma <= 0){    
        return;
    }
    
    f32 x_shake_power = 10;
    f32 y_shake_power = 7;
    f32 x_shake_speed = 7;
    f32 y_shake_speed = 10;
    
    f32 x_offset = perlin_noise3(core.time.game_time * x_shake_speed, 0, 1) * x_shake_power;
    f32 y_offset = perlin_noise3(0, core.time.game_time * y_shake_speed, 2) * y_shake_power;
    
    context.cam.position += ((Vector2){x_offset, y_offset}) * context.cam.trauma * context.cam.trauma;
}

Cam saved_cam;

void draw_game(){
    // if (game_state == GAME){
    f32 zoom_speed = game_state == GAME ? 3 : 10;
        context.cam.cam2D.zoom = lerp(context.cam.cam2D.zoom, context.cam.target_zoom, core.time.real_dt * zoom_speed);
        
        if (abs(context.cam.cam2D.zoom - context.cam.target_zoom) <= EPSILON){
            context.cam.cam2D.zoom = context.cam.target_zoom;
        }
    // }

    saved_cam = context.cam;

    apply_shake();

    BeginDrawing();
    BeginShaderMode(render.test_shader);
    BeginTextureMode(render.main_render_texture);
    BeginMode2D(context.cam.cam2D);
    Context *c = &context;
    
    ClearBackground(GRAY);
    
    draw_entities();
    draw_particles();
    
    if (player_entity && debug.draw_player_collisions){
        for (int i = 0; i < player_data.collisions.count; i++){
            Collision col = player_data.collisions.get(i);
            
            draw_game_line(col.point, col.point + col.normal * 4, 0.2f, GREEN);
            draw_game_rect(col.point + col.normal * 4, {1, 1}, {0.5f, 0.5f}, 0, GREEN * 0.9f);
        }
    }
    
    if (game_state == EDITOR || game_state == PAUSE){
        draw_editor();
    }
    
    EndMode2D();
    EndTextureMode();
    draw_render_texture(render.main_render_texture.texture, {1, 1}, WHITE);
    EndShaderMode();
    
    draw_ui("");
    
    context.cam = saved_cam;
    context.cam.trauma -= core.time.dt * context.cam.trauma_decrease_rate;
    context.cam.trauma = clamp01(context.cam.trauma);
    
    f32 v_pos = 10;
    f32 font_size = 18;
    if (debug.info_fps){
        draw_text(TextFormat("FPS: %d", GetFPS()), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (game_state == GAME && player_entity){            
        if (debug.info_spin_progress){
            draw_text(TextFormat("Spin progress: %.2f", player_data.sword_spin_progress), 10, v_pos, font_size, RED);
            v_pos += font_size;
        }
        
        if (debug.info_blood_progress){
            draw_text(TextFormat("Blood progress: %.2f", player_data.blood_progress), 10, v_pos, font_size, RED);
            v_pos += font_size;
        }
    }
    
    if (debug.info_particle_count){
        draw_text(TextFormat("Particles count: %d", enabled_particles_count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (debug.info_emitters_count){
        draw_text(TextFormat("Emitters count: %d", context.emitters.count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (debug.info_player_speed){
        draw_text(TextFormat("Player speed: %.1f", magnitude(player_data.velocity)), 10, v_pos, font_size, RED);
        v_pos += font_size;
        draw_text(TextFormat("Player Velocity: {%.1f, %.1f}", player_data.velocity.x, player_data.velocity.y), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    v_pos += font_size;
    draw_text(TextFormat("Ammo: %d", player_data.ammo_count), 10, v_pos, font_size * 1.5f, VIOLET);
    v_pos += font_size * 1.5f;

    
    if (console.is_open){
        //draw console
        
        Color text_color = lerp(GREEN * 0, GREEN, console.open_progress * console.open_progress);
        f32 y_position = lerp(-screen_height * 0.6f, 0.0f, EaseOutQuint(console.open_progress));
        
        draw_rect({0, y_position}, {(f32)screen_width, screen_height * 0.5f}, BLUE * 0.2f);
        draw_text_boxed(console.str.data, {4, 4 + y_position, (f32)screen_width, screen_height * 0.5f - 30.0f}, 16, 3, text_color);
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
            draw_line(mouse_position - Vector2_right * 10 - Vector2_up * 10, mouse_position + Vector2_right * 10 + Vector2_up * 10, WHITE);
            draw_line(mouse_position + Vector2_right * 10 - Vector2_up * 10, mouse_position - Vector2_right * 10 + Vector2_up * 10, WHITE);
            draw_rect({mouse_position.x - 2.5f, mouse_position.y - 2.5f}, {5, 5}, GREEN);
        } else{
            draw_rect({mouse_position.x - 5, mouse_position.y - 5}, {10, 10}, RED);
        }
    } else{
        draw_circle({mouse_position.x, mouse_position.y}, 20, Fade(RED, 0.1f));
        draw_rect({mouse_position.x - 5, mouse_position.y - 5}, {10, 10}, WHITE);
    }
    
    EndDrawing();
}

void setup_color_changer(Entity *entity){
    //if (entity->color_changer.start_color == BLACK){
        entity->color_changer.start_color = entity->color;
    //}
    //if (entity->color_changer.target_color == BLACK){
        entity->color_changer.target_color = Fade(ColorBrightness(entity->color, 0.5f), 0.5f);
    //}
}

void check_avaliable_ids_and_set_if_found(int *id){
    int try_count = 0;
    while (context.entities.has_key(*id) && try_count <= 1000){
        (*id)++;
        try_count += 1;
    }
    
    assert(try_count < 1000);
}

Entity* add_entity(Entity *copy, b32 keep_id){
    Entity e = Entity(copy);
    
    if (!keep_id){
        e.id = context.entities.total_added_count + core.time.app_time * 10000 + 100;
    }
    
    check_avaliable_ids_and_set_if_found(&e.id);
    
    if (!keep_id && game_state == EDITOR){
        ForTable(context.entities, i){
            Entity *table_entity = context.entities.get_ptr(i);
            if (table_entity->flags & TRIGGER && table_entity->trigger.connected.contains(copy->id)){
                table_entity->trigger.connected.add(e.id);
            }
        }
    }
        
    context.entities.add(e.id, e);
    //setup_color_changer(
    return context.entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    //Entity *e = add_entity(pos, scale, rotation, flags);    
    Entity e = Entity(pos, scale, pivot, rotation, flags);    
    e.id = context.entities.total_added_count + core.time.app_time * 10000 + 100;
    //e.pivot = pivot;
    
    check_avaliable_ids_and_set_if_found(&e.id);


    context.entities.add(e.id, e);
    return context.entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Texture texture, FLAGS flags){
    //Entity *e = add_entity(pos, scale, rotation, flags);    
    Entity e = Entity(pos, scale, pivot, rotation, texture, flags);    
    e.id = context.entities.total_added_count + core.time.app_time * 10000 + 100;
    //e.pivot = pivot;
    
    check_avaliable_ids_and_set_if_found(&e.id);


    context.entities.add(e.id, e);
    return context.entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags){
    Entity *e = add_entity(pos, scale, pivot, rotation, flags);    
    e->color = color;
    setup_color_changer(e);
    return e;
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    Entity e = Entity(pos, scale, pivot, rotation, flags);    
    //Entity *e = add_entity(pos, scale, pivot, rotation, flags);    
    e.id = id;
    
    check_avaliable_ids_and_set_if_found(&e.id);
    
    
    context.entities.add(e.id, e);
    return context.entities.last_ptr();
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags){
    Entity *e = add_entity(id, pos, scale, pivot, rotation, flags);    
    e->color = color;
    setup_color_changer(e);
    return e;
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags, Array<Vector2, MAX_VERTICES> vertices){
    Entity *e = add_entity(id, pos, scale, pivot, rotation, color, flags);    
    //e->vertices.free_arr();
    e->vertices = vertices;
    setup_color_changer(e);
    return e;
}

Particle_Emitter* add_emitter(){
    Particle_Emitter e = Particle_Emitter();
    
    context.emitters.add(e);    
    return context.emitters.last_ptr();
}

// Particle_Emitter* add_emitter(Particle_Emitter *copy){
//     Particle_Emitter e = Particle_Emitter();
//     e = *copy;
    
//     context.emitters.add(e);    
//     return context.emitters.last_ptr();
// }

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
    Vector2 cam_pos = context.cam.position;

    Vector2 with_cam = subtract(position, cam_pos);
    Vector2 pixels   = multiply(with_cam, UNIT_SIZE);
    //Vector2 pixels   = multiply(position, UNIT_SIZE);
    
    //Horizontal center and vertical bottom
    
    f32 width_add, height_add;
    
    width_add = screen_width * 0.5f;    
    height_add = screen_height * 0.5f;    
    Vector2 to_center = {pixels.x + width_add, height_add - pixels.y};

    return to_center;
}

inline Vector2 rect_screen_pos(Vector2 position, Vector2 scale, Vector2 pivot){
    Vector2 pivot_add = multiply(pivot, scale);
    Vector2 with_pivot_pos = {position.x - pivot_add.x, position.y + pivot_add.y};
    Vector2 screen_pos = world_to_screen(with_pivot_pos);
    
    return screen_pos;
}


void draw_game_circle(Vector2 position, f32 radius, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_circle(screen_pos, radius * UNIT_SIZE, color);
}

void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect(screen_pos, multiply(scale, UNIT_SIZE), color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    //Vector2 screen_pos = world_to_screen(position);
    draw_rect_lines(screen_pos, scale * UNIT_SIZE, thick, color);
}

inline void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    //Vector2 screen_pos = world_to_screen(position);
    draw_rect_lines(screen_pos, scale * UNIT_SIZE, color);
}

void draw_game_line_strip(Entity *entity, Color color){
    Vector2 screen_positions[entity->vertices.count];
    
    for (int i = 0; i < entity->vertices.count; i++){
        screen_positions[i] = world_to_screen(global(entity, entity->vertices.get(i)));
    }
    
    draw_line_strip(screen_positions, entity->vertices.count, color);
}

void draw_game_line_strip(Vector2 *points, int count, Color color){
    Vector2 screen_positions[count];
    
    for (int i = 0; i < count; i++){
        screen_positions[i] = world_to_screen(points[i]);
    }
    
    draw_line_strip(screen_positions, count, color);
}

void draw_game_triangle_strip(Array<Vector2, MAX_VERTICES> vertices, Vector2 position, Color color){
    Vector2 screen_positions[vertices.count];
    
    for (int i = 0; i < vertices.count; i++){
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
    draw_rect(screen_pos, multiply(scale, UNIT_SIZE), pivot, rotation, color);
}

void draw_game_text(Vector2 position, const char *text, f32 size, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_text(text, screen_pos, size, color);
}

void draw_game_texture(Texture tex, Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color){
    //Vector2 screen_pos = rect_screen_pos(position, {(float)tex.width / UNIT_SIZE, (f32)tex.height / UNIT_SIZE}, pivot);
    // Vector2 screen_pos = world_to_screen(position);
    Vector2 screen_pos = world_to_screen(position);
    draw_texture(tex, screen_pos, transform_texture_scale(tex, scale), pivot, rotation, color);
}

void draw_game_line(Vector2 start, Vector2 end, f32 thick, Color color){
    draw_line(world_to_screen(start), world_to_screen(end), thick * UNIT_SIZE, color);
}

void draw_game_line(Vector2 start, Vector2 end, Color color){
    draw_line(world_to_screen(start), world_to_screen(end), color);
}

void draw_game_ring_lines(Vector2 center, f32 inner_radius, f32 outer_radius, i32 segments, Color color, f32 start_angle, f32 end_angle){
    draw_ring_lines(world_to_screen(center), inner_radius * UNIT_SIZE, outer_radius * UNIT_SIZE, segments, color);
}

void draw_game_triangle_lines(Vector2 v1, Vector2 v2, Vector2 v3, Color color){
    draw_triangle_lines(world_to_screen(v1), world_to_screen(v2), world_to_screen(v3), color);
}

f32 zoom_unit_size(){
    f32 zoom = context.cam.cam2D.zoom;
    
    if (zoom <= EPSILON){
        zoom = 1;
    }

    return UNIT_SIZE / zoom;
}

Vector2 get_left_up_no_rot(Entity *e){
    return {e->position.x - e->pivot.x * e->bounds.size.x, e->position.y + e->pivot.y * e->bounds.size.y};
}

Vector2 get_left_up(Entity *e){
    //f32 x_add = sinf(e->rotation * DEG2RAD) * e->bounds.size.x;
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