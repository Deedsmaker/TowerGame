#pragma once

void add_changes_to_undo(Array <Entity_Undo_Change> *changes) {
    assert(changes->count > 0);
    current_level_context->undo_actions.append(*changes);
    current_level_context->max_undos_added = current_level_context->undo_actions.count;
}

Array <Entity_Undo_Change> get_entities_difference(Entity *changed, Entity *original, Allocator *allocator = HEAP_ALLOCATOR) {
    // @LEAK Probably. Could think about using undo level context memory arena in undo_actions. Could work if we'll reuse things.
    Array <Entity_Undo_Change> changes = {.allocator = allocator};

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
    if (original->scale != changed->scale) {
        changes.append({
            .entity_id = changed->id,
            .change_type = SCALE_CHANGE,
            .vector_change = changed->scale - original->scale,
        });
    }
    if (original->rotation != changed->rotation) {
        changes.append({
            .entity_id = changed->id,
            .change_type = ROTATION_CHANGE,
            .float_change = changed->rotation - original->rotation,
        });
    }
    if (original->draw_order != changed->draw_order) {
        changes.append({
            .entity_id = changed->id,
            .change_type = INTEGER_CHANGE,
            .integer_change = changed->draw_order - original->draw_order,
            .changed_integer = &changed->draw_order
        });
    }
    
    if (changed->flags & TRIGGER) {
        if (changed->trigger->connected.count > original->trigger->connected.count) {
            changes.append({
                .entity_id = changed->id,
                .change_type = ARRAY_APPENDED,
                .number_appended = changed->trigger->connected.last_value(),
                .changed_array = &changed->trigger->connected
            });
        } else if (changed->trigger->connected.count < original->trigger->connected.count) {
            i32 removed_number = 0;            
            for_array(i, &original->trigger->connected) {
                if (i >= changed->trigger->connected.count) {
                    removed_number = original->trigger->connected.get_value(i);
                    break;
                }
                if (changed->trigger->connected.get_value(i) != original->trigger->connected.get_value(i)) {
                    removed_number = original->trigger->connected.get_value(i);
                    break;
                }
            }
            
            assert(removed_number > 0); // That's because in case of triggers we're talking about entities ids.
            
            changes.append({
                .entity_id = changed->id,
                .change_type = ARRAY_REMOVED,
                .number_removed = removed_number,
                .changed_array = &changed->trigger->connected
            });
        }
    }
    
    return changes;
}

inline void update_undo_logic() {
    // First of all detecting changed entities to add them to undo actions.
    
    if (editor.just_deleted_entity) {
        editor.just_deleted_entity = false;
        // In case of deleting entity we want to go through all of entities and look at entities that might 
        // reffer to deleted one and mark them changed aswell, so for example trigger will detect that 
        // one of his connected is will be destroyed -> mark itself as changed and all of it's changed 
        // actions will be in one undo.
        b32 found_one_that_will_be_destroyed = false;
        
        Array <Entity_Undo_Change> changes = {.allocator = HEAP_ALLOCATOR};
        
        for_chunk_array(i, &current_level_context->entities) {
            Entity *entity = current_level_context->entities.get(i);
            if (entity->will_be_destroyed) found_one_that_will_be_destroyed = true;
            Entity *unchanged_entity = copy_and_add_entity(entity, &undo_level_context);
            
            // On verifying trigger and kill switch will detect if someone will be destroyed -> will remive
            // it from an array and mark itself as EDITOR_CHANGED.
            b32 removed_from_trigger = verify_trigger_connected(entity);
            b32 removed_from_kill_switch = verify_kill_switch_connected(entity);
            
            if (entity->runtime_only_flags & EDITOR_CHANGED) {
                entity->runtime_only_flags ^= EDITOR_CHANGED;

                auto entity_change = get_entities_difference(entity, unchanged_entity, temp);
                changes.append_another_array(&entity_change);
            }
            
            free_entity(unchanged_entity);
        }
        
        assert(changes.count > 0);
        add_changes_to_undo(&changes);
        
        assert(found_one_that_will_be_destroyed);
    } else if (editor.just_spawned_entities_ids.count > 0) {
        Array <Entity_Undo_Change> changes = {.allocator = HEAP_ALLOCATOR};
        for_array(i, &editor.just_spawned_entities_ids) {
            Entity *spawned = get_entity(editor.just_spawned_entities_ids.get_value(i));    
            
            changes.append({
                .entity_id = spawned->id,
                .change_type = ENTITY_SPAWNED,
                .spawned_entity_copy = copy_and_add_entity(spawned, &undo_level_context)
            });
        }
        
        add_changes_to_undo(&changes);
        editor.just_spawned_entities_ids.clear();
    } else if (editor.multiselection.entities.count > 0) {
        assert(editor.multiselection.entities.count == editor.multiselection.unchanged_copies.count);
        if (get_entity(editor.multiselection.entities.get_value(0))->runtime_only_flags & EDITOR_CHANGED) {
            Array <Entity_Undo_Change> changes = {.allocator = HEAP_ALLOCATOR};
            
            for_array(i, &editor.multiselection.entities) {
                Entity *entity = get_entity(editor.multiselection.entities.get_value(i));
                entity->runtime_only_flags ^= EDITOR_CHANGED;
                Entity *unchanged  = editor.multiselection.unchanged_copies.get_value(i);
                auto entity_changes = get_entities_difference(entity, unchanged, temp);
                changes.append_another_array(&entity_changes);
                free_entity(unchanged);
                
                editor.multiselection.unchanged_copies.insert(copy_and_add_entity(entity, &undo_level_context), i);               
            }
            
            assert(changes.count > 0);
            add_changes_to_undo(&changes);
        }
    } else if (editor.selected && editor.selected->runtime_only_flags & EDITOR_CHANGED) {
        editor.selected->runtime_only_flags ^= EDITOR_CHANGED;
        
        Array <Entity_Undo_Change> changes = get_entities_difference(editor.selected, editor.selected_unchanged_copy);
        
        add_changes_to_undo(&changes);
        
        // Here we're updating unchanged copy of selected entity because we've stored all the information that we wanted.
        free_entity(editor.selected_unchanged_copy);
        editor.selected_unchanged_copy = copy_and_add_entity(editor.selected, &undo_level_context);
    }

    // Undo logic.
    b32 undo_required_helper_keys_down = !IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL);
    b32 undo_pressed = undo_required_helper_keys_down && IsKeyPressed(KEY_Z);
    if (undo_pressed && current_level_context->undo_actions.count > 0) {
        Array <Entity_Undo_Change> *changes = current_level_context->undo_actions.last();
        for_array(i, changes) {
            Entity_Undo_Change *change = changes->get(i);
            // Entity *changed_entity = get_entity(change->entity_id);
            
            switch (change->change_type) {
                case VECTOR2_CHANGE: {
                    *change->changed_vector -= change->vector_change;
                } break;
                case FLOAT_CHANGE: {
                    *change->changed_float -= change->float_change;
                } break;
                case INTEGER_CHANGE: {
                    *change->changed_integer -= change->integer_change;
                } break;
                case SCALE_CHANGE: {
                    add_scale(get_entity(change->entity_id), change->vector_change * -1);
                } break;
                case ROTATION_CHANGE: {
                    rotate(get_entity(change->entity_id), change->float_change * -1);
                } break;
                case ENTITY_DESTROYED: {
                    Entity *restored_entity = copy_and_add_entity(change->destroyed_entity_copy, current_level_context, change->entity_id);
                } break;
                case ENTITY_SPAWNED: {
                    Entity *to_destroy = get_entity(change->entity_id);
                    mark_entity_destroyed(to_destroy);
                } break;
                case ARRAY_APPENDED: {
                    Entity *entity = get_entity(change->entity_id);    
                    i32 appended_index = change->changed_array->find(change->number_appended);
                    assert(appended_index >= 0);
                    change->changed_array->remove(appended_index);
                } break;
                case ARRAY_REMOVED: {
                    // In case of arrays we currently don't care about placement of values. That means that even if 
                    // value was removed from the middle of a array - we'll insert it at the end.
                    Entity *entity = get_entity(change->entity_id);  
                    i32 removed_value = change->number_removed;
                    change->changed_array->append(removed_value);
                } break;
                case NO_CHANGE: { 
                    assert(false);
                } break;
                default: {
                    printf("Forgot some change type in undo!\n");
                } break;
            }
            
            Entity *changed_entity = get_entity(change->entity_id);
            // i == 0 check is because we want to select main affected entity and next ones could be just another entities 
            // that detected changes in this entity and recorded undo.
            if (!changed_entity->will_be_destroyed && i == 0) assign_selected_entity(get_entity(change->entity_id)); 
            
            // We're updating unchanged copy so that we for sure would have latest unchanged copy. 
            // In other case it will cause issues if, for example, we would undo something (scaling for example)
            // and then decided to move entity - in this case unchanged copy would be of entity that we've scaled last time,
            // because we're updated this only on adding undo action.
            if (editor.selected) {
                free_entity(editor.selected_unchanged_copy);   
                editor.selected_unchanged_copy = copy_and_add_entity(editor.selected, &undo_level_context);
            }
        }
        
        current_level_context->undo_actions.just_decrease_count();
    }
    
    // Redo logic.
    b32 redo_required_helper_keys_down = IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL);
    b32 redo_pressed = redo_required_helper_keys_down && IsKeyPressed(KEY_Z);
    b32 can_make_redo = current_level_context->max_undos_added > current_level_context->undo_actions.count;
    if (redo_pressed && can_make_redo) {
        Array <Entity_Undo_Change> *changes = current_level_context->undo_actions.increase_count_and_get_last();
        for_array(i, changes) {
            Entity_Undo_Change *change = changes->get(i);
            
            switch(change->change_type) {
                case VECTOR2_CHANGE: {
                    *change->changed_vector += change->vector_change;
                } break;
                case FLOAT_CHANGE: {
                    *change->changed_float += change->float_change;
                } break;
                case INTEGER_CHANGE: {
                    *change->changed_integer += change->integer_change;
                } break;
                case SCALE_CHANGE: {
                    add_scale(get_entity(change->entity_id), change->vector_change);
                } break;
                case ROTATION_CHANGE: {
                    rotate(get_entity(change->entity_id), change->float_change);
                } break;
                case ENTITY_DESTROYED: {
                    Entity *to_destroy = get_entity(change->entity_id);
                    mark_entity_destroyed(to_destroy);
                } break;
                case ENTITY_SPAWNED: {
                    Entity *restored_entity = copy_and_add_entity(change->spawned_entity_copy, current_level_context, change->entity_id);
                } break;
                case ARRAY_APPENDED: {
                    Entity *entity = get_entity(change->entity_id);  
                    i32 value_to_append = change->number_appended;
                    change->changed_array->append(value_to_append);
                } break;
                case ARRAY_REMOVED: {
                    Entity *entity = get_entity(change->entity_id);    
                    i32 index_to_remove = change->changed_array->find(change->number_removed);
                    assert(index_to_remove >= 0);
                    change->changed_array->remove(index_to_remove);
                } break;
                default: {
                    printf("Forgot some change type in redo!\n");
                } break;
            }
            
            Entity *changed_entity = get_entity(change->entity_id);
            // i == 0 explained above in undo.
            if (!changed_entity->will_be_destroyed && i == 0) assign_selected_entity(get_entity(change->entity_id)); 
            
            // Explained above in undo.
            if (editor.selected) {
                free_entity(editor.selected_unchanged_copy);   
                editor.selected_unchanged_copy = copy_and_add_entity(editor.selected, &undo_level_context);
            }
        }
    }
}
