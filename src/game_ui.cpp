#pragma once

void update_ui(Context *context) {
    if (editor_state == EDITOR) {
        return;
    } 
        
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (!(context->flags & HUB_CONTEXT)) {
            load_hub_level();
        }
    }
}
