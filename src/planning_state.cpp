#pragma once

enum Planning_Flags : FLAGS {
};

struct Planning_Data {
    Entity *dragged_entity = NULL;
    
};

global_variable Planning_Data planning = {0};

void planning_start_holding_entity(Entity *entity_to_drag) {
    planning.dragged_entity = copy_and_add_entity(entity_to_drag, current_level_context);
    planning.dragged_entity->position = input.mouse_position;
}

void update_planning() { 
    if (planning.dragged_entity) {
        planning.dragged_entity->position = input.mouse_position;
        planning.dragged_entity->position = round_to_factor(planning.dragged_entity->position, CELL_SIZE);
        
        if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E)) {   
            f32 to_rotate = 0;
            if (IsKeyPressed(KEY_Q)) to_rotate = -15;
            if (IsKeyPressed(KEY_E)) to_rotate = 15;
            
            f32 next_rotation = round_to_factor(planning.dragged_entity->rotation + to_rotate, 15);
            rotate_to(planning.dragged_entity, next_rotation);
        }
    }
}

void planning_draw_ui() {
    Vector2 panel_pos = {50, 300};
    Vector2 panel_size = {100, 400};
    make_ui_image(panel_pos, panel_size, {0, 0}, color_fade(SKYBLUE, 0.2f), "planning_panel");
    
    i32 buttons_count = 0;
    for_array(i, &spawn_objects) {
        Spawn_Object *object = spawn_objects.get(i);
        if (!(object->flags & PLANNING_OBJECT)) continue;
        
        buttons_count += 1;
        Vector2 pos = panel_pos + Vector2_up * 10 * buttons_count;
        Vector2 size = {panel_size.x - 20, 100};
        if (make_button(pos, size, object->name, tprintf("planning_object_%d", i))) {
            planning_start_holding_entity(&object->entity);
        }
    }
}
