#pragma once

inline f32 get_light_zoom(f32 radius) {
    return SCREEN_WORLD_SIZE / radius;
}

inline void add_light_to_draw_queue(Light light) {
    render.lights_draw_queue.append(light);
}

void init_light(Light *light) {
    //@TODO: rewrite lights.
    return;

    // if (light->exists) {
    //     free_light(light);        
    // }

    if (light->shadows_size_flags        & ULTRA_SMALL_LIGHT) {
        light->shadows_size = 64;
    } else if (light->shadows_size_flags & SMALL_LIGHT) {
        light->shadows_size = 128;
    } else if (light->shadows_size_flags & MEDIUM_LIGHT) {
        light->shadows_size = 256;
    } else if (light->shadows_size_flags & BIG_LIGHT) {
        light->shadows_size = 512;
    } else if (light->shadows_size_flags & HUGE_LIGHT) {
        light->shadows_size = 1024;
    } else if (light->shadows_size_flags & GIANT_LIGHT) {
        light->shadows_size = 2048;
    }
    
    if (light->backshadows_size_flags        & ULTRA_SMALL_LIGHT) {
        light->backshadows_size = 64;
    } else if (light->backshadows_size_flags & SMALL_LIGHT) {
        light->backshadows_size = 128;
    } else if (light->backshadows_size_flags & MEDIUM_LIGHT) {
        light->backshadows_size = 256;
    } else if (light->backshadows_size_flags & BIG_LIGHT) {
        light->backshadows_size = 512;
    } else if (light->backshadows_size_flags & HUGE_LIGHT) {
        light->backshadows_size = 1024;
    } else if (light->backshadows_size_flags & GIANT_LIGHT) {
        light->backshadows_size = 2048;
    }
    
    if (light->bake_shadows) {
        // light->geometry_size = fminf(light->shadows_size * 4, 2048);
    }
    
    // @VISUAL: We're removing dynamic shadows on lights for now because I'm on the middle of huge engine rewrite and that's not 
    // the main point, so that's for later.
    
    // if (light->make_shadows) {
    //     light->shadowmask_rt  = LoadRenderTexture(light->shadows_size, light->shadows_size);
    // } else {
    //     light->shadowmask_rt = {};
    // }
    
    // if (light->make_backshadows) {
    //     light->backshadows_rt = LoadRenderTexture(light->backshadows_size, light->backshadows_size);
    // } else {
    //     light->backshadows_rt = {};
    // }
            
    if (light->make_shadows || light->make_backshadows) {
        // light->geometry_rt = LoadRenderTexture(light->geometry_size, light->geometry_size);
    }
}

Light copy_light(Light *src) {
    // Light original_dest = *dest;
    Light result = *src;
    if (!result.level_context) result.level_context = NULL;
    return result;
    // dest->shadowmask_rt = original_dest.shadowmask_rt;
    // dest->backshadows_rt = original_dest.backshadows_rt;
}

void free_light(Light *light, i32 index, Level_Context *level_context) {
    if (light->connected_entity_id > 0) {
        Entity *connected_entity = get_entity(light->connected_entity_id, level_context);
        connected_entity->lights.remove_first_encountered(index);
        assert(!connected_entity->lights.contains(index));
    }

    level_context->lights.remove(index);
    // @TODO: Here we'll want to tell array of light render textures that we're not occupying this space anymore and that's it.
    // if (light->exists) {
    //     if (light->make_shadows) {
    //         UnloadRenderTexture(light->shadowmask_rt);
    //         light->shadowmask_rt = {};
    //     }
    //     // UnloadRenderTexture(light->geometry_rt);
    //     if (light->make_backshadows) {
    //         UnloadRenderTexture(light->backshadows_rt);
    //         light->backshadows_rt = {};
    //     }
        
    //     light->exists = false;
        
    //     if (light->connected_entity_id != -1) {
    //         Entity *connected_entity = get_entity(light->connected_entity_id);
    //         if (connected_entity) {
    //             connected_entity->light_index = -1;
    //         }
    //         light->connected_entity_id = -1;
    //     }
    // }
}

void free_lights_connected_to_entity(Entity *entity) {
    for_array(i, &entity->lights) {
        i32 light_index = entity->lights.get_value(i);
        Light *light = entity->level_context->lights.get(light_index);
        free_light(light, light_index, entity->level_context);
        
        i -= 1; // Because free_light removes light from entity->lights.
    }
    
    assert(entity->lights.count == 0); // We're removing light index from entity on free_entity, so here count should be zero.
    // entity->lights.clear();
}

inline Light *copy_and_add_light(Light *to_copy) {
    Level_Context *level_context = to_copy->level_context ? to_copy->level_context : current_level_context;
    return level_context->lights.append(copy_light(to_copy));
}

Light *copy_and_add_light_to_entity(Entity *entity, Light *to_copy, b32 free_all_entity_lights_first = false) {
    if (free_all_entity_lights_first) {
        free_lights_connected_to_entity(entity);
    }
    
    i32 light_index = 0;
    Light *new_light = entity->level_context->lights.append(copy_light(to_copy), &light_index);
    new_light->level_context = entity->level_context;
    new_light->connected_entity_id = entity->id;
    init_light(new_light);
    entity->lights.append(light_index);
    
    return new_light;
}

void update_dynamic_lights() {
    for_chunk_array(i, &current_level_context->lights) {
        Light *light = current_level_context->lights.get(i);
        
        Entity *connected_entity = NULL;
        if (light->connected_entity_id > 0) connected_entity = get_entity(light->connected_entity_id);
        
        // Here we're updating temp lights. Lights that's not considered temporary will not have grow_time and shrink_time set.
        if (light->grow_time > 0 || light->shrink_time > 0) {
            f32 lifetime = core.time.game_time - light->birth_time;
            if (lifetime < light->grow_time) {
                f32 grow_t        = lifetime / light->grow_time;
                light->radius = lerp(0.0f, light->target_radius, sqrtf(grow_t));
                light->power  = lerp(4.0f, 2.0f, grow_t * grow_t);
            } else { //shrinking
                f32 shrink_t       = clamp01((lifetime - light->grow_time) / light->shrink_time);
                light->radius  = lerp(light->target_radius, light->target_radius * 0.5f, shrink_t * shrink_t);
                light->opacity = lerp(light->start_opacity, 0.0f, shrink_t * shrink_t);
                light->power   = lerp(2.0f, 1.0f, shrink_t * shrink_t);
            }
            
            if (lifetime > light->grow_time + light->shrink_time) {
                // light->exists = false;
                free_light(light, i, light->level_context);
                continue;
            }
        }
        
        //update light
        if (connected_entity) {
            light->position = connected_entity->position;
        }
            
        if (light->fire_effect) {
            f32 perlin_rnd = (perlin_noise3(core.time.game_time * 5, i, core.time.game_time * 4) + 1) * 0.5f;
            light->radius = perlin_rnd * 30 + 45;
            light->power  = perlin_rnd * 1.0f + 0.5f;
        }
    }
}

void draw_dynamic_lights(RenderTexture *render_texture_for_lights) {
    // Drawing dynamic lights on camera plane.
    local_persist Shader smooth_edges_shader = LoadShader(0, "./resources/shaders/smooth_edges.fs");
    
    auto original_drawing_state = drawing_state;
    
    drawing_state = LIGHTING_DRAWING;
    
    BeginTextureMode(light_geometry_rt); {
        ClearBackground(Fade(BLACK, 0));
        BeginMode2D(current_level_context->cam.cam2D);
        BeginShaderMode(gaussian_blur_shader);
            i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
            set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / (screen_width), (1.0f) / (screen_height)});

            // ForEntities(entity, GROUND | ENEMY | PLAYER | PLATFORM | SWORD) {
            for (i32 i = 0; i < session_context.entities_draw_queue.count; i++) {
                Entity *entity = session_context.entities_draw_queue.get(i);
                if (entity->hidden || should_not_draw_entity(entity, current_level_context->cam)) {
                    continue;
                }
                Color prev_color = entity->color;
                entity->color = WHITE;
                draw_entity(entity);
                entity->color = prev_color;
            }
            //draw_particles();
        EndShaderMode();
        EndMode2D();
    }EndTextureMode();
    
        local_persist Texture smooth_circle_texture = white_pixel_texture;
    
    for_chunk_array(light_index, &current_level_context->lights) {
        Light light = current_level_context->lights.get_value(light_index);
        
        // That means the light is actually a lightmap. Shoulda change that at some point.        
        if (light.bake_shadows) {
            continue;
        }
        
        Entity *connected_entity = NULL;
        if (light.connected_entity_id > 0) connected_entity = get_entity(light.connected_entity_id);
        
        // Vector2 light_position = light.position;
        Vector2 lightmap_game_scale = {light.radius, light.radius};
        
        b32 should_calculate_light_anyway = light.bake_shadows && session_context.just_entered_game_state;
        
        Bounds lightmap_bounds = {lightmap_game_scale, {0, 0}};
        if (!should_calculate_light_anyway && (!check_bounds_collision(current_level_context->cam.view_position, light.position, get_cam_bounds(current_level_context->cam, current_level_context->cam.cam2D.zoom), lightmap_bounds) || (connected_entity && connected_entity->hidden && game_state == GAME)) || debug.full_light) {
            continue;
        }
        
        // @VISUAL: Disabling dynamic shadows for now. Will return it later and stronger.
        
        // Vector2 shadows_texture_size = {(f32)light.shadows_size, (f32)light.shadows_size};
        
        // if (light.make_shadows && (!light.bake_shadows || (core.time.app_time - light.last_bake_time > 1 && (game_state == EDITOR) && !session_context.baked_shadows_this_frame) || session_context.just_entered_game_state || !light_ptr->baked && game_state == GAME)) {
        //     light_ptr->last_bake_time = core.time.app_time;
            
        //     if (light.bake_shadows) {
        //         session_context.baked_shadows_this_frame = true;
        //         if (game_state == GAME) {
        //             light_ptr->baked = true;
        //         }
        //     }
            
        //     BeginTextureMode(light.shadowmask_rt); {
        //         ClearBackground(Fade(WHITE, 0));
        //         current_level_context->cam = get_cam_for_resolution(shadows_texture_size.x, shadows_texture_size.y);
        //         current_level_context->cam.position = light.position;
        //         current_level_context->cam.view_position = light.position;
        //         current_level_context->cam.cam2D.zoom = get_light_zoom(light.radius);
        //         BeginMode2D(current_level_context->cam.cam2D);
        //         ForEntities(entity, GROUND | light.additional_shadows_flags) {
        //             if (entity->hidden || entity->id == light.connected_entity_id || should_not_draw_entity(entity, current_level_context->cam)) {
        //                 continue;
        //             }
                    
        //             if (light.bake_shadows && (entity->flags & DOOR || entity->flags & PHYSICS_OBJECT)) {
        //                 continue;
        //             }
                    
        //             Color prev_color = entity->color;
        //             entity->color = BLACK;
        //             draw_entity(entity);
        //             entity->color = prev_color;
        //         }
        //         EndMode2D();
        //         current_level_context->cam = with_shake_cam;
        //     }EndTextureMode();
            
        //     assert(shadows_texture_size.x >= 1);
        //     f32 mult = 2.0f / shadows_texture_size.x;
        //     for (; ; mult *= 1.5f) {
        //         BeginTextureMode(light.shadowmask_rt); {
        //             BeginShaderMode(gaussian_blur_shader);
        //             i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
        //             set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / light.shadows_size, (1.0f) / light.shadows_size});
        //             draw_texture(light.shadowmask_rt.texture, shadows_texture_size * 0.5f, {1.0f + mult, 1.0f + mult}, {0.5f, 0.5f}, 0, WHITE, true);
        //             // if (0 && !light.bake_shadows) {
        //                 EndShaderMode();
        //             // }
        //         }EndTextureMode();
                
        //         // need to check and think about this threshold
        //         if (mult >= 1) {
        //             break;
        //         }
        //     }
        // }        

        // Vector2 backshadows_texture_size = {(f32)light.backshadows_size, (f32)light.backshadows_size};
        // if (light.make_backshadows) {
        //     BeginTextureMode(light.backshadows_rt); {
        //         ClearBackground(Fade(WHITE, 0));
        //         current_level_context->cam = get_cam_for_resolution(backshadows_texture_size.x, backshadows_texture_size.y);
        //         current_level_context->cam.position = light.position;
        //         current_level_context->cam.view_position = light.position;
        //         current_level_context->cam.cam2D.zoom = get_light_zoom(light.radius);
        //         BeginMode2D(current_level_context->cam.cam2D); {
        //         ForEntities(entity, ENEMY | BLOCK_ROPE | SPIKES | PLAYER | PLATFORM | SWORD) {
        //             if (entity->hidden || entity->id == light.connected_entity_id || should_not_draw_entity(entity, current_level_context->cam)) {
        //                 continue;
        //             }
        //             Color prev_color = entity->color;
        //             entity->color = Fade(BLACK, 0.7f);
        //             draw_entity(entity);
        //             entity->color = prev_color;
    
        //         }
        //         // draw_particles();
        //         } EndMode2D();
                
        //         BeginShaderMode(gaussian_blur_shader);
        //             i32 u_pixel_loc     = get_shader_location(gaussian_blur_shader, "u_pixel");
        //             set_shader_value(gaussian_blur_shader, u_pixel_loc, {(1.0f) / light.backshadows_size, (1.0f) / light.backshadows_size});
    
        //             draw_texture(light.backshadows_rt.texture, backshadows_texture_size * 0.5f, {1.0f + 0.2f, 1.0f + 0.2f}, {0.5f, 0.5f}, 0, Fade(BLACK, 0.7f), true);
        //         EndShaderMode();
        //         current_level_context->cam = with_shake_cam;
        //     }; EndTextureMode();
        // }
        
        add_light_to_draw_queue(light);
    } // Light for loop end.
    
    BeginTextureMode(*render_texture_for_lights); {
    BeginShaderMode(smooth_edges_shader); {
    for (i32 i = 0; i <  render.lights_draw_queue.count; i++) {
        Light light = render.lights_draw_queue.get_value(i);
        Vector2 lightmap_game_scale = {light.radius, light.radius};
            // Texture shadowmask_texture = light.make_shadows ? light.shadowmask_rt.texture : white_transparent_pixel_texture;
            Texture shadowmask_texture = white_transparent_pixel_texture;
        
            Vector2 lightmap_texture_pos = get_left_down_texture_screen_position(shadowmask_texture, light.position, lightmap_game_scale);
            BeginMode2D(current_level_context->cam.cam2D); {
                local_persist i32 light_power_loc         = get_shader_location(smooth_edges_shader, "light_power");
                local_persist i32 light_color_loc         = get_shader_location(smooth_edges_shader, "light_color");
                local_persist i32 my_pos_loc              = get_shader_location(smooth_edges_shader, "my_pos");
                local_persist i32 my_size_loc             = get_shader_location(smooth_edges_shader, "my_size");
                local_persist i32 gi_size_loc             = get_shader_location(smooth_edges_shader, "gi_size");
                local_persist i32 gi_texture_loc          = get_shader_location(smooth_edges_shader, "gi_texture");
                local_persist i32 geometry_texture_loc    = get_shader_location(smooth_edges_shader, "geometry_texture");
                local_persist i32 light_texture_loc       = get_shader_location(smooth_edges_shader, "light_texture");
                local_persist i32 backshadows_texture_loc = get_shader_location(smooth_edges_shader, "backshadows_texture");
                
                Vector4 color = ColorNormalize(Fade(light.color, light.opacity));
                set_shader_value_color(smooth_edges_shader, light_color_loc, color);
                set_shader_value(smooth_edges_shader, light_power_loc, light.power);
                set_shader_value(smooth_edges_shader, my_pos_loc, lightmap_texture_pos);
                set_shader_value(smooth_edges_shader, my_size_loc, get_texture_pixels_size(shadowmask_texture, lightmap_game_scale));
                set_shader_value(smooth_edges_shader, gi_size_loc, {(f32)render_texture_for_lights->texture.width, (f32)render_texture_for_lights->texture.height});
                set_shader_value_tex(smooth_edges_shader, gi_texture_loc,          render_texture_for_lights->texture);
                set_shader_value_tex(smooth_edges_shader, light_texture_loc,       smooth_circle_texture);
                // set_shader_value_tex(smooth_edges_shader, backshadows_texture_loc, light.make_backshadows ? light.backshadows_rt.texture : white_transparent_pixel_texture);
                set_shader_value_tex(smooth_edges_shader, geometry_texture_loc,    light.make_shadows || light.make_backshadows ? light_geometry_rt.texture : black_pixel_texture);
                
                draw_game_texture(shadowmask_texture, light.position, lightmap_game_scale, {0.5f, 0.5f}, 0, WHITE, true);
            } EndMode2D();
            current_level_context->cam = with_shake_cam;
    }
    } EndShaderMode();
    } EndTextureMode();
    
    render.lights_draw_queue.clear();

    // In original lighting there goes blur pass, but we can think about drawing dynamic lights in other render texture and blur
    // it instead, because there's no way we want to blur whole global illumination including current_level_context->lightmaps.
    
    drawing_state = original_drawing_state;
}

void add_explosion_light(Vector2 position, f32 radius, f32 grow_time, f32 shrink_time, Color color, i32 size, i32 entity_id) {
    add_explosion_trauma(radius);
    
    Light explosion_light = {0};
    
    explosion_light.birth_time    = core.time.game_time;
    explosion_light.target_radius = radius;
    explosion_light.grow_time     = grow_time;
    explosion_light.shrink_time   = shrink_time;
    explosion_light.color         = color;
    explosion_light.opacity       = (f32)color.a / 255.0f;
    explosion_light.start_opacity = explosion_light.opacity;
    explosion_light.position      = position;
    
    explosion_light.additional_shadows_flags = ENEMY | PLAYER | SWORD;
    explosion_light.connected_entity_id = entity_id;
    
    explosion_light.level_context = current_level_context;
    
    if (entity_id > 0) {
        Entity *entity_to_connect_to = get_entity(entity_id);
        copy_and_add_light_to_entity(entity_to_connect_to, &explosion_light);
    } else {
        copy_and_add_light(&explosion_light);
    }
}

void add_fire_light_to_entity(Entity *entity) {
    Light fire_light = {0};
    fire_light.make_shadows = false;
    fire_light.make_backshadows = false;
    fire_light.shadows_size_flags = MEDIUM_LIGHT;
    fire_light.backshadows_size_flags = MEDIUM_LIGHT;
    fire_light.color = ColorBrightness(ORANGE, 0.4f);
    fire_light.fire_effect = true;

    // Right now fire light considered to live forever and will be freed when entity will be destroyed.
    Light *new_fire_light = copy_and_add_light_to_entity(entity, &fire_light, false);
}

void make_light(Vector2 position, f32 radius, f32 power, f32 opacity, Color color) {
    if (!should_add_immediate_stuff()) return;
    
    Light light = {};
    light.position = position;
    light.radius = radius;
    light.power = power;
    light.opacity = opacity;
    light.color = color;
    light.make_shadows = false;
    light.make_backshadows = false;
    
    add_light_to_draw_queue(light);
}
