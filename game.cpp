#pragma once

//#define assert(a) (if (!a) (int*)void*);
//#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

#include "game.h"

#define ForTable(table, xx) for(int xx = table_next_avaliable(table, 0);  xx < table.max_count; xx = table_next_avaliable(table, xx+1))
//#define For(arr, type, value) for(int ii = 0; ii < arr.count; ii++){ type value = arr.get(ii);

global_variable Input input;
global_variable Level current_level;
global_variable Context context = {};
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

global_variable Hash_Table_Int<Texture> textures_table = Hash_Table_Int<Texture>(512);

#include "../my_libs/random.hpp"
#include "particles.hpp"
#include "text_input.hpp"
#include "ui.hpp"

void free_entity(Entity *e){
    if (e->flags & AGRO_AREA){
        e->agro_area.connected.free_arr();
    }
}

void add_rect_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    vertices->add({pivot.x, pivot.y});
    vertices->add({-pivot.x, pivot.y});
    vertices->add({pivot.x, pivot.y - 1.0f});
    vertices->add({pivot.x - 1.0f, pivot.y - 1.0f});
}

void add_sword_vertices(Array<Vector2, MAX_VERTICES> *vertices, Vector2 pivot){
    vertices->add({pivot.x * 0.3f, pivot.y});
    vertices->add({-pivot.x * 0.3f, pivot.y});
    vertices->add({pivot.x, pivot.y - 1.0f});
    vertices->add({pivot.x - 1.0f, pivot.y - 1.0f});
}

void add_texture_vertices(Array<Vector2, MAX_VERTICES> *vertices, Texture texture, Vector2 pivot){
    Vector2 scaled_size = {texture.width / UNIT_SIZE, texture.height / UNIT_SIZE};
    vertices->add({pivot.x * scaled_size.x, pivot.y * scaled_size.y});
    vertices->add({-pivot.x * scaled_size.x, pivot.y * scaled_size.y});
    vertices->add({pivot.x * scaled_size.x, (pivot.y - 1.0f) * scaled_size.y});
    vertices->add({(pivot.x - 1.0f) * scaled_size.x, (pivot.y - 1.0f) * scaled_size.y});
}

void pick_vertices(Entity *entity){
    if (entity->flags & TEXTURE){
        add_texture_vertices(&entity->vertices, entity->texture, entity->pivot);
        add_texture_vertices(&entity->unscaled_vertices, entity->texture, entity->pivot);
        return;
    }

    if (entity->flags & (SWORD | BIRD_ENEMY)){
        add_sword_vertices(&entity->vertices, entity->pivot);
        add_sword_vertices(&entity->unscaled_vertices, entity->pivot);
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
    id = copy->id;
    position = copy->position;
    pivot    = copy->pivot;
    
    vertices = copy->vertices;
    unscaled_vertices = copy->unscaled_vertices;
    rotation = copy->rotation;
    scale = copy->scale;
    flags = copy->flags;
    collision_flags = copy->collision_flags;
    color = copy->color;
    draw_order = copy->draw_order;
    str_copy(name, copy->name);
    
    if (flags & TEXTURE){
        texture = copy->texture;
        scaling_multiplier = {texture.width / UNIT_SIZE, texture.height / UNIT_SIZE};
        str_copy(texture_name, copy->texture_name);
    }
    color_changer = copy->color_changer;
    
    if (flags & DRAW_TEXT){
        text_drawer = copy->text_drawer;
    }
    if (flags & ENEMY){
        enemy = copy->enemy;
    }
    
    if (flags & AGRO_AREA){
        agro_area = copy->agro_area;
    }
    
    calculate_bounds(this);
    setup_color_changer(this);
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
    c->entities.clear();
    c->particles.clear();
    c->emitters.clear();
    
    c ->we_got_a_winner = false;
    player_data = {};
}

int save_level(const char *level_name){
    String level_path = init_string();
    level_path += "levels/";
    level_path += level_name;
    FILE *fptr;
    fptr = fopen(level_path.data, "w");
    
    if (fptr == NULL){
        return 0;
    }
    
    console.str += "Level saved: ";
    console.str += level_name;
    console.str += "\n";
    
    printf("level saved: %s\n", level_path.data);
    
    level_path.free_str();
    
    fprintf(fptr, "Setup Data:\n");
    
    fprintf(fptr, "player_spawn_point:{:%f:, :%f:} ", editor.player_spawn_point.x, editor.player_spawn_point.y);
    
    fprintf(fptr, ";\n");
    
    fprintf(fptr, "Entities:\n");
    for (int i = 0; i < context.entities.max_count; i++){        
        if (!context.entities.has_index(i)){
            continue;
        }
    
        Entity *e = context.entities.get_ptr(i);
        
        Color color = e->color_changer.start_color;
        fprintf(fptr, "name:%s: id:%d: pos{:%f:, :%f:} scale{:%f:, :%f:} pivot{:%f:, :%f:} rotation:%f: color{:%d:, :%d:, :%d:, :%d:}, flags:%d:, draw_order:%d: ", e->name, e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags, e->draw_order);
        
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
        
        if (e->flags & AGRO_AREA){
            fprintf(fptr, "agro_connected [ ");
            for (int v = 0; v < e->agro_area.connected.count; v++){
                fprintf(fptr, ":%d: ", e->agro_area.connected.get(v)); 
            }
            fprintf(fptr, "] "); 
        }
        
        if (e->flags & TEXTURE){
            fprintf(fptr, "texture_name:%s: ", e->texture_name);
        }
        
        fprintf(fptr, ";\n"); 
    }
    
    fclose(fptr);
    
    if (!str_start_with_const(level_name, "TEMP_")){
        str_copy(context.current_level_name, level_name);
    }
    
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
    String level_path = init_string();
    level_path += "levels/";
    level_path += level_name;
    
    File file = load_file(level_path.data, "r");
    
    if (!file.loaded){
        console.str += "Could not load level: ";
        console.str += level_name;
        console.str += "\n";
        return 0;
    }
    
    clear_context(&context);
    
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
            } else if (str_equal(splitted_line.get(i).data, "agro_connected")){
                fill_int_array_from_string(&entity_to_fill.agro_area.connected, splitted_line, &i);
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
            
            Entity *added_entity = add_entity(&entity_to_fill, true);
            
            calculate_bounds(added_entity);
        }
    }
    
    if (!str_start_with_const(level_name, "TEMP_")){
        str_copy(context.current_level_name, level_name);
    }
    
    //free_string_array(&splitted_line);
    splitted_line.free_arr();
    unload_file(&file);

    {  
        console.str += "Loaded level: ";
        console.str += level_name;
        console.str += "\n";
    }
        
    loop_entities(init_loaded_entity);
    
    level_path.free_str();
    
    setup_particles();
    
    return 1;
}

global_variable Array<Collision, MAX_COLLISIONS> collisions_data = Array<Collision, MAX_COLLISIONS>();

#define MAX_SPAWN_OBJECTS 128

global_variable Array<Spawn_Object, MAX_SPAWN_OBJECTS> spawn_objects = Array<Spawn_Object, MAX_SPAWN_OBJECTS>();

Texture cat_texture;

#define BIRD_ENEMY_COLLISION_FLAGS (GROUND | PLAYER | BIRD_ENEMY)

void init_spawn_objects(){
    Entity block_base_entity = Entity({0, 0}, {10, 5}, {0.5f, 0.5f}, 0, GROUND);
    block_base_entity.color = BROWN;
    block_base_entity.color_changer.start_color = block_base_entity.color;
    block_base_entity.color_changer.target_color = block_base_entity.color * 1.5f;
    str_copy(block_base_entity.name, "block_base"); 
    
    Spawn_Object block_base_object;
    copy_entity(&block_base_object.entity, &block_base_entity);
    str_copy(block_base_object.name, block_base_entity.name);
    
    spawn_objects.add(block_base_object);
    
    Entity enemy_base_entity = Entity({0, 0}, {3, 5}, {0.5f, 0.5f}, 0, ENEMY);
    enemy_base_entity.color = RED * 0.9f;
    enemy_base_entity.color_changer.start_color = enemy_base_entity.color;
    enemy_base_entity.color_changer.target_color = enemy_base_entity.color * 1.5f;
    str_copy(enemy_base_entity.name, "enemy_base"); 
    
    Spawn_Object enemy_base_object;
    copy_entity(&enemy_base_object.entity, &enemy_base_entity);
    str_copy(enemy_base_object.name, enemy_base_entity.name);
    
    spawn_objects.add(enemy_base_object);
    
    Entity enemy_bird_entity = Entity({0, 0}, {3, 5}, {0.5f, 0.5f}, 0, ENEMY | BIRD_ENEMY);
    enemy_bird_entity.collision_flags = BIRD_ENEMY_COLLISION_FLAGS;//GROUND | PLAYER | BIRD_ENEMY;
    enemy_bird_entity.color = YELLOW * 0.9f;
    enemy_bird_entity.color_changer.start_color = enemy_bird_entity.color;
    enemy_bird_entity.color_changer.target_color = enemy_bird_entity.color * 1.5f;
    str_copy(enemy_bird_entity.name, "enemy_bird"); 
    
    Spawn_Object enemy_bird_object;
    copy_entity(&enemy_bird_object.entity, &enemy_bird_entity);
    str_copy(enemy_bird_object.name, enemy_bird_entity.name);
    
    spawn_objects.add(enemy_bird_object);
    
    Entity win_block_entity = Entity({0, 0}, {3, 3}, {0.5f, 0.5f}, 0, WIN_BLOCK);
    win_block_entity.color = GREEN * 0.9f;
    win_block_entity.color_changer.start_color = win_block_entity.color;
    win_block_entity.color_changer.target_color = win_block_entity.color * 1.5f;
    str_copy(win_block_entity.name, "win_block"); 
    
    Spawn_Object win_block_object;
    copy_entity(&win_block_object.entity, &win_block_entity);
    str_copy(win_block_object.name, win_block_entity.name);
    
    spawn_objects.add(win_block_object);
    
    Entity agro_area_entity = Entity({0, 0}, {20, 20}, {0.5f, 0.5f}, 0, AGRO_AREA);
    agro_area_entity.color = RED * 0.9f;
    agro_area_entity.color_changer.start_color = agro_area_entity.color;
    agro_area_entity.color_changer.target_color = agro_area_entity.color * 1.5f;
    str_copy(agro_area_entity.name, "agro_area"); 
    
    Spawn_Object argo_area_object;
    copy_entity(&argo_area_object.entity, &agro_area_entity);
    str_copy(argo_area_object.name, agro_area_entity.name);
    
    spawn_objects.add(argo_area_object);
}

void add_spawn_object_from_texture(Texture texture, char *name){
    Entity texture_entity = Entity({0, 0}, {1, 1}, {0.5f, 0.5f}, 0, texture, TEXTURE);
    texture_entity.color = WHITE;
    texture_entity.color_changer.start_color = texture_entity.color;
    texture_entity.color_changer.target_color = texture_entity.color * 1.5f;
    str_copy(texture_entity.name, name); 
    
    texture_entity.texture = texture;
    str_copy(texture_entity.texture_name, name);
    
    Spawn_Object texture_object;
    copy_entity(&texture_object.entity, &texture_entity);
    str_copy(texture_object.name, texture_entity.name);
    
    spawn_objects.add(texture_object);
}

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
    console.str += text;
    console.str += "\n";
}

void print_to_console(const char *text){
    console.str += text;
    console.str += "\n";
}

inline void init_loaded_entity(Entity *entity){
    if (entity->flags & BIRD_ENEMY){
        entity->collision_flags = BIRD_ENEMY_COLLISION_FLAGS;
    }
}


inline int table_next_avaliable(Hash_Table_Int<Entity> table, int index){
    for (int i = index; i < table.max_count; i++){
        if (table.has_index(i)){
            return i;
        }
    }
    
    return table.max_count;
}

void init_entity(Entity *entity){
    setup_color_changer(entity);
    
    if (entity->flags & AGRO_AREA){
        // ForTable(context.entities, i){
        //     Entity *e = context.entities.get_ptr(i);
        //     if (e->flags & ENEMY){
        //         entity->agro_area.connected.add(e->id);
        //     }
        // }
    }
}

inline void save_current_level(){
    save_level(context.current_level_name);
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
    console.str += context.current_level_name;
    console.str += "\n";
}

void init_console(){
    console.str = init_string();

    console.commands.add(make_console_command("hotkeys", print_hotkeys_to_console));
    console.commands.add(make_console_command("hotkey",  print_hotkeys_to_console));
    console.commands.add(make_console_command("key",     print_hotkeys_to_console));
    console.commands.add(make_console_command("keys",    print_hotkeys_to_console));
    console.commands.add(make_console_command("help",    print_hotkeys_to_console));
    
    console.commands.add(make_console_command("save",    save_current_level, save_level_by_name));
    console.commands.add(make_console_command("load",    NULL, load_level_by_name));
    console.commands.add(make_console_command("level",    print_current_level, load_level_by_name));
}

void init_game(){
    game_state = EDITOR;

    context = {};    
    str_copy(context.current_level_name, "test_level.level");

    input = {};
    init_console();

    current_level = {};
    //current_level.context = (Context*)malloc(sizeof(Context));
    //context = *current_level.context;
    
    init_spawn_objects();
    load_textures();
    
    //mouse_entity = add_entity(input.mouse_position, {1, 1}, {0.5f, 0.5f}, 0, -1);
    mouse_entity = Entity(input.mouse_position, {1, 1}, {0.5f, 0.5f}, 0, 0);
    
    load_level("test_level.level");
    
    ForTable(context.entities, i){
        init_entity(context.entities.get_ptr(i));
    }
    
    Context *c = &context;
    
    c->unit_screen_size = {screen_width / UNIT_SIZE, screen_height / UNIT_SIZE};
    
    c->cam.position = Vector2_zero;
    c->cam.cam2D.target = world_to_screen({0, 0});
    c->cam.cam2D.offset = (Vector2){ screen_width/2.0f, (f32)screen_height * 0.5f };
    c->cam.cam2D.rotation = 0.0f;
    c->cam.cam2D.zoom = 0.5f;
}

void enter_game_state(){
    assign_selected_entity(NULL);

    game_state = GAME;
    
    String temp_level_name = init_string();
    temp_level_name += "TEMP_";
    temp_level_name += context.current_level_name;
    
    save_level(temp_level_name.data);
    temp_level_name.free_str();
    
    //copy_context(&saved_level_context, &context);
    
    player_entity = add_entity(editor.player_spawn_point, {1.0f, 2.0f}, {0.5f, 0.5f}, 0, RED, PLAYER);
    player_entity->collision_flags = GROUND | ENEMY;
    player_entity->draw_order = 30;
    
    Entity *ground_checker = add_entity(player_entity->position - player_entity->up * player_entity->scale.y * 0.5f, {player_entity->scale.x * 0.9f, player_entity->scale.y * 1.5f}, {0.5f, 0.5f}, 0, 0); 
    ground_checker->collision_flags = GROUND;
    ground_checker->color = Fade(PURPLE, 0.8f);
    ground_checker->draw_order = 31;
    
    Entity *sword_entity = add_entity(editor.player_spawn_point, player_data.sword_start_scale, {0.5f, 1.0f}, 0, GRAY + RED * 0.1f, SWORD);
    sword_entity->collision_flags = ENEMY;
    sword_entity->color   = GRAY + RED * 0.1f;
    sword_entity->color.a = 255;
    sword_entity->color_changer.start_color = sword_entity->color;
    sword_entity->color_changer.target_color = RED * 0.99f;
    sword_entity->color_changer.interpolating = true;
    sword_entity->draw_order = 25;
    
    //sword_entity->index = sword_entity->id % MAX_ENTITIES;
    
    player_data.ground_checker_id = ground_checker->id;
    player_data.sword_entity_id = sword_entity->id;
    player_data.dead_man = false;
}

void kill_player(){
    emit_particles(big_blood_emitter, player_entity->position, player_entity->up, 1, 1);
    player_data.dead_man = true;
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

void enter_editor_state(){
    game_state = EDITOR;
    
    editor.in_editor_time = 0;
    close_create_box();
    
    if (player_entity){
        destroy_player();
        player_data.dead_man = false;
    }
    
    clear_context(&context);
    
    String temp_level_name = init_string();
    temp_level_name += "TEMP_";
    temp_level_name += context.current_level_name;
    
    load_level(temp_level_name.data);
    temp_level_name.free_str();

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
    return screen_to_world(GetMousePosition());
}

void fixed_game_update(){
    //update_entities();
}

void print_hotkeys_to_console(){
    console.str += "Ctrl+Shift+Space - Toggle Game/Editor\n";
    console.str += "Ctrl+Shift+J - Save current level\n";
    console.str += "Alt - See and move vertices\n";
    console.str += "Space - Create menu\n";
    console.str += "P - Move player spawn point\n";
}

void update_console(){
    b32 can_control_console = !editor.create_box_active;
    if (can_control_console && (IsKeyPressed(KEY_SLASH) || (console.is_open && IsKeyPressed(KEY_ESCAPE)))){
        if (console.is_open){
            console.is_open = false;
            console.closed_time = core.time.game_time;
            
            if (str_equal(focus_input_field.tag, "console_input_field")){
                focus_input_field.in_focus = false;
            }
        } else{
            console.is_open = true;
            console.opened_time = core.time.game_time;
            make_next_input_field_in_focus();
        }
    }
            
    if (console.is_open && can_control_console){
        f32 time_since_open = core.time.game_time - console.opened_time;
        console.open_progress = clamp01(time_since_open / 0.3f);
        
        Color color = lerp(WHITE * 0, GRAY, console.open_progress * console.open_progress);
        
        console.args.clear();
        split_str(focus_input_field.content, " ", &console.args);
        
        b32 content_changed = false;
        for (int i = 0; i < console.commands.count && console.args.count > 0; i++){
            if (str_start_with(console.commands.get(i).name, console.args.get(0).data)){
                make_ui_text(console.commands.get(i).name, {0.0f, (f32)screen_height * 0.5f}, focus_input_field.font_size, color * 0.7f, "console_hint_text");
                
                if (IsKeyPressed(KEY_TAB) && console.args.count == 1){
                    set_focus_input_field(console.commands.get(i).name);
                    content_changed = true;
                }
                break;
            }
        }
        
        if (make_input_field("", {0.0f, (f32)screen_height * 0.5f}, {(f32)screen_width, 30.0f}, "console_input_field", color, false)){
            console.str += focus_input_field.content;
            console.str += "\n";
            
            //if we used hint while input
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
                console.str += "Command was not found :D\n";
            }
            
            clear_focus_input_field();
        }
    }
}

void update_game(){
    //dt *= dt_scale;
    
    core.time.unscaled_dt = GetFrameTime();
    core.time.dt = GetFrameTime() * core.time.time_scale;
    
    core.time.game_time += core.time.dt;
    
    frame_rnd = rnd01();
    frame_on_circle_rnd = rnd_on_circle();

    //update input
    input.mouse_position = game_mouse_pos();
    input.mouse_delta = GetMouseDelta();
    input.mouse_wheel = GetMouseWheelMove();
    
    input.direction.x = 0;
    input.direction.y = 0;
    
    if (IsKeyDown(KEY_RIGHT)){
        input.direction.x = 1;
    } else if (IsKeyDown(KEY_LEFT)){
        input.direction.x = -1;
    }
    if (IsKeyDown(KEY_UP)){
        input.direction.y = 1;
    } else if (IsKeyDown(KEY_DOWN)){
        input.direction.y = -1;
    }
    if (IsKeyDown(KEY_D)){
        input.direction.x = 1;
    } else if (IsKeyDown(KEY_A)){
        input.direction.x = -1;
    }
    if (IsKeyDown(KEY_W)){
        input.direction.y = 1;
    } else if (IsKeyDown(KEY_S)){
        input.direction.y = -1;
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

    
    if (screen_size_changed){
        context.unit_screen_size = {screen_width / UNIT_SIZE, screen_height / UNIT_SIZE};
        context.cam.cam2D.target = (Vector2){ screen_width/2.0f, screen_height/2.0f };
        context.cam.cam2D.offset = (Vector2){ screen_width/2.0f, screen_height/2.0f };
        
        // UnloadRenderTexture(context.up_render_target);
        // UnloadRenderTexture(context.down_render_target);
        // UnloadRenderTexture(context.up_render_target);
        
        // context.up_render_target = LoadRenderTexture(context.up_screen_size.x, context.up_screen_size.y);
        // context.down_render_target = LoadRenderTexture(context.down_screen_size.x, context.down_screen_size.y);
        // context.right_render_target = LoadRenderTexture(context.right_screen_size.x, context.right_screen_size.y);
    }
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_SPACE)){
        if (game_state == EDITOR){
            enter_game_state();
        } else if (game_state == GAME){
            enter_editor_state();
        }
    }
    
    update_input_field();
    update_console();
    
    if (game_state == EDITOR){
        update_editor_ui();
        update_editor();
    }
    
    //zoom_entity->text_drawer.text = TextFormat("%f", context.cam.cam2D.zoom);
    
    float full_delta = core.time.unscaled_dt + core.time.previous_dt;
    core.time.previous_dt = 0;
    
    full_delta = Clamp(full_delta, 0, 0.1f);
    
    while (full_delta >= TARGET_FRAME_TIME){
        float dt = TARGET_FRAME_TIME * core.time.time_scale;
        if (dt == 0){
            break;
        }
        
        if (core.time.time_scale > 1){
            while (dt >= TARGET_FRAME_TIME){
                core.time.fixed_dt = TARGET_FRAME_TIME;
                fixed_game_update();
                dt -= TARGET_FRAME_TIME;
            }
        
            //if (dt > 0) update(dt);
            core.time.previous_dt += dt;
        } else{
            core.time.fixed_dt = TARGET_FRAME_TIME;
            fixed_game_update();
        }
        full_delta -= TARGET_FRAME_TIME;
    }
    
    core.time.previous_dt = full_delta;
    
    update_entities();
    update_emitters();
    update_particles();
    
    if (game_state == GAME && player_entity){
        Vector2 target_position = player_entity->position + Vector2_up * 10;
        context.cam.position = lerp(context.cam.position, target_position, core.time.dt * 10);
    }
    
    if (editor.update_cam_view_position){
        context.cam.view_position = context.cam.position;
    }
    
    draw_game();
}

void update_color_changer(Entity *entity, f32 dt){
    Color_Changer *changer = &entity->color_changer;
    
    if (changer->changing){
        f32 t = abs(sinf(core.time.game_time * changer->change_time));
        entity->color = lerp(changer->start_color, changer->target_color, t);
    }
    
    if (changer->interpolating){
        entity->color = lerp(changer->start_color, changer->target_color, changer->progress);
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

b32 check_bounds_collision(Vector2 position1, Bounds bounds1, Vector2 position2, Bounds bounds2){
    return check_rectangles_collision(position1 + bounds1.offset, bounds1.size, position2 + bounds2.offset, bounds2.size);
}

Collision check_entities_collision(Entity *entity1, Entity *entity2){
    Collision result = {};
    result.other_entity = entity2;
    
    if (!check_bounds_collision(entity1->position, entity1->bounds, entity2->position, entity2->bounds)){
        return result;
    }

    //Array<Vector2> normals = Array<Vector2>(entity1->vertices.count + entity2->vertices.count);
    global_normals.count = 0;
    fill_arr_with_normals(&global_normals, entity1->vertices);
    fill_arr_with_normals(&global_normals, entity2->vertices);
    
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
            Entity *entity;
            if (shape == 0) {
                vertices = entity1->vertices;
                entity = entity1;
            } else{
                vertices = entity2->vertices;
                entity = entity2;
            }
            
            for (int j = 0; j < vertices.count; j++){            
                f32 p = dot(global(entity, vertices.get(j)), axis);
                
                f32 min = fmin(projections[shape].x, p);
                f32 max = fmax(projections[shape].y, p);
                
                projections[shape].x = min;
                projections[shape].y = max;
            }
            
            //vertices.free_arr();
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
    
    Vector2 vec_to_first = entity1->position - entity2->position;
    
    result.collided = true;
    //if (entity1->flags > 0){
        result.overlap = overlap;
        //result.normal = dir_to_first;
        result.dir_to_first = normalized(vec_to_first);
        result.normal = dot(result.dir_to_first, min_overlap_axis) > 0 ? min_overlap_axis : min_overlap_axis * -1.0f;
        //result.point = entity1->position - dir_to_first * overlap;
        result.point = entity1->position - result.normal * ((min_overlap_projection.y - min_overlap_projection.x) / 2);
    //}
    
    //normals.free_arr();
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
        
        if (other->destroyed || !other->enabled || other == entity || other->flags <= 0 || (other->flags & include_flags) <= 0){
            continue;
        }
        
        Collision col = check_entities_collision(entity, other);
        
        if (col.collided){
            result->add(col);
        }
    }
}

void assign_moving_vertex_entity(Entity *e, int vertex_index){
    Vector2 *vertex = e->vertices.get_ptr(vertex_index);

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

    Vector2 displacement = local_target - *vertex;

    Vector2 unscaled_displacement = {displacement.x / entity->scale.x, displacement.y / entity->scale.y};
    
    *vertex = local_target;
    Vector2 modified_unscaled = *unscaled_vertex + unscaled_displacement;
    *unscaled_vertex = normalized(*vertex) * magnitude(modified_unscaled);
    
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
        Vector2 inspector_size = {screen_width * 0.2f, screen_height * 0.4f};
        Vector2 inspector_position = {screen_width - inspector_size.x - inspector_size.x * 0.1f, 0 + inspector_size.y * 0.05f};
        make_ui_image(inspector_position, inspector_size, {0, 0}, SKYBLUE * 0.7f, "inspector_window");
        f32 height_add = 30;
        f32 v_pos = inspector_position.y + height_add + 40;
        
        make_ui_text(TextFormat("ID: %d", selected->id), {inspector_position.x + 100, inspector_position.y - 10}, 18, WHITE, "inspector_id"); 
        
        make_ui_text(TextFormat("Name: ", selected->id), {inspector_position.x, inspector_position.y + 10}, 24, BLACK, "inspector_id"); 
        if (make_input_field(TextFormat("%s", selected->name), {inspector_position.x + 65, inspector_position.y + 10}, {200, 25}, "inspector_name") || focus_input_field.changed && (str_equal(focus_input_field.tag, "inspector_name"))){
            str_copy(selected->name, focus_input_field.content);
        }
        
        make_ui_text("POSITION", {inspector_position.x + 100, inspector_position.y + 40}, 24, WHITE * 0.9f, "inspector_pos");
        make_ui_text("X:", {inspector_position.x + 5, v_pos}, 22, BLACK * 0.9f, "inspector_pos_x");
        make_ui_text("Y:", {inspector_position.x + 5 + 35 + 100, v_pos}, 22, BLACK * 0.9f, "inspector_pos_y");
        if (make_input_field(TextFormat("%.3f", selected->position.x), {inspector_position.x + 30, v_pos}, {100, 25}, "inspector_pos_x")
            || make_input_field(TextFormat("%.3f", selected->position.y), {inspector_position.x + 30 + 100 + 35, v_pos}, {100, 25}, "inspector_pos_y")
            || focus_input_field.changed && (str_equal(focus_input_field.tag, "inspector_pos_x") || str_equal(focus_input_field.tag, "inspector_pos_y"))){
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
            || focus_input_field.changed && (str_equal(focus_input_field.tag, "inspector_scale_x") || str_equal(focus_input_field.tag, "inspector_scale_y"))){
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
            || focus_input_field.changed && str_equal(focus_input_field.tag, "inspector_rotation")){
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
            || focus_input_field.changed && str_equal(focus_input_field.tag, "inspector_draw_order")){
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
        
        if (selected->flags & AGRO_AREA){
            make_ui_text(TextFormat("Connected: %d", selected->agro_area.connected.count), {inspector_position.x + 5, (f32)screen_height - 100}, 24, RED * 0.9f, "agro_count");
            make_ui_text("Assign New: Alt+A", {inspector_position.x + 5, (f32)screen_height - 75}, 24, RED * 0.9f, "agro_assign");
            make_ui_text("Remove selected: Alt+D", {inspector_position.x + 5, (f32)screen_height - 50}, 24, RED * 0.9f, "agro_remove");
            make_ui_text("Clear Connected: Alt+L", {inspector_position.x + 5, (f32)screen_height - 25}, 24, RED * 0.9f, "agro_clear");
        }
    }
}

Entity *get_cursor_entity(){
    Entity *cursor_entity_candidate = NULL;
    
    b32 mouse_on_selected_entity = editor.selected_entity && check_entities_collision(&mouse_entity, editor.selected_entity).collided;

    for (int i = 0; i < context.entities.max_count; i++){        
        Entity *e = context.entities.get_ptr(i);
        
        if (!e->enabled/* || e->flags == -1*/){
            continue;
        }
        
        if ((check_entities_collision(&mouse_entity, e)).collided){
            if (editor.last_click_position == input.mouse_position){
                //If we long enough on one entity we assume that we want to pick it up and not to cycle
                f32 time_since_last_click = core.time.game_time - editor.last_click_time;
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
                    cursor_entity_candidate = e;
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
    
    f32 dt = core.time.dt;
    
    editor.in_editor_time += dt;

    if (editor.need_validate_entity_pointers){
        validate_editor_pointers();
        editor.need_validate_entity_pointers = false;
    }
    
    f32 zoom = context.cam.cam2D.zoom;
    
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_L)){
        editor.update_cam_view_position = !editor.update_cam_view_position;
    }
    
    b32 moving_editor_cam = false;
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        context.cam.position += ((Vector2){-input.mouse_delta.x / zoom, input.mouse_delta.y / zoom}) / (UNIT_SIZE);
        moving_editor_cam = true;
    }
    
    if (input.mouse_wheel != 0){
        if (input.mouse_wheel > 0 && zoom < 5 || input.mouse_wheel < 0 && zoom > 0.1f){
            context.cam.cam2D.zoom += input.mouse_wheel * 0.05f;
        }
    }
    
    b32 need_move_vertices = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select;
    b32 need_snap_vertex = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_V);
    
    int selected_vertex_index;
    Vector2 closest_vertex_global;
    f32 distance_to_closest_vertex = INFINITY;
    
    mouse_entity.position = input.mouse_position;
    
    //Entity *cursor_entity_candidate = NULL;
    
    int cursor_entities_count = 0;
    
    //editor entities loop
    for (int i = 0; i < context.entities.max_count; i++){        
        Entity *e = context.entities.get_ptr(i);
        
        if (!e->enabled/* || e->flags == -1*/){
            continue;
        }
        
        if ((check_entities_collision(&mouse_entity, e)).collided){
            cursor_entities_count++;
        //     if (editor.last_click_position == input.mouse_position){
        //         if (!cursor_entity_candidate){
        //             cursor_entity_candidate = e;
        //         } else if (!editor.place_cursor_entities.contains(e)){
        //             cursor_entity_candidate = e;
        //         }
        //     } else{ //Mouse moved from last click
        //         //If mouse moved we always want cursor entity to be a selected entity
        //         if (editor.selected_entity && check_entities_collision(&mouse_entity, editor.selected_entity).collided){
        //             if (e->id == editor.selected_entity->id){
        //                 cursor_entity_candidate = e;
        //             }
        //         } else{
        //             cursor_entity_candidate = e;
        //         }
        //         editor.place_cursor_entities.clear();
        //     }
        // } else if (cursor_entities_count == 0){
        //     editor.cursor_entity = NULL;
        // }
        }
        
        //editor vertices
        for (int v = 0; v < e->vertices.count && (need_move_vertices || need_snap_vertex); v++){
            Vector2 *vertex = e->vertices.get_ptr(v);
            
            Vector2 vertex_global = global(e, *vertex);
            
            if (need_move_vertices && editor.moving_vertex == NULL){
                if (check_col_circles({input.mouse_position, 1}, {vertex_global, 0.5f})){
                    assign_moving_vertex_entity(e, v);
                    undo_remember_vertices_start(e);
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
        //editor move vertices
    
         //editor snap closest vertex to closest vertex
    }
    
    // if (cursor_entity_candidate == NULL){
    //     //something to think about
    //     editor.place_cursor_entities.clear();
    // } else{
    //     editor.cursor_entity = cursor_entity_candidate;
    // }
    
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
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && can_select){
        if (editor.cursor_entity != NULL){ //selecting entity
            b32 is_same_selected_entity = editor.selected_entity != NULL && editor.selected_entity->id == editor.cursor_entity->id;
            if (!is_same_selected_entity){
                assign_selected_entity(editor.cursor_entity);
                editor.place_cursor_entities.add(editor.selected_entity);
                
                editor.selected_this_click = true;
            }
        }
    } else if (editor.dragging_entity == NULL && !editor.selected_this_click && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && editor.selected_entity != NULL){ 
        if (editor.cursor_entity != NULL){
            if (editor.moving_vertex == NULL && editor.selected_entity->id == editor.cursor_entity->id){
                editor.dragging_entity = editor.selected_entity;
                editor.dragging_entity_id = editor.selected_entity->id;
                editor.dragging_start = editor.dragging_entity->position;
            }
        }
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){ //stop dragging entity
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
        
        // *editor.moving_vertex = global(editor.moving_vertex_entity, *editor.moving_vertex);
        // editor.moving_vertex->x = input.mouse_position.x;
        // editor.moving_vertex->y = input.mouse_position.y;
        // *editor.moving_vertex = local(editor.moving_vertex_entity, *editor.moving_vertex);
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
            editor.create_box_lifetime -= dt;
            if (editor.create_box_lifetime <= 0){
                need_close_create_box = true;
            }
        } else{
            editor.create_box_lifetime += dt;
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
            
            if (make_button(obj_position, obj_size, {0, 0}, obj.name, 24, "create_box") || (this_object_selected && IsKeyPressed(KEY_ENTER))){
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
                f32 color_multiplier = lerp(0.7f, 0.9f, (sinf(core.time.game_time * 3) + 1) * 0.5f);
                make_ui_image(obj_position, {obj_size.x * 0.2f, obj_size.y}, {1, 0}, WHITE * color_multiplier, "create_box");
            }
            
            fitting_count++;
        }
        
        if (fitting_count > 0 && editor.create_box_selected_index > fitting_count - 1){
            editor.create_box_selected_index = fitting_count - 1;   
        }
    
        if (make_input_field("", field_position, field_size, "create_box")){
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
    
    if (editor.dragging_entity != NULL){
        editor.dragging_time += dt;
    }
    
    if (editor.dragging_entity != NULL && !moving_editor_cam){
        Vector2 move_delta = ((Vector2){input.mouse_delta.x / zoom, -input.mouse_delta.y / zoom}) / (UNIT_SIZE);
        editor.dragging_entity->position += move_delta;
    }
    
    //editor Entity to mouse or go to entity
    if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.dragging_entity != NULL){
        editor.dragging_entity->position = input.mouse_position;
    } else if (can_control_with_single_button && IsKeyPressed(KEY_F) && editor.selected_entity != NULL){
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
            // something_in_undo = true;
            // undo_action.rotation_change = editor.selected_entity->rotation - editor.rotating_start;
            // undo_action.entity = editor.selected_entity;
            // undo_apply_vertices_change(editor.selected_entity, &undo_action);
            undo_add_rotation(editor.selected_entity, editor.selected_entity->rotation - editor.rotating_start);
            editor.is_rotating_entity = false;
        } 
    }
    
    //editor entity scaling
    if (can_control_with_single_button && editor.selected_entity != NULL){
        Vector2 scaling = {};
        f32 speed = 40;
        
        // if (IsKeyDown(KEY_LEFT_SHIFT)){
        //     speed *= 3.0f;
        //}
        
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
            
            //undo_action.vertices_change.clear();
            //VECTOR2_ARR_FILL_ZERO(undo_action.vertices_change.data, editor.selected_entity->vertices.count)
            //undo_apply_vertices_change(editor.selected_entity, &undo_action);
            
            undo_add_scaling(editor.selected_entity, scale_change);
            editor.is_scaling_entity = false;
        } 
    }
    
    //editor components management
    if (editor.selected_entity){
        Entity *selected = editor.selected_entity;
        if (selected->flags & AGRO_AREA){
            b32 wanna_assign = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_A);
            b32 wanna_remove = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_D);
            //agro assign or remove
            if (wanna_assign || wanna_remove){
                fill_collisions(&mouse_entity, &collisions_data, ENEMY);
                
                for (int i = 0; i < collisions_data.count; i++){
                    Collision col = collisions_data.get(i);
                    
                    if (wanna_assign && !wanna_remove && !selected->agro_area.connected.contains(col.other_entity->id)){
                        selected->agro_area.connected.add(col.other_entity->id);
                        break;
                    } else if (wanna_remove && !wanna_assign && selected->agro_area.connected.contains(col.other_entity->id)){
                        selected->agro_area.connected.remove(i);
                        break;
                    }
                }
            }
            //agro clear
            if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_L)){
                selected->agro_area.connected.clear();
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
            
            calculate_bounds(undo_entity);
        }
    }
    
    //editor Save level
    if (IsKeyPressed(KEY_J) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL)){
        save_level("test_level.level");
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        editor.last_click_position = input.mouse_position;
        editor.last_click_time = core.time.game_time;
    }
    
    if (IsKeyPressed(KEY_P) && !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL)){
        editor.player_spawn_point = input.mouse_position;
    }

    clicked_ui = false;
} // update editor end

void calculate_bounds(Entity *entity){
    // if (entity->flags & TEXTURE){
    //     //Vector2 middle_position = {entity->texture.width * (0.5f - p
    //     entity->bounds = {{(f32)entity->texture.width / UNIT_SIZE, (f32)entity->texture.height / UNIT_SIZE}, {0.0f, 0.0f}};
    //     return;
    // }

    f32 top_vertex    = -INFINITY;
    f32 bottom_vertex =  INFINITY;
    f32 right_vertex  = -INFINITY;
    f32 left_vertex   =  INFINITY;
    
    Vector2 middle_position;
    
    for (int i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        
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
    
    middle_position = {(1.0f - entity->pivot.x) * left_vertex + entity->pivot.x * right_vertex,
                       entity->pivot.y * bottom_vertex + (1.0f - entity->pivot.y) * top_vertex};
    
    entity->bounds = {{right_vertex - left_vertex, top_vertex - bottom_vertex}, middle_position};
}

void change_scale(Entity *entity, Vector2 new_scale){
    Vector2 old_scale = entity->scale;
    
    entity->scale = new_scale;
    
    clamp(&entity->scale.x, 0.01f, 10000);
    clamp(&entity->scale.y, 0.01f, 10000);

    for (int i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        Vector2 unscaled_vertex = entity->unscaled_vertices.get(i);
        
        Vector2 dir_to_vertex = normalized(unscaled_vertex - entity->position);
        
        f32 up_dot    = dot(entity->up,    unscaled_vertex);
        f32 right_dot = dot(entity->right, unscaled_vertex);

        *vertex = normalized(unscaled_vertex) + (entity->up * up_dot * entity->scale.y) + (entity->right * right_dot * entity->scale.x);
    }
    
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
    entity->right = {cosf(new_rotation * DEG2RAD), -sinf(new_rotation * DEG2RAD)};
    
    for (int i = 0; i < entity->vertices.count; i++){
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
    if (input.direction.y < 0){
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
    if (dot(player_data.velocity, input.direction) <= 0){
        acceleration = player_data.ground_deceleration;
        if (input.direction.y < 0){
            acceleration *= 0.3f;
        }
    }
    
    f32 wish_speed = sqr_magnitude(input.direction) * max_move_speed;
    
    player_accelerate(entity, input.direction, wish_speed, acceleration, dt);
}

void player_air_move(Entity *entity, f32 dt){
    f32 max_move_speed = player_data.base_move_speed;
    
    f32 acceleration = dot(player_data.velocity, input.direction) > 0 ? player_data.air_acceleration : player_data.air_deceleration;
    
    f32 wish_speed = sqr_magnitude(input.direction) * max_move_speed;
    
    player_accelerate(entity, input.direction, wish_speed, acceleration, dt);
}

void add_blood_amount(Player *player, Entity *sword, f32 added){
    player->blood_amount += added;
    clamp(&player->blood_amount, 0, player->max_blood_amount);
    player->blood_progress = player->blood_amount / player->max_blood_amount;
    
    Vector2 sword_target_scale = player->sword_start_scale * 2;
    change_scale(sword, lerp(player->sword_start_scale, sword_target_scale, player->blood_progress * player->blood_progress));
}

void win_level(){
    if (!context.we_got_a_winner){    
        kill_player();
        context.we_got_a_winner = true;
    }
}

void calculate_sword_collisions(Entity *sword, Entity *player_entity){
    fill_collisions(sword, &player_data.collisions, GROUND | ENEMY | WIN_BLOCK);
    
    Player *player = &player_data;
    
    for (int i = 0; i < player->collisions.count; i++){
        Collision col = player->collisions.get(i);
        Entity *other = col.other_entity;
        
        if (other->flags & ENEMY && player->sword_spin_progress > 0.12f && !other->enemy.dead_man){
            kill_enemy(other, sword->position + sword->up * sword->scale.y * sword->pivot.y, col.normal);
            
            f32 max_speed_boost = 6 * player->sword_spin_direction;
            if (!player->grounded){
                max_speed_boost *= -1;
            }
            f32 max_vertical_speed_boost = player->grounded ? 0 : 20;
            if (player_data.velocity.y > 0){
                max_vertical_speed_boost *= 0.3f;   
            }
            f32 spin_t = player->sword_spin_progress;
            player->velocity += Vector2_up   * lerp(0.0f, max_vertical_speed_boost, spin_t * spin_t)
                             + Vector2_right * lerp(0.0f, max_speed_boost, spin_t * spin_t); 
                             
            add_blood_amount(player, sword, 10);
        }
        
        if (other->flags & WIN_BLOCK){
            win_level();
        }
    }
}

void update_player(Entity *entity, f32 dt){
    assert(entity->flags & PLAYER);

    if (player_data.dead_man){
        return;
    }

    //Player *p player_data;
    Entity *ground_checker = context.entities.get_by_key_ptr(player_data.ground_checker_id);
    Entity *sword          = context.entities.get_by_key_ptr(player_data.sword_entity_id);
    
    ground_checker->position = entity->position - entity->up * entity->scale.y * 0.5f;
    sword->position = entity->position;
    
    Vector2 sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
    
    Vector2 vec_to_mouse = input.mouse_position - entity->position;
    Vector2 dir_to_mouse = normalized(vec_to_mouse);
    //Vector2 vec_tip_to_mouse = input.mouse_position - sword_tip;
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        chainsaw_emitter->position = input.mouse_position;
        chainsaw_emitter->last_emitted_position = input.mouse_position;
        chainsaw_emitter->enabled = true;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
        chainsaw_emitter->enabled = false;
    }
    
    player_data.sword_angular_velocity *= 1.0f - (dt);
    
    b32 can_sword_spin = !player_data.rifle_active;
    
    if (can_sword_spin){
        f32 sword_spin_sense = 2;
        if (can_sword_spin && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            player_data.sword_angular_velocity += input.mouse_delta.x * sword_spin_sense;
        } else{
            //chainsaw_emitter->last_emitted_position = input.mouse_position;
        }
    }
    
    b32 can_shoot_rifle = player_data.rifle_active;
    if (can_shoot_rifle && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        //rifle shoot
        player_data.rifle_active = false;
        
        f32 projectile_speed = 1800;
        
        Entity *projectile_entity = add_entity(sword_tip, {2, 2}, {0.5f, 0.5f}, 0, PINK, PROJECTILE | PARTICLE_EMITTER);
        projectile_entity->projectile.velocity = sword->up * projectile_speed;
        projectile_entity->projectile.max_lifetime = 0.5f;
        copy_emitter(&projectile_entity->emitter, &rifle_bullet_emitter, sword_tip);
    }
    
    if (player_data.rifle_active){
        change_up(sword, move_towards(sword->up, dir_to_mouse, 100, dt));        
    }
    
    //rifle activate
    b32 can_activate_rifle = !player_data.rifle_active && !can_shoot_rifle;
    if (can_activate_rifle && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        player_data.rifle_active = true;
        
        player_data.rifle_current_power += player_data.sword_spin_progress * 50;
        //player_data.sword_spin_progress *= 0.1f;
        player_data.sword_angular_velocity = 0;
    }
    
    f32 sword_max_spin_speed = 5000;
    player_data.sword_spin_progress = clamp01(abs(player_data.sword_angular_velocity) / sword_max_spin_speed);
    
    sword->color_changer.progress = player_data.blood_progress * player_data.blood_progress;//player_data.sword_spin_progress * player_data.sword_spin_progress;
    
    { 
        f32 spin_t = player_data.sword_spin_progress;
        f32 blood_t = player_data.blood_progress;
    
        chainsaw_emitter->position = input.mouse_position;
        chainsaw_emitter->lifetime_multiplier = 1.0f + spin_t * spin_t * 2; //@VISUAL: change color
        chainsaw_emitter->speed_multiplier    = 1.0f + spin_t * spin_t * 2; //@VISUAL: change color
        
        sword_tip_emitter->position = sword_tip;
        sword_tip_emitter->lifetime_multiplier = 1.0f + blood_t * blood_t * 3.0f;
        sword_tip_emitter->speed_multiplier    = 1.0f + blood_t * blood_t * 5.0f;
        sword_tip_emitter->count_multiplier    = blood_t * blood_t * 2.0f;
              
        f32 blood_decease = 5 + blood_t * 10;
              
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
    
    //player movement
    if (player_data.grounded){
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
            
            f32 max_spin_acceleration = 500;
            f32 min_spin_acceleration = 200;
            f32 spin_acceleration = lerp(min_spin_acceleration, max_spin_acceleration, blood_t * blood_t);
            player_data.velocity += plane * lerp(0.0f, spin_acceleration, spin_t * spin_t) * dt;
        }
        
        player_data.since_airborn_timer = 0;
    } else{
        if (player_data.velocity.y > 0){
            f32 max_height_jump_time = 0.2f;
            f32 jump_t = clamp01(player_data.since_jump_timer / max_height_jump_time);
            player_data.gravity_mult = lerp(2.0f, 1.0f, jump_t * jump_t * jump_t);
            
            if (player_data.since_jump_timer > 0.3f){ //so we don't care about jump gravity
                 if (input.direction.y < 0){
                    player_data.gravity_mult = 5;
                 } else{
                    player_data.gravity_mult = lerp(1.0f, 0.5f, player_data.sword_spin_progress * player_data.sword_spin_progress);
                 }
            }
            
        } else{
            if (input.direction.y < 0){
                player_data.gravity_mult = 5;
            } else{
                player_data.gravity_mult = lerp(1.0f, 0.5f, player_data.sword_spin_progress * player_data.sword_spin_progress);
            }
        }
        
        if (1 /*!sword_attacking*/){
            player_air_move(entity, dt);
            
            player_data.velocity.y -= player_data.gravity * player_data.gravity_mult * dt;
        }
        
        player_data.since_airborn_timer += dt;
        
        if (player_data.sword_spin_progress > 0.3f){
            f32 spin_t = player_data.sword_spin_progress;
            f32 blood_t = player_data.blood_progress;
            
            f32 max_spin_acceleration = 200;
            f32 min_spin_acceleration = 50;
            f32 spin_acceleration = lerp(min_spin_acceleration, max_spin_acceleration, blood_t * blood_t);
        
            f32 airborn_reduce_spin_acceleration_time = 0.5f;
            f32 t = clamp01(spin_t - clamp01(airborn_reduce_spin_acceleration_time - player_data.since_airborn_timer));
            player_data.velocity.x += lerp(0.0f, spin_acceleration, t * t) * dt * -player_data.sword_spin_direction;
        }
        
    }
    
    if (player_data.grounded && IsKeyPressed(KEY_SPACE)){
        player_data.velocity.y = player_data.jump_force;
        player_data.since_jump_timer = 0;
    }
    
    
    Vector2 next_pos = {entity->position.x + player_data.velocity.x * dt, entity->position.y + player_data.velocity.y * dt};
    
    entity->position = next_pos;
    
    f32 found_ground = false;
    f32 just_grounded = false;
    
    fill_collisions(ground_checker, &player_data.collisions, GROUND);
    for (int i = 0; i < player_data.collisions.count; i++){
        Collision col = player_data.collisions.get(i);
        assert(col.collided);
        
        if (dot(col.normal, player_data.velocity) >= 0){
            continue;
        }
            
        entity->position.y += col.overlap;
        if (dot(((Vector2){0, 1}), col.normal) > 0.5f){
            player_data.velocity -= col.normal * dot(player_data.velocity, col.normal);
        }
        
        f32 angle = fangle(col.normal, entity->up);
        
        if (angle <= player_data.max_ground_angle){
            found_ground = true;
            player_data.ground_normal = col.normal;
            
            if (!player_data.grounded && !just_grounded){
                
                player_data.plane_vector = get_rotated_vector_90(player_data.ground_normal, -normalized(player_data.velocity.x));
                player_data.velocity = player_data.plane_vector * magnitude(player_data.velocity);
                just_grounded = true;
            }
        }
    }
    
    fill_collisions(entity, &player_data.collisions, GROUND);
    for (int i = 0; i < player_data.collisions.count; i++){
        Collision col = player_data.collisions.get(i);
        assert(col.collided);
        
        if (dot(col.normal, player_data.velocity) >= 0){
            continue;
        }
        
        resolve_collision(entity, col);
        
        player_data.velocity -= col.normal * dot(player_data.velocity, col.normal);
    }
    
    player_data.grounded = found_ground;
}

inline void calculate_collisions(void (respond_func)(Entity*, Collision), Entity *entity){
    fill_collisions(entity, &collisions_data, entity->collision_flags);
    
    for (int i = 0; i < collisions_data.count; i++){
        Collision col = collisions_data.get(i);
        respond_func(entity, col);
    }
}

void respond_bird_collision(Entity *bird_entity, Collision col){
    assert(bird_entity->flags & BIRD_ENEMY);

    Bird_Enemy *bird = &bird_entity->bird_enemy;
    Entity *other = col.other_entity;
    
    if (other->flags & GROUND){
        resolve_collision(bird_entity, col);
        bird->velocity = reflected_vector(bird->velocity * 0.4f, col.normal);
        bird->attacking = false;
        bird->attack_timer = 0;
        bird->roaming = true;
    }
    
    if (other->flags & BIRD_ENEMY && !bird->attacking){
        resolve_collision(bird_entity, col);
        bird->velocity = reflected_vector(bird->velocity * 0.8f, col.normal);
    }
    
    if (other->flags & PLAYER && !player_data.dead_man){
        if (player_data.sword_spin_progress < 0.4f){
            kill_player();
        } else if (player_data.sword_spin_progress >= 0.4f){
            kill_enemy(bird_entity, bird_entity->position, bird->velocity);
        }
    }
}

// inline void calculate_bird_collisions(Entity *bird_entity){
//     calculate_collisions(&respond_bird_collision, bird_entity, GROUND | BIRD_ENEMY | PLAYER);
// }    

void move_by_velocity_with_collisions(Entity *entity, Vector2 velocity, f32 max_frame_move_len, void (respond_collision_func)(Entity*, Collision), f32 dt){
    Vector2 this_frame_move_direction = normalized(velocity);
    f32 this_frame_move_len = magnitude(velocity * dt); 
    
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
    
    if (!entity->enemy.in_agro){
        return;
    }
    
    Bird_Enemy *bird = &entity->bird_enemy;
    
    Vector2 vec_to_player = player_entity->position - entity->position;
    Vector2 dir_to_player = normalized(vec_to_player);
    f32    distance_to_player = magnitude(vec_to_player);
    
    if (bird->roaming){
        bird->roam_timer += dt;
        
        if (bird->roam_timer >= bird->max_roam_time){
            bird->roam_timer = 0;
            bird->roaming = false;
            bird->charging_attack = true;
            bird->velocity = Vector2_zero;
        }
    }
    
    if (bird->charging_attack){
        bird->charging_attack_timer += dt;
        if (bird->charging_attack_timer >= bird->max_charging_attack_time){
            change_up(entity, dir_to_player);         
            bird->charging_attack_timer = 0;            
            bird->charging_attack = false;
            bird->attacking = true;
            
            f32 bird_attack_speed = 300;
            bird->velocity = dir_to_player * bird_attack_speed;
            
        } else{
            change_up(entity, dir_to_player);         
            f32 charging_back_movement_amount = 3.0f;
            
            entity->position -= entity->up * charging_back_movement_amount * dt;
        }
    }
    
    if (bird->attacking){
        bird->attack_timer += dt;
        if (bird->attack_timer >= bird->max_attack_time){
            bird->attack_timer = 0;
            bird->attacking = false;
            bird->roaming = true;
        } else{
            f32 speed = magnitude(bird->velocity);
            change_up(entity, lerp(entity->up, dir_to_player, dt));
            bird->velocity = entity->up * speed;
            move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
        }
    }
    
    if (bird->roaming){
        bird->target_position = player_entity->position + Vector2_up * 60;        
        bird->target_position.y += sinf(core.time.game_time * entity->position.y) * 10;
        bird->target_position.x += cosf(core.time.game_time * entity->position.x) * 50;
        bird->velocity += (bird->target_position - entity->position) * bird->roam_acceleration * dt;
        clamp_magnitude(&bird->velocity, bird->max_roam_speed);
        
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
        
        change_up(entity, bird->velocity);
    }
    
    // f32 max_frame_move_len = entity->scale.y * 0.8f;
    // Vector2 this_frame_move_direction = normalized(bird->velocity);
    // f32 this_frame_move_len = magnitude(bird->velocity * dt); 
    
    // while(this_frame_move_len > max_frame_move_len){
    //     entity->position += this_frame_move_direction * max_frame_move_len;
    //     calculate_bird_collisions(entity);
    //     this_frame_move_len -= max_frame_move_len;
    //     this_frame_move_direction = normalized(bird->velocity);
    // }
    
    // entity->position += this_frame_move_direction * this_frame_move_len;
    // calculate_bird_collisions(entity);
    
    //change_up(entity, bird->velocity);
    //entity->position += bird->velocity * dt;
}

void kill_enemy(Entity *enemy_entity, Vector2 kill_position, Vector2 kill_direction){
    assert(enemy_entity->flags & ENEMY);
    
    if (!enemy_entity->enemy.dead_man){
        emit_particles(*blood_emitter, kill_position, kill_direction, 1, 1);
    
        enemy_entity->enemy.dead_man = true;
        enemy_entity->enabled = false;
        enemy_entity->destroyed = true;
    }
}

void calculate_projectile_collisions(Entity *entity){
    fill_collisions(entity, &player_data.collisions, GROUND | ENEMY | WIN_BLOCK);
    
    Player *player = &player_data;
    Projectile *projectile = &entity->projectile;
    
    for (int i = 0; i < player->collisions.count; i++){
        Collision col = player->collisions.get(i);
        Entity *other = col.other_entity;
        
        if (other->flags & ENEMY && !other->enemy.dead_man){
            kill_enemy(other, entity->position, col.normal);    
        }
        
        if (other->flags & GROUND){
            if (fangle(col.normal, entity->projectile.velocity) - 90 < 40){
                projectile->velocity = reflected_vector(projectile->velocity, col.normal);
            } else{
                entity->destroyed = true;
            }
        }
        
        if (other->flags & WIN_BLOCK){
            win_level();
        }
    }
}

void update_projectile(Entity *entity, f32 dt){
    assert(entity->flags & PROJECTILE);
    
    Projectile *projectile = &entity->projectile;
    
    projectile->lifetime += dt;
    
    if (projectile->max_lifetime > 0 && projectile->lifetime > projectile->max_lifetime){
        entity->destroyed = true;    
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
}

void agro_area_verify_connected(Entity *e){
    for (int i = 0; i < e->agro_area.connected.count; i++){   
        //So if entiity was somehow destoyed, annighilated
        if (!context.entities.has_key(e->agro_area.connected.get(i))){
            e->agro_area.connected.remove(i);
            i--;
            continue;
        }
    }
}

void update_editor_entity(Entity *e){
    if (e->flags & AGRO_AREA){
        agro_area_verify_connected(e);
    }
}

void update_agro_area(Entity *e){
    assert(e->flags & AGRO_AREA);
    
    Agro_Area *area = &e->agro_area;
    
    if (check_entities_collision(e, player_entity).collided){
        for (int i = 0; i < area->connected.count; i++){
            int id = area->connected.get(i);
            
            if (!context.entities.has_key(e->agro_area.connected.get(i))){
                e->agro_area.connected.remove(i);
                i--;
                continue;
            }
            
            assert(context.entities.has_key(id));
            
            Entity *connected_entity = context.entities.get_by_key_ptr(id);
            assert(connected_entity->flags & ENEMY);
            
            connected_entity->enemy.in_agro = true;
        }
    }
}

void update_entities(){
    Context *c = &context;
    Hash_Table_Int<Entity> *entities = &c->entities;
    
    for (int i = 0; i < entities->max_count; i++){
        if (!entities->has_index(i)){
            continue;
        }
    
        Entity *e = entities->get_ptr(i);
        
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
            entities->remove_index(i);    
            //i--;
            
            if (game_state == EDITOR){
                editor.need_validate_entity_pointers = true;
            }
            continue;
        }
        
        if (!e->enabled || e->flags == -1){
            continue;
        }
        
        update_color_changer(e, core.time.dt);            
        
        if (game_state == EDITOR){
            update_editor_entity(e);
            continue;
        }
        
        if (e->flags & PLAYER){
            update_player(e, core.time.dt);
        }
          
        if (e->flags & BIRD_ENEMY){
            update_bird_enemy(e, core.time.dt);
        }
        
        if (e->flags & PROJECTILE){
            update_projectile(e, core.time.dt);
        }
        
        if (e->flags & PARTICLE_EMITTER){
            e->emitter.position = e->position;
            update_emitter(&e->emitter);
        }
        
        if (e->flags & AGRO_AREA){
            update_agro_area(e);
        }
    }
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

void draw_sword(Entity *entity){
    assert(entity->flags & SWORD);
    
    Entity visual_entity = *entity;
    if (player_data.rifle_active){
        visual_entity.position += rnd_in_circle() * 0.5f;
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
    if (entity->bird_enemy.charging_attack){
        f32 charging_progress = entity->bird_enemy.charging_attack_timer / entity->bird_enemy.max_charging_attack_time;
        visual_entity.position += rnd_in_circle() * lerp(0.0f, 1.0f, charging_progress * charging_progress);
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

void fill_entities_draw_queue(){
    context.entities_draw_queue.clear();

    for (int i = 0; i < context.entities.max_count; i++){
        if (!context.entities.has_index(i)){
            continue;
        }
        
        Entity e = context.entities.get(i);
        
        if (!e.enabled){
            continue;
        }
        
        Bounds cam_bounds;
        cam_bounds.size = {(f32)screen_width, (f32)screen_height};
        cam_bounds.size /= context.cam.cam2D.zoom;
        cam_bounds.size /= UNIT_SIZE;
        
        cam_bounds.offset = {0, 0};
        if (!check_bounds_collision(context.cam.view_position, cam_bounds, e.position, e.bounds)){
            continue;
        }
        
        context.entities_draw_queue.add(e);
    }
    
    qsort(context.entities_draw_queue.data, context.entities_draw_queue.count, sizeof(Entity), compare_entities_draw_order);
}

void draw_entities(){
    fill_entities_draw_queue();

    //Hash_Table_Int<Entity> *entities = &context.entities;
    Dynamic_Array<Entity> *entities = &context.entities_draw_queue;
    
    for (int i = 0; i < entities->count; i++){
        // if (!entities->has_index(i)){
        //     continue;
        // }
    
        Entity *e = entities->get_ptr(i);
    
        if (!e->enabled/* || e->flags == -1*/){
            continue;
        }
        
        if (e->flags & TEXTURE){
            draw_game_texture(e->texture, e->position, e->scale, e->pivot, e->rotation, e->color);
        }
        
        if (e->flags & GROUND || e->flags == 0 || e->flags & PROJECTILE){
            if (e->vertices.count > 0){
                draw_game_triangle_strip(e);
            } else{
                draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
            }
        }
        
        if (e->flags & PLAYER){
            draw_player(e);
        }
        
        if (e->flags & SWORD){
            draw_sword(e);
        }
        
        if (e->flags & BIRD_ENEMY){
            draw_bird_enemy(e);
        } else if (e->flags & ENEMY){
            draw_enemy(e);
        }
        
        if (e->flags & WIN_BLOCK){
            draw_game_triangle_strip(e);
            Vector2 line_from = e->position - e->up * e->win_block.kill_direction.y * e->scale.y * 0.5f - e->right * e->win_block.kill_direction.x * e->scale.x * 0.5f;
            Vector2 line_to   = e->position + e->up * e->win_block.kill_direction.y * e->scale.y * 0.5f + e->right * e->win_block.kill_direction.x * e->scale.x * 0.5f;
            draw_game_line(line_from, line_to, 0.8f, PURPLE * 0.9f);
        }
        
        if (e-> flags & DRAW_TEXT){
            draw_game_text(e->position, e->text_drawer.text, e->text_drawer.size, RED);
        }
        
        if (e->flags & AGRO_AREA && (game_state == EDITOR || debug.draw_areas_in_game)){
            //draw_game_rect_lines(e->position, e->scale, e->pivot, 5, e->color);
            draw_game_line_strip(e, e->color);
            draw_game_triangle_strip(e, e->color * 0.1f);
            
            b32 agro_selected = editor.selected_entity && editor.selected_entity->id == e->id;
            for (int ii = 0; agro_selected && ii < e->agro_area.connected.count; ii++){
                int id = e->agro_area.connected.get(ii);
                
                Entity *connected_enemy = context.entities.get_by_key_ptr(id);
                
                draw_game_line(e->position, connected_enemy->position, 0.2f, RED);
            }
        }
        
        if (e->flags & TEST){
        }
        
        if (game_state == EDITOR || debug.draw_up_right){
            draw_game_line(e->position, e->position + e->right * 3, 0.3f, RED);
            draw_game_line(e->position, e->position + e->up    * 3, 0.3f, GREEN);
        }
        
        if (debug.draw_bounds || editor.selected_entity && game_state == EDITOR && e->id == editor.selected_entity->id){
            draw_game_rect_lines(e->position + e->bounds.offset, e->bounds.size, e->pivot, 2, GREEN);
            //draw_game_text(e->position, TextFormat("{%.2f, %.2f}", e->bounds.offset.x, e->bounds.offset.y), 22, PURPLE);
        }
    }
}

void draw_editor(){
    f32 closest_len = 1000000;
    Entity *closest;

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
        
        b32 draw_circles_on_vertices = IsKeyDown(KEY_LEFT_ALT) && true;
        if (draw_circles_on_vertices){
            for (int v = 0; v < e->vertices.count; v++){
                draw_game_circle(global(e, e->vertices.get(v)), 1, PINK);
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
    
    if (editor.dragging_entity != NULL){
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
    for (int i = 0; i < context.particles.count; i++){
        Particle particle = context.particles.get(i);
        
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
        
        if (element.ui_flags & BUTTON){
            Button button = element.button;
            
            draw_rect(element.position, element.size, element.pivot, 0, element.color);
        }
        
        if (element.ui_flags & UI_IMAGE){
            Ui_Image ui_image = element.ui_image;
            draw_rect(element.position, element.size, element.pivot, 0, element.color);
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
        
        f32 color_multiplier = 0.8f;
        if (input_field.in_focus){
            f32 blink_speed = 4.0f;
            color_multiplier = lerp(0.5f, 0.8f, (sinf(core.time.game_time * blink_speed) + 1) * 0.5f);
        }
        draw_rect(input_field.position, input_field.size, {0, 0}, 0, input_field.color * color_multiplier);
        
        draw_text(input_field.content, input_field.position, input_field.font_size, WHITE * 0.9f);
    }
    
    if (tag_len == 0){
        ui_context.elements.clear();
        input_fields.clear();
    }
}

void draw_game(){
    BeginDrawing();
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
    
    if (game_state == EDITOR){
        draw_editor();
    }
    
    EndMode2D();
    
    draw_ui("");
    
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
        draw_text(TextFormat("Particles count: %d", context.particles.count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (debug.info_emitters_count){
        draw_text(TextFormat("Emitters count: %d", context.emitters.count), 10, v_pos, font_size, RED);
        v_pos += font_size;
    }
    
    if (console.is_open){
        //draw console
        
        Color text_color = lerp(GREEN * 0, GREEN, console.open_progress * console.open_progress);
        f32 y_position = lerp(-screen_height * 0.6f, 0.0f, EaseOutQuint(console.open_progress));
        
        draw_rect({0, y_position}, {(f32)screen_width, screen_height * 0.5f}, BLUE * 0.2f);
        draw_text_boxed(console.str.data, {4, 4 + y_position, (f32)screen_width, screen_height * 0.5f - 30.0f}, 16, 3, text_color);
    } else{
        f32 since_console_closed = core.time.game_time - console.closed_time;
        
        if (since_console_closed <= 0.4f){
            f32 t = clamp01(since_console_closed / 0.4f);
            
            Color text_color = lerp(GREEN, GREEN * 0, t * t);
            f32 y_position = lerp(0.0f, -screen_height * 0.6f, EaseOutQuint(t));
            
            draw_rect({0, y_position}, {(f32)screen_width, screen_height * 0.5f}, BLUE * 0.2f);
            draw_text_boxed(console.str.data, {4, 4 + y_position, (f32)screen_width, screen_height * 0.5f - 30.0f}, 16, 3, text_color);
        }
    }
    
    EndDrawing();
}

void setup_color_changer(Entity *entity){
    entity->color_changer.start_color = entity->color;
    entity->color_changer.target_color = entity->color * 1.4f;
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
        e.id = context.entities.total_added_count + core.time.game_time * 10000 + 100;
    }
    
    check_avaliable_ids_and_set_if_found(&e.id);
        
    context.entities.add(e.id, e);
    //setup_color_changer(
    return context.entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    //Entity *e = add_entity(pos, scale, rotation, flags);    
    Entity e = Entity(pos, scale, pivot, rotation, flags);    
    e.id = context.entities.total_added_count + core.time.game_time * 10000 + 100;
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

inline Vector2 local(Entity *e, Vector2 global_pos){
    return global_pos - e->position;
}

Vector2 world_to_screen(Vector2 position){
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

void draw_game_line_strip(Entity *entity, Color color){
    Vector2 screen_positions[entity->vertices.count];
    
    for (int i = 0; i < entity->vertices.count; i++){
        screen_positions[i] = world_to_screen(global(entity, entity->vertices.get(i)));
    }
    
    draw_line_strip(screen_positions, entity->vertices.count, color);
}

void draw_game_triangle_strip(Entity *entity, Color color){
    Vector2 screen_positions[entity->vertices.count];
    
    for (int i = 0; i < entity->vertices.count; i++){
        screen_positions[i] = world_to_screen(global(entity, entity->vertices.get(i)));
    }
    
    draw_triangle_strip(screen_positions, entity->vertices.count, color);
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
    Vector2 screen_pos = world_to_screen(position);
    draw_texture(tex, screen_pos, scale, pivot, rotation, color);
}

void draw_game_line(Vector2 start, Vector2 end, f32 thick, Color color){
    draw_line(world_to_screen(start), world_to_screen(end), thick * UNIT_SIZE, color);
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
