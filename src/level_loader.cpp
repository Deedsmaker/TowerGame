#pragma once

inline void level_loader_validate_color(Entity *entity) {
    auto loader = entity->level_loader;
    
    if (loader->flags & LEVEL_LOADER_OPEN) {
        change_color(entity, WHITE);
    } else {
        change_color(entity, ColorBrightness(WHITE, -0.4f));
    }
}

inline void update_level_loader(Entity *entity) {
}

inline void update_standing_on_level_loader(Entity *entity, Entity *player_entity) {
    auto loader = entity->level_loader;

    Color color = loader->flags & LEVEL_LOADER_OPEN ? WHITE : ColorBrightness(WHITE, -0.4f);
    
    Vector2 pos = {screen_width * 0.4f, 300};
    Old::make_ui_text(c_str(loader->level_to_load), pos, 60, color, "level_loader_text");
    
    if (loader->flags & LEVEL_LOADER_OPEN) {
        make_texture(*get_texture("ArrowSign"), player_entity->position + Vector2_up * player_entity->scale.y * 6, Vector2_one * 15, {0.5f, 0.5f}, -90, WHITE);
        if (input.press_flags & UP_KEY_PRESSED && loader->level_to_load.count > 0) {
            
        
            load_level(loader->level_to_load, ERROR_IF_NO_SUCH_LEVEL | ENTER_GAME_STATE_AFTER);
        }
    }
}

