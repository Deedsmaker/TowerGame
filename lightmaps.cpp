#pragma once

// 2 is light version. 4 is high quality version.
f32 pixel_per_unit = 2;

const char* get_lightmap_name(i32 index){
    return tprintf("resources/lightmaps/%s_%d_lightmap.png", current_level_context->level_name, index);
}

Vector2 get_lightmap_pixel_size(Lightmap_Data* l){
    return {l->game_size.x * pixel_per_unit, l->game_size.y * pixel_per_unit};    
}

void save_lightmap_to_file(i32 index){
    Image lightmap_image = LoadImageFromTexture(current_level_context->lightmaps.get(index).global_illumination_rt.texture);
    ExportImage(lightmap_image, get_lightmap_name(index));
    UnloadImage(lightmap_image);
}

void save_lightmaps_to_file(){
    for (i32 i = 0; i < current_level_context->lightmaps.count; i++){
        save_lightmap_to_file(i);
    }
}

void load_lightmap_render_textures(i32 index){
    assert(index < current_level_context->lightmaps.count);
    Lightmap_Data* l = current_level_context->lightmaps.get_ptr(index);
    
    Vector2 pixel_size = get_lightmap_pixel_size(l);
    
    l->global_illumination_rt = LoadRenderTexture(pixel_size.x, pixel_size.y);
    l->emitters_occluders_rt  = LoadRenderTexture(pixel_size.x, pixel_size.y);
    l->distance_field_rt      = LoadRenderTexture(pixel_size.x, pixel_size.y);
    l->normal_rt              = LoadRenderTexture(pixel_size.x, pixel_size.y);
    l->voronoi_seed_rt        = LoadRenderTexture(pixel_size.x, pixel_size.y);
    l->jump_flood_rt          = LoadRenderTexture(pixel_size.x, pixel_size.y);
}

void load_lightmaps(){
    for (i32 i = 0; i < current_level_context->lightmaps.count; i++){
        Lightmap_Data* l = current_level_context->lightmaps.get_ptr(i);
        
        load_lightmap_render_textures(i);

        // Doing it in cycle so it become true only if there was at least one.
        current_level_context->lightmaps_render_textures_loaded = true;
    }
}

inline void unload_lightmap_render_textures(i32 index){
    assert(index < current_level_context->lightmaps.count);

    UnloadRenderTexture(current_level_context->lightmaps.get_ptr(index)->global_illumination_rt);
    UnloadRenderTexture(current_level_context->lightmaps.get_ptr(index)->emitters_occluders_rt);
    UnloadRenderTexture(current_level_context->lightmaps.get_ptr(index)->distance_field_rt);
    UnloadRenderTexture(current_level_context->lightmaps.get_ptr(index)->normal_rt);
    UnloadRenderTexture(current_level_context->lightmaps.get_ptr(index)->voronoi_seed_rt);
    UnloadRenderTexture(current_level_context->lightmaps.get_ptr(index)->jump_flood_rt);
}

void unload_lightmaps(){
    for (i32 i = 0; i < current_level_context->lightmaps.count; i++){
        unload_lightmap_render_textures(i);
    }
    current_level_context->lightmaps_render_textures_loaded = false;
}

struct Bake_Settings{
    i32 rays_per_pixel = 128;
    i32 raymarch_steps = 256;
    f32 distance_mod = 1;
};

Bake_Settings light_bake_settings = {128, 256, 1};
Bake_Settings heavy_bake_settings = {512, 1024, 4};
Bake_Settings final_bake_settings = {1024, 2048, 4};

Bake_Settings bake_settings = light_bake_settings;

b32 need_to_bake = false;
f32 bake_progress = 0;
i32 currently_baking_index = -1;
i32 bake_only_one_index = -1;

void bake_lightmaps_if_need(){
    // Currently baking one by one so we could see that something happening. 
    // Later we probably should do that in separate thread so everything does not stall, or just show progress.
    if (need_to_bake){
        assign_selected_entity(NULL);
    
        currently_baking_index += 1;
        if (currently_baking_index >= current_level_context->lightmaps.count){
            currently_baking_index = -1;
        }
        
        // All unloading of previous textures should happen here.
        if (bake_only_one_index > -1){
            currently_baking_index = bake_only_one_index;
            Lightmap_Data* l = current_level_context->lightmaps.get_ptr(bake_only_one_index);
            if (l->has_loaded_texture){
                UnloadTexture(l->lightmap_texture);
                l->has_loaded_texture = false;
            }
        } else{
            for (i32 i = 0; i < current_level_context->lightmaps.count; i++){
                Lightmap_Data* l = current_level_context->lightmaps.get_ptr(i);
                if (l->has_loaded_texture){
                    UnloadTexture(l->lightmap_texture);
                    l->has_loaded_texture = false;
                }
            }
        }
        
        if (IsKeyPressed(KEY_F9)){
            bake_settings = light_bake_settings;
        } else if (IsKeyPressed(KEY_F10)){
            bake_settings = heavy_bake_settings;
        } else if (IsKeyPressed(KEY_F11)){
            bake_settings = final_bake_settings;
        }
    }
    
    // Could happen that we starting to bake and render textures are still loaded, but only if we started new bake 
    // before previous is finished. Would like to clearly check that lighmaps will be unloaded correctly, but that probably
    // will not be a problem.
    if (bake_progress == 0 && currently_baking_index > -1){
        load_lightmap_render_textures(currently_baking_index);     
    }
    
    // We do this calculations only on very first bake progress, right after button press.
    // Because here we're drawing all the emitters/occlusions/normal_maps that will be used for calculating GI.
    for (i32 lightmap_index = 0; lightmap_index < current_level_context->lightmaps.max_count && need_to_bake; lightmap_index++){
        if (lightmap_index != currently_baking_index){
            continue;
        }
        
        if (!need_to_bake){
            continue;
        }
    
        Lightmap_Data* lightmap_data = current_level_context->lightmaps.get_ptr(lightmap_index);
        RenderTexture* emitters_occluders_rt = &lightmap_data->emitters_occluders_rt;
        RenderTexture* distance_field_rt = &lightmap_data->distance_field_rt;
        RenderTexture* normal_rt = &lightmap_data->normal_rt;
        RenderTexture* voronoi_rt = &lightmap_data->voronoi_seed_rt;
        RenderTexture* jump_flood_rt = &lightmap_data->jump_flood_rt;

        drawing_state = LIGHTING_DRAWING;
        
        Vector2 pixel_size = get_lightmap_pixel_size(lightmap_data);
        
        current_level_context->cam = get_cam_for_resolution(pixel_size.x, pixel_size.y);
        current_level_context->cam.position = lightmap_data->position;
        current_level_context->cam.view_position = lightmap_data->position;
        current_level_context->cam.cam2D.zoom = get_light_zoom(lightmap_data->game_size.x);
        
        BeginTextureMode(*normal_rt);{
            BeginMode2D(current_level_context->cam.cam2D);{
            ClearBackground(Fade(BLACK, 0));
            
            ForEntities(texture_entity, TEXTURE){
                if (texture_entity->have_normal_map){
                    draw_game_texture(texture_entity->normal_map_texture, texture_entity->position, texture_entity->scale, texture_entity->pivot, texture_entity->rotation, texture_entity->color_changer.start_color);
                }
            }
            } EndMode2D();
        } EndTextureMode();
        
        BeginTextureMode(*emitters_occluders_rt);{
        BeginMode2D(current_level_context->cam.cam2D);
        ClearBackground(Fade(BLACK, 0));
        BeginBlendMode(BLEND_ALPHA);
            ForEntities(entity, LIGHT){   
                if (entity->flags & LIGHT){
                    Light *light = current_level_context->lights.get_ptr(entity->light_index);
                    if (light->bake_shadows){
                        if (entity->flags & TEXTURE){
                            draw_game_texture(entity->texture, entity->position, entity->scale, entity->pivot, entity->rotation, Fade(light->color, light->opacity));
                        } else{
                            draw_game_triangle_strip(entity, Fade(light->color, light->opacity));
                        }
                    }
                }
            }
            ForEntities(entity2, GROUND){   
                if (entity2->flags & DOOR || entity2->flags & PHYSICS_OBJECT || entity2->flags & LIGHT || entity2->flags & MOVE_SEQUENCE){
                    continue;
                }
                
                draw_game_triangle_strip(entity2, ColorBrightness(entity2->color, -0.75f));
                
                if (entity2->flags & NO_MOVE_BLOCK){
                    draw_game_line_strip(entity2->position, entity2->vertices, PURPLE);
                }
            }
        EndBlendMode();
        EndMode2D();
        } EndTextureMode();
        
        current_level_context->cam = with_shake_cam;
        
        BeginTextureMode(*voronoi_rt);{
            ClearBackground({0, 0, 0, 0});
            BeginShaderMode(voronoi_seed_shader);
                draw_render_texture(emitters_occluders_rt->texture, {1.0f, 1.0f}, WHITE);
            EndShaderMode();
        }EndTextureMode();
        
        RenderTexture prev = *voronoi_rt;
        RenderTexture next = *jump_flood_rt;
        
        //jump flood voronoi render pass
        {
            i32 passes = ceilf(logf(fmaxf(pixel_size.x, pixel_size.y)) / logf(2.0f));
            
            i32 level_loc     = get_shader_location(jump_flood_shader, "u_level");
            i32 max_steps_loc = get_shader_location(jump_flood_shader, "u_max_steps");
            i32 offset_loc    = get_shader_location(jump_flood_shader, "u_offset");
            i32 tex_loc       = get_shader_location(jump_flood_shader, "u_tex");
            
            for (int i = 0; i < passes; i++){
                f32 offset = powf(2.0f, passes - i - 1);
                BeginTextureMode(next);{
                    BeginShaderMode(jump_flood_shader);
                    set_shader_value(jump_flood_shader, max_steps_loc, passes);
                    set_shader_value(jump_flood_shader, level_loc, i);
                    set_shader_value(jump_flood_shader, offset_loc, offset);
                    
                    i32 screen_pixel_size_loc  = get_shader_location(jump_flood_shader, "u_screen_pixel_size");
            
                    set_shader_value(jump_flood_shader, screen_pixel_size_loc, {(1.0f) / pixel_size.x, (1.0f) / pixel_size.y});
    
                    // set_shader_value(jump_flood_shader, step_loc, 1 << i);
                    // set_shader_value(jump_flood_shader, pixel_loc, {LIGHT_TEXTURE_SCALING_FACTOR / screen_width, LIGHT_TEXTURE_SCALING_FACTOR / screen_height});
                    set_shader_value_tex(jump_flood_shader, tex_loc, prev.texture);
                    draw_render_texture(voronoi_rt->texture, {1.0f, 1.0f}, WHITE);
                    EndShaderMode();
                }EndTextureMode();
                
                RenderTexture temp = next;
                next = prev;
                prev = temp;
            }
        }
        
        //distance field pass (Render voronoi)
        BeginTextureMode(*distance_field_rt);{
            ClearBackground({0, 0, 0, 0});
            BeginShaderMode(distance_field_shader);
            i32 tex_loc = get_shader_location(distance_field_shader, "u_tex");
            i32 tex2_loc = get_shader_location(distance_field_shader, "obstacles_texture");
            set_shader_value_tex(distance_field_shader, tex_loc, prev.texture);
            set_shader_value_tex(distance_field_shader, tex2_loc, emitters_occluders_rt->texture);
            draw_render_texture(prev.texture, {1.0f, 1.0f}, WHITE);
            EndShaderMode();
        }EndTextureMode();
        
        lightmap_data->distance_texture_loc = get_shader_location(global_illumination_shader, "distance_texture");        
        lightmap_data->emitters_occluders_loc = get_shader_location(global_illumination_shader, "emitters_occluders_texture");        
    }
    
    // At this point we computed emitters/occluders and distnace fields for every lightmap.
    // Now we do real global illumination work and we will need this info for calculating neighbours.
    // (we're not calculating neighbours anymore).
    for (i32 lightmap_index = 0; lightmap_index < current_level_context->lightmaps.max_count; lightmap_index++){
        // Baking one at the time to see that something is happening.
        // if (lightmap_index != currently_baking_index){
        //     continue;
        // }
        
        if (lightmap_index != currently_baking_index){
            continue;
        }
        
        bake_progress += 0.05f;
    
        Lightmap_Data *lightmap_data         = current_level_context->lightmaps.get_ptr(lightmap_index);
        RenderTexture *gi_rt                 = &lightmap_data->global_illumination_rt;
        RenderTexture *my_emitters_occluders_rt = &lightmap_data->emitters_occluders_rt;
        RenderTexture *my_distance_field_rt     = &lightmap_data->distance_field_rt;
        RenderTexture *my_normal_rt             = &lightmap_data->normal_rt;
        
        Vector2 pixel_size = get_lightmap_pixel_size(lightmap_data);
        
        //global illumination pass
        BeginTextureMode(*gi_rt);{
            // Means we're just started
            if (need_to_bake){
                ClearBackground(Fade(BLACK, 0));
            }
            
            BeginShaderMode(global_illumination_shader);
            
            set_shader_value_tex(global_illumination_shader, lightmap_data->distance_texture_loc, my_distance_field_rt->texture);
            set_shader_value_tex(global_illumination_shader, lightmap_data->emitters_occluders_loc, my_emitters_occluders_rt->texture);
            
            i32 rays_per_pixel_loc     = get_shader_location(global_illumination_shader, "u_rays_per_pixel");
            i32 emission_multi_loc     = get_shader_location(global_illumination_shader, "u_emission_multi");
            i32 max_raymarch_steps_loc = get_shader_location(global_illumination_shader, "u_max_raymarch_steps");
            i32 time_loc               = get_shader_location(global_illumination_shader, "u_time");
            i32 distance_mod           = get_shader_location(global_illumination_shader, "u_dist_mod");
            i32 screen_pixel_size_loc  = get_shader_location(global_illumination_shader, "u_screen_pixel_size");
            
            i32 normal_texture_loc     = get_shader_location(global_illumination_shader, "u_normal_texture");
            set_shader_value_tex(global_illumination_shader, normal_texture_loc, my_normal_rt->texture);
            
            set_shader_value(global_illumination_shader, distance_mod, bake_settings.distance_mod);
            set_shader_value(global_illumination_shader, screen_pixel_size_loc, {(1.0f) / pixel_size.x, (1.0f) / pixel_size.y});
            set_shader_value(global_illumination_shader, time_loc, core.time.app_time + PI * 10);
            
            set_shader_value(global_illumination_shader, rays_per_pixel_loc, bake_settings.rays_per_pixel);
            set_shader_value(global_illumination_shader, emission_multi_loc, 1.5f);
            set_shader_value(global_illumination_shader, max_raymarch_steps_loc, bake_settings.raymarch_steps);
            
            i32 bake_progress_loc = get_shader_location(global_illumination_shader, "bake_progress");
            set_shader_value(global_illumination_shader, bake_progress_loc, bake_progress);
            
            i32 perlin_texture_loc = get_shader_location(global_illumination_shader, "perlin_texture");
            set_shader_value_tex(global_illumination_shader, perlin_texture_loc, perlin_texture);
            
            draw_render_texture(gi_rt->texture, {1.0f, 1.0f}, WHITE);
            
            // draw_rect({1, 1}, {1, 1}, WHITE);
            EndShaderMode();
        } EndTextureMode();
    }
    
    if (bake_progress >= 1){
        assert(currently_baking_index > -1 && currently_baking_index < current_level_context->lightmaps.count);
        
        save_lightmap_to_file(currently_baking_index);
        
        unload_lightmap_render_textures(currently_baking_index);
        
        // We unloaded this when started.
        assert(current_level_context->lightmaps.get_ptr(currently_baking_index)->has_loaded_texture == false);
        current_level_context->lightmaps.get_ptr(currently_baking_index)->lightmap_texture = LoadTexture(get_lightmap_name(currently_baking_index));
        current_level_context->lightmaps.get_ptr(currently_baking_index)->has_loaded_texture = true;
        
        currently_baking_index += 1;
        
        if (currently_baking_index >= current_level_context->lightmaps.count){
            currently_baking_index = -1;
        }
        
        if (bake_only_one_index > -1){
            bake_only_one_index = -1;
            currently_baking_index = -1;
        }
        
        bake_progress = 0;
    }
    
    // We want this to be true only on the first bake cycle.
    need_to_bake = false;
}

i32 hovered_edit_button_index = -1;
void make_lightmap_settings_panel(){
    if (IsKeyPressed(KEY_ESCAPE)){
        editor.editing_lightmap = false;
        editor.picking_lightmap_position = false;        
    }

    if (make_button({5, screen_height * 0.2f - 20}, {screen_width * 0.08f, 20.0f}, {0.0f, 0.0f}, "Lightmap settings", 18, "lightmap_settings_button", Fade(SKYBLUE, 0.4f))){
        editor.lightmap_settings_active = !editor.lightmap_settings_active;
    }
    
    if (!editor.lightmap_settings_active){
        return;
    }

    begin_panel({5, screen_height * 0.2f}, {screen_width * 0.15f, screen_height * 0.4f}, Fade(SKYBLUE, 0.4f), "lightmap_settings_panel");
        
    make_panel_text(tprintf("Lightmaps count: %d", current_level_context->lightmaps.count), "lightmap_count_panel_text");
    
    if (make_panel_button("Add lightmap", "add_lightmap_button")){
        current_level_context->lightmaps.add({});
    }
    
    local_persist i32 lightmap_level = 0;
    const char* lightmap_level_text = NULL;
    if (0){}
    else if (lightmap_level == 0) lightmap_level_text = tprintf("Fast bake setting");
    else if (lightmap_level == 1) lightmap_level_text = tprintf("Nice bake setting");
    else if (lightmap_level == 2) lightmap_level_text = tprintf("Highest bake setting");
    
    if (make_panel_button(lightmap_level_text, "lightmap_level_text")){
        lightmap_level += 1;    
        lightmap_level %= 3;
        if (0){}
        else if (lightmap_level == 0) bake_settings = light_bake_settings;
        else if (lightmap_level == 1) bake_settings = heavy_bake_settings;
        else if (lightmap_level == 2) bake_settings = final_bake_settings;
    }
    
    const char* pixels_per_unit_text = NULL;
    if (0){}
    else if (pixel_per_unit == 2) pixels_per_unit_text = tprintf("2 pixel per unit");
    else if (pixel_per_unit == 4) pixels_per_unit_text = tprintf("4 pixel per unit");
    
    if (make_panel_button(pixels_per_unit_text, "pixels_per_unit_text")){
        pixel_per_unit += 2;    
        if (pixel_per_unit > 4) pixel_per_unit = 2;
    }
    
    i32 hovered_index = -1;
    
    for (i32 i = 0; i < current_level_context->lightmaps.count; i++){
        make_panel_text(tprintf("Lightmap: %d", i+1), tprintf("lightmap_text_%d", i+1));
        panel_indent();
        
        if (make_panel_button(tprintf("Bake %d", i+1), tprintf("bake_lightmap_%d"))){
            bake_only_one_index = i;
            need_to_bake = true;
        }
        
        if (editor.editing_lightmap && editor.editing_lightmap_index == i){
            if (make_panel_button(tprintf("Cancel edit %d", i+1), tprintf("cancel_edit_%d"))){
                editor.editing_lightmap = false;
            }
        } else{
            if (make_panel_button(tprintf("Edit lightmap %d", i+1), tprintf("edit_lightmap_%d", i+1))){
                editor.editing_lightmap = true;
                editor.editing_lightmap_index = i;
            }
            
            if (last_ui_element_hovered()) hovered_index = i;
        }
        
        if (editor.picking_lightmap_position && editor.editing_lightmap_index == i){
            if (make_panel_button(tprintf("Cancel picking %d position", i+1), tprintf("cancel_pick_lightmap_position_%d", i+1))){
                editor.picking_lightmap_position = false;                
            }
        } else{
            if (make_panel_button(tprintf("Pick %d position", i+1), tprintf("pick_lightmap_position_%d", i+1))){
                editor.picking_lightmap_position = true;                
                editor.editing_lightmap_index = i;
            }
            
            if (last_ui_element_hovered()) hovered_index = i;
        }
        
        if (make_panel_button(tprintf("Remove lightmap %d", i+1), tprintf("remove_lightmap_%d", i+1))){
            unload_lightmap_render_textures(i);
            
            current_level_context->lightmaps.remove(i);
            editor.editing_lightmap = false;
        }
        
        panel_unindent();
    }

    end_panel();
    
    hovered_edit_button_index = hovered_index;
    
    if (editor.picking_lightmap_position && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !clicked_ui){
        assert(editor.editing_lightmap_index != -1);
        assert(editor.editing_lightmap_index < current_level_context->lightmaps.count);
        current_level_context->lightmaps.get_ptr(editor.editing_lightmap_index)->position = round_to_factor(input.mouse_position, CELL_SIZE);
    }
    
    // Making size choosing panel.
    if (editor.editing_lightmap){
        begin_panel({screen_width * 0.15f + 5, screen_height * 0.2f}, {screen_width * 0.15f, screen_height * 0.3f}, Fade(GREEN, 0.4f), "lightam_editing_panel");
        
        assert(editor.editing_lightmap_index > -1 && editor.editing_lightmap_index < current_level_context->lightmaps.count);
        Lightmap_Data* l = current_level_context->lightmaps.get_ptr(editor.editing_lightmap_index);
        make_panel_text(tprintf("Position : {%.0f, %.0f}", l->position.x, l->position.y), "editing_lightmap_position");
        make_panel_text(tprintf("Game size: {%.0f, %.0f}", l->game_size.x, l->game_size.y), "editing_lightmap_gamesize");
        
        if (make_panel_button("Up 50", "up_position_lightmap_50")){
            l->position = round_to_factor(l->position + Vector2_up * 50, 50);
        }
        if (make_panel_button("Down 50", "down_position_lightmap_50")){
            l->position = round_to_factor(l->position - Vector2_up * 50, 50);
        }
        if (make_panel_button("Right 50", "right_position_lightmap_50")){
            l->position = round_to_factor(l->position + Vector2_right * 50, 50);
        }
        if (make_panel_button("Left 50", "left_position_lightmap_50")){
            l->position = round_to_factor(l->position - Vector2_right * 50, 50);
        }
        
        if (make_panel_button("+Height 100", "plus_height_lightmap_100")){
            l->game_size = round_to_factor(l->game_size + Vector2_up * 100, 100);
        }
        if (make_panel_button("-Height 100", "minus_height_lightmap_100")){
            l->game_size = round_to_factor(l->game_size - Vector2_up * 100, 100);
        }
        if (make_panel_button("+Width 100", "plus_width_lightmap_100")){
            l->game_size = round_to_factor(l->game_size + Vector2_right * 100, 100);
        }
        if (make_panel_button("-Width 100", "minus_width_lightmap_100")){
            l->game_size = round_to_factor(l->game_size - Vector2_right * 100, 100);
        }
        
        end_panel();
    }
}

inline void draw_game_lightmap_editing(){
    if (!editor.editing_lightmap && !editor.picking_lightmap_position && hovered_edit_button_index == -1){
        return;
    }
    
    i32 index = editor.editing_lightmap_index;
    if (!editor.editing_lightmap && !editor.picking_lightmap_position){
        assert(hovered_edit_button_index > -1);
        index = hovered_edit_button_index;
    }
    assert(index < current_level_context->lightmaps.count);
    
    for (i32 i = 0; i < current_level_context->lightmaps.count; i++){
        Lightmap_Data* l = current_level_context->lightmaps.get_ptr(i);
        f32 fade_progress = ((sinf(core.time.app_time * 2) + 1) * 0.5f + 0.2f) * 0.4f;
        
        // Index is lightmap that we currently editing, but we want to see others aswell.
        if (i == index){
            draw_game_rect(l->position, l->game_size, {0.5f, 0.5f}, 0, Fade(YELLOW, fade_progress));
        } else{
            draw_game_rect(l->position, l->game_size, {0.5f, 0.5f}, 0, Fade(BLUE, fade_progress * 0.5f));
        }
    }
}
