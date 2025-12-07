#pragma once

f32 radius_from_node(Planning_Node *node) {
    switch (node->type) {
        case NO_NODE:
            return 10.0f;
        case SPACE_NODE:
            return 50.0f;
        case HIT_BOOSTER_NODE:
            return 25.0f;
        default: 
            return 100.0f; // Fallback.
    }
}

Color color_from_node(Planning_Node *node) {
    switch (node->type) {
        case NO_NODE:
            return Fade(BLACK, 0.4f);
        case SPACE_NODE:
            return Fade(ColorBrightness(SKYBLUE, -0.3f), 0.6f);
        case HIT_BOOSTER_NODE:
            return Fade(RED, 0.7f);
        default: 
            return PINK; // Fallback.
    }
}

Color radius_color_from_node(Planning_Node *node) {
    Color node_color = color_from_node(node);
    
    return color_fade(ColorBrightness(node_color, 0.3f), 0.4f);
}

i32 *point_count_from_node(Planning_Data *planning, Planning_Node *node) { 
    switch (node->type) {
        case SPACE_NODE: 
            return &planning->space_points;
        default:
            return &planning->item_points;
    }
}

void add_node_icon(Context *context, String name, Planning_Node_Type type, Entity *node_entity) {
    Planning_Node_Icon icon = {0};
    icon.name = copy_string(name, &context->memory_arena);
    icon.type = type;
    icon.entity_to_spawn = node_entity;
    context->planning.node_icons.append(icon);
}

void init_planning_data(Context *context) {
    context->planning.nodes.clear();
    context->planning.nodes.allocator = &context->memory_arena;
    
    context->planning.node_icons.clear();
    context->planning.node_icons.allocator = &context->memory_arena;
    
    add_node_icon(context, tstring("Space node"), SPACE_NODE, NULL);
    
    auto hit_booster_object = get_spawn_object_by_name(tstring("hit_booster"));
    add_node_icon(context, tstring("Booster node"), HIT_BOOSTER_NODE, &hit_booster_object->entity);
}

void reset_planning_data(Context *context) {
    auto planning = &context->planning;

    planning->dragged_entity = NULL;
    planning->spawned_ids.clear();
    
    planning->selected_icon_index = 0;
    
    planning->nodes.clear();
    planning->nodes.append({.position = current_context->player_spawn_point, .type = SPACE_NODE}); // First is the base node.
    planning->nodes.append({.position = current_context->player_spawn_point, .type = SPACE_NODE}); // Second is the new node. We're always keeping node that will be spawned, even if we have no points for it.
    
    planning->space_points = planning->max_space_points;
    planning->item_points = planning->max_item_points;
}

void validate_node(Context *context, Planning_Node *node) {
    auto active_icon = context->planning.node_icons.get(context->planning.selected_icon_index);
    if (active_icon->type == SPACE_NODE) {
        if (context->planning.space_points <= 0) {
            if (node->entity) {
                mark_entity_destroyed(node->entity);
                node->entity = NULL;
            }
            node->type = NO_NODE;
            return;
        }
    } else {
        if (context->planning.item_points <= 0) {
            if (node->entity) {
                mark_entity_destroyed(node->entity);
                node->entity = NULL;
            }
            node->type = NO_NODE;
            return;
        }
    }
    
    node->type = active_icon->type;
    if (active_icon->entity_to_spawn && !node->entity) {           
        node->entity = copy_and_add_entity(active_icon->entity_to_spawn, context);
    }
    if (active_icon->entity_to_spawn == NULL && node->entity != NULL) {
        mark_entity_destroyed(node->entity);
        node->entity = NULL;
    }
}

void planning_prepare_to_enter_gaming(Context *context) { 
    Planning_Data *planning = &context->planning;
    For (&planning->nodes) {
        if (i == planning->nodes.count - 2 && it->entity != NULL) {
            // If last node is node with entity on it - we want to keep it's orientation on direction from previous to it, 
            // instead of it to point at a imaginary next node.
            
            Vector2 previous_to_it = it->position - planning->nodes.get(i - 1)->position;
            Vector2 dir = normalized(previous_to_it);
            
            change_up(it->entity, dir);
        }
    }
}

void update_planning(Context *context) { 
    Planning_Data *planning = &context->planning;

    if (IsKeyPressed(KEY_Z)) {
        if (planning->nodes.count > 2) {
            auto removed_node = planning->nodes.pop();
            if (removed_node->entity) {
                mark_entity_destroyed(removed_node->entity);
                removed_node->entity = NULL;
            }
            
            // NOTE: This removed_node was not real, this was the node that we were holding in our hand to place. 
            // Actual node that was removed is replaced_node that is currently last. This replaced node will be called "new_node"
            // in the following code.
            
            auto replaced_node = planning->nodes.last();
            
            if (replaced_node->type == SPACE_NODE) {
                planning->space_points += 1;
                assert(planning->space_points <= planning->max_space_points);
            } else {
                planning->item_points += 1;
                assert(planning->item_points <= planning->max_item_points);
            }
        }
    }
    
    assert(planning->nodes.count >= 2); // We're always keeping base node on player spawn point and a new node that will be spawned.
    auto last_node = planning->nodes.get(planning->nodes.count - 2);
    auto new_node = planning->nodes.last();
    
    if (IsKeyPressed(KEY_TAB)) {
        planning->selected_icon_index += 1;
        planning->selected_icon_index %= planning->node_icons.count;
    }
    
    validate_node(context, new_node);
    
    
    f32 target_radius = radius_from_node(last_node);
    Vector2 last_to_mouse = input.mouse_position - last_node->position;
    Vector2 dir = normalized(last_to_mouse);
    
    static const f32 ANGLE_STEP = 30;
    Planning_Node_Icon *node_icon = planning->node_icons.get(planning->selected_icon_index);
    
    f32 angle = fangle(dir);
    angle = round_to_factor(angle, ANGLE_STEP) + 90; // "+ 90" because we have retarged angle system where UP is zero.
    f32 rad = angle * DEG2RAD;
        
    dir = {-cosf(rad), sinf(rad)};
    
    Vector2 target_position = last_node->position + dir * target_radius;
    
    bool can_place_new_node = true;
    
    Collision col = raycast(last_node->position, dir, target_radius, GROUND);
    Color line_color = BLUE;
    if (col.collided) {
        can_place_new_node = false;
        line_color = RED;
    }
    
    if (new_node->type == NO_NODE) {
        line_color = BLACK;
        can_place_new_node = false;
    }
    
    make_line(last_node->position, target_position, 1.0f, line_color);
    
    new_node->position = target_position;
    if (new_node->entity) {
        new_node->entity->position = target_position;
        change_up(new_node->entity, dir);
    }
    
    if (last_node->entity && new_node->type != NO_NODE) {
        change_up(last_node->entity, dir);
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (can_place_new_node) {
            // new_node already in array, now we're adding something node that will be "next new_entity".
            auto added_node = planning->nodes.append({.position = target_position, .type = node_icon->type}); 
            validate_node(context, added_node);
            
            if (added_node->type == SPACE_NODE) {
                planning->space_points -= 1;
                assert(planning->space_points >= 0);
            } else {
                planning->item_points -= 1;
                assert(planning->item_points >= 0);
            }
        } else {
            play_sound("FailedRifleActivation", 0.4f);
        }
    }
}

void planning_draw(Context *context) {
    For (&context->planning.nodes) {
        f32 radius = radius_from_node(it);
        Color node_color = color_from_node(it);
        Color radius_color = radius_color_from_node(it);
        draw_game_circle(it->position, radius, radius_color);
        draw_game_circle(it->position, 5.0f, node_color);
    }
}

void planning_draw_ui(Context *context) {
    Vector2 panel_pos  = {screen_width * 0.05f, screen_height * 0.2f};
    Vector2 panel_size = {screen_width * 0.1f, screen_height * 0.4f};
    Old::make_ui_image(panel_pos, panel_size, {0, 0}, color_fade(SKYBLUE, 0.2f), "planning_panel");
          
    // Old::make_ui_image(
    
    auto planning = &context->planning;
    
    Old::make_ui_text(tprintf("Space points: %d", planning->space_points), panel_pos, "space_points_info");
    Old::make_ui_text(tprintf("Item points: %d", planning->item_points), panel_pos + Vector2_up * 25, "item_points_info");
    
    For (&planning->node_icons) {
        Vector2 pos = panel_pos + Vector2_up * 50 * (i + 1);
        Vector2 size = {panel_size.x - 20, 40};
        
        Color color = BLUE;
        
        assert(it->type != NO_NODE); // NO_NODE type is only for nodes that's not for the placement, so there should be no icon with that type.
        
        b32 cannot_place_that_node = false;
        if (it->type == SPACE_NODE && planning->space_points <= 0) {
            cannot_place_that_node = true;
            color = GRAY;
        }
        if (it->type != SPACE_NODE && planning->item_points <= 0) {
            cannot_place_that_node = true;
            color = GRAY;
        }
        
        if (i == planning->selected_icon_index && !cannot_place_that_node) {
            color = SKYBLUE;
        }
        
        Old::make_ui_image(pos, size, {0, 0}, Fade(color, 0.8f), tprintf("planning_icon_%d", i));
        Old::make_ui_text(c_str(it->name), pos, 22, WHITE, tprintf("planning_text_%d", i));
        // if (Old::make_button(pos, size, object->name, tprintf("planning_object_%d", i))) {
        //     // planning_start_holding_entity(&object->entity);
        // }
    }
    
    // Now some hints.
    
    panel_pos = {screen_width * 0.3f, screen_height * 0.85f};
    panel_size = {screen_width * 0.3f, screen_height * 0.1f};
    Old::make_ui_image(panel_pos, panel_size, {0, 0}, color_fade(VIOLET, 0.3f), "planning_hints");
    Old::make_ui_text("SPACE - go gaming.", panel_pos, "gaming_hint"); 
    Old::make_ui_text("X - delete thing.", panel_pos + Vector2_up * 20, "gaming_hint2"); 
    Old::make_ui_text("Q/E - rotate thing.", panel_pos + Vector2_up * 40, "gaming_hint2"); 
}
