#pragma once

void update_hub(Context *context, f32 dt) {
    if (!(context->flags & HUB_CONTEXT)) {
        return;
    }
    
    auto hub = &context->hub;
    auto player = &context->player;
    
    if (player->sword_charges < player->max_sword_charges) {
        hub->sword_recharge_timer += dt;
        if (hub->sword_recharge_timer >= hub->SWORD_RECHARGE_TIME) {
            hub->sword_recharge_timer -= hub->SWORD_RECHARGE_TIME;
            player->sword_charges += 1;
            assert(player->sword_charges <= player->max_sword_charges);
        }
    }
    
    if (player->ammo < player->max_ammo) {
        hub->ammo_recharge_timer += dt;
        if (hub->ammo_recharge_timer >= hub->AMMO_RECHARGE_TIME) {
            hub->ammo_recharge_timer -= hub->AMMO_RECHARGE_TIME;
            player->ammo += 1;
            assert(player->ammo <= player->max_ammo);
        }
    }
    
    static const f32 HUB_ENTITY_RESTORE_TIME = 3.0f;
    for_array_backwards (i, &context->hub.destroyed_entities) {
        auto it = context->hub.destroyed_entities.get(i);
        
        it->timer += dt;
        if (it->timer >= HUB_ENTITY_RESTORE_TIME) {
            it->entity->enabled = true;
            remove_flag(&it->entity->runtime_only_flags, SHOULD_NOT_BE_DESTROYED);
            
            if (it->entity->flags & CENTIPEDE) {
                // for_array(j, 
            }
            
            context->hub.destroyed_entities.remove(i);
        }
    }
    
    for_array_backwards (i, &context->hub.enemies_to_revive) {
        auto it = context->hub.enemies_to_revive.get(i);
        assert(it->entity->union_enemy);
        
        it->timer += dt;
        if (it->timer >= HUB_ENTITY_RESTORE_TIME) {
            it->entity->union_enemy->dead_man = false;
            context->hub.enemies_to_revive.remove(i);
        }
    }
    
    for_array_backwards (centipede_index, &hub->damaged_centipedes) {
        auto it = hub->damaged_centipedes.get(centipede_index);
        
        it->timer += dt;
        if (it->timer >= HUB_ENTITY_RESTORE_TIME) {
            foreach (segment, &it->entity->centipede->segments) {
                (*segment)->centipede_segment->dead_man = false;
            }
            hub->damaged_centipedes.remove(centipede_index);
        }
    }
}

void disable_and_remember_entity_for_restoration(Context *context, Entity *entity) {
    // We don't copy entity here and instead just disabling it. 
    // For that to work we rely on flag SHOULD_NOT_BE_DESTROYED.

    entity->will_be_destroyed = false;
    entity->destroyed = false;
    if (entity->union_enemy) {
        entity->union_enemy->dead_man = false;
    }
    entity->enabled = false;
    entity->runtime_only_flags |= SHOULD_NOT_BE_DESTROYED;
    
    if (entity->flags & CENTIPEDE) {
        auto centipede = entity->centipede;
        For (&centipede->segments) {
            auto segment = *it;
            segment->will_be_destroyed = false;
            segment->destroyed = false;
            segment->enabled = false;
            segment->centipede_segment->dead_man = false;
            segment->runtime_only_flags |= SHOULD_NOT_BE_DESTROYED;
        }
    }
    
    context->hub.destroyed_entities.append({.entity = entity, .timer = 0}, &context->allocator);
}

void remember_killed_enemy(Context *context, Entity *entity) {
    auto hub = &context->hub;
    
    auto allowed_flags = ENEMY;
    if (!(entity->flags & allowed_flags)) {
        return;
    }

    auto enemy = entity->union_enemy;
    
    if (!enemy->dead_man && !entity->will_be_destroyed) {
        log("On remembering killed enemy enemy was not, in fact, dead man, and was not marked as will_be_destroyed. That's an error.", LOG_ERROR);
        return;
    }
    
    if (entity->flags & CENTIPEDE_SEGMENT) {
        // Segment marked as will_be_destroyed means that centipede itself will be destroyed and that case we handle separately.
        if (entity->will_be_destroyed) {
            return;
        }
        auto head = entity->centipede_segment->head; 
        
        bool found = false;
        // In case of centipede segments we have one timer for all segments. When one segment is damaged we reseting timer for centipede 
        // and when timer will be incremented enough - we will revive all damaged segments.
        For (&hub->damaged_centipedes) {
            if (it->entity == head) {
                it->timer = 0;
                found = true;
            }
        }
             
        if (!found) {
            hub->damaged_centipedes.append({.entity = head, .timer = 0}, &context->allocator);
        }
        
        return;
    }
    
    // Entity here presumably should be destroyed any second now, so we will copy it and restore when the time comes.
    if (!entity->will_be_destroyed) {
        log("On remembering destroyed enemy entity is actually is not marked as will_be_destroyed, so there's no reason to copy it for restoration.", LOG_ERROR);
        return;
    }
    
    disable_and_remember_entity_for_restoration(context, entity);
}
