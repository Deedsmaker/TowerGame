#pragma once

Array <Entity_Undo_Change> get_entities_difference(Entity *changed, Entity *original) {
    // @LEAK Probably. Could think about using undo level context memory arena in undo_actions. Could work if we'll reuse things.
    Array <Entity_Undo_Change> changes = {.allocator = HEAP_ALLOCATOR};

    if (changed->will_be_destroyed) {
        changes.append({
            .entity_id = changed->id,
            .change_type = ENTITY_DESTROYED,
            .destroyed_entity_copy = copy_and_add_entity(changed, &undo_level_context)
        });
    }

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

b32 add_undo_changes_if_entity_changed(Entity *entity, Entity *unchanged_entity) {
    if (entity->runtime_only_flags & EDITOR_CHANGED) {
        entity->runtime_only_flags ^= EDITOR_CHANGED;
        
        Array <Entity_Undo_Change> changes = get_entities_difference(entity, unchanged_entity);
        current_level_context->undo_actions.append(changes);
        
        // @LEAK: Here we probably need to go through all of undos above and free some shit there. Maybe not if we'll reuse it fully.
        editor.max_undos_added = current_level_context->undo_actions.count;
        
        return true;
    }
    
    return false;
}

inline void update_undo_logic() {
    // First of all detecting changed entities to add them to undo actions.
    
    if (editor.deleted_entity_this_frame) {
        editor.deleted_entity_this_frame = false;
        // In case of deleting entity we want to go through all of entities and look at entities that might 
        // reffer to deleted one and mark them changed aswell, so for example trigger will detect that 
        // one of his connected is will be destroyed -> mark itself as changed and all of it's changed 
        // actions will be in one undo.
        
        for_chunk_array(i, &current_level_context->entities) {
            Entity *entity = current_level_context->entities.get(i);
            Entity *unchanged_entity = copy_and_add_entity(entity, &undo_level_context);
            
            
            // On verifying trigger and kill switch will detect if someone will be destroyed -> will remive
            // it from an array and mark itself as EDITOR_CHANGED.
            verify_trigger_connected(entity);
            verify_kill_switch_connected(entity);
            
            add_undo_changes_if_entity_changed(entity, unchanged_entity);
            
            free_entity(unchanged_entity);
        }
    } else if (editor.selected_entity && editor.selected_entity->runtime_only_flags & EDITOR_CHANGED) {
        b32 added_undo = add_undo_changes_if_entity_changed(editor.selected_entity, editor.selected_entity_unchanged_copy);
        
        assert(added_undo);
        
        // Here we're updating unchanged copy of selected entity because we've stored all the information that we wanted.
        free_entity(editor.selected_entity_unchanged_copy);
        editor.selected_entity_unchanged_copy = copy_and_add_entity(editor.selected_entity, &undo_level_context);
    }

    // Undo logic.
    b32 undo_required_helper_keys_down = !IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL);
    b32 undo_pressed = undo_required_helper_keys_down && IsKeyPressed(KEY_Z);
    if (undo_pressed && current_level_context->undo_actions.count > 0) {
        Array <Entity_Undo_Change> *changes = current_level_context->undo_actions.last();
        for_array(i, changes) {
            Entity_Undo_Change *change = changes->get(i);
            // Entity *changed_entity = get_entity(change->entity_id);
            // assert(get_entity(change->entity_id)); // Probably will break on destroying entites etc.
            
            switch (change->change_type) {
                case VECTOR2_CHANGE: {
                    *change->changed_vector -= change->vector_change;
                } break;
                case ENTITY_DESTROYED: {
                    Entity *restored_entity = copy_and_add_entity(change->destroyed_entity_copy, current_level_context, change->entity_id);
                } break;
                case NO_CHANGE: { 
                    assert(false);
                } break;
            }
        }
        
        current_level_context->undo_actions.count -= 1;
    }
    
    // Redo logic.
    b32 redo_required_helper_keys_down = IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL);
    b32 redo_pressed = redo_required_helper_keys_down && IsKeyPressed(KEY_Z);
    b32 can_make_redo = editor.max_undos_added > current_level_context->undo_actions.count;
    if (redo_pressed && can_make_redo) {
        Array <Entity_Undo_Change> *changes = current_level_context->undo_actions.increase_count_and_get_last();
        for_array(i, changes) {
            Entity_Undo_Change *change = changes->get(i);
            
            assert(get_entity(change->entity_id)); 
            switch(change->change_type) {
                case VECTOR2_CHANGE: {
                    *change->changed_vector += change->vector_change;
                } break;
                case ENTITY_DESTROYED: {
                    Entity *to_destroy_again = get_entity(change->entity_id);
                    mark_entity_destroyed(to_destroy_again);
                } break;
            }
        }
    }
}
