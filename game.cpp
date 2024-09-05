#pragma once

//#define assert(a) (if (!a) (int*)void*);
//#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

f32 dt;
f32 game_time;

#define FLAGS i32
//#define EPSILON 0.0000000000000001f

enum Flags{
    GROUND = 1 << 0,
    DRAW_TEXT = 1 << 1
};

struct Ground{
      
};

struct Color_Changer{
    b32 changing = 0;

    Color start_color;
    Color target_color;
    
    f32 change_time = 2.0f;
};

struct Text_Drawer{
    const char *text;  
    f32 size = 30;
};

struct Entity{
    Entity();
    Entity(Vector2 _pos);
    Entity(Vector2 _pos, Vector2 _scale);
    Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags);
    Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);
    Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);
    Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags, Array<Vector2> _vertices);

    i32 id = -1;

    b32 enabled = 1;
    
    b32 destroyed = 0;
    
    Array<Vector2> vertices = Array<Vector2>(20);
    
    Vector2 up = {0, 1};
    Vector2 right = {1, 0};
    
    FLAGS flags;
    //b32 need_to_destroy = 0;
    
    //lower - closer to camera
    i32 draw_order = 1;
    
    Vector2 position;
    Vector2 scale = {1, 1};
    Vector2 bounds = {1, 1};
    Vector2 pivot = {0.5f, 1.0f};
    f32 rotation = 0;
    
    Color color = WHITE;
    
    Ground ground;
    Color_Changer color_changer;
    Text_Drawer text_drawer;
};

//scale 150 should be full screen;

struct Cam{
    Vector2 position;
    float rotation;
    
    Camera2D cam2D = {};
};

struct Context{
    Array<Entity> entities = Array<Entity>(1000);

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

struct Collision{
    b32 collided;
    f32 overlap;
    
    Vector2 normal;    
    Vector2 point;
    Vector2 dir_to_first;
};

struct Circle{
    Vector2 position;  
    f32 radius;
};

struct Editor{
    Entity  *selected_entity;
    Entity  *dragging_entity;
    Entity  *moving_vertex_entity;
    Entity  *cursor_entity;
    
    Vector2 *last_selected_vertex;
    Vector2 *moving_vertex;
    
    b32 selected_this_click = 0;
    
    f32 dragging_time = 0;
    
    Collision last_collision;
    
    Vector2 player_spawn_point = {0, 0};
};

global_variable Input input;
global_variable Level current_level;
global_variable Context context = {};
global_variable Editor  editor  = {};

#include "game.h"

void free_entity(Entity *e){
    e->vertices.free_arr();
}

void add_rect_vertices(Array<Vector2> *vertices){
    vertices->add({0.5f, 0.5f});
    vertices->add({-0.5f, 0.5f});
    vertices->add({0.5f, -0.5f});
    vertices->add({-0.5f, -0.5f});
}
Entity::Entity(){
    
}

Entity::Entity(Vector2 _pos){
    position = _pos;
    // vertices.add({-0.5f, 0.5f});
    // vertices.add({0.5f, 0.5f});
    // vertices.add({0.5f, -0.5f});
    //vertices.add({-0.5f, -0.5f});
    add_rect_vertices(&vertices);

    change_scale(this, {1, 1});
    rotation = 0;
    up = {0, 1};
    right = {1, 0};
    
    flags = 0;
}

Entity::Entity(Vector2 _pos, Vector2 _scale){
    position = _pos;
    
    add_rect_vertices(&vertices);

    change_scale(this, _scale);
    rotation = 0;
    
    rotation = 0;
    up = {0, 1};
    right = {1, 0};
    flags = 0;
}

Entity::Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags){
    position = _pos;
    add_rect_vertices(&vertices);
    

    change_scale(this, _scale);
    rotation = 0;
    
    rotate_to(this, _rotation);
    flags    = _flags;
}

Entity::Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags){
    position = _pos;
    pivot = _pivot;
    
    add_rect_vertices(&vertices);
    
    change_scale(this, _scale);
    rotation = 0;
    
    rotate_to(this, _rotation);
    flags    = _flags;
    
    
}

Entity::Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags){
    id = _id;
    position = _pos;
    pivot = _pivot;
    
    add_rect_vertices(&vertices);
    
    change_scale(this, _scale);
    rotation = 0;
    rotate_to(this, _rotation);
    flags    = _flags;
}


Entity::Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags, Array<Vector2> _vertices){
    id = _id;
    position = _pos;
    pivot = _pivot;
    
    //add_rect_vertices(&vertices);
    vertices.free_arr();
    vertices = _vertices;
    
    change_scale(this, _scale);
    rotation = 0;
    rotate_to(this, _rotation);
    flags    = _flags;
}


Entity *zoom_entity;

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

void init_game(){
    game_state = EDITOR;

    game_time = 0;

    input = {};

    current_level = {};
    current_level.context = (Context*)malloc(sizeof(Context));
    context = *current_level.context;
    context = {};    
    
    //load level
    FILE *fptr = fopen("test_level.level", "r");
    
    if (fptr == NULL){
        fptr = fopen("../test_level.level", "r");
    }
    
    const unsigned MAX_LENGTH = 1000;
    char buffer[MAX_LENGTH];

    while (fptr != NULL && fgets(buffer, MAX_LENGTH, fptr)){
        if (strcmp(buffer, "Entities:\n") == 0){
            continue;   
        }
    
        i32 entity_id;
        Vector2 entity_position;
        Vector2 entity_scale;
        Vector2 entity_pivot;
        f32     entity_rotation;
        Color   entity_color;
        FLAGS   entity_flags;
        
        Array<Vector2> entity_vertices = Array<Vector2>(20); 
        Vector2 vertex;
        
        b32 found_id = false;
        b32 found_position_x = false;
        b32 found_position_y = false;
        b32 found_scale_x    = false;
        b32 found_scale_y    = false;
        b32 found_pivot_x    = false;
        b32 found_pivot_y    = false;
        b32 found_rotation   = false;
        b32 found_color_r    = false;
        b32 found_color_g    = false;
        b32 found_color_b    = false;
        b32 found_color_a    = false;
        b32 found_flags      = false;
        
        b32 parsing_vertices = false;
        b32 found_vertex_x   = false;
        b32 found_vertex_y   = false;
        b32 found_vertices   = false;
        
        i32 parsed_count = 50;
        char parsed_data[parsed_count];
        
        for (int i = 0; buffer[i] != NULL && buffer[i] != ';'; i++){
            if (buffer[i] == '['){
                parsing_vertices = true;
            }
            
            if (buffer[i] == ']' && parsing_vertices){
                parsing_vertices = false;
                found_vertices = true;
            }
        
            if (buffer[i] == ':'){
                zero_array(parsed_data, parsed_count);
                parse_line(buffer, parsed_data, &i);
                
                if (!found_id){
                    entity_id = atoi(parsed_data);
                    found_id = true;
                } else if (!found_position_x){
                    entity_position.x = atof(parsed_data);
                    found_position_x = true;
                } else if (!found_position_y){
                    entity_position.y = atof(parsed_data);
                    found_position_y = true;
                } else if (!found_scale_x){
                    entity_scale.x = atof(parsed_data);
                    found_scale_x = true;
                } else if (!found_scale_y){
                    entity_scale.y = atof(parsed_data);
                    found_scale_y = true;
                } else if (!found_pivot_x){
                    entity_pivot.x = atof(parsed_data);
                    found_pivot_x = true;
                } else if (!found_pivot_y){
                    entity_pivot.y = atof(parsed_data);
                    found_pivot_y = true;
                } else if (!found_rotation){
                    entity_rotation = atof(parsed_data);
                    found_rotation = true;
                } else if (!found_color_r){
                    entity_color.r = (char)atoi(parsed_data);
                    found_color_r = true;
                } else if (!found_color_g){
                    entity_color.g = (char)atoi(parsed_data);
                    found_color_g = true;
                } else if (!found_color_b){
                    entity_color.b = (char)atoi(parsed_data);
                    found_color_b = true;
                } else if (!found_color_a){
                    entity_color.a = (char)atoi(parsed_data);
                    found_color_a = true;
                } else if (!found_flags){
                    entity_flags = atoi(parsed_data);
                    found_flags = true;
                } else if (!found_vertices && parsing_vertices){
                    if (!found_vertex_x){
                        vertex.x = atof(parsed_data);
                        found_vertex_x = true;
                        found_vertex_y = false;
                    } else if (!found_vertex_y){
                        vertex.y = atof(parsed_data);
                        entity_vertices.add(vertex);
                        found_vertex_y = true;
                        found_vertex_x = false;
                    }
                }
            }
        }
        
        //No need to assert every variable. We can add something to entity and old levels should not broke
        assert(found_id && found_position_x && found_position_y);
        
        add_entity(entity_id, entity_position, entity_scale, entity_pivot, entity_rotation, entity_color, entity_flags, entity_vertices);
    }
    
    if (fptr){
        fclose(fptr);
    }
    
    Context *c = &context;
    
    c->unit_screen_size = {screen_width / UNIT_SIZE, screen_height / UNIT_SIZE};
    
    c->cam.position = Vector2_zero;
    c->cam.cam2D.target = world_to_screen({0, 0});
    c->cam.cam2D.offset = (Vector2){ screen_width/2.0f, (f32)screen_height * 0.5f };
    c->cam.cam2D.rotation = 0.0f;
    c->cam.cam2D.zoom = 1.0f;
    
    //zoom_entity = add_text({-20, 20}, 40, "DSF");
    
    //add_entity({0, 10}, {10, 3}, 0, GROUND | COLOR_CHANGE);
}

void enter_game_state(){
    game_state = GAME;
}

void enter_editor_state(){
    game_state = EDITOR;
    
}

Vector2 game_mouse_pos(){
    f32 zoom = context.cam.cam2D.zoom;

    f32 width = screen_width   ;
    f32 height = screen_height ;

    Vector2 screen_pos = GetMousePosition();
    Vector2 world_pos = {(screen_pos.x - width * 0.5f) / UNIT_SIZE, (height * 0.5f - screen_pos.y) / UNIT_SIZE};
    world_pos /= zoom;
    world_pos = world_pos + context.cam.position;// + ( (Vector2){0, -context.unit_screen_size.y * 0.5f});
    return world_pos;
}

void update_game(){
    game_time += dt;

    //update input
    input.mouse_position = game_mouse_pos();
    input.mouse_delta = GetMouseDelta();
    input.mouse_wheel = GetMouseWheelMove();
    
    input.direction.x = 0;
    input.direction.y = 0;
    
    // if (IsKeyDown(KEY_D))
    //     input.direction.x = 1;
    // } else if (IsKeyDown(KEY_A)){
    //     input.direction.x = -1;
    // }
    // if (IsKeyDown(KEY_W))
    //     input.direction.y = 1;
    // } else if (IsKeyDown(KEY_S)){
    //     input.direction.y = -1;
    // }
    
    //Paper *p = &context.paper;
    if (screen_size_changed){
        context.unit_screen_size = {screen_width / UNIT_SIZE, screen_height / UNIT_SIZE};
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
    
    if (game_state == EDITOR){
        update_editor();
    }
    
    //zoom_entity->text_drawer.text = TextFormat("%f", context.cam.cam2D.zoom);
    
    update_entities();
    
    draw_game();
}

void update_color_changer(Entity *entity){
    Color_Changer *changer = &entity->color_changer;
    
    if (!changer->changing){
        return;
    }
    
    f32 t = abs(sinf(game_time * changer->change_time));
    entity->color = lerp(changer->start_color, changer->target_color, t);
}

b32 check_col_point_rec(Vector2 point, Entity e){
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

Array<Vector2> get_normals(Array<Vector2> vertices){
    Array<Vector2> normals = Array<Vector2>(vertices.count);
    
    for (int i = 0; i < vertices.count; i++){
        Vector2 edge = vertices.get(i) - vertices.get((i + 1) % vertices.count);
        
        normals.add(normalized(get_rotated_vector_90(edge, 1)));
    }
    
    return normals;
}

void fill_arr_with_normals(Array<Vector2> *normals, Array<Vector2> vertices){
    for (int i = 0; i < vertices.count; i++){
        Vector2 edge = vertices.get(i) - vertices.get((i + 1) % vertices.count);
        
        normals->add(normalized(get_rotated_vector_90(edge, -1)));
    }
}

Collision check_rectangles_col(Entity *entity1, Entity *entity2){
    Collision result = {};

    Array<Vector2> normals = Array<Vector2>(entity1->vertices.count + entity2->vertices.count);
    fill_arr_with_normals(&normals, entity1->vertices);
    fill_arr_with_normals(&normals, entity2->vertices);
    
    f32 overlap = INFINITY;
    Vector2 min_overlap_axis = Vector2_zero;
    
    Vector2 min_overlap_projection = {};

    for (int i = 0; i < normals.count; i++){
        Vector2 projections[2];
        //x - min, y - max
        projections[0].x =  INFINITY;
        projections[1].x =  INFINITY;
        projections[0].y = -INFINITY;
        projections[1].y = -INFINITY;
        
        Vector2 axis = normals.get(i);

        for (int shape = 0; shape < 2; shape++){
            Array<Vector2> vertices = Array<Vector2>(4);
            Entity *entity;
            if (shape == 0) {
                vertices = entity1->vertices;
                entity = entity1;
            } else{
                vertices = entity2->vertices;
                entity = entity2;
            }
            
            for (int j = 0; j < vertices.count; j++){            
                f32 p = dot(global(*entity, vertices.get(j)), axis);
                
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
            normals.free_arr();
            return result;
        }
    }
    
    Vector2 vec_to_first = entity1->position - entity2->position;
    
    result.collided = true;
    if (entity1->flags > 0){
        result.overlap = overlap;
        //result.normal = dir_to_first;
        result.dir_to_first = normalized(vec_to_first);
        result.normal = dot(result.dir_to_first, min_overlap_axis) > 0 ? min_overlap_axis : min_overlap_axis * -1.0f;
        //result.point = entity1->position - dir_to_first * overlap;
        result.point = entity1->position - result.normal * ((min_overlap_projection.y - min_overlap_projection.x) / 2);
    }
    
    normals.free_arr();
    return result;
}

void resolve_collision(Entity *entity, Collision col){
    if (!col.collided){
        return;
    }

    entity->position += col.dir_to_first * col.overlap;
}

Array<Collision> get_collisions(Entity *entity){
    Array<Collision> result = Array<Collision>(10);

    for (int i = 0; i < context.entities.count; i++){
        Entity *other = context.entities.get_ptr(i);
        
        if (other == entity){
            continue;
        }
        
        Collision col = check_rectangles_col(entity, other);
        
        if (col.collided){
            result.add(col);
        }
    }
    
    return result;
}

void update_editor(){
    if (IsKeyPressed(KEY_B)){
        Entity *e = add_entity(input.mouse_position, {5, 5}, {0.5f, 0.5f}, 0, GROUND);
        e->color = BROWN;
        e->color_changer.start_color = e->color;
        e->color_changer.target_color = e->color * 1.5f;
    }
    
    f32 zoom = context.cam.cam2D.zoom;
    
    b32 moving_editor_cam = false;
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        context.cam.position += ((Vector2){-input.mouse_delta.x / zoom, input.mouse_delta.y / zoom}) / (UNIT_SIZE);
        moving_editor_cam = true;
    }
    
    if (input.mouse_wheel != 0){
        //So if wheel positive - don't allow zoom any further, same with negative
        if (input.mouse_wheel > 0 && zoom < 5 || input.mouse_wheel < 0 && zoom > 0.1f){
            context.cam.cam2D.zoom += input.mouse_wheel * 0.05f;
        }
    }
    
    b32 found_cursor_entity_this_frame = false;
    
    b32 need_move_vertices = IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    b32 need_snap_vertex = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_V);
    
    int selected_vertex_index;
    Vector2 closest_vertex_global;
    f32 distance_to_closest_vertex = INFINITY;
    
    //editor entities loop
    for (int i = 0; i < context.entities.count; i++){        
        Entity *e = context.entities.get_ptr(i);
        
        if (!e->enabled){
            continue;
        }
        
        Rectangle rect = {e->position.x - e->pivot.x * e->bounds.x, e->position.y - e->pivot.y * e->bounds.y, e->bounds.x * UNIT_SIZE, e->bounds.y * UNIT_SIZE};
        
        
        Entity mouse_entity = Entity(input.mouse_position);
        if ((check_rectangles_col(&mouse_entity, e)).collided){
            editor.cursor_entity = e;
            found_cursor_entity_this_frame = true;
        } else if (!found_cursor_entity_this_frame){
            editor.cursor_entity = NULL;
        }
        
        if (editor.dragging_entity != NULL && e->id != editor.dragging_entity->id){
            Collision col = check_rectangles_col(editor.dragging_entity, e);
            if (col.collided){
                //resolve_collision(editor.dragging_entity, col);
                e->color = WHITE * abs(sinf(game_time * 10));
            }
        }
        
        //editor vertices
        for (int v = 0; v < e->vertices.count && (need_move_vertices || need_snap_vertex); v++){
            Vector2 *vertex = e->vertices.get_ptr(v);
            
            Vector2 vertex_global = global(*e, *vertex);
            
            if (need_move_vertices && editor.moving_vertex == NULL){
                if (check_col_circles({input.mouse_position, 1}, {vertex_global, 2})){
                    editor.moving_vertex = vertex;
                    editor.last_selected_vertex = vertex;
                    editor.moving_vertex_entity = e;
                    editor.dragging_entity = NULL;
                }
            }
            
            if (editor.selected_entity != NULL && editor.last_selected_vertex != NULL && need_snap_vertex && e->id != editor.selected_entity->id){
                f32 sqr_distance = sqr_magnitude(global(*editor.selected_entity, *editor.last_selected_vertex) - vertex_global);
                if (sqr_distance < distance_to_closest_vertex){
                    distance_to_closest_vertex = sqr_distance;
                    closest_vertex_global = vertex_global;
                }
            }
        }
        //editor move vertices
    
         //editor snap closest vertex to closest vertex

        
        free_entity(&mouse_entity);
    }
    
    if (need_snap_vertex && editor.last_selected_vertex != NULL && editor.selected_entity != NULL){
        *editor.last_selected_vertex = global(*editor.selected_entity, *editor.last_selected_vertex);
        editor.last_selected_vertex->x = closest_vertex_global.x;
        editor.last_selected_vertex->y = closest_vertex_global.y;
        *editor.last_selected_vertex = local(*editor.selected_entity, *editor.last_selected_vertex);
        
        editor.moving_vertex = NULL;
        editor.moving_vertex_entity = NULL;
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        if (editor.cursor_entity != NULL){
            b32 is_same_selected_entity = editor.selected_entity != NULL && editor.selected_entity->id == editor.cursor_entity->id;
            if (!is_same_selected_entity){
                if (editor.selected_entity != NULL){
                    editor.selected_entity->color_changer.changing = 0;
                    editor.selected_entity->color = editor.selected_entity->color_changer.start_color;
                }
                editor.cursor_entity->color_changer.changing = 1;
                editor.selected_entity = editor.cursor_entity;
                
                editor.selected_this_click = true;
            }
        }
    } else if (editor.dragging_entity == NULL && !editor.selected_this_click && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && editor.selected_entity != NULL){
        if (editor.cursor_entity != NULL){
            if (editor.moving_vertex == NULL && editor.selected_entity != NULL && editor.selected_entity->id == editor.cursor_entity->id){
                editor.dragging_entity = editor.selected_entity;
            }
        }
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        if (editor.selected_entity != NULL && !editor.selected_this_click && editor.cursor_entity != NULL){
            if (editor.dragging_time <= 0.1f && editor.cursor_entity->id == editor.selected_entity->id){
                editor.selected_entity->color_changer.changing = 0;
                editor.selected_entity->color = editor.selected_entity->color_changer.start_color;
                editor.selected_entity = NULL;        
            }
        }
        
        editor.dragging_time = 0;
        editor.selected_this_click = false;
        editor.dragging_entity = NULL;
        editor.moving_vertex = NULL;
        editor.moving_vertex_entity = NULL;
    }
    
    if (editor.moving_vertex != NULL){
        *editor.moving_vertex = global(*editor.moving_vertex_entity, *editor.moving_vertex);
        editor.moving_vertex->x = input.mouse_position.x;
        editor.moving_vertex->y = input.mouse_position.y;
        *editor.moving_vertex = local(*editor.moving_vertex_entity, *editor.moving_vertex);
    }
    
    //editor Delete entity
    if (IsKeyPressed(KEY_X) && editor.selected_entity){
        editor.selected_entity->destroyed = true;
        editor.selected_entity = NULL;
        editor.dragging_entity = NULL;
        editor.cursor_entity   = NULL;
    }
    
    
    if (editor.dragging_entity != NULL){
        editor.dragging_time += dt;
    }
    
    if (editor.dragging_entity != NULL && !moving_editor_cam){
        Vector2 move_delta = ((Vector2){input.mouse_delta.x / zoom, -input.mouse_delta.y / zoom}) / (UNIT_SIZE);
        editor.dragging_entity->position += move_delta;
    }
    
    //editor Entity to mouse or go to entity
    if (IsKeyPressed(KEY_F) && editor.dragging_entity != NULL){
        editor.dragging_entity->position = input.mouse_position;
    } else if (IsKeyPressed(KEY_F) && editor.selected_entity != NULL){
        context.cam.position = editor.selected_entity->position;
    }
    
    //editor Entity rotation
    if (editor.selected_entity != NULL){
        f32 rotation = 0;
        f32 speed = 50;
        if (IsKeyDown(KEY_E)){
            rotation = dt * speed;
        } else if (IsKeyDown(KEY_Q)){
            rotation = -dt * speed;
        }
        
        if (rotation != 0){
            rotate(editor.selected_entity, rotation);
        }
    }
    
    //editor entity scaling
    if (editor.selected_entity != NULL){
        Vector2 scaling = {};
        f32 speed = 5;
        
        if (IsKeyDown(KEY_LEFT_SHIFT)){
            speed *= 3.0f;
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
    }
    
    //editor Save level
    if (IsKeyPressed(KEY_J) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL)){
        FILE *fptr;
        fptr = fopen("test_level.level", "w");
        printf("level saved\n");
        
        fprintf(fptr, "Entities:\n");
        for (int i = 0; i < context.entities.count; i++){        
            Entity *e = context.entities.get_ptr(i);
            
            Color color = e->color_changer.start_color;
            fprintf(fptr, "id:%d: pos{:%f:, :%f:} scale{:%f:, :%f:} pivot{:%f:, :%f:} rotation:%f: color{:%d:, :%d:, :%d:, :%d:}, flags:%d: ", e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags);
            
            fprintf(fptr, "vertices[ ");
            for (int v = 0; v < e->vertices.count; v++){
                fprintf(fptr, "{:%f:, :%f:} ", e->vertices.get(v).x, e->vertices.get(v).y); 
            }
            
            fprintf(fptr, "] "); 
            
            fprintf(fptr, ";\n"); 
        }

        
        fclose(fptr);
    }
}

void change_scale(Entity *entity, Vector2 new_scale){
    Vector2 old_scale = entity->scale;
    
    entity->scale = new_scale;
    
    clamp(&entity->scale.x, 0.01f, 10000);
    clamp(&entity->scale.y, 0.01f, 10000);

    //@TODO properly calculate bounds
    entity->bounds = entity->scale;
    
    Vector2 vec_scale_difference = entity->scale - old_scale;
    
    for (int i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        f32 up_dot    = normalized(dot(entity->up,    *vertex));
        f32 right_dot = normalized(dot(entity->right, *vertex));
        
        *vertex += entity->up    * up_dot    * vec_scale_difference.y * 0.5f;
        *vertex += entity->right * right_dot * vec_scale_difference.x * 0.5f;
    }
}

void add_scale(Entity *entity, Vector2 added){
    change_scale(entity, entity->scale + added);
}

void change_up(Entity *entity, Vector2 new_up){
    entity->up = normalized(new_up);
    entity->rotation = atan2f(new_up.y, new_up.x) * RAD2DEG;
}

void change_right(Entity *entity, Vector2 new_right){
    entity->right = normalized(new_right);
    entity->rotation = atan2f(new_right.y, new_right.x) * RAD2DEG;
}

void rotate_around_point(Vector2 *target, Vector2 origin, f32 rotation){
    float s = -sinf(rotation * DEG2RAD);
    float c =  cosf(rotation * DEG2RAD);
    
    target->x -= origin.x;
    target->y -= origin.y;
    
    // rotate point
    float xnew = target->x * c - target->y * s;
    float ynew = target->x * s + target->y * c;
    
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

    // for (int i = 0; i < entity->vertices.count; i++){
    //     Vector2 *vertex = entity->vertices.get_ptr(i);
    //     rotate_around_point(vertex, {0, 0}, -entity->rotation);
    // }
    
    f32 old_rotation = entity->rotation;

    entity->rotation = new_rotation;
    
    entity->up    = {sinf(new_rotation * DEG2RAD),  cosf(new_rotation * DEG2RAD)};
    entity->right = {cosf(new_rotation * DEG2RAD), -sinf(new_rotation * DEG2RAD)};
    
    for (int i = 0; i < entity->vertices.count; i++){
        Vector2 *vertex = entity->vertices.get_ptr(i);
        rotate_around_point(vertex, {0, 0}, entity->rotation - old_rotation);
    }
}

void rotate(Entity *entity, f32 rotation){
    rotate_to(entity, entity->rotation + rotation);
}

void update_entities(){
    Context *c = &context;
    Array<Entity> *entities = &c->entities;
    
    for (int i = 0; i < entities->count; i++){
        Entity *e = entities->get_ptr(i);
        if (!e->enabled){
            continue;
        }
        
        if (e->destroyed){
            //@TODO properly destroy different entities
            entities->remove(i);    
            i--;
            continue;
        }
          
        update_color_changer(e);            
    }
}

void draw_entities(){
    Array<Entity> *entities = &context.entities;
    
    for (int i = 0; i < entities->count; i++){
        Entity *e = entities->get_ptr(i);
    
        if (!e->enabled){
            continue;
        }
    
        if (e->flags & GROUND){
            if (e->vertices.count > 0){
                draw_game_triangle_strip(*e);
            } else{
                draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
            }
        }
        
        if (e-> flags & DRAW_TEXT){
            draw_game_text(e->position, e->text_drawer.text, e->text_drawer.size, RED);
        }
    }
}

void draw_editor(){
    f32 closest_len = 1000000;
    Entity *closest;

    Array<Entity> *entities = &context.entities;

    for (int i = 0; i < entities->count; i++){
        Entity *e = entities->get_ptr(i);
    
        if (!e->enabled){
            continue;
        }
        
        draw_game_circle(editor.player_spawn_point, 3, BLUE);
        
        b32 draw_circles_on_vertices = IsKeyDown(KEY_LEFT_ALT) && true;
        if (draw_circles_on_vertices){
            for (int v = 0; v < e->vertices.count; v++){
                draw_game_circle(global(*e, e->vertices.get(v)), 1, PINK);
            }
        }
        
        // b32 draw_circles_on_bounds = false;
        // if (draw_circles_on_bounds){
        //     Vector2 left_up = world_to_screen(get_left_up(*e));
        //     Vector2 right_down = world_to_screen(get_right_down(*e));
        //     Vector2 left_down = world_to_screen(get_left_down(*e));
        //     Vector2 right_up = world_to_screen(get_right_up(*e));
        //     DrawCircle(left_up.x, left_up.y, 5, RED);
        //     DrawCircle(right_down.x, right_down.y, 5, BLUE);
        //     DrawCircle(left_down.x, left_down.y, 5, GREEN);
        //     DrawCircle(right_up.x, right_up.y, 5, PURPLE);
        // }
        
        draw_game_line(e->position, e->position + e->right * 3, 0.1f, RED);
        draw_game_line(e->position, e->position + e->up    * 3, 0.1f, GREEN);
        
        draw_game_text(e->position + ((Vector2){0, -3}), TextFormat("POS:   {%.2f, %.2f}", e->position.x, e->position.y), 20, RED);
        
        b32 draw_rotation = false;
        if (draw_rotation){
            draw_game_text(e->position, TextFormat("%d", (i32)e->rotation), 20, RED);
        }
        
        b32 draw_directions = false;
        if (draw_directions){
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
        
        draw_game_line(editor.last_collision.point, editor.last_collision.point + editor.last_collision.normal * 4, 0.2f, GREEN);
        draw_game_rect(editor.last_collision.point + editor.last_collision.normal * 4, {1, 1}, {0.5f, 0.5f}, 0, GREEN * 0.9f);
        
        b32 draw_bounds = true;
        if (draw_bounds){
            
        }
    }
    
    if (editor.dragging_entity != NULL){
        draw_game_line(editor.dragging_entity->position, closest->position, 0.1f, PINK);
    }
}

void draw_game(){
    //context.cam.cam2D.rotation = 20;
    //context.cam.cam2D.target = world_to_screen({0, context.unit_screen_size.y * 0.5f});
    //context.cam.cam2D.target = context.cam.cam2D.target + (context.cam.position * UNIT_SIZE);

    BeginDrawing();
    BeginMode2D(context.cam.cam2D);
    Context *c = &context;
    
    ClearBackground(GRAY);
    
    //draw_rect({200, 200}, {200, 100}, BLUE);
    
    draw_entities();
    
    if (game_state == EDITOR){
        draw_editor();
    }
    
    EndMode2D();
    EndDrawing();
}

void setup_color_changer(Entity *entity){
    entity->color_changer.start_color = entity->color;
    entity->color_changer.target_color = entity->color * 1.4f;
}

Entity* add_entity(Vector2 pos, Vector2 scale, f32 rotation, FLAGS flags){
    Entity e = Entity(pos, scale, rotation, flags);    
    e.id = context.entities.count + game_time * 1000;
    context.entities.add(e);
    return context.entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    Entity *e = add_entity(pos, scale, rotation, flags);    
    e->pivot = pivot;
    return e;
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    Entity *e = add_entity(pos, scale, pivot, rotation, flags);    
    e->id = id;
    return e;
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags){
    Entity *e = add_entity(id, pos, scale, pivot, rotation, flags);    
    e->color = color;
    setup_color_changer(e);
    return e;
}

Entity* add_entity(i32 id, Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, Color color, FLAGS flags, Array<Vector2> vertices){
    Entity *e = add_entity(id, pos, scale, pivot, rotation, color, flags);    
    e->vertices.free_arr();
    e->vertices = vertices;
    setup_color_changer(e);
    return e;
}

Entity *add_text(Vector2 pos, f32 size, const char *text){
    Entity e = Entity(pos, {1, 1}, {0.5f, 0.5f}, 0, DRAW_TEXT);    
    //@TODO: Check for type and set bounds correctly (for textures for example)
    e.bounds = {1, 1};
    e.text_drawer.text = text;
    e.text_drawer.size = size;
    e.id = context.entities.count + game_time * 1000;
    context.entities.add(e);
    return context.entities.last_ptr();
}

Vector2 global(Entity e, Vector2 local_pos){
    return e.position + local_pos;
}

Vector2 local(Entity e, Vector2 global_pos){
    return global_pos - e.position;
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

Vector2 rect_screen_pos(Vector2 position, Vector2 scale, Vector2 pivot){
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

void draw_game_rect_lines(Vector2 position, Vector2 scale, Vector2 pivot, f32 thick, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect_lines(position, scale * UNIT_SIZE, thick, color);
}

void draw_game_triangle_strip(Entity entity){
    Vector2 screen_positions[entity.vertices.count];
    
    for (int i = 0; i < entity.vertices.count; i++){
        screen_positions[i] = world_to_screen(global(entity, entity.vertices.get(i)));
    }
    
    draw_triangle_strip(screen_positions, entity.vertices.count, entity.color);
}

void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, f32 rotation, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, {0, 0});
    draw_rect(screen_pos, multiply(scale, UNIT_SIZE), pivot, rotation, color);
}

void draw_game_text(Vector2 position, const char *text, f32 size, Color color){
    Vector2 screen_pos = world_to_screen(position);
    draw_text(text, screen_pos, size, color);
}

void draw_game_texture(Texture tex, Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    tex.width *= scale.x;
    tex.height *= scale.y;
    // scale.x *= tex.width  / UNIT_SIZE;
    // scale.y *= tex.height / UNIT_SIZE;
    Vector2 screen_pos = rect_screen_pos(position, {(float)tex.width / UNIT_SIZE, (f32)tex.height / UNIT_SIZE}, pivot);
    draw_texture(tex, screen_pos, color);
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

Vector2 get_left_up_no_rot(Entity e){
    return {e.position.x - e.pivot.x * e.bounds.x, e.position.y + e.pivot.y * e.bounds.y};
}

Vector2 get_left_up(Entity e){
    //f32 x_add = sinf(e.rotation * DEG2RAD) * e.bounds.x;
    Vector2 lu = get_left_up_no_rot(e);
    
    rotate_around_point(&lu, e.position, e.rotation);
    return lu;
}

Vector2 get_right_down_no_rot(Entity e){
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x + e.bounds.x, lu.y - e.bounds.y};
}

Vector2 get_right_down(Entity e){
    Vector2 lu = get_left_up(e);
    Vector2 rd = lu + e.right * e.bounds.x;
    rd -= e.up * e.bounds.y;
    return rd;
}

Vector2 get_left_down_no_rot(Entity e){
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x, lu.y - e.bounds.y};
}

Vector2 get_left_down(Entity e){
    Vector2 rd = get_right_down(e);
    return rd - e.right * e.bounds.x;
}

Vector2 get_right_up_no_rot(Entity e){
    Vector2 lu = get_left_up_no_rot(e);
    return {lu.x + e.bounds.x, lu.y};
}

Vector2 get_right_up(Entity e){
    Vector2 lu = get_left_up(e);
    return lu + e.right * e.bounds.x;
}
