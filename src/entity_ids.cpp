#pragma once

inline bool remove_id_reference_from_entity(Entity *entity, i32 to_remove) {
    b32 handled_all_references = true;
    // Here we're going to remove *to_remove* id from entity->connected_entities and from every specific entity type id reference.
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
            } else if ((index = entity->trigger.tracking.find(id)) != -1) {
                entity->trigger.tracking.remove(index);
                handled = true;
            }
        }        
        if (!handled && entity->flags & KILL_SWITCH) {
            i32 index = -1;
            if ((index = entity->enemy.kill_switch.connected.find(id)) != -1) {
                entity->enemy.kill_switch.connected.remove(index);
                handled = true;
            }
        }        
        if (!handled && entity->flags & PLAYER) {
            if (0) {
            } else if (id == player_data->connected_entities_ids.ground_checker_id) {         
                player_data->connected_entities_ids.ground_checker_id = 0;    
                handled = true;
            } else if (id == player_data->connected_entities_ids.left_wall_checker_id) {         
                player_data->connected_entities_ids.left_wall_checker_id = 0;    
                handled = true;
            } else if (id == player_data->connected_entities_ids.right_wall_checker_id) {         
                player_data->connected_entities_ids.right_wall_checker_id = 0;    
                handled = true;
            } else if (id == player_data->connected_entities_ids.sword_entity_id) {         
                player_data->connected_entities_ids.sword_entity_id = 0;    
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
            if ((index = entity->centipede.segments_ids.find(id)) != -1) {
                entity->centipede.segments_ids.remove(index);
                handled = true;
            }
        }
        if (!handled && entity->flags & STICKY_TEXTURE) {
            if (id == entity->sticky_texture.follow_id) {            
                entity->sticky_texture.follow_id = 0;
                handled = true;
            }
        }
        
        if (!handled) {
            // That could mean that id is directly on entity struct (but we should change that stupidity).
            if (id == entity->spawned_enemy_id) {
                entity->spawned_enemy_id = 0;
                handled = true;
            }
        }

        
        if (!handled) handled_all_references = false;
        
        entity->connected_entities.remove(id_index);
        id_index -= 1;
    }
    
    // Also if *to_remove* is pointing at this entity - we should remove it aswell (Because it's not exist anymore!).
    for_array(i, (&entity->entities_pointing_at_me)) {
        i32 id = entity->entities_pointing_at_me.get_value(i);
        if (id != to_remove) continue;
        
        entity->entities_pointing_at_me.remove(i);
        i--;
    }
    
    if (!handled_all_references) {
        printf("Did not handle some reference in id system removal! Everything should work for some time, but some id was not cleared and now invalid.\n");
    }
    
    return handled_all_references;
}

inline void notify_destruction_to_connected_entities(Entity *destroyed_entity) {
    // Notifying entities pointing at me and entities that we're connected to.

    // Here we're telling entities that pointing at destroyed_entity that it's no longer alive. They will remove 
    // reference of this entity id from every instance of how they're storing this entity id.
    for_array(i, (&destroyed_entity->entities_pointing_at_me)) {
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
        
        remove_id_reference_from_entity(get_entity(destroyed_entity->entities_pointing_at_me.get_value(i)), destroyed_entity->id);
    }
    
    // Now telling entiteis that we're connected to that we're no longer walking this earth.
    //
    // ALL connected entity ids, even ones that is present in trigger, should be duplicated in destroyed_entity->connected_entities.
    // That would simplify things and I think it's not that much of a memory/performance hit.
    
    while (destroyed_entity->connected_entities.count > 0) {
        // It's not just a for loop because nocheckin explain.
        if (remove_id_reference_from_entity(destroyed_entity, destroyed_entity->connected_entities.get_value(0))) {
        } else {
            // That's a fail case. That means that we forgot to handle something, but actually we'll probably still remove
            // id from connected entities, so we'll never be here. nocheckin explain better after implementation.
        }
    }
    
    //nocheckin also probably should notify particle emitters and lights because they're holding references to entity.
    // Maybe we could think of doing it the other way and holding particle emitters references on entities.
    
    //nocheckin do checks for ids that's not stored in entity (like state context threat id or cam state rails trigger id).
    // But here we're losing verification that we don't forget anything. That could be fixed if we're going to decide ot add 
    // flags to pointing_at_me. That way we will be know for sure.
}

inline i32 register_entity_id_reference(Entity *entity, i32 connected_id) {
    assert(entity->id != connected_id);
    //nocheckin clear all stuff on free_entity
    get_entity(connected_id)->entities_pointing_at_me.append(entity->id);
    return *entity->connected_entities.append(connected_id);
}

inline void unregister_one_entity_id_reference(Entity *entity, i32 id) {
    assert(entity->id != id);   
    
    Entity *currently_connected_entity = get_entity(id);
    currently_connected_entity->entities_pointing_at_me.remove_first_encountered(entity->id);
    
    entity->connected_entities.remove_first_encountered(id);
}

inline void unregister_entity_ids_reference(Entity *entity, Array<i32> *ids) {
    assert(!ids->contains(entity->id));   
    
    Entity *currently_connected_entity = get_entity(id);
    currently_connected_entity->entities_pointing_at_me.remove_first_encountered(entity->id);
    
    entity->connected_entities.remove_first_encountered(id);
}
