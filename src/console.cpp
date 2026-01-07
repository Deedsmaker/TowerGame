#pragma once

void print_to_console(const char *text) {
    builder_append(&console.content_builder, tstring("\t>%s\n", text));
}

void try_load_next_level() {
    b32 success = maybe_load_next_level();
    if (!success) {
        print_to_console("This level is either last or not in level order file.");
    }
}

void try_load_previous_level() {
    if (global_data.old_previous_level_name[0]) {
        if (load_level(tstring(global_data.old_previous_level_name))) {
            print_to_console("Previous level loaded successfuly");
            // editor_enter_editor_state();
        } else {
            print_to_console("Previous level FAILED TO LOAD");
        }
    } else {
        print_to_console("No previous level saved in buffer");
    }
}

inline void save_current_level() {
    save_level(current_context->level_name);
}

inline void save_level_by_name(const char *name) {
    save_level(tstring(name));
}

void load_level_by_name(const char *name) {
    editor.last_autosave_time = core.time.app_time;
    load_level(tstring(name));
    
    // editor_enter_editor_state();
}

void reload_level() {
    load_level(current_context->level_name);       
}

Console_Command make_console_command(const char *name, void (func)() = NULL, void (func_arg)(const char*) = NULL) {
    Console_Command command;
    str_copy(command.name, name);
    command.func = func;
    command.func_arg = func_arg;
    return command;
}

inline void print_current_level() {
    print_to_console(c_str(current_context->level_name));
}

void print_create_level_hint() {
    print_to_console("Provide level name");
}

void reload_level_names() {
    for_array (i, &console.level_names) {
        console.level_names.get(i)->free_data();
    }
    console.level_names.clear();
    
    Array <String> levels = get_files_in_directory(S("levels"), temp);
    
    for_array (i, &levels) {
        String path = levels.get_value(i);
        if (!directory_exists(path)) continue;
        String name = strip_path_to_just_name(path, &default_allocator);
        if (name == S("autosaves") || name == S("temp")) {
            name.free_data();
            continue;
        }
        
        console.level_names.append(name, &default_allocator);
    }
}

void print_hotkeys_to_console() {
    print_to_console("Ctrl+LeftMouse - Multiselect");
    print_to_console("Ctrl+Shift+Mouse - Move multiselected");
    print_to_console("Ctrl+RightMouse - Exclude multiselect");
    print_to_console("Ctrl+Shift+Space - Toggle Game/Editor");
    print_to_console("Ctrl+S - Save current level.");
    print_to_console("Alt - See and move vertices with mouse.");
    print_to_console("Alt+V - While moving vertex for snap it to closest.");
    print_to_console("Alt+1-5 - Fast entities creation.");
    print_to_console("Space - Create menu");
    print_to_console("P - Move player spawn point");
    print_to_console("TAB - Pause in game");
    print_to_console("Ctrl+Shift+H - Freecam in game");
    print_to_console("Right_Ctrl+L - Unlock camera");
    print_to_console("K - Teleport player to mouse in game");
    print_to_console("WASD - Scaling selected entity. Hold Alt for big.");
    print_to_console("QE - Rotating selected entity. Hold Alt for big.");
    
    print_to_console("Commands:\n\t>debug - debug commands info");
    print_to_console("save <level> - save current level or specify level where to save");
    print_to_console("level <level> - get current level name or load level if provided");
    print_to_console("load <level> - load level");
    // print_to_console("create / new_level <level> - create empty level");
    print_to_console("next / previous / reload / restart_game / first");
    print_to_console("level_speedrun - Speedrun for levels");
    print_to_console("game_speedrun - Whole game speedrun. Death puts in game begining.");
    print_to_console("speedrun_disable");
}

void debug_unlock_camera() {
    state_context.cam_state.locked = false;
    state_context.cam_state.on_rails_horizontal = false;
    state_context.cam_state.on_rails_vertical   = false;
}

void print_debug_commands_to_console() {
    print_to_console("\t>Debug Functions:");
    print_to_console("infinite_ammo");
    print_to_console("die");
    print_to_console("enemy_ai");
    print_to_console("god_mode");
    print_to_console("add_ammo");
    print_to_console("unlock_camera");
    print_to_console("full_light");
    print_to_console("collision_grid");
    print_to_console("timescale <scale>");
    print_to_console("draw_triggers");
    
    print_to_console("play_replay - Plays current recorded game state replay. (Single level)");
    print_to_console("save_replay - Saves current replay to file");
    print_to_console("replay_load - Loads replay from file");
    
    print_to_console("\t>Debug Info:");
    print_to_console("player_speed");
    print_to_console("entities_count");
}

void debug_toggle_player_speed() {
    debug.info_player_speed = !debug.info_player_speed;
}

void debug_toggle_view_only_lightmaps() {
    debug.view_only_lightmaps = !debug.view_only_lightmaps;
}

void debug_print_entities_count() {
    i32 count = 0;
    ForEntities(entity, 0) {
        count++;
    }
    builder_append(&console.content_builder, tstring("\t>Entities count: %d\n", count));
}

void debug_infinite_ammo() {
    debug.infinite_ammo = !debug.infinite_ammo;
    builder_append(&console.content_builder, tstring("\t>Infinite ammo %s\n", debug.infinite_ammo ? "enabled" : "disabled"));
}

void debug_enemy_ai() {
    debug.enemy_ai = !debug.enemy_ai;
    builder_append(&console.content_builder, tstring("\t>Enemy ai %s\n", debug.enemy_ai ? "enabled" : "disabled"));
}

void debug_god_mode() {
    debug.god_mode = !debug.god_mode;
    builder_append(&console.content_builder, tstring("\t>God mode %s\n", debug.god_mode ? "enabled" : "disabled"));
}

void debug_toggle_full_light() {
    debug.full_light = !debug.full_light;
    builder_append(&console.content_builder, tstring("\t>Full light is %s\n", debug.full_light ? "enabled" : "disabled"));
}

void debug_toggle_lightmap_view() {
    if (debug.full_light) {
        debug.full_light = false;
        debug.view_only_lightmaps = true;
    } else if (debug.view_only_lightmaps) {
        debug.view_only_lightmaps = false;
    } else {
        debug.full_light = true;
    }
}

void debug_toggle_collision_grid() {
    debug.draw_collision_grid = !debug.draw_collision_grid;
    builder_append(&console.content_builder, tstring("\t>Collision grid is %s\n", debug.draw_collision_grid ? "enabled" : "disabled"));
}

void debug_set_default_time_scale() {
    core.time.debug_target_time_scale = 1;    
}

void debug_set_time_scale(const char *text) {
    core.time.debug_target_time_scale = to_f32(text);    
}

void debug_set_time_scale(f32 scale) {
    core.time.debug_target_time_scale = scale;    
}

void save_replay(const char *replay_name) {
    const char *name = tprintf("replays/%s.replay", get_substring_before_symbol(replay_name, '.'));
    
    FILE *fptr;
    fptr = fopen(name, "wb");
    
    size_t write_result = fwrite(level_replay.input_record.data, sizeof(Replay_Frame_Data), level_replay.input_record.count, fptr);
    
    if (write_result != -1) {
        builder_append(&console.content_builder, tstring("\t>Temp replay named %s is saved\n", name));
    }
}

void save_temp_replay() {
    save_replay(tprintf("TEMP_%s", get_substring_before_symbol(c_str(current_context->level_name), '.')));
}

void play_loaded_replay() {
    global_data.playing_replay = true;
    Entity *replay_player_entity = add_player_entity(current_context, &replay_player_data);
    // replay_player_entity->flags |= REPLAY_PLAYER;
}

void load_replay(const char *replay_name) {
    const char *name = tprintf("replays/%s.replay", get_substring_before_symbol(replay_name, '.'));
    FILE *fptr;
    fptr = fopen(name, "rb");

    if (!fptr) {
        print_to_console(tprintf("No such replay %s", name));
        return;
    }

    size_t read_result = fread(level_replay.input_record.data, sizeof(Replay_Frame_Data), level_replay.input_record.capacity, fptr);
    level_replay.input_record.count = read_result;    
    level_replay.start_frame = global_data.game_frame_count;
    
    if (read_result != -1) {
        // global_data.playing_replay = true;
        // editor_enter_editor_state();
        // enter_and_reload_game_state(current_context, true);
        play_loaded_replay();
    
        builder_append(&console.content_builder, tstring("\t>Replay named %s is loaded\n", name));
    }
}

void load_temp_replay() {
    load_replay(tprintf("TEMP_%s", get_substring_before_symbol(c_str(current_context->level_name), '.')));
}


void debug_toggle_play_replay() {
    global_data.playing_replay = !global_data.playing_replay;
        
    // if (global_data.playing_replay) {
    //     enter_and_reload_game_state(editor_context, true);
    //     play_loaded_replay();
    // }
    
    builder_append(&console.content_builder, tstring("\t>Replay mode is %s\n", global_data.playing_replay ? "enabled" : "disabled"));
}

void restart_game() {
    load_level(tstring(first_level_name));
    // editor_enter_editor_state();
    
    if (editor_state == EDITOR) {
        editor_enter_game_state(current_context);
    } else {
        enter_planning_state();
    }
    
    // player_data->ammo_count = 0;
    global_data.speedrun_timer.time = 0;        
}

void begin_level_speedrun() {
    if (!global_data.speedrun_timer.level_timer_active) {
        reload_level();
        // editor_enter_editor_state();
        // enter_and_reload_game_state(current_context, true);
        
        global_data.speedrun_timer.level_timer_active = true;        
        global_data.speedrun_timer.game_timer_active  = false;        
        global_data.speedrun_timer.time = 0;        
    } else {
        disable_speedrun();
    }
}

void disable_speedrun() {
    global_data.speedrun_timer.level_timer_active = false;
    global_data.speedrun_timer.game_timer_active = false;
    global_data.speedrun_timer.time = 0;
}

void begin_game_speedrun() {
    if (!global_data.speedrun_timer.game_timer_active) {
        restart_game();
        editor_enter_editor_state();
        // enter_and_reload_game_state(current_context, true);
        
        global_data.speedrun_timer.level_timer_active = false;        
        global_data.speedrun_timer.game_timer_active  = true;        
        global_data.speedrun_timer.time = 0;        
    } else {
        disable_speedrun();
    }
}

void debug_add_100_ammo() {
    if (current_context->player_entity) {
        add_player_ammo(100);
    }    
}

void debug_stop_game() {
    core.time.target_time_scale = 0;
    core.time.time_scale = 0;
    debug.drawing_stopped = true;
}

void debug_toggle_draw_triggers() {
    debug.draw_areas_in_game = !debug.draw_areas_in_game;
}

void editor_select_entity_by_id(const char *id_str) {
    Entity *entity_to_select = maybe_get_entity(to_i32(id_str));    
    if (entity_to_select) {
        assign_selected_entity(entity_to_select);
    } else {
        print_to_console("Did not find that id!");
    }
}

void editor_print_select_entity_by_id_hint() {
    print_to_console("Provide a id!");
}

void rename_current_level(const char *new_name_str) {
    String new_name = tstring(new_name_str);
    if (new_name.count < 2) {
        print_to_console("Level name too short. That could be a misslick. We're not renaming level.");
        return;
    }
    
    String old_name = editor_context->level_name;
    
    save_level(new_name);
    load_level(new_name);
    
    delete_directory(level_name_to_path(old_name, temp));
    
    reload_level_names();
}

void console_win_level() {
    win_level();
}

void console_clear_save_data() {
    clear_current_save_data(temp);
}

void init_console() {
    init_array(&console.args, 8, &default_allocator);
    init_array(&console.commands, 128, &default_allocator);

    reload_level_names();    

    console.commands.append(make_console_command("hotkeys", print_hotkeys_to_console), &default_allocator);
    console.commands.append(make_console_command("hotkey",  print_hotkeys_to_console), &default_allocator);
    console.commands.append(make_console_command("key",     print_hotkeys_to_console), &default_allocator);
    console.commands.append(make_console_command("keys",    print_hotkeys_to_console), &default_allocator);
    console.commands.append(make_console_command("help",    print_hotkeys_to_console), &default_allocator);
    
    console.commands.append(make_console_command("debug",          print_debug_commands_to_console), &default_allocator);
    console.commands.append(make_console_command("player_speed",   debug_toggle_player_speed), &default_allocator);
    console.commands.append(make_console_command("entities_count", debug_print_entities_count), &default_allocator);
    console.commands.append(make_console_command("infinite_ammo",  debug_infinite_ammo), &default_allocator);
    console.commands.append(make_console_command("die",  kill_player), &default_allocator);
    console.commands.append(make_console_command("enemy_ai",       debug_enemy_ai), &default_allocator);
    console.commands.append(make_console_command("god_mode",       debug_god_mode), &default_allocator);
    console.commands.append(make_console_command("add_ammo",       debug_add_100_ammo), &default_allocator);
    console.commands.append(make_console_command("unlock_camera",  debug_unlock_camera), &default_allocator);
    console.commands.append(make_console_command("full_light",     debug_toggle_full_light), &default_allocator);
    console.commands.append(make_console_command("collision_grid", debug_toggle_collision_grid), &default_allocator);
    console.commands.append(make_console_command("draw_triggers", debug_toggle_draw_triggers), &default_allocator);
    
    console.commands.append(make_console_command("find", editor_print_select_entity_by_id_hint, editor_select_entity_by_id), &default_allocator);
    
    console.commands.append(make_console_command("save",     save_current_level, save_level_by_name), &default_allocator);
    console.commands.append(make_console_command("load",     NULL, load_level_by_name), &default_allocator);
    console.commands.append(make_console_command("level",    print_current_level, load_level_by_name), &default_allocator);
    console.commands.append(make_console_command("l",    print_current_level, load_level_by_name), &default_allocator);
    console.commands.append(make_console_command("rename_current_level",    NULL, rename_current_level), &default_allocator);
    console.commands.append(make_console_command("next",     try_load_next_level, NULL), &default_allocator);
    console.commands.append(make_console_command("previous", try_load_previous_level, NULL), &default_allocator);
    console.commands.append(make_console_command("reload",   reload_level, NULL), &default_allocator);
    
    console.commands.append(make_console_command("save_lightmap", save_lightmaps_to_file, NULL), &default_allocator);
    
    console.commands.append(make_console_command("restart_game",      restart_game, NULL), &default_allocator);
    console.commands.append(make_console_command("first",             restart_game, NULL), &default_allocator);
    console.commands.append(make_console_command("level_speedrun",    begin_level_speedrun, NULL), &default_allocator);
    console.commands.append(make_console_command("game_speedrun",     begin_game_speedrun, NULL), &default_allocator);
    console.commands.append(make_console_command("speedrun_disable",  disable_speedrun, NULL), &default_allocator);
    
    console.commands.append(make_console_command("timescale", debug_set_default_time_scale, debug_set_time_scale), &default_allocator);
    
    console.commands.append(make_console_command("clear_save_data", console_clear_save_data, NULL), &default_allocator);
    console.commands.append(make_console_command("win", console_win_level, NULL), &default_allocator);
    
    // console.commands.append(make_console_command("create",    print_create_level_hint, create_level), &default_allocator);
    // console.commands.append(make_console_command("new_level", print_create_level_hint, create_level), &default_allocator);
    
    console.commands.append(make_console_command("play_replay", debug_toggle_play_replay, NULL), &default_allocator);
    console.commands.append(make_console_command("save_replay", save_temp_replay, save_replay), &default_allocator);
    console.commands.append(make_console_command("replay_load", load_temp_replay, load_replay), &default_allocator);
}

void close_console() {
    console.is_open = false;
    console.closed_time = core.time.app_time;
    
    if (str_equal(focus_input_field.tag, "console_input_field")) {
        focus_input_field.in_focus = false;
    }
}

void update_console() {
    b32 can_control_console = !editor.create_box_active;
    if (can_control_console && (IsKeyPressed(KEY_SLASH) || (console.is_open && IsKeyPressed(KEY_ESCAPE)))) {
        if (console.is_open) {
            close_console();
        } else {
            console.is_open = true;
            console.opened_time = core.time.app_time;
            make_next_input_field_in_focus("console");
        }
    }
            
    if (console.is_open && can_control_console) {
        f32 time_since_open = core.time.app_time - console.opened_time;
        console.open_progress = clamp01(time_since_open / 0.3f);
        
        Color color = lerp(WHITE * 0, GRAY, console.open_progress * console.open_progress);
        
        console.args.clear();
        // split_str(focus_input_field.content, " ", &console.args);
        split_string(&console.args, S(focus_input_field.content), S(" "), &default_allocator);
        
        b32 content_changed = false;
        for (i32 i = 0; i < console.commands.count && console.args.count == 1; i++) {
            b32 arg_matches_some_command = str_start_with(console.commands.get_value(i).name, console.args.get_value(0).data);
            
            if (arg_matches_some_command) {
                Old::make_ui_text(console.commands.get_value(i).name, {3.0f, (f32)screen_height * 0.5f + focus_input_field.font_size}, focus_input_field.font_size, color * 0.7f, "console_hint_text");
                
                if (IsKeyPressed(KEY_TAB) && console.args.count == 1) {
                    set_focus_input_field(console.commands.get_value(i).name);
                    content_changed = true;
                }
                break;
            }
        }
        
                
        local_persist i32 last_level_autocomplete_index = -1; // That's for going through all of levels with tab key.
        local_persist String first_level_autocomplete_name = {.allocator = &default_allocator};
        
        if (console.args.count == 2 && (console.args.get_value(0) == "level" || console.args.get_value(0) == "l" || console.args.get_value(0) == "load")) {
            Array <String> matching_level_names = {};
        
            if (last_level_autocomplete_index < 0) {
                first_level_autocomplete_name.free_data();
                first_level_autocomplete_name = copy_string(console.args.get_value(1), &default_allocator);
            }
        
            for_array (i, &console.level_names) {
                if (string_contains(console.level_names.get_value(i), first_level_autocomplete_name)) {
                    matching_level_names.append(console.level_names.get_value(i), temp);
                }
            }
            
            b32 autocompleted_level_name = false;
            for_array (i, &matching_level_names) {
                const char *new_console_content = tprintf("%s %s", c_str(console.args.get_value(0)), c_str(matching_level_names.get_value(i)));
                if (i == 0) {             
                    Old::make_ui_text(new_console_content, {3.0f, (f32)screen_height * 0.5f + focus_input_field.font_size}, focus_input_field.font_size, color * 0.7f, "console_hint_text");
                }
                
                if (IsKeyPressed(KEY_TAB) && (i > last_level_autocomplete_index || last_level_autocomplete_index >= matching_level_names.count - 1)) {
                    set_focus_input_field(new_console_content);
                    content_changed = true;
                    
                    last_level_autocomplete_index = i;
                    autocompleted_level_name = true;
                    break;
                }
            }
        }
        
        if (!content_changed) {
            if (IsKeyPressed(KEY_UP) && console.history_max > 0 && console.history.count > 0) {
                set_focus_input_field(console.history.get_value(console.history.count - 1).data);
                console.history.count--;
                content_changed = true;
            }
            if (IsKeyPressed(KEY_DOWN) && console.history_max >= console.history.count) {
                if (console.history_max == console.history.count) {
                    clear_focus_input_field();
                } else {
                    set_focus_input_field(console.history.get_value(console.history.count).data);
                    console.history.count++;
                }
                content_changed = true;
            }
        }
        
        if (make_input_field("", {0.0f, (f32)screen_height * 0.5f}, {(f32)screen_width, focus_input_field.font_size}, "console_input_field", color, false)) {
            builder_append(&console.content_builder, tstring("%s\n", focus_input_field.content));
            
            //if we used hint while input, for example
            if (content_changed) {
                console.args.clear();
                // split_str(focus_input_field.content, " ", &console.args);
                split_string(&console.args, S(focus_input_field.content), S(" "), &default_allocator);
            }
            
            b32 found_command = false;
            
            for (i32 i = 0; i < console.commands.count && console.args.count > 0; i++) {
                Console_Command command = console.commands.get_value(i);
                if (str_equal(command.name, c_str(console.args.get_value(0)))) {
                    if (command.func_arg && console.args.count > 1) {
                        command.func_arg(console.args.get_value(1).data);
                    } else if (command.func) {
                        command.func();   
                    }
                    found_command = true;
                    break;
                }
            }
            
            if (!found_command && console.args.count > 0) {
                print_to_console("Command was not found :D");
            }
            
            Medium_Str history_str;            
            str_copy(history_str.data, focus_input_field.content);
            console.history.append(history_str, &default_allocator);
            console.history_max = console.history.count;
            
            clear_focus_input_field();
        }
        
        if (focus_input_field.changed) {
            last_level_autocomplete_index = -1;
        }
    } // End of console open update.
}

void draw_console() {
    // Draw console.
    if (console.is_open) {
        Color text_color = lerp(GREEN * 0, GREEN, console.open_progress * console.open_progress);
        f32 y_position = lerp(-screen_height * 0.6f, 0.0f, EaseOutQuint(console.open_progress));
        
        draw_rect({0, y_position}, {(f32)screen_width, screen_height * 0.5f}, BLUE * 0.2f);
        draw_text_boxed(console.content_builder.data, {4, 4 + y_position, (f32)screen_width, screen_height * 0.5f - 30.0f}, 16, 3, text_color, false);
        draw_text(tprintf("App time: %.2f", core.time.app_time), {screen_width * 0.46f, 5.0f}, 14, ColorBrightness(lerp(LIME * 0, LIME, console.open_progress * console.open_progress), 0.5f));
        draw_text(tprintf("Game time: %.2f", current_context->game_time), {screen_width * 0.46f, 20.0f}, 14, ColorBrightness(lerp(LIME * 0, LIME, console.open_progress * console.open_progress), 0.5f));
    } else {
        f32 since_console_closed = core.time.app_time - console.closed_time;
        
        if (since_console_closed <= 0.4f) {
            f32 t = clamp01(since_console_closed / 0.4f);
            Color text_color = lerp(GREEN, GREEN * 0, t * t);
            f32 y_position = lerp(0.0f, -screen_height * 0.6f, EaseOutQuint(t));
            
            draw_rect({0, y_position}, {(f32)screen_width, screen_height * 0.5f}, BLUE * 0.2f);
            draw_text_boxed(console.content_builder.data, {4, 4 + y_position, (f32)screen_width, screen_height * 0.5f - 30.0f}, 16, 3, text_color);
        }
    }
}
