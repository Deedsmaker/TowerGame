#pragma once

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
    Entity(Vector2 _pos, Vector2 _scale, f32 _rotation, FLAGS _flags){
        position = _pos;
        scale    = _scale;
        rotation = _rotation;
        flags    = _flags;
    }
    
    Entity(Vector2 _pos, Vector2 _scale, Vector2 _pivot, f32 _rotation, FLAGS _flags){
        position = _pos;
        scale    = _scale;
        pivot = _pivot;
        rotation = _rotation;
        flags    = _flags;
    }

    b32 enabled = 1;
    
    FLAGS flags;
    
    i32 index = 0;
    //b32 need_to_destroy = 0;
    
    //lower - closer to camera
    int draw_order = 1;
    
    Vector2 position;
    Vector2 scale = {1, 1};
    Vector2 bounds = {1, 1};
    Vector2 pivot = {0.5f, 1.0f};
    float rotation;
    
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

    Vector2 mouse_pos;
    Vector2 mouse_delta;
    f32     mouse_wheel;
    Vector2 unit_screen_size;
    
    Entity *selected_entity;
    
    Cam cam = {};
};


global_variable Context context = {};

#include "game.h"

Entity *zoom_entity;

void init_game(){
    game_time = 0;

    context = {};
    
    Context *c = &context;
    
    c->unit_screen_size = {screen_width / UNIT_SIZE, screen_height / UNIT_SIZE};
    
    c->cam.position = Vector2_zero;
    c->cam.cam2D.target = world_to_screen({0, 0});
    c->cam.cam2D.offset = (Vector2){ screen_width/2.0f, (f32)screen_height * 0.5f };
    c->cam.cam2D.rotation = 0.0f;
    c->cam.cam2D.zoom = 1.0f;
    
    zoom_entity = add_text({-20, 20}, 40, "DSF");
    
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

    context.mouse_pos = game_mouse_pos();
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
    
    zoom_entity->text_drawer.text = TextFormat("%f", context.cam.cam2D.zoom);
    
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
    Vector2 left_up    = {e.position.x - e.pivot.x * e.bounds.x, e.position.y + e.pivot.y * e.bounds.y};
    Vector2 right_down = {left_up.x + e.bounds.x, left_up.y - e.bounds.y};

    return ((point.x >= left_up.x) && (point.x <= right_down.x) && (point.y >= right_down.y) && (point.y <= left_up.y));
}

Entity *selected_entity;

void update_editor(){
    if (IsKeyPressed(KEY_B)){
        Entity *e = add_entity(context.mouse_pos, {5, 5}, {0.5f, 0.5f}, 0, GROUND);
        e->color = BROWN;
        e->color_changer.start_color = e->color;
        e->color_changer.target_color = e->color * 1.5f;
    }
    
    f32 zoom = context.cam.cam2D.zoom;
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        context.cam.position += ((Vector2){-context.mouse_delta.x / zoom, context.mouse_delta.y / zoom}) / (UNIT_SIZE);
    }
    
    if (context.mouse_wheel != 0){
        //So if wheel positive - don't allow zoom any further, same with negative
        if (context.mouse_wheel > 0 && zoom < 5 || context.mouse_wheel < 0 && zoom > 0.1f){
            context.cam.cam2D.zoom += context.mouse_wheel * 0.05f;
        }
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        for (int i = 0; i < context.entities.count; i++){        
            Entity *e = context.entities.get_ptr(i);
            
            if (!e->enabled){
                continue;
            }
            
            Rectangle rect = {e->position.x - e->pivot.x * e->bounds.x, e->position.y - e->pivot.y * e->bounds.y, e->bounds.x * UNIT_SIZE, e->bounds.y * UNIT_SIZE};
            if (check_col_point_rec(context.mouse_pos, *e)){
                if (selected_entity != NULL){
                    selected_entity->color_changer.changing = 0;
                    selected_entity->color = selected_entity->color_changer.start_color;
                }
                
                e->color_changer.changing = 1;
                selected_entity = e;
            }
        }
    }
    
    if (IsKeyPressed(KEY_F) && selected_entity != NULL){
        context.cam.position = selected_entity->position;
    }
}

void update_entities(){
    Context *c = &context;
    Array<Entity> *entities = &c->entities;
    
    for (int i = 0; i < entities->count; i++){
        Entity *e = entities->get_ptr(i);
        if (!e->enabled){
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
            draw_game_rect(e->position, e->scale, e->pivot, e->color);
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

Entity* add_entity(Vector2 pos, Vector2 scale, f32 rotation, FLAGS flags){
    Entity e = Entity(pos, scale, rotation, flags);    
    context.entities.add(e);
    return context.entities.last_ptr();
}

Entity* add_entity(Vector2 pos, Vector2 scale, Vector2 pivot, f32 rotation, FLAGS flags){
    Entity e = Entity(pos, scale, pivot, rotation, flags);    
    //@TODO: Check for type and set bounds correctly (for textures for example)
    e.bounds = scale;
    context.entities.add(e);
    return context.entities.last_ptr();
}

Entity *add_text(Vector2 pos, f32 size, const char *text){
    Entity e = Entity(pos, {1, 1}, {0.5f, 0.5f}, 0, DRAW_TEXT);    
    //@TODO: Check for type and set bounds correctly (for textures for example)
    e.bounds = {1, 1};
    e.text_drawer.text = text;
    e.text_drawer.size = size;
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
    Vector2 screen_pos = rect_screen_pos(position, scale, pivot);
    draw_rect(screen_pos, multiply(scale, UNIT_SIZE), rotation, color);
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
