#pragma once

inline void notify_destruction_to_connected_entities(Entity *entity) {
    // Notifying entities pointing at me and entities that we're connected to.

    for_array(i, (&entity->entities_pointing_at_me)) {
        // Entity *pointing_entity = get_entity
        // @TODO: Right now it think that we should keep that pointing_at_me thing and store in that array flag about type 
        // of pointing (trigger connected, trigger tracking, kill switch connected etc. basically for every type.
        // Here we will be checking in switch type of pointing where on default could be assert(false) and we will basically
        // know that we did not forget to add something. 
        // That will highly restrict us and maybe will add some friction, but we will know 
        // that at any given moment all entities have correct referring ids and get_entity will never return NULL.
        //
        // Also that means that responsibility to remove id from entity that pointing at me is completely on that function
        // and entities that actually pointing at me will always have valid ids.
        //
        // But don't forget that when entity that pointing at me is destroyed itself it should go to me and remove 
        // it's id from here.
        //
        // Actually right now I realized that we're not exactly *should* have type of pointing, because when entity is 
        // destroyed it should remove reference to itself from every instance of pointing, but type will help us 
        // know that we don't forget to check anything so we should try that.
        //
        // Maybe we could go without type - we will just check all we know and if we did not find anything where this 
        // entity id is stored - we're asserting. Thing that bugs me is that we really will not know how exactly this entity
        // is pointing at us and that maybe will create bugs. But maybe all of this we could catch with asserts.
        
        remove_connected_entity(get_entity(entity->entities_pointing_at_me.get_value(i)), entity->id);
    }
    
    // Now telling entiteis that we're connected to that we're no longer walking this earth.
    //
    // ALL connected entity ids, even ones that is present in trigger, should be duplicated in entity->connected_entities.
    // That would simplify things and I think it's not that much of a memory/performance hit.
    
    while (entity->connected_entities.count > 0) {
        // It's not just a for loop because nocheckin explain.
        if (remove_connected_entity(entity, entity->connected_entities.get_value(0))) {
        } else {
            // That's a fail case. That means that we forgot to handle something, but actually we'll probably still remove
            // id from connected entities, so we'll never be here. nocheckin explain better after implementation.
            break;
        }
    }
    
    //nocheckin also probably should notify particle emitters because they're holding references to entity.
    // Maybe we could think of doing it the other way and holding particle emitters references on entities.
    
    //nocheckin do checks for ids that's not stored in entity (like state context threat id or cam state rails trigger id).
    // But here we're losing verification that we don't forget anything. That could be fixed if we're going to decide ot add 
    // flags to pointing_at_me. That way we will be know for sure.
}

inline i32 register_entity_id_reference(Entity *entity, i32 connected_id) {
    return *entity->connected_entities.append(connected_id);
}

inline bool remove_connected_entity(Entity *entity, i32 to_remove) {
    b32 handled_all_references = true;
    for_array(id_index, (&entity->connected_entities)) {
        i32 id = entity->connected_entities.get_value(id_index);    
        if (id != to_remove) continue;
        
        b32 handled = false;
        
        if (!handled && entity->flags & TRIGGER) {
            i32 index = -1;
            if (0) {
            } else if ((index = entity->trigger.tracking.find(id)) != -1) {
                entity->trigger.tracking.remove(index);
                handled = true;
                break;
            } else if ((index = entity->trigger.tracking.find(id)) != -1) {
                entity->trigger.tracking.remove(index);
                handled = true;
                break;
            }
        }        
        if (!handled && entity->flags & KILL_SWITCH) {
            i32 index = -1;
            if ((index = entity->enemy.connected.find(id)) != -1) {
                entity->enemy.connected.remove(index);
                handled = true;
                break;
            }
        }        
        if (!handled && entity->flags & PLAYER) {
            if (0) {
            } else if (id == player_entity->connected_entities_ids.ground_checker_id) {         
                player_entity->connected_entities_ids.ground_checker_id = 0;    
                handled = true;
            } else if (id == player_entity->connected_entities_ids.left_wall_checker_id) {         
                player_entity->connected_entities_ids.left_wall_checker_id = 0;    
                handled = true;
            } else if (id == player_entity->connected_entities_ids.right_wall_checker_id) {         
                player_entity->connected_entities_ids.right_wall_checker_id = 0;    
                handled = true;
            } else if (id == player_entity->connected_entities_ids.sword_entity_id) {         
                player_entity->connected_entities_ids.sword_entity_id = 0;    
                handled = true;
            }

        }
        if (!handled && entity->flags & ENEMY) {
            if (0) {
            } else if (id == entity->enemy.blocker_sticky_id) {
                entity->enemy.blocker_sticky_id = 0;
                handled = true;
            } else if (id == entity->enemy.sword_required_sticky_id) {
                entity->enemy.sword_required_sticky_id = 0;
                handled = true;
            }
        }
        if (!handled && entity->flags & CENTIPEDE_SEGMENT) {
            if (0) {
            } else if (id == entity->centipede_segment.head_id) {
                entity->centipede_segment.head_id = 0;
                handled = true;
            } else if (id == entity->centipede_segment.previous_id) {
                entity->centipede_segment.previous_id = 0;
                handled = true;
            }
        }
        if (!handled && entity->flags & CENTIPEDE) {
            i32 index = -1;
            if (id == entity->centipede.segments_ids.get_value(i)) {
                entity->centipede.segments_ids.remove(i);
                handled = true;
                break;
        }
        if (!handled && entity->flags & STICKY_TEXTURE) {
            
        }
        
        if (!handled) {
            // That could mean that id is directly on entity struct (but we should change that stupidity).
            //
            // Also that could mean that id is stored directly in 
        }

        
        if (!handled) handled_all_references = false;
        
        entity->connected_entities.remove(i);
        i -= 1;
    }
}
