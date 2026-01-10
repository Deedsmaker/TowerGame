#pragma once

inline void level_loader_validate(Entity *entity) {
    auto loader = entity->level_loader;
    
    // Here we will check if level loader level is completed and if it is - trigger all connected entities
    // so any connected level loaders will become open.
    //
    // We don't check if level loader itself is open now on purpuse - becauase that way check order will not matter and we probably 
    // will see any bugs visually anyway.
    // (For clarification - loader "open" means that we can enter level from that level loader
    // and "connected" would be mainly other level loaders that we will want to "open" after that level is completed.
    // Could happen that we will "open" other level loaders before current loader itself is considered open, 
    // but that situation should occur only during loading).
    
    auto save = &global_data.game_save;
    b32 level_completed = save->completed_levels.contains(loader->level_to_load);
    if (level_completed) {
        For (&entity->trigger->connected) {
            auto connected = get_entity(*it, entity->context);
            trigger_entity(entity, connected);
        }
        
        
        if (save->just_completed_level_name && (*save->just_completed_level_name) == loader->level_to_load) {
            // To not give buffs multiple times we're catching exact moment where level was completed and doing what has to be done 
            // when level on this level loader was completed for the first time.
            save->just_completed_level_name = NULL;
            if (loader->flags & LOADER_GIVE_SWORD_CHARGE) {
                save->sword_charges_collected += 1;
                game_log("Sword charge aquired.");
            }
            if (loader->flags & LOADER_GIVE_AMMO) {
                static const u32 AMMO_TO_GIVE = 5;
                save->ammo_collected += AMMO_TO_GIVE;
                game_log(tstring("%d additional ammo aquired.", AMMO_TO_GIVE));
            }
        }
        
        save_game_data(temp);
        player_validate_stats_by_game_save(&entity->context->player);
    }
}

void validate_level_loaders(Context *context) {
    for_chunk_array(i, &context->level_loaders) {
        level_loader_validate(context->level_loaders.get(i)->entity);
    }
}

inline void update_level_loader(Entity *entity) {
}

inline void update_standing_on_level_loader(Entity *entity, Entity *player_entity) {
    auto loader = entity->level_loader;

    Color color = loader->flags & LOADER_OPEN ? WHITE : ColorBrightness(WHITE, -0.4f);
    
    Vector2 pos = {screen_width * 0.4f, 300};
    Old::make_ui_text(c_str(loader->level_to_load), pos, 60, color, "level_loader_text");
    
    if (loader->flags & LOADER_OPEN) {
        make_texture(*get_texture("ArrowSign"), player_entity->position + Vector2_up * player_entity->scale.y * 6, Vector2_one * 15, {0.5f, 0.5f}, -90, WHITE);
        if (input.press_flags & UP_KEY_PRESSED && loader->level_to_load.count > 0) {
            
        
            load_level(loader->level_to_load, ERROR_IF_NO_SUCH_LEVEL | ENTER_GAME_STATE_AFTER);
        }
        
        if (IsKeyPressed(KEY_P)) {
            record_completed_level(loader->level_to_load, temp);
            level_loader_validate(entity);
        }
    }
}
