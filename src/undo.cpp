#pragma once

Array <Entity_Undo_Change> get_entities_difference(Entity *original, Entity *changed) {
    // @LEAK Probably. Could think about using undo level context memory arena in undo_actions. Could work if we'll reuse things.
    Array <Entity_Undo_Change> changes = {.allocator = HEAP_ALLOCATOR};

    if (original->position != changed->position) {
        changes.append({
            .entity_id = changed->id,
            .change_type = VECTOR2_CHANGE,
            .vector_change = changed->position - original->position,
            .changed_vector = &changed->position
        });
    }
    
    return changes;
}

inline void update_undo_logic() {
    // First of all detecting changed entities to add them to undo actions.
    if (editor.selected_entity && editor.selected_entity->runtime_only_flags & EDITOR_CHANGED) {
        editor.selected_entity->runtime_only_flags ^= EDITOR_CHANGED;
        
        Array <Entity_Undo_Change> undo_changes = get_entities_difference(editor.selected_entity_unchanged_copy, editor.selected_entity);
        free_entity(editor.selected_entity_unchanged_copy);
        
        editor.selected_entity_unchanged_copy = copy_and_add_entity(editor.selected_entity, &undo_level_context);
        
        current_level_context->undo_actions.append(undo_changes);
    }

    b32 undo_required_helper_keys_down = !IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL);
    b32 undo_pressed = undo_required_helper_keys_down && IsKeyPressed(KEY_Z);
    if (undo_pressed && current_level_context->undo_actions.count > 0) {
        Array <Entity_Undo_Change> *undo_changes = current_level_context->undo_actions.last();
        for_array(i, undo_changes) {
            Entity_Undo_Change *change = undo_changes->get(i);
            // Entity *changed_entity = get_entity(change->entity_id);
            assert(get_entity(change->entity_id));
            
            switch (change->change_type) {
                case VECTOR2_CHANGE: {
                    *change->changed_vector -= change->vector_change;
                } break;
                case NO_CHANGE: { 
                    assert(false);
                } break;
            }
        }
        
        current_level_context->undo_actions.count -= 1;
    }
}
