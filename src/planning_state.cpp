#pragma once

enum Planning_Flags : FLAGS {
};

struct Planning_Data {
    Entity *dragged_entity = NULL;
        
    Array <i32> spawned_ids = {0};
};

global_variable Planning_Data planning = {0};

void reset_planning_data() {
    planning.dragged_entity = NULL;
    planning.spawned_ids.clear();
}

void planning_start_holding_entity(Entity *entity_to_drag) {
    planning.dragged_entity = copy_and_add_entity(entity_to_drag, current_context);
    planning.dragged_entity->position = input.mouse_position;
}

void update_planning() { 
    if (planning.dragged_entity) {
        Entity *entity = planning.dragged_entity;
        entity->position = input.mouse_position;
        entity->position = round_to_factor(entity->position, CELL_SIZE);
        
        if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E)) {   
            f32 to_rotate = 0;
            if (IsKeyPressed(KEY_Q)) to_rotate = -15;
            if (IsKeyPressed(KEY_E)) to_rotate = 15;
            
            f32 next_rotation = round_to_factor(entity->rotation + to_rotate, 15);
            rotate_to(entity, next_rotation);
        }
        
        if (IsKeyPressed(KEY_X)) {
            i32 dragged_index = planning.spawned_ids.find(planning.dragged_entity->id);
            assert(dragged_index >= 0);
            planning.spawned_ids.remove(dragged_index);
                       
            mark_entity_destroyed(planning.dragged_entity);
        }
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            entity->position = round_to_factor(entity->position, CELL_SIZE);
            
            planning.spawned_ids.append(entity->id);
            
            planning.dragged_entity = NULL;
        }
        
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        auto mouse_collisions = get_tcollisions(&mouse_entity, 0);
        for_array(i, &mouse_collisions) {
            Entity *other = mouse_collisions.get(i)->other_entity;
            
            i32 spawned_index = planning.spawned_ids.find(other->id);
            if (spawned_index >= 0) {
                Entity *entity = get_entity(planning.spawned_ids.get_value(spawned_index));
                planning.dragged_entity = entity;
                break;
            }
        }
    }
}

void planning_draw_ui() {
    Vector2 panel_pos  = {screen_width * 0.05f, screen_height * 0.2f};
    Vector2 panel_size = {screen_width * 0.1f, screen_height * 0.4f};
    Old::make_ui_image(panel_pos, panel_size, {0, 0}, color_fade(SKYBLUE, 0.2f), "planning_panel");
    
    i32 buttons_count = 0;
    for_array(i, &spawn_objects) {
        Spawn_Object *object = spawn_objects.get(i);
        if (!(object->flags & PLANNING_OBJECT)) continue;
        
        buttons_count += 1;
        Vector2 pos = panel_pos + Vector2_up * 10 * buttons_count;
        Vector2 size = {panel_size.x - 20, 100};
        if (Old::make_button(pos, size, object->name, tprintf("planning_object_%d", i))) {
            planning_start_holding_entity(&object->entity);
        }
    }
    
    // Now some hints.
    
    panel_pos = {screen_width * 0.3f, screen_height * 0.85f};
    panel_size = {screen_width * 0.3f, screen_height * 0.1f};
    Old::make_ui_image(panel_pos, panel_size, {0, 0}, color_fade(VIOLET, 0.3f), "planning_hints");
    Old::make_ui_text("SPACE - go gaming.", panel_pos, "gaming_hint"); 
    Old::make_ui_text("X - delete thing.", panel_pos + Vector2_up * 20, "gaming_hint2"); 
    Old::make_ui_text("Q/E - rotate thing.", panel_pos + Vector2_up * 40, "gaming_hint2"); 
}
