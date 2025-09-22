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
        String_Builder level_info_builder = make_string_builder(256, &temp_allocator);
        
        String spawn_point = tstring("player_spawn_point {%f, %f}\n", current_level_context->player_spawn_point.x, current_level_context->player_spawn_point.y);
        builder_append(&level_info_builder, spawn_point);
        
        String_Builder lightmaps_builder = make_string_builder(128, &temp_allocator);
        builder_append(&lightmaps_builder, tstring("lightmaps [ "));
        for (i32 i = 0; i < current_level_context->lightmaps.count; i++) {
            Lightmap_Data* l = current_level_context->lightmaps.get(i);
            builder_append(&lightmaps_builder, tstring("{pos {%f, %f}, size {%f, %f}} ", l->position.x, l->position.y, l->game_size.x, l->game_size.y));
        }
        builder_append(&lightmaps_builder, tstring("];\n")); 
        
        builder_append(&level_info_builder, make_string_from_builder(&lightmaps_builder, &temp_allocator));
        
        write_entire_file(tstring("%s/Level_Info.txt", c_str(level_directory_name)), &level_info_builder);
    }
    
    // Now saving all entities. Each entity in separate file.
    for_chunk_array(entity_index, (&current_level_context->entities)) {
        Entity *e = current_level_context->entities.get(entity_index);
        
        if (!e->need_to_save) {
            continue;
        }
        
        String_Builder builder = make_string_builder(256, &temp_allocator);
        
        Color color = e->color_changer.start_color;
        builder_append(&builder, tstring("name %s\nid %d\nposition { %f,  %f}\nscale { %f,  %f}\npivot { %f,  %f}\nrotation %f\ncolor { %d,  %d,  %d,  %d}\nflags %llu\ndraw_order %d\n", temp_entity_name(e).data, e->id, e->position.x, e->position.y, e->scale.x, e->scale.y, e->pivot.x, e->pivot.y, e->rotation, (i32)color.r, (i32)color.g, (i32)color.b, (i32)color.a, e->flags, e->draw_order));
        
        builder_append(&builder, tstring("vertices [ "));
        for (i32 v = 0; v < e->vertices.count; v++) {
            builder_append(&builder, tstring("{ %f,  %f} ", e->vertices.get_value(v).x, e->vertices.get_value(v).y)); 
        }
        builder_append(&builder, tstring("] \n")); 
        
        builder_append(&builder, tstring("unscaled_vertices [ "));
        for (i32 v = 0; v < e->unscaled_vertices.count; v++) {
            builder_append(&builder, tstring("{ %f,  %f} ", e->unscaled_vertices.get_value(v).x, e->unscaled_vertices.get_value(v).y)); 
        }
        builder_append(&builder, tstring("] \n")); 
        
        builder_append(&builder, tstring("hidden %d \n", e->hidden));
        builder_append(&builder, tstring("spawn_enemy_when_no_ammo %d \n", e->spawn_enemy_when_no_ammo));
        
        if (e->lights.count > 0) {
            Light *light = current_level_context->lights.get(e->lights.get_value(0));
            builder_append(&builder, tstring("light_shadows_size_flag %d \n",     light->shadows_size_flags));
            builder_append(&builder, tstring("light_backshadows_size_flag %d \n", light->backshadows_size_flags));
            builder_append(&builder, tstring("light_make_shadows %d \n",          light->make_shadows));
            builder_append(&builder, tstring("light_make_backshadows %d \n",      light->make_backshadows));
            builder_append(&builder, tstring("light_bake_shadows %d \n",          light->bake_shadows));
            builder_append(&builder, tstring("light_radius %f \n",                light->radius));
            builder_append(&builder, tstring("light_opacity %f \n",               light->opacity));
            builder_append(&builder, tstring("light_power %f \n",                 light->power));
            builder_append(&builder, tstring("light_color { %d,  %d,  %d,  %d} \n", (i32)light->color.r, (i32)light->color.g, (i32)light->color.b, (i32)light->color.a));
        }
        
        if (e->flags & TRIGGER) {
            assert(e->trigger);
            if (e->trigger->connected.count > 0) {
                builder_append(&builder, tstring("trigger_connected [ "));
                for (i32 v = 0; v < e->trigger->connected.count; v++) {
                    builder_append(&builder, tstring(" %d ", e->trigger->connected.get_value(v))); 
                }
                builder_append(&builder, tstring("] \n")); 
            }
            
            if (e->trigger->tracking.count > 0) {
                builder_append(&builder, tstring("trigger_tracking [ "));
                for (i32 v = 0; v < e->trigger->tracking.count; v++) {
                    builder_append(&builder, tstring(" %d ", e->trigger->tracking.get_value(v))); 
                }
                builder_append(&builder, tstring("] \n")); 
            }
            
            builder_append(&builder, tstring("trigger_kill_player %d \n",                    e->trigger->kill_player));
            builder_append(&builder, tstring("trigger_die_after_trigger %d \n",              e->trigger->die_after_trigger));
            builder_append(&builder, tstring("trigger_kill_enemies %d \n",                   e->trigger->kill_enemies));
            builder_append(&builder, tstring("trigger_open_doors %d \n",                     e->trigger->open_doors));
            builder_append(&builder, tstring("trigger_start_physics_simulation %d \n",       e->trigger->start_physics_simulation));
            builder_append(&builder, tstring("trigger_track_enemies %d \n",                  e->trigger->track_enemies));
            builder_append(&builder, tstring("trigger_draw_lines_to_tracked %d \n",          e->trigger->draw_lines_to_tracked));
            builder_append(&builder, tstring("trigger_agro_enemies %d \n",                   e->trigger->agro_enemies));
            builder_append(&builder, tstring("trigger_player_touch %d \n",                   e->trigger->player_touch));
            builder_append(&builder, tstring("trigger_shows_entities %d \n",                 e->trigger->shows_entities));
            builder_append(&builder, tstring("trigger_starts_moving_sequence %d \n",         e->trigger->starts_moving_sequence));
            builder_append(&builder, tstring("trigger_lock_camera %d \n",                    e->trigger->lock_camera));
            builder_append(&builder, tstring("trigger_unlock_camera %d \n",                  e->trigger->unlock_camera));
            builder_append(&builder, tstring("trigger_allow_player_shoot %d \n",               e->trigger->allow_player_shoot));
            builder_append(&builder, tstring("trigger_forbid_player_shoot %d \n",               e->trigger->forbid_player_shoot));
            builder_append(&builder, tstring("trigger_locked_camera_position { %f,  %f} \n", e->trigger->locked_camera_position.x, e->trigger->locked_camera_position.y));
            
            builder_append(&builder, tstring("trigger_load_level %d \n", e->trigger->load_level));
            if (e->trigger->load_level) {
                builder_append(&builder, tstring("trigger_level_name %s \n", e->trigger->level_name));
            }
            
            builder_append(&builder, tstring("trigger_play_replay %d \n", e->trigger->play_replay));
            if (e->trigger->play_replay) {
                builder_append(&builder, tstring("trigger_replay_name %s \n", e->trigger->replay_name));
            }
            
            builder_append(&builder, tstring("trigger_change_zoom %d \n", e->trigger->change_zoom));
            if (e->trigger->change_zoom) {
                builder_append(&builder, tstring("trigger_zoom_value %f \n", e->trigger->zoom_value));
            }
            
            builder_append(&builder, tstring("trigger_play_sound %d \n", e->trigger->play_sound));
            if (e->trigger->play_sound) {
                builder_append(&builder, tstring("trigger_sound_name %s \n", e->trigger->sound_name));
            }
            
            builder_append(&builder, tstring("trigger_start_cam_rails_horizontal %d \n", e->trigger->start_cam_rails_horizontal));
            builder_append(&builder, tstring("trigger_start_cam_rails_vertical %d \n", e->trigger->start_cam_rails_vertical));
            builder_append(&builder, tstring("trigger_stop_cam_rails %d \n", e->trigger->stop_cam_rails));
            if (e->trigger->cam_rails_points.count > 0) {
                builder_append(&builder, tstring("trigger_cam_rails_points [ "));
                for (i32 v = 0; v < e->trigger->cam_rails_points.count; v++) {
                    builder_append(&builder, tstring("{ %f,  %f} ", e->trigger->cam_rails_points.get_value(v).x, e->trigger->cam_rails_points.get_value(v).y)); 
                }
                builder_append(&builder, tstring("] \n")); 
            }
        }
        
        if (e->flags & KILL_SWITCH) {
            if (e->kill_switch->connected.count > 0) {
                builder_append(&builder, tstring("kill_switch_connected [ "));
                for (i32 v = 0; v < e->kill_switch->connected.count; v++) {
                    builder_append(&builder, tstring(" %d ", e->kill_switch->connected.get_value(v))); 
                }
                builder_append(&builder, tstring("] \n")); 
            }
        }
        
        if (e->flags & MOVE_SEQUENCE) {
            if (e->move_sequence->points.count > 0) {
                builder_append(&builder, tstring("move_sequence_points [ "));
                for (i32 v = 0; v < e->move_sequence->points.count; v++) {
                    builder_append(&builder, tstring("{ %f,  %f} ", e->move_sequence->points.get_value(v).x, e->move_sequence->points.get_value(v).y)); 
                }
                builder_append(&builder, tstring("] \n")); 
            }
            
            builder_append(&builder, tstring("move_sequence_moving %d \n",                        e->move_sequence->moving));
            builder_append(&builder, tstring("move_sequence_speed %f \n",                         e->move_sequence->speed));
            builder_append(&builder, tstring("move_sequence_loop %d \n",                          e->move_sequence->loop));
            builder_append(&builder, tstring("move_sequence_rotate %d \n",                        e->move_sequence->rotate));
            builder_append(&builder, tstring("move_sequence_speed_related_player_distance %d \n", e->move_sequence->speed_related_player_distance));
            builder_append(&builder, tstring("move_sequence_min_distance %f \n",                  e->move_sequence->min_distance));
            builder_append(&builder, tstring("move_sequence_max_distance %f \n",                  e->move_sequence->max_distance));
            builder_append(&builder, tstring("move_sequence_max_distance_speed %f \n",            e->move_sequence->max_distance_speed));
        }
        
        if (e->flags & CENTIPEDE) {
            builder_append(&builder, tstring("spikes_on_right %d \n", e->centipede->spikes_on_right));
            builder_append(&builder, tstring("spikes_on_left %d \n", e->centipede->spikes_on_left));
            builder_append(&builder, tstring("segments_count %d \n", e->centipede->segments_count));
        }
        
        if (e->flags & JUMP_SHOOTER) {
            builder_append(&builder, tstring("jump_shooter_shots_count %d \n", e->jump_shooter->shots_count));
            builder_append(&builder, tstring("jump_shooter_spread %f \n", e->jump_shooter->spread));
            builder_append(&builder, tstring("jump_shooter_explosive_count %d \n", e->jump_shooter->explosive_count));
            builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers %d \n", e->jump_shooter->shoot_sword_blockers));
            // builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers_clockwise %d \n", e->jump_shooter->shoot_sword_blockers_clockwise));
            // builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers_random_direction %d \n", e->jump_shooter->shoot_sword_blockers_random_direction));
            builder_append(&builder, tstring("jump_shooter_shoot_sword_blockers_immortal %d \n", e->jump_shooter->shoot_sword_blockers_immortal));
            builder_append(&builder, tstring("jump_shooter_shoot_bullet_blockers %d \n", e->jump_shooter->shoot_bullet_blockers));
        }
        
        if (e->flags & TURRET) {
            Turret *turret = e->turret;
            builder_append(&builder, tstring("turret_projectile_flags %llu \n", turret->projectile_settings.enemy_flags));
            builder_append(&builder, tstring("turret_shoot_sword_blocker_clockwise %d \n", turret->projectile_settings.blocker_clockwise));
            builder_append(&builder, tstring("turret_activated %d \n", turret->activated));
            builder_append(&builder, tstring("turret_homing_projectiles %d \n", turret->homing));
            builder_append(&builder, tstring("turret_shoot_every_tick %d \n", turret->shoot_every_tick));
            builder_append(&builder, tstring("turret_start_tick_delay %d \n", turret->start_tick_delay));
            builder_append(&builder, tstring("turret_projectile_speed %f \n", turret->projectile_settings.launch_speed));
            builder_append(&builder, tstring("turret_projectile_max_lifetime %f \n", turret->projectile_settings.max_lifetime));
            builder_append(&builder, tstring("turret_shoot_width %f \n", turret->shoot_width));
            builder_append(&builder, tstring("turret_shoot_height %f \n", turret->shoot_height));
        }
        
        if (e->flags & DOOR) {
            builder_append(&builder, tstring("door_open %d \n", e->door.is_open));
        }
        
        if (e->flags & BLOCKER) {
            builder_append(&builder, tstring("blocker_clockwise %d \n", e->union_enemy->blocker_clockwise));
            builder_append(&builder, tstring("blocker_immortal %d \n", e->union_enemy->blocker_immortal));
        }
        
        if (e->flags & SWORD_SIZE_REQUIRED) {
            builder_append(&builder, tstring("enemy_big_or_small_killable %d \n", e->union_enemy->big_sword_killable));
        }
        
        if (e->flags & ENEMY && e->union_enemy->sword_kill_speed_modifier != 1) {
            builder_append(&builder, tstring("sword_kill_speed_modifier %.1f \n", e->union_enemy->sword_kill_speed_modifier));
        }
        
        if (e->flags & ENEMY) {
            builder_append(&builder, tstring("enemy_gives_ammo %d \n", e->union_enemy->gives_ammo));
        }
        
        if (e->flags & EXPLOSIVE) {
            builder_append(&builder, tstring("explosive_radius_multiplier %fs \n", e->union_enemy->explosive_radius_multiplier));
        }
        
        if (e->flags & PROPELLER) {
            builder_append(&builder, tstring("propeller_power %f \n", e->propeller->power));
            builder_append(&builder, tstring("propeller_spin_sensitive %d \n", e->propeller->spin_sensitive));
        }
        
        if (e->flags & SHOOT_BLOCKER) {
            builder_append(&builder, tstring("shoot_blocker_direction { %f,  %f} \n", e->union_enemy->shoot_blocker_direction.x, e->union_enemy->shoot_blocker_direction.y));
            builder_append(&builder, tstring("shoot_blocker_immortal %d \n", e->union_enemy->shoot_blocker_immortal));
        }
        
        if (e->flags & TEXTURE) {
            builder_append(&builder, tstring("texture_name %s \n", e->texture_name));
        }
        
        if (e->flags & NOTE) {
            assert(e->note_index != -1);
            builder_append(&builder, tstring("note_content:\" %s \": \n", current_level_context->notes.get(e->note_index)->content));
            builder_append(&builder, tstring("note_draw_in_game %d \n", current_level_context->notes.get(e->note_index)->draw_in_game));
        }
        
        write_entire_file(tstring("%s/%s_%d.txt", c_str(level_directory_name), c_str(*e->name), e->id), &builder);
    }
    
    delete_directory(old_directory_name);
} // save level end.

Vector2 parse_vector2(Array <String> *splitted, i32 start_index) {
    if (start_index + 2 >= splitted->count) return {0};
    
    Vector2 v = {0};
    v.x = to_f32(splitted->get_value(start_index));
    v.y = to_f32(splitted->get_value(start_index + 1));
    
    return v;
}

b32 new_load_level(String name) {
    clear_allocator(&temp_allocator);
    
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
    
    Array <String> splitted = {.allocator = &temp_allocator};
    Array <Entity> loaded_entities = {.allocator = &temp_allocator};
    
    Array <String> level_files = get_files_in_directory(level_path, &temp_allocator);
    if (level_files.count == 0) {
        printf("Level directory was empty!\n");
        return false;
    }
    
    String separators = tstring(":{}[], ;");
    
    for_array(i, &level_files) {
        String file_name = level_files.get_value(i);    
        
        if (file_name == tstring("Level_Info.txt")) {
            b32 success = false;
            String level_info = read_entire_file(file_name, &success, &temp_allocator);
            if (!success) {
                printf("Failed to read level info file!\n");
                return false;
            }
            
            split_string(&splitted, level_info, separators);
            
            i32 spawn_point_index = splitted.find(tstring("player_spawn_point"));
            
        } else {
            // There goes entity parsing.
        }
    }
    
    return true;
} // load level end.

