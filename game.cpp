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
    Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags);
    Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);
    Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags);

    i32 id = -1;

    b32 enabled = 1;
    
    b32 destroyed = 0;
    
    Vector2 up;
    Vector2 right;
    
    FLAGS flags;
    //b32 need_to_destroy = 0;
    
    //lower - closer to camera
    i32 draw_order = 1;
    
    Vector2 position;
    Vector2 scale = {1, 1};
    Vector2 bounds = {1, 1};
    Vector2 pivot = {0.5f, 1.0f};
    f32 rotation;
    
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

    Vector2 mouse_position;
    Vector2 mouse_delta;
    f32     mouse_wheel;
    Vector2 unit_screen_size;
    
    Entity *selected_entity;
    
    Cam cam = {};
};

struct Level{
    Context *context;  
};

struct Editor{
      
};

global_variable Level current_level;
global_variable Context context = {};
global_variable Editor  editor  = {};

#include "game.h"

Entity::Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags){
    position = _pos;
    change_scale(this, _scale);
    rotate_to(this, _rotation);
    flags    = _flags;
}

Entity::Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags){
    position = _pos;
    pivot = _pivot;
    change_scale(this, _scale);
    rotate_to(this, _rotation);
    flags    = _flags;
}

Entity::Entity(i32 _id, Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags){
    id = _id;
    position = _pos;
    change_scale(this, _scale);
    pivot = _pivot;
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
    game_time = 0;

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
        
        i32 parsed_count = 50;
        char parsed_data[parsed_count];
        
        for (int i = 0; buffer[i] != NULL && buffer[i] != ';'; i++){
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
                }
            }
        }
        
        //No need to assert every variable. We can add something to entity and old levels should not broke
        assert(found_id && found_position_x && found_position_y);
        
        add_entity(entity_id, entity_position, entity_scale, entity_pivot, entity_rotation, entity_color, entity_flags);
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

    context.mouse_position = game_mouse_pos();
    context.mouse_delta = GetMouseDelta();
    context.mouse_wheel = GetMouseWheelMove();
    
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
    
    update_editor();
    
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
    Vector2 l_u = get_left_up(e);
    Vector2 r_d = get_right_down(e);

    return ((point.x >= l_u.x) && (point.x <= r_d.x) && (point.y >= r_d.y) && (point.y <= l_u.y));
}

Entity *selected_entity;
Entity *dragging_entity;
Entity *cursor_entity;

void update_editor(){
    if (IsKeyPressed(KEY_B)){
        Entity *e = add_entity(context.mouse_position, {5, 5}, {0.5f, 0.5f}, 0, GROUND);
        e->color = BROWN;
        e->color_changer.start_color = e->color;
        e->color_changer.target_color = e->color * 1.5f;
    }
    
    f32 zoom = context.cam.cam2D.zoom;
    
    b32 moving_editor_cam = false;
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        context.cam.position += ((Vector2){-context.mouse_delta.x / zoom, context.mouse_delta.y / zoom}) / (UNIT_SIZE);
        moving_editor_cam = true;
    }
    
    if (context.mouse_wheel != 0){
        //So if wheel positive - don't allow zoom any further, same with negative
        if (context.mouse_wheel > 0 && zoom < 5 || context.mouse_wheel < 0 && zoom > 0.1f){
            context.cam.cam2D.zoom += context.mouse_wheel * 0.05f;
        }
    }
    
    local_persist b32 selected_this_click = 0;
    local_persist f32 dragging_time = 0;
    
    b32 found_cursor_entity_this_frame = false;
    
    //editor entities loop
    for (int i = 0; i < context.entities.count; i++){        
        Entity *e = context.entities.get_ptr(i);
        
        if (!e->enabled){
            continue;
        }
        
        Rectangle rect = {e->position.x - e->pivot.x * e->bounds.x, e->position.y - e->pivot.y * e->bounds.y, e->bounds.x * UNIT_SIZE, e->bounds.y * UNIT_SIZE};
        
        
        if (check_col_point_rec(context.mouse_position, *e)){
            cursor_entity = e;
            found_cursor_entity_this_frame = true;
        } else if (!found_cursor_entity_this_frame){
            cursor_entity = NULL;
        }
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        if (cursor_entity != NULL){
            b32 is_same_selected_entity = selected_entity != NULL && selected_entity->id == cursor_entity->id;
            if (!is_same_selected_entity){
                if (selected_entity != NULL){
                    selected_entity->color_changer.changing = 0;
                    selected_entity->color = selected_entity->color_changer.start_color;
                }
                cursor_entity->color_changer.changing = 1;
                selected_entity = cursor_entity;
                
                selected_this_click = true;
            }
        }
    } else if (dragging_entity == NULL && !selected_this_click && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && selected_entity != NULL){
        if (cursor_entity != NULL){
            if (selected_entity != NULL && selected_entity->id == cursor_entity->id){
                dragging_entity = selected_entity;
            }
        }
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        if (selected_entity != NULL && !selected_this_click && cursor_entity != NULL){
            if (dragging_time <= 0.1f && cursor_entity->id == selected_entity->id){
                selected_entity->color_changer.changing = 0;
                selected_entity->color = selected_entity->color_changer.start_color;
                selected_entity = NULL;        
            }
        }
        
        dragging_time = 0;
        selected_this_click = false;
        dragging_entity = NULL;
    }
    
    //editor Delete entity
    if (IsKeyPressed(KEY_X) && selected_entity){
        selected_entity->destroyed = true;
        selected_entity = NULL;
        dragging_entity = NULL;
        cursor_entity   = NULL;
    }
    
    
    if (dragging_entity != NULL){
        dragging_time += dt;
    }
    
    if (dragging_entity != NULL && !moving_editor_cam){
        Vector2 move_delta = ((Vector2){context.mouse_delta.x / zoom, -context.mouse_delta.y / zoom}) / (UNIT_SIZE);
        dragging_entity->position += move_delta;
    }
    
    //editor Entity to mouse or go to entity
    if (IsKeyPressed(KEY_F) && dragging_entity != NULL){
        dragging_entity->position = context.mouse_position;
    } else if (IsKeyPressed(KEY_F) && selected_entity != NULL){
        context.cam.position = selected_entity->position;
    }
    
    //editor Entity rotation
    if (selected_entity != NULL){
        f32 rotation = 0;
        f32 speed = 50;
        if (IsKeyDown(KEY_E)){
            rotation = dt * speed;
        } else if (IsKeyDown(KEY_Q)){
            rotation = -dt * speed;
        }
        
        if (rotation != 0){
            rotate(selected_entity, rotation);
        }
    }
    
    //editor entity scaling
    if (selected_entity != NULL){
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
            add_scale(selected_entity, scaling);
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
            fprintf(fptr, "id:%d: pos{:%f:, :%f:} scale{:%f:, :%f:} pivot{:%f:, :%f:} rotation:%f: color{:%d:, :%d:, :%d:, :%d:}, flags:%d:;\n", e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags);
        }

        
        fclose(fptr);
    }
}

void change_scale(Entity *entity, Vector2 new_scale){
    entity->scale = new_scale;
    
    clamp(&entity->scale.x, 0.01f, 10000);
    clamp(&entity->scale.y, 0.01f, 10000);

    //@TODO properly calculate bounds
    entity->bounds = new_scale;
}

void add_scale(Entity *entity, Vector2 added){
    change_scale(entity, entity->scale + added);
}

void change_up(Entity *entity, Vector2 new_up){
    entity->up = new_up;
    entity->rotation = atan2f(new_up.y, new_up.x) * RAD2DEG;
}

void change_right(Entity *entity, Vector2 new_right){
    entity->right = new_right;
    entity->rotation = atan2f(new_right.y, new_right.x) * RAD2DEG;
}

void rotate_to(Entity *entity, f32 new_rotation){
    while (new_rotation >= 360){
        new_rotation -= 360;
    }
    while (new_rotation < 0){
        new_rotation += 360;
    }

    entity->rotation = new_rotation;
    
    entity->up    = {sinf(new_rotation), cosf(new_rotation)};
    entity->right = {cosf(new_rotation), sinf(new_rotation)};
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
    Context *c = &context;
    Array<Entity> *entities = &c->entities;
    
    for (int i = 0; i < entities->count; i++){
        Entity *e = entities->get_ptr(i);
    
        if (!e->enabled){
            continue;
        }
    
        if (e->flags & GROUND){
            draw_game_rect(e->position, e->scale, e->pivot, e->rotation, e->color);
        }
        
        if (e-> flags & DRAW_TEXT){
            draw_game_text(e->position, e->text_drawer.text, e->text_drawer.size, RED);
        }
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

void draw_game_rect(Vector2 position, Vector2 scale, Vector2 pivot, Color color){
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect(screen_pos, multiply(scale, UNIT_SIZE), color);
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

Vector2 get_left_up(Entity e){
    Vector2 lu = {e.position.x - e.pivot.x * e.bounds.x, e.position.y + e.pivot.y * e.bounds.y};
    return lu;
}

Vector2 get_right_down(Entity e){
    Vector2 lu = get_left_up(e);
    Vector2 rd = {lu.x + e.bounds.x, lu.y - e.bounds.y};
    return rd;
}
