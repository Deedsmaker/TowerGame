enum Undo_Change_Type {
    NO_CHANGE       = 0,       
    VECTOR2_CHANGE = 1,
    FLOAT_CHANGE   = 2,
    INTEGER_CHANGE = 3,
};

struct Entity_Undo_Change {
    i32 entity_id = 0;
    Undo_Change_Type change_type = NO_CHANGE;
    
    union {  
        Vector2 vector_change;
        f32 float_change;
        i32 integer_change;
    };
    
    union {
        Vector2 *changed_vector;
        f32 *changed_float;
        i32 *changed_integer;
    };
};

Entity_Undo_Change get_entities_difference(Entity *original, Entity *changed) {
    Entity_Undo_Change undo_change = {.entity_id = changed->id};
    if (original->position != changed->position) {
        undo_change.change_type = VECTOR2_CHANGE;
        undo_change.vector_change = changed->position - original->position;
        undo_change.changed_vector = &changed->position;
    }
    
    return undo_change;
}
