#pragma once

// It's a buffer that entities uses when finding collision cells that they're in (in fill_collisions nad fill_affected_collision_cells).
global_variable Array <Collision_Grid_Cell*> collision_cells_buffer = {0};

inline void fill_arr_with_normals(Array <Vector2> *normals, Static_Array <Vector2, MAX_VERTICES> vertices) {
    //@INCOMPLETE now only for rects and triangles, need to find proper algorithm for calculating edge normals from vertices because 
    //we add vertices in triangle shape
    // Update 03.03.2025: Graham scan algorithm should do the job if we will really need it.
    
    if (vertices.count == 4) {
        //up
        Vector2 edge1 = vertices.get_value(0) - vertices.get_value(1);
        normals->append(normalized(get_rotated_vector_90(edge1, 1)));
        //left
        Vector2 edge2 = vertices.get_value(1) - vertices.get_value(3);
        normals->append(normalized(get_rotated_vector_90(edge2, 1)));
        //bottom
        Vector2 edge3 = vertices.get_value(3) - vertices.get_value(2);
        normals->append(normalized(get_rotated_vector_90(edge3, 1)));
        //right
        Vector2 edge4 = vertices.get_value(2) - vertices.get_value(0);
        normals->append(normalized(get_rotated_vector_90(edge4, 1)));
    } else if (vertices.count == 3) {
        Vector2 edge1 = vertices.get_value(0) - vertices.get_value(1);
        normals->append(normalized(get_rotated_vector_90(edge1, 1)));
        
        Vector2 edge2 = vertices.get_value(1) - vertices.get_value(2);
        normals->append(normalized(get_rotated_vector_90(edge2, 1)));
        
        Vector2 edge3 = vertices.get_value(2) - vertices.get_value(0);
        normals->append(normalized(get_rotated_vector_90(edge3, 1)));
    } else {
        assert(false);
    }
}

inline b32 check_rectangles_collision(Vector2 pos1, Vector2 scale1, Vector2 pos2, Vector2 scale2) {
    b32 solution = pos1.x + scale1.x * 0.5f > pos2.x - scale2.x * 0.5f &&
                   pos1.x - scale1.x * 0.5f < pos2.x + scale2.x * 0.5f &&
                   pos1.y + scale1.y * 0.5f > pos2.y - scale2.y * 0.5f &&
                   pos1.y - scale1.y * 0.5f < pos2.y + scale2.y * 0.5f;
                   
    return solution;
}

inline b32 check_bounds_collision(Vector2 pos1, Vector2 pos2, Bounds bounds1, Bounds bounds2, Vector2 pivot1, Vector2 pivot2) {
    Vector2 pivot_add1 = multiply(pivot1, bounds1.size);
    Vector2 position1 = pos1 + bounds1.offset;
    //firstly for left up
    Vector2 with_pivot_pos1 = {position1.x - pivot_add1.x, position1.y + pivot_add1.y};
    Vector2 final1 = {with_pivot_pos1.x + bounds1.size.x * 0.5f, with_pivot_pos1.y - bounds1.size.y * 0.5f};
    
    Vector2 pivot_add2 = multiply(pivot2, bounds2.size);
    Vector2 position2 = pos2 + bounds2.offset;
    //firstly for left up
    Vector2 with_pivot_pos2 = {position2.x - pivot_add2.x, position2.y + pivot_add2.y};
    Vector2 final2 = {with_pivot_pos2.x + bounds2.size.x * 0.5f, with_pivot_pos2.y - bounds2.size.y * 0.5f};
    
    
    return check_rectangles_collision(final1, bounds1.size, final2, bounds2.size);
}

inline b32 check_bounds_collision(Entity *entity1, Entity *entity2) {
    return check_bounds_collision(entity1->position, entity2->position, entity1->bounds, entity2->bounds, entity1->pivot, entity2->pivot);
}

inline b32 check_bounds_collision(Vector2 position1, Bounds bounds1, Entity *entity2) {
    Vector2 pivot_add2 = multiply(entity2->pivot, entity2->bounds.size);
    Vector2 position2 = entity2->position + entity2->bounds.offset;
    //firstly for left up
    Vector2 with_pivot_pos2 = {position2.x - pivot_add2.x, position2.y + pivot_add2.y};
    Vector2 final2 = {with_pivot_pos2.x + entity2->bounds.size.x * 0.5f, with_pivot_pos2.y - entity2->bounds.size.y * 0.5f};
    
    return check_rectangles_collision(position1 + bounds1.offset, bounds1.size, final2, entity2->bounds.size);
}

Collision check_collision(Vector2 position1, Vector2 position2, Static_Array <Vector2, MAX_VERTICES> vertices1, Static_Array <Vector2, MAX_VERTICES> vertices2, Vector2 pivot1 = {0.5f, 0.5f}, Vector2 pivot2 = {0.5f, 0.5f}) {
    Collision result = {0};
    
    Bounds bounds1 = get_bounds(vertices1, pivot1);
    Bounds bounds2 = get_bounds(vertices2, pivot2);
    if (!check_bounds_collision(position1, position2, bounds1, bounds2, pivot1, pivot2)) {
        return result;
    }

    global_normals.clear();
    fill_arr_with_normals(&global_normals, vertices1);
    fill_arr_with_normals(&global_normals, vertices2);
    
    f32 overlap = INFINITY;
    Vector2 min_overlap_axis = Vector2_zero;
    
    Vector2 min_overlap_projection = {0};

    for (i32 i = 0; i < global_normals.count; i++) {
        Vector2 projections[2];
        //x - min, y - max
        projections[0].x =  INFINITY;
        projections[1].x =  INFINITY;
        projections[0].y = -INFINITY;
        projections[1].y = -INFINITY;
        
        Vector2 axis = global_normals.get_value(i);

        for (i32 shape = 0; shape < 2; shape++) {
            Static_Array <Vector2, MAX_VERTICES> vertices;
            Vector2 position;
            if (shape == 0) {
                vertices = vertices1;
                position = position1;
            } else {
                vertices = vertices2;
                position = position2;
            }
            
            for (i32 j = 0; j < vertices.count; j++) {            
                f32 p = dot(global_position(position, vertices.get_value(j)), axis);
                
                f32 min = fmin(projections[shape].x, p);
                f32 max = fmax(projections[shape].y, p);
                
                projections[shape].x = min;
                projections[shape].y = max;
            }
        }
        
        f32 new_overlap = fmin(fmin(projections[0].y, projections[1].y) - fmax(projections[0].x, projections[1].x), overlap);
        if (new_overlap != overlap) {
            overlap = new_overlap;
            min_overlap_axis = axis;
            min_overlap_projection.x = projections[0].x;
            min_overlap_projection.y = projections[0].y;
        }
        
        if (!(projections[1].y >= projections[0].x && projections[0].y >= projections[1].x)) {
            return result;
        }
    }
    
    Vector2 vec_to_first = position1 - position2;
    
    result.collided = true;
    result.overlap = overlap;
    result.dir_to_first = normalized(vec_to_first);
    result.normal = dot(result.dir_to_first, min_overlap_axis) > 0 ? min_overlap_axis : min_overlap_axis * -1.0f;
    result.point = position1 - result.normal * ((min_overlap_projection.y - min_overlap_projection.x) * 0.5f);
    
    return result;
}

Collision check_entities_collision(Entity *entity1, Entity *entity2) {
    Collision result = check_collision(entity1->position, entity2->position, entity1->vertices, entity2->vertices, entity1->pivot, entity2->pivot);
    result.other_entity = entity2;
    
    return result;
}

void resolve_collision(Entity *entity, Collision col) {
    if (col.collided) {
        entity->position += col.normal * col.overlap;
    }
}

Collision_Grid_Cell *get_collision_cell_from_position(Vector2 position) {
    Collision_Grid *grid = &current_context->collision_grid;    
    
    Vector2 origin_to_pos = position - grid->origin;
    
    i32 max_columns = (i32)(grid->size.x / grid->cell_size.x);
    
    i32 column = floor(((origin_to_pos.x + grid->size.x * 0.5f) / grid->cell_size.x));
    i32 row    = floor(((origin_to_pos.y + grid->size.y * 0.5f) / grid->cell_size.y));
    
    if (column < 0 || column >= max_columns || row < 0 || row >= (i32)(grid->size.y / grid->cell_size.y)) {
        // We just don't want to assert because that's just could happen that projectile goes too far beyond reach of a collision
        // grid, so everyone who calls it should check for null.
        return NULL;
    }
    
    Collision_Grid_Cell *cell = grid->cells.get(column + row * max_columns);
    return cell;
}

void fill_affected_collision_cells(Vector2 position, Static_Array <Vector2, MAX_VERTICES> vertices, Bounds bounds, Vector2 pivot, Array <Collision_Grid_Cell*> *out_cells) {
    out_cells->clear();
    Collision_Grid grid = current_context->collision_grid;
    Vector2 center = position + bounds.offset;
    center += {(0.5f - pivot.x) * bounds.size.x, (pivot.y - 0.5f) * bounds.size.y};
    
    // In this for loops we go left to right | bottom to top and it doesn't cover right side, so in loop after we cover fully right side.
    for (f32 h_pos = center.x - bounds.size.x * 0.5f; h_pos < center.x + bounds.size.x * 0.5f; h_pos += grid.cell_size.x) {
        for (f32 v_pos = center.y - bounds.size.y * 0.5f; v_pos < center.y + bounds.size.y * 0.5f; v_pos += grid.cell_size.y) {    
            Collision_Grid_Cell *cell = get_collision_cell_from_position({h_pos, v_pos});
            if (cell) {
                out_cells->append(cell);
            }
        }
        
        Collision_Grid_Cell *cell = get_collision_cell_from_position({h_pos, center.y + bounds.size.y * 0.5f});
        if (cell) {
            out_cells->append(cell);
        }
    }
    
    for (f32 v_pos = center.y - bounds.size.y * 0.5f; v_pos < center.y + bounds.size.y * 0.5f; v_pos += grid.cell_size.y) {
        Collision_Grid_Cell *cell = get_collision_cell_from_position({center.x + bounds.size.x * 0.5f, v_pos});
        if (cell) {
            out_cells->append(cell);
        }
    }
    
    Collision_Grid_Cell *cell = get_collision_cell_from_position({center.x + bounds.size.x * 0.5f, center.y + bounds.size.y * 0.5f});
    if (cell) {
        out_cells->append(cell);
    }
}

inline b32 is_entity_static(Entity *entity) {
    return entity->flags == GROUND || entity->flags == TEXTURE;
}

inline void update_entity_collision_cells(Entity *entity, b32 update_cells_for_static_entities) {
    b32 is_static = is_entity_static(entity);
    if (!update_cells_for_static_entities && is_static) {
        return;
    }

    fill_affected_collision_cells(entity->position, entity->vertices, entity->bounds, entity->pivot, &collision_cells_buffer);    
    
    assert(!entity->will_be_destroyed);
    
    for (i32 i = 0; i < collision_cells_buffer.count; i++) {
        Collision_Grid_Cell *cell = collision_cells_buffer.get_value(i);
        Array <i32> *cell_entities = is_static ? &cell->static_entities : &cell->dynamic_entities;
        
        if (cell) {
            cell_entities->append(entity->id);
        }
    }
}

global_variable Array <i32> added_collision_ids = {0};

void fill_collisions(Vector2 position, Static_Array <Vector2, MAX_VERTICES> vertices, Bounds bounds, Vector2 pivot, Array <Collision> *result, FLAGS include_flags, i32 my_id) {
    result->clear();
    
    fill_affected_collision_cells(position, vertices, bounds, pivot, &collision_cells_buffer);
    added_collision_ids.clear();
    
    for (i32 i = 0; i < collision_cells_buffer.count; i++) {
        Collision_Grid_Cell *cell = collision_cells_buffer.get_value(i);
        
        // Here we just combine static and dynamic entities.
        Array <i32> entities_in_cell = {.allocator = temp};
        entities_in_cell.append_another_array(&cell->dynamic_entities);
        entities_in_cell.append_another_array(&cell->static_entities);
        
        for (i32 c = 0; c < entities_in_cell.count; c++) {
            Entity *other = get_entity(entities_in_cell.get_value(c));
            
            if (other->flags <= 0 || ((other->flags & include_flags) <= 0 && include_flags > 0) || (other->hidden && editor_state == GAME && !state_context.in_pause_editor) || other->id == my_id || added_collision_ids.contains(other->id) || other->will_be_destroyed) {
                continue;
            }
            
            Collision col = check_collision(position, other->position, vertices, other->vertices, pivot, other->pivot);
            
            if (col.collided) {
                added_collision_ids.append(other->id);
                col.other_entity = other;
                result->append(col);
            }
        }
    }
}

void fill_collisions(Entity *entity, Array <Collision> *result, FLAGS include_flags) {
    if (entity->destroyed || !entity->enabled) {
        return;
    }
    
    fill_collisions(entity->position, entity->vertices, entity->bounds, entity->pivot, result, include_flags, entity->id);
}

Array <Collision> get_tcollisions(Entity *entity, FLAGS include_flags) {
    Array <Collision> collisions = {.allocator = temp};
    fill_collisions(entity, &collisions, include_flags);
    return collisions;
}

void fill_collisions_rect(Vector2 position, Vector2 scale, Vector2 pivot, Array <Collision> *result, FLAGS include_flags) {
    Static_Array <Vector2, MAX_VERTICES> vertices = Static_Array <Vector2, MAX_VERTICES>();
    add_rect_vertices(&vertices, pivot);    
    for (i32 i = 0; i < vertices.count; i++) {
        vertices.get(i)->x *= scale.x;
        vertices.get(i)->y *= scale.y;
    }
    Bounds bounds = get_bounds(vertices, pivot);
    bounds.offset = Vector2_zero;
    make_rect_lines(position, bounds.size, pivot, 5, RED);
    
    
    fill_collisions(position, vertices, bounds, pivot, result, include_flags);   
}

