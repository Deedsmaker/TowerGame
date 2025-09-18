Entity_Undo_Change get_entities_difference(Entity *original, Entity *changed) {
    Entity_Undo_Change undo_change = {.entity_id = changed->id};
    if (original->position != changed->position) {
        undo_change.change_type = VECTOR2_CHANGE;
        undo_change.vector_change = changed->position - original->position;
        undo_change.changed_vector = &changed->position;
    }
    
    return undo_change;
}
