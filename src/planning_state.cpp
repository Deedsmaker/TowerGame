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
            return &planning->current_space_points;
        default:
            return &planning->current_item_points;
    }
}

void add_node_icon(Context *context, String name, Planning_Node_Type type, Entity *node_entity) {
    Planning_Node_Icon icon = {0};
    icon.name = copy_string(name, &context->memory_arena);
    icon.type = type;
    icon.entity_to_spawn = node_entity;
    context->planning.node_icons.append(icon);
}

Array <Planning_Point *> get_points_around_node(Context *context, Planning_Node *node) {
    Array <Planning_Point *> result = {.arena = temp};

    f32 radius = radius_from_node(node);
    
    static const f32 ITEMS_RADIUS = 5.0f;
    for_chunk_array (i, &context->planning_points) {
        Entity *e = context->planning_points.get(i)->entity;
        auto vec = node->position - e->position;
        auto len = magnitude(vec);
        
        auto point = e->planning_point;
        assert(point);
        
        if (len <= radius + ITEMS_RADIUS) {
            result.append(point);
        }
    }
    
    return result;
}

void planning_validate_node_points(Context *context) {
    auto planning = &context->planning;
    
    // Clearing taken flag at first and will reassign this for all taken at once.
    for_chunk_array (i, &context->planning_points) {
        context->planning_points.get(i)->taken = false;
    }

    i32 used_space_points = 0;
    i32 used_item_points = 0;
    
    i32 collected_item_points = 0;
    i32 collected_space_points = 0;
    
    For (&planning->nodes) {
        if (i == planning->nodes.count - 1) {
            continue;
        }
        
        assert(it->type != NO_NODE);
        
        if (it->type == SPACE_NODE) {            
            if (i > 0) used_space_points += 1; // Not counting first base node.
        } else {
            used_item_points += 1;
        }
                   
        auto points_in_radius = get_points_around_node(context, it);
        for (i32 j = 0; j < points_in_radius.count; j++) {
            auto point = points_in_radius.get_value(j);
            if (point->taken) continue;
            
            point->taken = true;
            
            if (point->flags & SPACE_POINT) {
                collected_space_points += 1;
            }
            if (point->flags & ITEM_POINT) {
                collected_item_points += 1;
            }
        }
    }
    
    planning->current_space_points = planning->base_space_points - used_space_points + collected_space_points;
    assert(planning->current_space_points >= 0 && planning->current_space_points <= planning->base_space_points + collected_space_points);
    
    planning->current_item_points  = planning->base_item_points - used_item_points + collected_item_points;
    assert(planning->current_item_points >= 0 && planning->current_item_points <= planning->base_item_points + collected_item_points);
}

void init_planning_data(Context *context) {
    context->planning.nodes.clear();
    context->planning.nodes.arena = &context->memory_arena;
    
    context->planning.node_icons.clear();
    context->planning.node_icons.arena = &context->memory_arena;
    
    add_node_icon(context, tstring("Space node"), SPACE_NODE, NULL);
    
    auto hit_booster_object = get_spawn_object_by_name(tstring("hit_booster"));
    add_node_icon(context, tstring("Booster node"), HIT_BOOSTER_NODE, hit_booster_object->entity);
}

void reset_planning_data(Context *context) {
    auto planning = &context->planning;

    planning->dragged_entity = NULL;
    planning->spawned_ids.clear();
    
    planning->selected_icon_index = 0;
    
    planning->nodes.clear();
    planning->nodes.append({.position = context->player_spawn_point, .type = SPACE_NODE}); // First is the base node.
    planning->nodes.append({.position = context->player_spawn_point, .type = SPACE_NODE}); // Second is the new node. We're always keeping node that will be spawned, even if we have no points for it.
    
    planning->current_space_points = 0;
    planning->current_item_points  = 0;
    
    planning_validate_node_points(context);
}

void validate_corner_node(Context *context) {
    // In this node we're going to make sure that node type and node entity is correct according to points count 
    // and current icon type.

    auto planning = &context->planning;
    auto corner_node = planning->nodes.last();

    auto active_icon = planning->node_icons.get(planning->selected_icon_index);
    if (active_icon->type == SPACE_NODE) {
        if (planning->current_space_points <= 0) {
            goto no_points_for_corner_node_case;
        }
    } else {
        if (planning->current_item_points <= 0) {
            goto no_points_for_corner_node_case;
        }
    }
    
    corner_node->type = active_icon->type;
    if (active_icon->entity_to_spawn && !corner_node->entity) {           
        corner_node->entity = copy_and_add_entity(active_icon->entity_to_spawn, context);
    }
    if (active_icon->entity_to_spawn == NULL && corner_node->entity != NULL) {
        mark_entity_destroyed(corner_node->entity);
        corner_node->entity = NULL;
    }
    
    return;
    
    no_points_for_corner_node_case: {
        if (corner_node->entity) {
            mark_entity_destroyed(corner_node->entity);
            corner_node->entity = NULL;
        }
        corner_node->type = NO_NODE;
        
        // If we have no points for corner node - we want to orient last real node entity right.
        // Without that it will look at where was corner node before it become no_points case and will orient itself correctly 
        // only when we'll enter gaming state (if we're not going to spawn last node).
        if (planning->nodes.count > 2) {
            auto pre_last = planning->nodes.get(planning->nodes.count - 2);
            
            if (pre_last->entity != NULL) {
                auto pre_pre_last = planning->nodes.get(planning->nodes.count - 3);
                
                Vector2 vec = pre_last->position - pre_pre_last->position;
                Vector2 dir = normalized(vec);
                
                change_up(pre_last->entity, dir);
            }
        }
    }
}

void planning_prepare_to_enter_gaming(Context *context) { 
    Planning_Data *planning = &context->planning;
    
    // Corner node is one that we just hold in our hand to place somewhere.
    // So when it will no longer exists (as on entering gaming state) - we'll mark it NO_NODE and removing entity that it was 
    // holding.
    auto corner_node = planning->nodes.last();
    assert(corner_node);
    if (corner_node->entity != NULL) {
        mark_entity_destroyed(corner_node->entity);
        corner_node->entity = NULL;
    }
    if (corner_node->type != NO_NODE) {
        corner_node->type = NO_NODE;
    }
    
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
            
            // NOTE: This removed_node was not real, this was the node that we were holding in our hand to place somewhere. 
            // That node is called corner_node and now corner node will be previous standing.
            
            planning_validate_node_points(context);
        }
    }
    
    assert(planning->nodes.count >= 2); // We're always keeping base node on player spawn point and a corner node that we hold in hand to place somewhere.
    auto last_node = planning->nodes.get(planning->nodes.count - 2);
    auto corner_node = planning->nodes.last();
    
    if (IsKeyPressed(KEY_TAB)) {
        planning->selected_icon_index += 1;
        planning->selected_icon_index %= planning->node_icons.count;
    }
    
    validate_corner_node(context);
    
    f32 target_radius = radius_from_node(last_node);
    Vector2 last_to_mouse = input.mouse_position - last_node->position;
    Vector2 dir = normalized(last_to_mouse);
    
    static const f32 ANGLE_STEP = 15;
    Planning_Node_Icon *node_icon = planning->node_icons.get(planning->selected_icon_index);
    
    f32 angle = fangle(dir);
    angle = round_to_factor(angle, ANGLE_STEP) + 90; // "+ 90" because we have retarged angle system where UP is zero.
    f32 rad = angle * DEG2RAD;
        
    dir = {-cosf(rad), sinf(rad)};
    
    Vector2 target_position = last_node->position + dir * target_radius;
    
    bool can_place_corner_node = true;
    
    Collision col = raycast(last_node->position, dir, target_radius, GROUND);
    Color line_color = BLUE;
    if (col.collided) {
        can_place_corner_node = false;
        line_color = RED;
    }
    
    if (corner_node->type == NO_NODE) {
        line_color = BLACK;
        can_place_corner_node = false;
    }
    
    make_line(last_node->position, target_position, 1.0f, line_color);
    
    corner_node->position = target_position;
    if (corner_node->entity) {
        corner_node->entity->position = target_position;
        change_up(corner_node->entity, dir);
    }
    
    if (last_node->entity && corner_node->type != NO_NODE) {
        change_up(last_node->entity, dir);
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (can_place_corner_node) {
            // new_node already in array, now we're adding something node that will be "next new_entity".
            auto added_node = planning->nodes.append({.position = target_position, .type = node_icon->type}); 
            validate_corner_node(context);
            
            planning_validate_node_points(context);
        } else {
            play_sound("FailedRifleActivation", 0.4f);
        }
    }
        
    if (planning->nodes_dirty) {
        planning_validate_node_points(context);
        planning->nodes_dirty = false;
    }
} // End update planning.

void planning_draw(Context *context) {
    For (&context->planning.nodes) {
        if (i > 0 && i < context->planning.nodes.count - 1) {
            auto previous = context->planning.nodes.get(i - 1);
            
            auto color = Fade(SKYBLUE, 0.5f);
            if (previous->type != SPACE_NODE) {
                color = Fade(RED, 0.4f);
            }
            
            make_line(previous->position, it->position, 0.5f, color);
        }
        
        if (game_state == GAME_PLANNING) { 
            f32 radius = radius_from_node(it);
            Color node_color = color_from_node(it);
            Color radius_color = radius_color_from_node(it);
            draw_game_circle(it->position, radius, radius_color);
            draw_game_circle(it->position, 5.0f, node_color);
        }
    }
}

void planning_draw_ui(Context *context) {
    Vector2 panel_pos  = {screen_width * 0.05f, screen_height * 0.2f};
    Vector2 panel_size = {screen_width * 0.1f, screen_height * 0.4f};
    Old::make_ui_image(panel_pos, panel_size, {0, 0}, color_fade(SKYBLUE, 0.2f), "planning_panel");
          
    // Old::make_ui_image(
    
    auto planning = &context->planning;
    
    Old::make_ui_text(tprintf("Space points: %d", planning->current_space_points), panel_pos, "space_points_info");
    Old::make_ui_text(tprintf("Item points: %d", planning->current_item_points), panel_pos + Vector2_up * 25, "item_points_info");
    
    For (&planning->node_icons) {
        Vector2 pos = panel_pos + Vector2_up * 50 * (i + 1);
        Vector2 size = {panel_size.x - 20, 40};
        
        Color color = BLUE;
        
        assert(it->type != NO_NODE); // NO_NODE type is only for nodes that's not for the placement, so there should be no icon with that type.
        
        b32 cannot_place_that_node = false;
        if (it->type == SPACE_NODE && planning->current_space_points <= 0) {
            cannot_place_that_node = true;
            color = GRAY;
        }
        if (it->type != SPACE_NODE && planning->current_item_points <= 0) {
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
