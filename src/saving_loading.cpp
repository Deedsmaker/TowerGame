#pragma once

#include "string.cpp"
#include "array.cpp"
#include "Allocator.cpp"

void new_save_level(String level_name) {
    String level_directory_name = tstring("levels/%s", c_str(level_name));

    String old_directory_name = tstring("%s_old", c_str(level_directory_name));
    
    // If it exists - we'll rename it so that we have copy of a level before saving another one.
    rename_directory(level_directory_name, old_directory_name);
    
    make_directory_if_not_exists(level_directory_name);
    
    { // First of all making file with level info
        String_Builder level_info_builder = make_string_builder(256, temp);
        
        String spawn_point = tstring("player_spawn_point {%f, %f}\n ", current_level_context->player_spawn_point.x, current_level_context->player_spawn_point.y);
        builder_append(&level_info_builder, spawn_point);
        
        String_Builder lightmaps_builder = make_string_builder(128, temp);
        builder_append(&lightmaps_builder, tstring("lightmaps [ "));
        for (i32 i = 0; i < current_level_context->lightmaps.count; i++) {
            Lightmap_Data* l = current_level_context->lightmaps.get(i);
            builder_append(&lightmaps_builder, tstring("{ lightmap_position {%f, %f}, lightmap_size {%f, %f}}; ", l->position.x, l->position.y, l->game_size.x, l->game_size.y));
        }
        builder_append(&lightmaps_builder, tstring("];\n ")); 
        
        builder_append(&level_info_builder, make_string_from_builder(&lightmaps_builder, temp));
        
        write_entire_file(tstring("%s/Level_Info.txt", c_str(level_directory_name)), &level_info_builder);
    }
    
    // Now saving all entities. Each entity in separate file.
    for_chunk_array(entity_index, (&current_level_context->entities)) {
        Entity *e = current_level_context->entities.get(entity_index);
        
        if (!e->need_to_save) {
            continue;
        }
        
        String_Builder builder = make_string_builder(256, temp);
        
        Color color = e->color_changer.start_color;
        builder_append(&builder, tstring("name %s \n id %d \n position { %f,  %f} \n scale { %f,  %f} \n pivot { %f,  %f} \n rotation %f \n color { %d,  %d,  %d,  %d} \n flags %llu \n draw_order %d \n ", temp_entity_name(e).data, e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags, e->draw_order));
        
        builder_append(&builder, tstring("vertices [ "));
        for (i32 v = 0; v < e->vertices.count; v++) {
            builder_append(&builder, tstring("{ %f,  %f} ", e->vertices.get_value(v).x, e->vertices.get_value(v).y)); 
        }
        builder_append(&builder, tstring("] \n ")); 
        
        builder_append(&builder, tstring("unscaled_vertices [  "));
        for (i32 v = 0; v < e->unscaled_vertices.count; v++) {
            builder_append(&builder, tstring("{ %f,  %f} ", e->unscaled_vertices.get_value(v).x, e->unscaled_vertices.get_value(v).y)); 
        }
        builder_append(&builder, tstring("] \n ")); 
        
        builder_append(&builder, tstring("hidden %d \n ", e->hidden));
        builder_append(&builder, tstring("spawn_enemy_when_no_ammo %d \n ", e->spawn_enemy_when_no_ammo));
        
        if (e->lights.count > 0) {
            Light *light = current_level_context->lights.get(e->lights.get_value(0));
            builder_append(&builder, tstring("light_shadows_size_flag %d \n ",     light->shadows_size_flags));
            builder_append(&builder, tstring("light_backshadows_size_flag %d \n ", light->backshadows_size_flags));
            builder_append(&builder, tstring("light_make_shadows %d \n ",          light->make_shadows));
            builder_append(&builder, tstring("light_make_backshadows %d \n ",      light->make_backshadows));
            builder_append(&builder, tstring("light_bake_shadows %d \n ",          light->bake_shadows));
            builder_append(&builder, tstring("light_radius %f \n ",                light->radius));
            builder_append(&builder, tstring("light_opacity %f \n ",               light->opacity));
            builder_append(&builder, tstring("light_power %f \n ",                 light->power));
            builder_append(&builder, tstring("light_color { %d,  %d,  %d,  %d} \n ", (i32)light->color.r, (i32)light->color.g, (i32)light->color.b, (i32)light->color.a));
        }
        
        if (e->flags & TRIGGER) {
            assert(e->trigger);
            if (e->trigger->connected.count > 0) {
                builder_append(&builder, tstring("trigger_connected [ "));
                for (i32 v = 0; v < e->trigger->connected.count; v++) {
                    builder_append(&builder, tstring("%d ", e->trigger->connected.get_value(v))); 
                }
                builder_append(&builder, tstring("] \n ")); 
            }
            
            if (e->trigger->tracking.count > 0) {
                builder_append(&builder, tstring("trigger_tracking [  "));
                for (i32 v = 0; v < e->trigger->tracking.count; v++) {
                    builder_append(&builder, tstring(" %d ", e->trigger->tracking.get_value(v))); 
                }
                builder_append(&builder, tstring("] \n ")); 
            }
            
            builder_append(&builder, tstring("trigger_kill_player %d \n ",                    e->trigger->kill_player));
            builder_append(&builder, tstring("trigger_die_after_trigger %d \n ",              e->trigger->die_after_trigger));
            builder_append(&builder, tstring("trigger_kill_enemies %d \n ",                   e->trigger->kill_enemies));
            builder_append(&builder, tstring("trigger_open_doors %d \n ",                     e->trigger->open_doors));
            builder_append(&builder, tstring("trigger_start_physics_simulation %d \n ",       e->trigger->start_physics_simulation));
            builder_append(&builder, tstring("trigger_track_enemies %d \n ",                  e->trigger->track_enemies));
            builder_append(&builder, tstring("trigger_draw_lines_to_tracked %d \n ",          e->trigger->draw_lines_to_tracked));
            builder_append(&builder, tstring("trigger_agro_enemies %d \n ",                   e->trigger->agro_enemies));
            builder_append(&builder, tstring("trigger_player_touch %d \n ",                   e->trigger->player_touch));
            builder_append(&builder, tstring("trigger_shows_entities %d \n ",                 e->trigger->shows_entities));
            builder_append(&builder, tstring("trigger_starts_moving_sequence %d \n ",         e->trigger->starts_moving_sequence));
            builder_append(&builder, tstring("trigger_lock_camera %d \n ",                    e->trigger->lock_camera));
            builder_append(&builder, tstring("trigger_unlock_camera %d \n ",                  e->trigger->unlock_camera));
            builder_append(&builder, tstring("trigger_allow_player_shoot %d \n ",               e->trigger->allow_player_shoot));
            builder_append(&builder, tstring("trigger_forbid_player_shoot %d \n ",               e->trigger->forbid_player_shoot));
            builder_append(&builder, tstring("trigger_locked_camera_position { %f,  %f} \n ", e->trigger->locked_camera_position.x, e->trigger->locked_camera_position.y));
            
            builder_append(&builder, tstring("trigger_load_level %d \n ", e->trigger->load_level));
            if (e->trigger->load_level) {
                builder_append(&builder, tstring("trigger_level_name %s \n ", e->trigger->level_name));
            }
            
            builder_append(&builder, tstring("trigger_play_replay %d \n ", e->trigger->play_replay));
            if (e->trigger->play_replay) {
                builder_append(&builder, tstring("trigger_replay_name %s \n ", e->trigger->replay_name));
            }
            
            builder_append(&builder, tstring("trigger_change_zoom %d \n ", e->trigger->change_zoom));
            if (e->trigger->change_zoom) {
                builder_append(&builder, tstring("trigger_zoom_value %f \n ", e->trigger->zoom_value));
            }
            
            builder_append(&builder, tstring("trigger_play_sound %d \n ", e->trigger->play_sound));
            if (e->trigger->play_sound) {
                builder_append(&builder, tstring("trigger_sound_name %s \n ", e->trigger->sound_name));
            }
            
            builder_append(&builder, tstring("trigger_start_cam_rails_horizontal %d \n ", e->trigger->start_cam_rails_horizontal));
            builder_append(&builder, tstring("trigger_start_cam_rails_vertical %d \n ", e->trigger->start_cam_rails_vertical));
            builder_append(&builder, tstring("trigger_stop_cam_rails %d \n ", e->trigger->stop_cam_rails));
            if (e->trigger->cam_rails_points.count > 0) {
                builder_append(&builder, tstring("trigger_cam_rails_points [  "));
                for (i32 v = 0; v < e->trigger->cam_rails_points.count; v++) {
                    builder_append(&builder, tstring("{ %f,  %f} ", e->trigger->cam_rails_points.get_value(v).x, e->trigger->cam_rails_points.get_value(v).y)); 
                }
                builder_append(&builder, tstring("] \n ")); 
            }
        }
        
        if (e->flags & KILL_SWITCH) {
            if (e->kill_switch->connected.count > 0) {
                builder_append(&builder, tstring("kill_switch_connected [  "));
                for (i32 v = 0; v < e->kill_switch->connected.count; v++) {
                    builder_append(&builder, tstring(" %d ", e->kill_switch->connected.get_value(v))); 
                }
                builder_append(&builder, tstring("] \n ")); 
            }
        }
        
        if (e->flags & MOVE_SEQUENCE) {
            if (e->move_sequence->points.count > 0) {
                builder_append(&builder, tstring("move_sequence_points [  "));
                for (i32 v = 0; v < e->move_sequence->points.count; v++) {
                    builder_append(&builder, tstring("{ %f,  %f} ", e->move_sequence->points.get_value(v).x, e->move_sequence->points.get_value(v).y)); 
                }
                builder_append(&builder, tstring("] \n ")); 
            }
            
            builder_append(&builder, tstring("move_sequence_moving %d \n ",                        e->move_sequence->moving));
            builder_append(&builder, tstring("move_sequence_speed %f \n ",                         e->move_sequence->speed));
            builder_append(&builder, tstring("move_sequence_loop %d \n ",                          e->move_sequence->loop));
            builder_append(&builder, tstring("move_sequence_rotate %d \n ",                        e->move_sequence->rotate));
            builder_append(&builder, tstring("move_sequence_speed_related_player_distance %d \n ", e->move_sequence->speed_related_player_distance));
            builder_append(&builder, tstring("move_sequence_min_distance %f \n ",                  e->move_sequence->min_distance));
            builder_append(&builder, tstring("move_sequence_max_distance %f \n ",                  e->move_sequence->max_distance));
            builder_append(&builder, tstring("move_sequence_max_distance_speed %f \n ",            e->move_sequence->max_distance_speed));
        }
        
        if (e->flags & CENTIPEDE) {
            builder_append(&builder, tstring("spikes_on_right %d \n ", e->centipede->spikes_on_right));
            builder_append(&builder, tstring("spikes_on_left %d \n ", e->centipede->spikes_on_left));
            builder_append(&builder, tstring("segments_count %d \n ", e->centipede->segments_count));
        }
        
        if (e->flags & JUMP_SHOOTER) {
            builder_append(&builder, tstring("jump_shooter_shots_count %d \n ", e->jump_shooter->shots_count));
            builder_append(&builder, tstring("jump_shooter_spread %f \n ", e->jump_shooter->spread));
            builder_append(&builder, tstring("jump_shooter_explosive_count %d \n ", e->jump_shooter->explosive_count));
            builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers %d \n ", e->jump_shooter->shoot_sword_blockers));
            // builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers_clockwise %d \n ", e->jump_shooter->shoot_sword_blockers_clockwise));
            // builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers_random_direction %d \n ", e->jump_shooter->shoot_sword_blockers_random_direction));
            builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers_immortal %d \n ", e->jump_shooter->shoot_sword_blockers_immortal));
            builder_append(&builder, tstring("jump_shooter_shoot_bullet_blockers %d \n ", e->jump_shooter->shoot_bullet_blockers));
        }
        
        if (e->flags & TURRET) {
            Turret *turret = e->turret;
            builder_append(&builder, tstring("turret_projectile_flags %llu \n ", turret->projectile_settings.enemy_flags));
            builder_append(&builder, tstring("turret_shoot_sword_blocker_clockwise %d \n ", turret->projectile_settings.blocker_clockwise));
            builder_append(&builder, tstring("turret_activated %d \n ", turret->activated));
            builder_append(&builder, tstring("turret_homing_projectiles %d \n ", turret->homing));
            builder_append(&builder, tstring("turret_shoot_every_tick %d \n ", turret->shoot_every_tick));
            builder_append(&builder, tstring("turret_start_tick_delay %d \n ", turret->start_tick_delay));
            builder_append(&builder, tstring("turret_projectile_speed %f \n ", turret->projectile_settings.launch_speed));
            builder_append(&builder, tstring("turret_projectile_max_lifetime %f \n ", turret->projectile_settings.max_lifetime));
            builder_append(&builder, tstring("turret_shoot_width %f \n ", turret->shoot_width));
            builder_append(&builder, tstring("turret_shoot_height %f \n ", turret->shoot_height));
        }
        
        if (e->flags & DOOR) {
            builder_append(&builder, tstring("door_open %d \n ", e->door.is_open));
        }
        
        if (e->flags & BLOCKER) {
            builder_append(&builder, tstring("blocker_clockwise %d \n ", e->union_enemy->blocker_clockwise));
            builder_append(&builder, tstring("blocker_immortal %d \n ", e->union_enemy->blocker_immortal));
        }
        
        if (e->flags & SWORD_SIZE_REQUIRED) {
            builder_append(&builder, tstring("enemy_big_or_small_killable %d \n ", e->union_enemy->big_sword_killable));
        }
        
        if (e->flags & ENEMY && e->union_enemy->sword_kill_speed_modifier != 1) {
            builder_append(&builder, tstring("sword_kill_speed_modifier %.1f \n ", e->union_enemy->sword_kill_speed_modifier));
        }
        
        if (e->flags & ENEMY) {
            builder_append(&builder, tstring("enemy_gives_ammo %d \n ", e->union_enemy->gives_ammo));
        }
        
        if (e->flags & EXPLOSIVE) {
            builder_append(&builder, tstring("explosive_radius_multiplier %fs \n ", e->union_enemy->explosive_radius_multiplier));
        }
        
        if (e->flags & PROPELLER) {
            builder_append(&builder, tstring("propeller_power %f \n ", e->propeller->power));
            builder_append(&builder, tstring("propeller_spin_sensitive %d \n ", e->propeller->spin_sensitive));
        }
        
        if (e->flags & SHOOT_BLOCKER) {
            builder_append(&builder, tstring("shoot_blocker_direction { %f,  %f} \n ", e->union_enemy->shoot_blocker_direction.x, e->union_enemy->shoot_blocker_direction.y));
            builder_append(&builder, tstring("shoot_blocker_immortal %d \n ", e->union_enemy->shoot_blocker_immortal));
        }
        
        if (e->flags & TEXTURE) {
            builder_append(&builder, tstring("texture_name %s \n ", e->texture_name));
        }
        
        if (e->flags & NOTE) {
            assert(e->note_index != -1);
            builder_append(&builder, tstring("note_content \" %s \" \n ", current_level_context->notes.get(e->note_index)->content));
            builder_append(&builder, tstring("note_draw_in_game %d \n ", current_level_context->notes.get(e->note_index)->draw_in_game));
        }
        
        write_entire_file(tstring("%s/Entity_%s_%d.txt", c_str(level_directory_name), c_str(*e->name), e->id), &builder);
    }
    
    delete_directory(old_directory_name);
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

void parse_lightmaps(Array <Lightmap_Data> *lightmaps, Array <String> *splitted, i32 start_index) {
    i32 end_index = splitted->find_from(tstring("\n"), start_index);
    if (end_index <= start_index) {
        printf("End index was somehow less than a start index. That could mean that there was no breakline character at the end.\n");
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
            lightmap.lightmap_texture = LoadTexture(c_str(lightmap_name(current_level_context->lightmaps.count)));
            lightmap.has_loaded_texture = true;
        }
        
        lightmaps->append(lightmap);
        
        i--; // So we won't skip next lightmap_position.
    }
}

void parse_i32_array(Array <i32> *array, Array <String> *splitted, i32 start_index) {
    i32 end_index = splitted->find_from(tstring("\n"), start_index); 
    if (end_index <= start_index) {
        printf("Could not find breakline in parse i32 array!\n");
        return;
    }
    
    for (i32 i = start_index; i < end_index; i++) {
        i32 number = to_i32(splitted->get_value(i));        
        array->append(number);        
    }
}

void parse_vector2_array(Array <Vector2> *array, Array <String> *splitted, i32 start_index) {
    i32 end_index = splitted->find_from(tstring("\n"), start_index); 
    if (end_index <= start_index) {
        printf("Could not find breakline in parse vector2 array!\n");
        return;
    }
    
    for (i32 i = start_index; i < end_index; i++) {
        Vector2 v = parse_vector2(splitted, i);
        i += 1;
        array->append(v);        
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

b32 new_load_level(String name) {
    clear_allocator(temp);
    
    Game_State original_game_state = game_state;
    game_state = EDITOR; // @TODO: Do we really need this?
    
    session_context.playing_replay = false;
    String level_path = tstring("levels/%s", c_str(name));
    
    if (!directory_exists(level_path)) {
        builder_append(&console.content_builder, tstring("Level does not exists!: %s\n", name));
        return false;
    }
    
    clean_up_scene();
    switch_current_level_context(&loaded_level_context, true);
    clear_level_context(&loaded_level_context);
    
    if (!(current_level_context->level_name == name)) {
        session_context.previous_level_name = copy_string(current_level_context->level_name, &current_level_context->memory_arena);
    }
    
    current_level_context->level_name = copy_string(name, &current_level_context->memory_arena);
    
    setup_particles();
    
    Array <String> splitted = {.allocator = temp};
    Array <Entity> loaded_entities = {.allocator = temp};
    
    Array <String> level_files = get_files_in_directory(level_path, temp);
    if (level_files.count == 0) {
        printf("Level directory was empty!\n");
        return false;
    }
    
    String separators = tstring(":{}[], ;");
    
    String level_info_file_name = tstring("Level_Info.txt");
    
    i32 level_info_file_index =  find_file_name_in_paths(&level_files, level_info_file_name);
    if (level_info_file_index < 0) {
        printf("Failed to find level info file!\n");
    } else {
        b32 success = false;
        String level_info = read_entire_file(level_files.get_value(level_info_file_index), &success, temp);
        if (!success) {
            printf("Failed to read level info file!\n");
            return false;
        }
        
        split_string(&splitted, level_info, separators);
        
        i32 spawn_point_index = splitted.find(tstring("player_spawn_point"));
        if (spawn_point_index >= 0) current_level_context->player_spawn_point = parse_vector2(&splitted, spawn_point_index + 1);
        
        i32 lightmaps_index = splitted.find(tstring("lightmaps"));
        if (lightmaps_index > 0) {
            parse_lightmaps(&current_level_context->lightmaps, &splitted, lightmaps_index + 1);
        }
    }
    
    for_array(i, &level_files) {
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
                printf("Failed to read entity file data!\n");
                continue;
            }
            
            split_string(&splitted, entity_info, separators);
            Entity *entity = NULL;
            
            Entity dummy_entity = {0}; // It's here just for setting data for initing.
            i32 i = -1;
            
            IF_FIND("flags") dummy_entity.flags = to_u64(splitted.get_value(i+1));
            
            entity = copy_and_add_entity(&dummy_entity, &loaded_level_context);
            
            if (entity->flags & LIGHT) {        
                // We're adding empty ligh just because on loading we're gonna fill this empty light.
                Light dummy_light = {0};
                copy_and_add_light_to_entity(entity, &dummy_light, true);
            }
            
            i32 old_id = 0;
            Note note_to_fill = {};
            
            // Entity loading.
            
            IF_FIND("id") old_id = to_i32(splitted.get_value(i+1));
            
            IF_FIND("position") entity->position = parse_vector2(&splitted, i+1);
            IF_FIND("scale") entity->scale = parse_vector2(&splitted, i+1);
            IF_FIND("pivot") entity->pivot = parse_vector2(&splitted, i+1);
            
            IF_FIND("rotation") entity->rotation = to_f32(splitted.get_value(i+1));
            
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
                IF_FIND("trigger_connected")                  parse_i32_array(&trigger->connected, &splitted, i+1);
                IF_FIND("trigger_tracking")                   parse_i32_array(&trigger->tracking, &splitted, i+1);
                IF_FIND("trigger_cam_rails_points")           parse_vector2_array(&trigger->cam_rails_points, &splitted, i+1);
                IF_FIND("trigger_locked_camera_position")     trigger->locked_camera_position     = parse_vector2(&splitted, i+1);
                IF_FIND("trigger_die_after_trigger")          trigger->die_after_trigger          = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_kill_player")                trigger->kill_player                = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_kill_enemies")               trigger->kill_enemies               = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_open_doors")                 trigger->open_doors                 = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_start_physics_simulation")   trigger->start_physics_simulation   = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_track_enemies")              trigger->track_enemies              = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_draw_lines_to_tracked")      trigger->draw_lines_to_tracked      = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_agro_enemies")               trigger->agro_enemies               = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_player_touch")               trigger->player_touch               = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_start_cam_rails_horizontal") trigger->start_cam_rails_horizontal = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_start_cam_rails_vertical")   trigger->start_cam_rails_vertical   = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_stop_cam_rails")             trigger->stop_cam_rails             = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_lock_camera")                trigger->lock_camera                = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_unlock_camera")              trigger->unlock_camera              = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_allow_player_shoot")         trigger->allow_player_shoot         = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_forbid_player_shoot")        trigger->forbid_player_shoot        = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_load_level")                 trigger->load_level                 = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_play_replay")                trigger->play_replay                = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_play_sound")                 trigger->play_sound                 = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_change_zoom")                trigger->change_zoom                = to_f32(splitted.get_value(i+1));
                IF_FIND("trigger_zoom_value")                 trigger->zoom_value                 = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_shows_entities")             trigger->shows_entities             = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_starts_moving_sequence")     trigger->starts_moving_sequence     = to_i32(splitted.get_value(i+1));
                IF_FIND("trigger_level_name")                 str_copy(trigger->level_name, c_str(splitted.get_value(i+1)));
                IF_FIND("trigger_sound_name")                 str_copy(trigger->sound_name, c_str(splitted.get_value(i+1)));
                IF_FIND("trigger_replay_name")                str_copy(trigger->replay_name, c_str(splitted.get_value(i+1)));
            }
            
            // Kill switch loading.
            if (entity->flags & KILL_SWITCH) {
                assert(entity->kill_switch);
                IF_FIND("kill_switch_connected") parse_i32_array(&entity->kill_switch->connected, &splitted, i+1);
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
                IF_FIND("move_sequence_points")                        parse_vector2_array(&sequence->points, &splitted, i+1);
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
                IF_FIND("segments_count")  centipede->segments_count = to_i32(splitted.get_value(i+1));
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
                Light *light = current_level_context->lights.get(entity->lights.get_value(0));
                
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
                
                Note *note = current_level_context->notes.get(entity->note_index);
                str_copy(note->content, c_str(note_string));
                
                IF_FIND("note_draw_in_game") note->draw_in_game = to_i32(splitted.get_value(i+1));
            }
            
            // Of course we want to init entity again even if we've done it with flags at the beginning, because some initing can 
            // depend on exact data that we filled on loading.
            init_entity(entity);
            
            Entity *loaded = loaded_entities.append(*entity);
            loaded->id = old_id;
        } // End of a entity file scope.
    } // End of a files for loop.
    
    // This code needs for loading because entity ids in level save file will not be the same 
    // as entity ids after loading.
    // That's because we're storing our entities in chunk array and entity id is actually just (index + 1)
    // of that chunk array. In chunk array removing object and growing is not moving pointers, 
    // so after work on level we could have a large gaps of empty slots (for example we have chunk size
    // of 128 and we're just adding entities and last entity id is 555. Then we're deleting all 
    // entities from 100 to 554 id and we have a huge huge gap, but last entity still saves with id 
    // of 555 and some entities is referring to that entity).
    //
    // Then on loading we don't want to generate that huge gap again - we actually just want to store 
    // them linearly (even though in chunks). That means we're going to change id of entities.
    // 
    // We're going through loaded_entities, which is just flat copies of actual before-created entities, but with original 
    // entity ids.
    // Note that because we're just before added all loaded entities to level context there's no gaps and indexes of 
    // loaded_entities and actual chunk array of entities will be the same.
    for_array(i, &loaded_entities) {
        Entity *loaded = loaded_entities.get(i);
        Entity *new_entity = get_entity(i + 1);
        
        if (new_entity->flags & TRIGGER) {
            for_array(j, &loaded_entities) {
                // i == j would mean that we're looking at the same entity.
                if (i == j) continue;
                              
                Entity *another_loaded = loaded_entities.get(j);
                // Going through all loaded entities and looking if this entity old id is contained in connected.
                // If it is - we're replacing id in connected with this entity new id (and this entity new id is current index (j) + 1).
                i32 connected_index = new_entity->trigger->connected.find(another_loaded->id);
                if (connected_index >= 0) {
                    new_entity->trigger->connected.insert(j + 1, connected_index);
                }
                i32 tracking_index = new_entity->trigger->tracking.find(another_loaded->id);
                if (tracking_index >= 0) {
                    new_entity->trigger->tracking.insert(j + 1, tracking_index);
                }
            }
        }
        if (new_entity->flags & KILL_SWITCH) {
            for_array(j, &loaded_entities) {
                if (i == j) continue;
                
                Entity *another_loaded = loaded_entities.get(j);
                
                // Explainded earlier.
                i32 connected_index = new_entity->kill_switch->connected.find(another_loaded->id);
                if (connected_index >= 0) {
                    new_entity->kill_switch->connected.insert(j + 1, connected_index);
                }
            }
        }
    }
    
    game_state = original_game_state;
    
    setup_context_cam(current_level_context);
    current_level_context->cam.cam2D.zoom = 0.35f;
    
    // We do that so editor has latest level in it.
    // switch_current_level_context(editor_level-cont)e
    clear_level_context(&game_level_context);
    
    // This shit so that we don't overwrite level that we currently on.
    do {
        last_loaded_editor_level_context_index += 1;    
        last_loaded_editor_level_context_index %= MAX_LOADED_LEVELS;    
    } while (last_loaded_editor_level_context_index == current_editor_level_context_index);
    editor_level_context = &loaded_levels_contexts[last_loaded_editor_level_context_index];
    current_editor_level_context_index = last_loaded_editor_level_context_index;
    clear_level_context(editor_level_context);
    copy_level_context(editor_level_context, &loaded_level_context, true);
    
    if (enter_game_state_on_new_level || game_state == GAME || (0 && initing_game && RELEASE_BUILD)) {
        enter_game_state(&loaded_level_context, true);
        
        if (enter_game_state_on_new_level) {
            player_data->blood_amount = last_player_data.blood_amount;
            player_data->blood_progress = last_player_data.blood_progress;
            player_data->ammo_count = last_player_data.ammo_count;
        }
        
        enter_game_state_on_new_level = false;
    } else {
        enter_editor_state();
    }
    
    current_level_context->cam.position = current_level_context->player_spawn_point;
    current_level_context->cam.target = current_level_context->player_spawn_point;
    
    clear_allocator(temp);
    
    return true;
} // load level end.

