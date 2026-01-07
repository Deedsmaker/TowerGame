#pragma once

#include "string.cpp"
#include "array.cpp"
#include "allocator.cpp"
#include "logger.h"

String level_name_to_path(String name, Allocator *allocator) {
    return string(allocator, "levels/%s", c_str(name));
}

b32 level_exists(String name, Allocator *allocator) {
    auto path = level_name_to_path(name, allocator);
    
    b32 result = directory_exists(path);    
    
    return result;
}

static const String COMPLETED_LEVELS_FILE_NAME = S("levels/completed_levels.txt");
void load_completed_levels() { 
    For (&global_data.completed_levels) it->free_data();
    global_data.completed_levels.clear();  
    
    if (!file_exists(COMPLETED_LEVELS_FILE_NAME)) {
        return;
    }
    
    b32 success = false;
    auto file_content = read_entire_file(COMPLETED_LEVELS_FILE_NAME, &success, temp);
    if (!success) {
        return;
    }
    
    auto completed_in_file = split_string(file_content, S("\n "), temp);
    global_data.completed_levels.append_another_array(&completed_in_file, &default_allocator);
}

void record_completed_level(String name) {
    if (global_data.completed_levels.contains(name)) {
        return;
    }
    
    if (!level_exists(name, temp)) {
        log(string(temp, "Level %s does not exists and we've tried to record it's complition!", c_str(name)), LOG_ERROR);
        return;
    }

    b32 success = append_text_to_file(COMPLETED_LEVELS_FILE_NAME, string(temp, "%s\n", c_str(name)));
    if (!success) {
        log(string(temp, "Failed to append level name %s to file %s", c_str(name), c_str(COMPLETED_LEVELS_FILE_NAME)), LOG_ERROR);
        return;
    }
    
    global_data.completed_levels.append(copy_string(name, &default_allocator), &default_allocator);
}

void clear_completed_levels() {
    global_data.completed_levels.clear();
    if (!file_exists(COMPLETED_LEVELS_FILE_NAME)) {
        return;
    }
       
    b32 success = write_entire_file(COMPLETED_LEVELS_FILE_NAME, S(""));
    if (!success) {
        log(string(temp, "Failed to clear %s file!", c_str(COMPLETED_LEVELS_FILE_NAME)), LOG_ERROR);
    }
    
    validate_level_loaders(current_context);
}

void load_level_order() {
    String level_order_path = S("levels/level_order.txt");
    
    b32 success = false;
    String content = read_entire_file(level_order_path, &success, temp);   
    if (!success) {
        log("Failed to read %s file!", LOG_ERROR);
        return;
    }
    
    global_data.level_order = split_string(content, S("\n"), &default_allocator);
}

void save_level(String level_name) {
    String level_directory_name = level_name_to_path(level_name, temp);

    b32 is_autosave = string_contains(level_directory_name, S("autosaves"));
    if (!is_autosave) {
        log(tstring("Starting saving level %s", c_str(level_directory_name)), PUSH_INDENTATION);
    }

    String old_directory_name = tstring("%s_old", c_str(level_directory_name));
    
    // If it exists - we'll rename it so that we have copy of a level before saving another one.
    rename_directory(level_directory_name, old_directory_name);
    
    make_directory_if_not_exists(level_directory_name);
    
    Context *context = current_context;
    
    { // First of all making file with level info
        String_Builder level_info_builder = make_string_builder(256, temp);
        
        b32 is_hub_level = context->flags & HUB_CONTEXT;
        builder_append(&level_info_builder, tstring("is_hub_level %d \n ", is_hub_level));
        
        String spawn_point = tstring("player_spawn_point {%f, %f}\n ", context->player_spawn_point.x, context->player_spawn_point.y);
        builder_append(&level_info_builder, spawn_point);
        
        String_Builder lightmaps_builder = make_string_builder(128, temp);
        builder_append(&lightmaps_builder, S("lightmaps [ "));
        for (i32 i = 0; i < context->lightmaps.count; i++) {
            Lightmap_Data* l = context->lightmaps.get(i);
            builder_append(&lightmaps_builder, tstring("{ lightmap_position {%f, %f}, lightmap_size {%f, %f}}; ", l->position.x, l->position.y, l->game_size.x, l->game_size.y));
        }
        builder_append(&lightmaps_builder, S("];\n ")); 
        
        builder_append(&level_info_builder, make_string_from_builder(&lightmaps_builder, temp));
        
        write_entire_file(tstring("%s/Level_Info.txt", c_str(level_directory_name)), &level_info_builder);
    }
    
    // Now saving all entities. Each entity in separate file.
    for_chunk_array(entity_index, (&context->entities)) {
        Entity *e = context->entities.get(entity_index);
        
        if (e->runtime_only_flags & SHOULD_NOT_SAVE) {
            continue;
        }
        
        String_Builder b = make_string_builder(256, temp);
        
        Color color = e->color_changer.start_color;
        builder_append(&b, tstring("name %s \n id %d \n position { %f,  %f} \n scale { %f,  %f} \n pivot { %f,  %f} \n rotation %f \n color { %d,  %d,  %d,  %d} \n flags %llu \n draw_order %d \n ", c_str(*e->name), e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags, e->draw_order));
        
        builder_append(&b, S("vertices [ "));
        for (i32 v = 0; v < e->vertices.count; v++) {
            builder_append(&b, tstring("{ %f,  %f} ", e->vertices.get_value(v).x, e->vertices.get_value(v).y)); 
        }
        builder_append(&b, S("] \n ")); 
        
        builder_append(&b, S("unscaled_vertices [  "));
        for (i32 v = 0; v < e->unscaled_vertices.count; v++) {
            builder_append(&b, tstring("{ %f,  %f} ", e->unscaled_vertices.get_value(v).x, e->unscaled_vertices.get_value(v).y)); 
        }
        builder_append(&b, S("] \n ")); 
        
        builder_append(&b, tstring("hidden %d \n ", e->hidden));
        builder_append(&b, tstring("spawn_enemy_when_no_ammo %d \n ", e->spawn_enemy_when_no_ammo));
        
        // Save light.
        if (e->lights.count > 0) {
            Light *light = context->lights.get(e->lights.get_value(0));
            builder_append(&b, tstring("light_shadows_size_flag %d \n ",     light->shadows_size_flags));
            builder_append(&b, tstring("light_backshadows_size_flag %d \n ", light->backshadows_size_flags));
            builder_append(&b, tstring("light_make_shadows %d \n ",          light->make_shadows));
            builder_append(&b, tstring("light_make_backshadows %d \n ",      light->make_backshadows));
            builder_append(&b, tstring("light_bake_shadows %d \n ",          light->bake_shadows));
            builder_append(&b, tstring("light_radius %f \n ",                light->radius));
            builder_append(&b, tstring("light_opacity %f \n ",               light->opacity));
            builder_append(&b, tstring("light_power %f \n ",                 light->power));
            builder_append(&b, tstring("light_color { %d,  %d,  %d,  %d} \n ", (i32)light->color.r, (i32)light->color.g, (i32)light->color.b, (i32)light->color.a));
        }
        
        // Save trigger.
        if (e->flags & TRIGGER) {
            assert(e->trigger);
            if (e->trigger->connected.count > 0) {
                builder_append(&b, S("trigger_connected [ "));
                for (i32 v = 0; v < e->trigger->connected.count; v++) {
                    builder_append(&b, tstring("%d ", e->trigger->connected.get_value(v))); 
                }
                builder_append(&b, S("] \n ")); 
            }
            
            if (e->trigger->tracking.count > 0) {
                builder_append(&b, S("trigger_tracking [  "));
                for (i32 v = 0; v < e->trigger->tracking.count; v++) {
                    builder_append(&b, tstring(" %d ", e->trigger->tracking.get_value(v))); 
                }
                builder_append(&b, S("] \n ")); 
            }
            
            builder_append(&b, tstring("trigger_settings %llu \n ", e->trigger->settings));
            
            builder_append(&b, tstring("trigger_locked_camera_position { %f,  %f} \n ", e->trigger->locked_camera_position.x, e->trigger->locked_camera_position.y));
            
            if (e->trigger->settings & LOAD_LEVEL && strlen(e->trigger->level_name) > 1) {
                builder_append(&b, tstring("trigger_level_name %s \n ", e->trigger->level_name));
            }
            
            if (e->trigger->settings & PLAY_REPLAY && strlen(e->trigger->replay_name) > 1) {
                builder_append(&b, tstring("trigger_replay_name %s \n ", e->trigger->replay_name));
            }
            
            if (e->trigger->settings & CHANGE_ZOOM) {
                builder_append(&b, tstring("trigger_zoom_value %f \n ", e->trigger->zoom_value));
            }
            
            if (e->trigger->settings & PLAY_SOUND && strlen(e->trigger->sound_name) > 1) {
                builder_append(&b, tstring("trigger_sound_name %s \n ", e->trigger->sound_name));
            }
            
            if (e->trigger->cam_rails_points.count > 0) {
                builder_append(&b, S("trigger_cam_rails_points [  "));
                for (i32 v = 0; v < e->trigger->cam_rails_points.count; v++) {
                    builder_append(&b, tstring("{ %f,  %f} ", e->trigger->cam_rails_points.get_value(v).x, e->trigger->cam_rails_points.get_value(v).y)); 
                }
                builder_append(&b, S("] \n ")); 
            }
        }
        
        // Save kill switch.
        if (e->flags & KILL_SWITCH) {
            if (e->kill_switch->connected.count > 0) {
                builder_append(&b, S("kill_switch_connected [  "));
                for (i32 v = 0; v < e->kill_switch->connected.count; v++) {
                    builder_append(&b, tstring(" %d ", e->kill_switch->connected.get_value(v))); 
                }
                builder_append(&b, S("] \n ")); 
            }
        }
        
        // Save move sequence.
        if (e->flags & MOVE_SEQUENCE) {
            if (e->move_sequence->points.count > 0) {
                builder_append(&b, S("move_sequence_points [  "));
                for (i32 v = 0; v < e->move_sequence->points.count; v++) {
                    builder_append(&b, tstring("{ %f,  %f} ", e->move_sequence->points.get_value(v).x, e->move_sequence->points.get_value(v).y)); 
                }
                builder_append(&b, S("] \n ")); 
            }
            
            builder_append(&b, tstring("move_sequence_moving %d \n ",                        e->move_sequence->moving));
            builder_append(&b, tstring("move_sequence_speed %f \n ",                         e->move_sequence->speed));
            builder_append(&b, tstring("move_sequence_loop %d \n ",                          e->move_sequence->loop));
            builder_append(&b, tstring("move_sequence_rotate %d \n ",                        e->move_sequence->rotate));
            builder_append(&b, tstring("move_sequence_speed_related_player_distance %d \n ", e->move_sequence->speed_related_player_distance));
            builder_append(&b, tstring("move_sequence_min_distance %f \n ",                  e->move_sequence->min_distance));
            builder_append(&b, tstring("move_sequence_max_distance %f \n ",                  e->move_sequence->max_distance));
            builder_append(&b, tstring("move_sequence_max_distance_speed %f \n ",            e->move_sequence->max_distance_speed));
        }
        
        // Save centipede.
        if (e->flags & CENTIPEDE) {
            builder_append(&b, tstring("spikes_on_right %d \n ", e->centipede->spikes_on_right));
            builder_append(&b, tstring("spikes_on_left %d \n ", e->centipede->spikes_on_left));
            builder_append(&b, tstring("segments_count %d \n ", e->centipede->segments_to_spawn));
        }
        
        // Save jump shooter.
        if (e->flags & JUMP_SHOOTER) {
            builder_append(&b, tstring("jump_shooter_shots_count %d \n ", e->jump_shooter->shots_count));
            builder_append(&b, tstring("jump_shooter_spread %f \n ", e->jump_shooter->spread));
            builder_append(&b, tstring("jump_shooter_explosive_count %d \n ", e->jump_shooter->explosive_count));
            builder_append(&b, tstring("jump_shooter_shoot_sword_blockers %d \n ", e->jump_shooter->shoot_sword_blockers));
            // builder_append(&b, tstring("jump_shooter_shoot_sword_blockers_clockwise %d \n ", e->jump_shooter->shoot_sword_blockers_clockwise));
            // builder_append(&b, tstring("jump_shooter_shoot_sword_blockers_random_direction %d \n ", e->jump_shooter->shoot_sword_blockers_random_direction));
            builder_append(&b, tstring("jump_shooter_shoot_sword_blockers_immortal %d \n ", e->jump_shooter->shoot_sword_blockers_immortal));
            builder_append(&b, tstring("jump_shooter_shoot_bullet_blockers %d \n ", e->jump_shooter->shoot_bullet_blockers));
        }
        
        // Save turret.
        if (e->flags & TURRET) {
            Turret *turret = e->turret;
            builder_append(&b, tstring("turret_projectile_flags %llu \n ", turret->projectile_settings.enemy_flags));
            builder_append(&b, tstring("turret_shoot_sword_blocker_clockwise %d \n ", turret->projectile_settings.blocker_clockwise));
            builder_append(&b, tstring("turret_activated %d \n ", turret->activated));
            builder_append(&b, tstring("turret_homing_projectiles %d \n ", turret->homing));
            builder_append(&b, tstring("turret_shoot_every_tick %d \n ", turret->shoot_every_tick));
            builder_append(&b, tstring("turret_start_tick_delay %d \n ", turret->start_tick_delay));
            builder_append(&b, tstring("turret_projectile_speed %f \n ", turret->projectile_settings.launch_speed));
            builder_append(&b, tstring("turret_projectile_max_lifetime %f \n ", turret->projectile_settings.max_lifetime));
            builder_append(&b, tstring("turret_shoot_width %f \n ", turret->shoot_width));
            builder_append(&b, tstring("turret_shoot_height %f \n ", turret->shoot_height));
        }
        
        // Save door.
        if (e->flags & DOOR) {
            builder_append(&b, tstring("door_open %d \n ", e->door.is_open));
        }
        
        // Save blocker.
        if (e->flags & BLOCKER) {
            builder_append(&b, tstring("blocker_clockwise %d \n ", e->union_enemy->blocker_clockwise));
            builder_append(&b, tstring("blocker_immortal %d \n ", e->union_enemy->blocker_immortal));
        }
        
        // Save sword size required.
        if (e->flags & SWORD_SIZE_REQUIRED) {
            builder_append(&b, tstring("enemy_big_or_small_killable %d \n ", e->union_enemy->big_sword_killable));
        }
        
        // Save enemy.
        if (e->flags & ENEMY && e->union_enemy->sword_kill_speed_modifier != 1) {
            builder_append(&b, tstring("sword_kill_speed_modifier %.1f \n ", e->union_enemy->sword_kill_speed_modifier));
        }
        // Save enemy.
        if (e->flags & ENEMY) {
            builder_append(&b, tstring("enemy_gives_ammo %d \n ", e->union_enemy->gives_ammo));
        }
        
        // Save explosive.
        if (e->flags & EXPLOSIVE) {
            builder_append(&b, tstring("explosive_radius_multiplier %fs \n ", e->union_enemy->explosive_radius_multiplier));
        }
        
        // Save propeller.
        if (e->flags & PROPELLER) {
            builder_append(&b, tstring("propeller_power %f \n ", e->propeller->power));
            builder_append(&b, tstring("propeller_spin_sensitive %d \n ", e->propeller->spin_sensitive));
        }
        
        // Save shoot blocker.
        if (e->flags & SHOOT_BLOCKER) {
            builder_append(&b, tstring("shoot_blocker_direction { %f,  %f} \n ", e->union_enemy->shoot_blocker_direction.x, e->union_enemy->shoot_blocker_direction.y));
            builder_append(&b, tstring("shoot_blocker_immortal %d \n ", e->union_enemy->shoot_blocker_immortal));
        }
        
        // Save texture name.
        if (e->texture && strlen(e->texture_name) > 1) {
            builder_append(&b, tstring("texture_name %s \n ", e->texture_name));
        }
        
        // Save note.
        if (e->flags & NOTE) {
            assert(e->note_index != -1);
            auto note = current_context->notes.get(e->note_index);
            if (strlen(note->content) > 1) {
                builder_append(&b, tstring("note_content \" %s \" \n ", note->content));
            }
                
            builder_append(&b, tstring("note_draw_in_game %d \n ", note->draw_in_game));
        }
        
        // Save level loader.
        if (e->flags & LEVEL_LOADER) {
            auto loader = e->level_loader;
            
            builder_append(&b, tstring("level_loader_flags %lld \n ", loader->flags));
            builder_append(&b, tstring("level_name_to_load %s \n ", c_str(loader->level_to_load)));
            
            // if (loader->levels_to_open.count > 0) {
                // builder_append(&b, S("level_loader_levels_to_open [  "));
                // for (i32 v = 0; v < loader->levels_to_open.count; v++) {
                //     builder_append(&b, tstring(" %d ", loader->levels_to_open.get_value(v))); 
                // }
                // builder_append(&b, S("] \n ")); 
            // }

        }
        
        // Save planning point.
        if (e->flags & PLANNING_POINT) {
            builder_append(&b, tstring("planning_point_flags %llu \n ", e->planning_point->flags));
        }
        
        write_entire_file(tstring("%s/Entity_%s_%d.txt", c_str(level_directory_name), c_str(*e->name), e->id), &b);
    }
    
    delete_directory(old_directory_name);
    
    if (!is_autosave) {
        log(tstring("Finished saving level %s", c_str(level_directory_name)), POP_INDENTATION);
        game_log("Saved.");
    }
    
} // Save level end.

Vector2 parse_vector2(Array <String> *splitted, i32 start_index) {
    if (start_index + 2 >= splitted->count) return {0};
    
    Vector2 v = {0};
    v.x = to_f32(splitted->get_value(start_index));
    v.y = to_f32(splitted->get_value(start_index + 1));
    
    return v;
}

Color parse_color(Array <String> *splitted, i32 start_index) {
    if (start_index + 4 >= splitted->count) return {0};
    
    Color c = {0};
    c.r = to_u8(splitted->get_value(start_index));
    c.g = to_u8(splitted->get_value(start_index + 1));
    c.b = to_u8(splitted->get_value(start_index + 2));
    c.a = to_u8(splitted->get_value(start_index + 3));
    
    return c;
}

void parse_lightmaps(Array <Lightmap_Data> *lightmaps, Array <String> *splitted, i32 start_index, Allocator *allocator) {
    i32 end_index = splitted->find_from(tstring("\n"), start_index);
    if (end_index <= start_index) {
        // printf("End index was somehow less than a start index. That could mean that there was no breakline character at the end.\n");
        return;
    }
    
    for (i32 i = start_index; i < end_index; i++) {
        Lightmap_Data lightmap = {0};
    
        i32 position_index = splitted->find_from(tstring("lightmap_position"), i);    
        if (position_index < 0) break;
        
        lightmap.position = parse_vector2(splitted, position_index + 1);
        i = position_index + 2;
        
        i32 size_index = splitted->find_from(tstring("lightmap_size"), i);
        if (size_index < 0) break;
        
        lightmap.game_size = parse_vector2(splitted, size_index + 1);
        i = size_index + 2;
        
        if (file_exists(lightmap_name(lightmaps->count))) {
            lightmap.lightmap_texture = LoadTexture(c_str(lightmap_name(current_context->lightmaps.count)));
            lightmap.has_loaded_texture = true;
        }
        
        lightmaps->append(lightmap, allocator);
        
        i--; // We're on a next lightmap_position index right now, so subtracting one and for loop i++ won't break our jaw.
    }
}

void parse_i32_array(Array <i32> *array, Array <String> *splitted, i32 start_index, Allocator *allocator) {
    i32 end_index = splitted->find_from(tstring("\n"), start_index); 
    if (end_index <= start_index) {
        printf("Could not find breakline in parse i32 array!\n");
        return;
    }
    
    for (i32 i = start_index; i < end_index; i++) {
        i32 number = to_i32(splitted->get_value(i));        
        array->append(number, allocator);
    }
}

void parse_vector2_array(Array <Vector2> *array, Array <String> *splitted, i32 start_index, Allocator *allocator) {
    i32 end_index = splitted->find_from(tstring("\n"), start_index); 
    if (end_index <= start_index) {
        printf("Could not find breakline in parse vector2 array!\n");
        return;
    }
    
    for (i32 i = start_index; i < end_index; i++) {
        Vector2 v = parse_vector2(splitted, i);
        i += 1;
        array->append(v, allocator);
    }
}
void parse_vertices_array(Static_Array <Vector2, MAX_VERTICES> *array, Array <String> *splitted, i32 start_index) {
    i32 end_index = splitted->find_from(tstring("\n"), start_index); 
    if (end_index <= start_index) {
        printf("Could not find breakline in parse vector2 static array!\n");
        return;
    }
    
    for (i32 i = start_index; i < end_index && array->count < MAX_VERTICES; i++) {
        Vector2 v = parse_vector2(splitted, i);
        i += 1;
        array->append(v);
    }
}

// String should have qute symbols marking start and end (note_content "some content").
// index in meaning that it's not index of string beginning, but rather note_content index from example above. 
// Then we'll find start and end of content string by yourself.
String parse_string(String whole_data, i32 index, Allocator *allocator) {
    i32 start_index = string_find_from(whole_data, tstring("\""), index);
    start_index += 1; // So now it's pointing at actual beginning of a content.
    
    i32 end_index = string_find_from(whole_data, tstring("\""), start_index);
    end_index -= 1; // Now it's pointing to last character and our substring function includes last symbol so that's what we need.
    
    if (start_index < 0 || end_index < 0 || end_index < start_index) return {0};
    
    String s = make_substring(whole_data, start_index, end_index, allocator);
    return s;
}

#define IF_FIND(str) if ((i = splitted.find(tstring(str))) >= 0)

void create_level(String name) {
    String path = level_name_to_path(name, temp);
    
    if (directory_exists(path)) {
        return;
    }
    
    log(tstring("Creating level %s", c_str(path)));
    
    clean_up_scene();
    // switch_current_context(&loaded_context);
    // clear_context(&loaded_context);
    clear_context(editor_context);
    editor_context->level_name = copy_string(name, &editor_context->allocator);
    
    // save_level(name);
    editor_enter_editor_state();
    print_to_console("Level successfuly created");
    
    reload_level_names();
}

enum Load_Flags {
    ENTER_GAME_STATE_AFTER = 0x1,  
    ERROR_IF_NO_SUCH_LEVEL = 0x2,
};

global_variable b32 prepared_loaded_level = false;
global_variable u64 last_load_flags = 0;

void finish_load_level() {
    clean_up_scene();
    clear_context(&game_context);
    
    validate_level_loaders(&loaded_context);
    
    // This shit so that we don't overwrite level that we currently on.
    do {
        last_loaded_editor_context_index += 1;    
        last_loaded_editor_context_index %= MAX_LOADED_LEVELS;    
    } while (last_loaded_editor_context_index == current_editor_context_index);
    editor_context = &loaded_editor_contexts[last_loaded_editor_context_index];
    current_editor_context_index = last_loaded_editor_context_index;
    clear_context(editor_context);
    copy_context(editor_context, &loaded_context, true);
    
    if (last_load_flags & ENTER_GAME_STATE_AFTER) {
        if (editor_state == EDITOR) {
            editor_enter_game_state(&loaded_context);
        } else {
            copy_context(&planning_context, &loaded_context, true);
            enter_planning_state(RESET_PLANNING_DATA);
        }
    } else {
        editor_enter_editor_state();
    }
    
    
    current_context->cam.position = current_context->player_spawn_point;
    current_context->cam.target = current_context->player_spawn_point;
    
    close_console();
}

b32 load_level(String name, u64 load_flags = 0) {
    clear_multiselected_entities();

    last_load_flags = load_flags;

    name = copy_string(name, temp); // Beacuse we're just cleared temp allocator and if "name" was temp allocated - it could go wrong.
    
    // editor_enter_editor_state();
    // editor_state = EDITOR; // @TODO: Do we really need this? Edit: Right now yes, for convinience.
    
    global_data.playing_replay = false;
    String level_path = level_name_to_path(name, temp);
    
    
    if (!directory_exists(level_path)) {
        if (load_flags & ERROR_IF_NO_SUCH_LEVEL) {
            log(string(temp, "No level named %s", c_str(name)), LOG_ERROR);
            game_log(string(temp, "No level named %s", c_str(name)));
            return false;
        } else {
            builder_append(&console.content_builder, S("Level does not exists! Will create new"));
            create_level(name);
            return true;
        }
    }
    
    log(tstring("Loading level %s", c_str(level_path)), PUSH_INDENTATION);
    
    // clean_up_scene();
    // switch_current_context(&loaded_context, true);
    clear_context(&loaded_context);
    
    Context *context = &loaded_context;
    
    if (!(context->level_name == name)) {
        global_data.previous_level_name = copy_string(context->level_name, &context->allocator);
    }
    
    context->level_name = copy_string(name, &context->allocator);
    
    setup_particles();
    
    Array <String> splitted = {};
    // Array <Entity> loaded_entities = {.allocator = temp};
    
    Array <String> level_files = get_files_in_directory(level_path, temp);
    if (level_files.count == 0) {
        log(tstring("Level directory %s was empty! We're stopping loading.\n", c_str(level_path)), LOG_ERROR | POP_INDENTATION);
        return false;
    }
    
    String separators = S(":{}[], ;");
    
    String level_info_file_name = S("Level_Info.txt");
    
    b32 is_hub_level = false;
    
    i32 level_info_file_index = find_file_name_in_paths(&level_files, level_info_file_name, temp);
    if (level_info_file_index < 0) {
        log(tstring("Failed to find %s file!\n", c_str(level_info_file_name)));
    } else {
        b32 success = false;
        String level_info = read_entire_file(level_files.get_value(level_info_file_index), &success, temp);
        if (!success) {
            log(tstring("Failed to read %s file! Stopping loading.\n", c_str(level_info_file_name)), LOG_ERROR | POP_INDENTATION);
            return false;
        }
        
        split_string(&splitted, level_info, separators, temp);
        
        i32 is_hub_level_index = splitted.find(tstring("is_hub_level"));
        if (is_hub_level_index >= 0) {
            is_hub_level = to_i32(splitted.get_value(is_hub_level_index + 1));
            if (is_hub_level) {
                context->flags |= HUB_CONTEXT;
            }
        }
        
        i32 spawn_point_index = splitted.find(tstring("player_spawn_point"));
        if (spawn_point_index >= 0) context->player_spawn_point = parse_vector2(&splitted, spawn_point_index + 1);
        
        i32 lightmaps_index = splitted.find(tstring("lightmaps"));
        if (lightmaps_index > 0) {
            parse_lightmaps(&context->lightmaps, &splitted, lightmaps_index + 1, &context->allocator);
        }
    }
    
    // Sorting level files so entities will come in correct id order.
    sort_file_paths(&level_files);
    
    for_array (i, &level_files) {
        String file_path = level_files.get_value(i);    
        String file_name = strip_path_to_just_name(file_path, temp);
        
        if (file_name == level_info_file_name) {
            // We've parsed that before.
            continue;
        } else if (string_find(file_name, tstring("Entity")) == 0) {
            // There goes entity parsing.
            
            b32 success = false;
            String entity_info = read_entire_file(file_path, &success, temp);
            if (!success) {
                log(tstring("Failed to read entity file data from file %s\n", c_str(file_path)), LOG_ERROR);
                continue;
            }
            
            split_string(&splitted, entity_info, separators, temp);
            
            auto dummy_entity = add_default_entity(&copied_entities_context); // It's here just for setting data for initing.
            
            i32 i = -1;
            i32 id = 0;
            IF_FIND("id") id = to_i32(splitted.get_value(i+1));
            
            IF_FIND("flags") dummy_entity->flags = to_u64(splitted.get_value(i+1));
            
            add_entity_types_and_take_allocator(dummy_entity);
            
            auto entity = copy_and_add_entity(dummy_entity, &loaded_context, id);
            
            free_entity(dummy_entity);
            
            if (entity->flags & LIGHT) {        
                // We're adding empty ligh just because on loading we're gonna fill this empty light.
                Light dummy_light = {0};
                copy_and_add_light_to_entity(entity, &dummy_light, true);
            }
            
            Note note_to_fill = {};
            
            // Entity loading.
            
            IF_FIND("position") entity->position = parse_vector2(&splitted, i+1);
            IF_FIND("scale") { entity->scale = parse_vector2(&splitted, i+1); change_scale(entity, entity->scale); }
            IF_FIND("pivot") entity->pivot = parse_vector2(&splitted, i+1);
            
            IF_FIND("rotation") { entity->rotation = to_f32(splitted.get_value(i+1)); }
            
            IF_FIND("color") change_color(entity, parse_color(&splitted, i+1));
            
            IF_FIND("hidden") entity->hidden = to_i32(splitted.get_value(i+1));
            IF_FIND("spawn_enemy_when_no_ammo") entity->spawn_enemy_when_no_ammo = to_i32(splitted.get_value(i+1));
            
            IF_FIND("vertices") parse_vertices_array(&entity->vertices, &splitted, i+1);
            IF_FIND("unscaled_vertices") parse_vertices_array(&entity->unscaled_vertices, &splitted, i+1);
            
            IF_FIND("texture_name") str_copy(entity->texture_name, c_str(splitted.get_value(i+1)));
            
            IF_FIND("draw_order") entity->draw_order = to_i32(splitted.get_value(i+1));
            
            // Trigger loading.
            if (entity->flags & TRIGGER) {
                assert(entity->trigger);
                Trigger *trigger = entity->trigger;
                // @TODO: All this thousand bools of trigger should be just flags...
                IF_FIND("trigger_connected")                  parse_i32_array(&trigger->connected, &splitted, i+1, entity->allocator);
                IF_FIND("trigger_tracking")                   parse_i32_array(&trigger->tracking, &splitted, i+1, entity->allocator);
                IF_FIND("trigger_cam_rails_points")           parse_vector2_array(&trigger->cam_rails_points, &splitted, i+1, entity->allocator);
                IF_FIND("trigger_locked_camera_position")     trigger->locked_camera_position     = parse_vector2(&splitted, i+1);
                
                IF_FIND("trigger_settings") trigger->settings = to_u64(splitted.get_value(i+1));
                
                IF_FIND("trigger_zoom_value")                 trigger->zoom_value                 = to_f32(splitted.get_value(i+1));
                IF_FIND("trigger_level_name")                 str_copy(trigger->level_name, c_str(splitted.get_value(i+1)));
                IF_FIND("trigger_sound_name")                 str_copy(trigger->sound_name, c_str(splitted.get_value(i+1)));
                IF_FIND("trigger_replay_name")                str_copy(trigger->replay_name, c_str(splitted.get_value(i+1)));
            }
            
            // Level loader loading.
            if (entity->flags & LEVEL_LOADER) {
                auto loader = entity-> level_loader;
                
                IF_FIND("level_loader_flags")          loader->flags          = to_u64(splitted.get_value(i+1));
                IF_FIND("level_name_to_load")          loader->level_to_load  = copy_string(splitted.get_value(i+1), &default_allocator);
                // IF_FIND("level_loader_levels_to_open") parse_i32_array(&loader->levels_to_open, &splitted, i+1);
            }
            
            // Kill switch loading.
            if (entity->flags & KILL_SWITCH) {
                assert(entity->kill_switch);
                IF_FIND("kill_switch_connected") parse_i32_array(&entity->kill_switch->connected, &splitted, i+1, entity->allocator);
            }
            
            // Jump shooter loading.
            if (entity->flags & JUMP_SHOOTER) {
                assert(entity->jump_shooter);
                Jump_Shooter *shooter = entity->jump_shooter;
                
                IF_FIND("jump_shooter_explosive_count")               shooter->explosive_count = to_i32(splitted.get_value(i+1));
                IF_FIND("jump_shooter_shoot_sword_blockers")          shooter->shoot_sword_blockers = to_i32(splitted.get_value(i+1));
                IF_FIND("jump_shooter_shoot_sword_blockers_immortal") shooter->shoot_sword_blockers_immortal = to_i32(splitted.get_value(i+1));
                IF_FIND("jump_shooter_shoot_bullet_blockers")         shooter->shoot_bullet_blockers = to_i32(splitted.get_value(i+1));
                IF_FIND("jump_shooter_shots_count")                   shooter->shots_count = to_i32(splitted.get_value(i+1));
                IF_FIND("jump_shooter_spread")                        shooter->spread = to_f32(splitted.get_value(i+1));
            }
            
            // Turret loading.
            if (entity->flags & TURRET) {
                assert(entity->turret);
                Turret *turret = entity->turret;
                
                IF_FIND("turret_projectile_flags")              turret->projectile_settings.enemy_flags = to_u64(splitted.get_value(i+1));
                IF_FIND("turret_shoot_sword_blocker_clockwise") turret->projectile_settings.blocker_clockwise = to_i32(splitted.get_value(i+1));
                IF_FIND("turret_homing_projectiles")            turret->homing= to_i32(splitted.get_value(i+1));
                IF_FIND("turret_shoot_every_tick")              turret->shoot_every_tick = to_i32(splitted.get_value(i+1));
                IF_FIND("turret_start_tick_delay")              turret->start_tick_delay = to_i32(splitted.get_value(i+1));
                IF_FIND("turret_projectile_speed")              turret->projectile_settings.launch_speed = to_f32(splitted.get_value(i+1));
                IF_FIND("turret_projectile_max_lifetime")       turret->projectile_settings.max_lifetime = to_f32(splitted.get_value(i+1));
                IF_FIND("turret_shoot_width")                   turret->shoot_width = to_f32(splitted.get_value(i+1));
                IF_FIND("turret_shoot_height")                  turret->shoot_height = to_f32(splitted.get_value(i+1));
                IF_FIND("turret_activated")                     turret->activated = to_i32(splitted.get_value(i+1));
            }
            
            // Move sequence loading.
            if (entity->flags & MOVE_SEQUENCE) {
                assert(entity->move_sequence);
                Move_Sequence *sequence = entity->move_sequence;
                
                IF_FIND("move_sequence_moving")                        sequence->moving = to_i32(splitted.get_value(i+1));
                IF_FIND("move_sequence_loop")                          sequence->loop = to_i32(splitted.get_value(i+1));
                IF_FIND("move_sequence_rotate")                        sequence->rotate = to_i32(splitted.get_value(i+1));
                IF_FIND("move_sequence_speed_related_player_distance") sequence->speed_related_player_distance = to_i32(splitted.get_value(i+1));
                IF_FIND("move_sequence_min_distance")                  sequence->min_distance = to_f32(splitted.get_value(i+1));
                IF_FIND("move_sequence_max_distance")                  sequence->max_distance = to_f32(splitted.get_value(i+1));
                IF_FIND("move_sequence_max_distance_speed")            sequence->max_distance_speed = to_f32(splitted.get_value(i+1));
                IF_FIND("move_sequence_speed")                         sequence->speed = to_f32(splitted.get_value(i+1));
                IF_FIND("move_sequence_points")                        parse_vector2_array(&sequence->points, &splitted, i+1, entity->allocator);
            }
            
            // Door loading.
            if (entity->flags & DOOR) {
                IF_FIND("door_open") entity->door.is_open = to_i32(splitted.get_value(i+1));
            }
            
            // Centipede loading.
            if (entity->flags & CENTIPEDE) {
                assert(entity->centipede);
                Centipede *centipede = entity->centipede;
                
                IF_FIND("spikes_on_right") centipede->spikes_on_right = to_i32(splitted.get_value(i+1));
                IF_FIND("spikes_on_left")  centipede->spikes_on_left = to_i32(splitted.get_value(i+1));
                IF_FIND("segments_count")  centipede->segments_to_spawn = to_i32(splitted.get_value(i+1));
            }
            
            // Enemy loading.
            if (entity->flags & ENEMY) {
                assert(entity->union_enemy);
                Enemy *enemy = entity->union_enemy;
                IF_FIND("enemy_big_or_small_killable") enemy->big_sword_killable = to_i32(splitted.get_value(i+1));
                IF_FIND("blocker_clockwise")           enemy->blocker_clockwise = to_i32(splitted.get_value(i+1));
                IF_FIND("blocker_immortal")            enemy->blocker_immortal = to_i32(splitted.get_value(i+1));
                IF_FIND("sword_kill_speed_modifier")   enemy->sword_kill_speed_modifier = to_i32(splitted.get_value(i+1));
                IF_FIND("shoot_blocker_direction") enemy->shoot_blocker_direction = parse_vector2(&splitted, i+1);
                IF_FIND("shoot_blocker_immortal") enemy->shoot_blocker_immortal = to_i32(splitted.get_value(i+1));
                IF_FIND("enemy_gives_ammo") enemy->gives_ammo = to_i32(splitted.get_value(i+1));
                IF_FIND("explosive_radius_multiplier") enemy->explosive_radius_multiplier = to_i32(splitted.get_value(i+1));
            }
            
            // Propeller loading.
            if (entity->flags & PROPELLER) {
                assert(entity->propeller);
                
                IF_FIND("propeller_power") entity->propeller->power = to_f32(splitted.get_value(i+1));
                IF_FIND("propeller_spin_sensitive") entity->propeller->spin_sensitive = to_i32(splitted.get_value(i+1));
            }
            
            // Light loading.
            if (entity->flags & LIGHT) {
                assert(entity->lights.count > 0);
                Light *light = context->lights.get(entity->lights.get_value(0));
                
                IF_FIND("light_shadows_size_flag")     light->shadows_size_flags = to_i32(splitted.get_value(i+1));
                IF_FIND("light_backshadows_size_flag") light->backshadows_size_flags = to_i32(splitted.get_value(i+1));
                IF_FIND("light_make_shadows")          light->make_shadows = to_i32(splitted.get_value(i+1));
                IF_FIND("light_make_backshadows")      light->make_backshadows = to_i32(splitted.get_value(i+1));
                IF_FIND("light_bake_shadows")          light->bake_shadows = to_i32(splitted.get_value(i+1));
                IF_FIND("light_radius")                light->radius = to_f32(splitted.get_value(i+1));
                IF_FIND("light_opacity")               light->opacity = to_f32(splitted.get_value(i+1));
                IF_FIND("light_power")                 light->power = to_f32(splitted.get_value(i+1));
                IF_FIND("light_color")                 light->color = parse_color(&splitted, i+1);
            }
            
            // Note loading.
            if (entity->flags & NOTE) {
                assert(entity->note_index >= 0);
                
                i32 note_content_index = string_find(entity_info, tstring("note_content"));
                String note_string = parse_string(entity_info, note_content_index, temp);
                
                Note *note = context->notes.get(entity->note_index);
                str_copy(note->content, c_str(note_string));
                
                IF_FIND("note_draw_in_game") note->draw_in_game = to_i32(splitted.get_value(i+1));
            }
            
            // Planning point loading.
            if (entity->flags & PLANNING_POINT) {
                auto point = entity->planning_point;
                
                IF_FIND("planning_point_flags") point->flags = to_u64(splitted.get_value(i+1));
            }
            
            // Of course we want to init entity again even if we've done it with flags at the beginning, because some initing can 
            // depend on exact data that we filled on loading.
            init_entity(entity);
            
            // Entity *loaded = loaded_entities.insert(*entity, entity->context->entities.get_total_occupied_count() - 1);
            // loaded->id = old_id;
        } // End of a entity file scope.
    } // End of a files for loop.
    
    setup_context_cam(context);
    context->cam.cam2D.zoom = 0.35f;
    
    log(tstring("Finished loading %s. Is hub level: %d", c_str(level_path), (context->flags & HUB_CONTEXT) > 0), POP_INDENTATION);
    
    prepared_loaded_level = true;
    
    return true;
} // End load level.

b32 maybe_load_next_level() { 
    i32 current_level_index = global_data.level_order.find(current_context->level_name);
    if (current_level_index >= 0 && current_level_index < global_data.level_order.count - 1) {
        u64 flags = ENTER_GAME_STATE_AFTER;
        if (editor_state == EDITOR) {
            flags = 0;
        }
        
        b32 success  = load_level(global_data.level_order.get_value(current_level_index + 1), flags);
        return success;
    }
    
    return false;
}

void load_hub_level() {
    load_level(S("hub"), ENTER_GAME_STATE_AFTER);
}

