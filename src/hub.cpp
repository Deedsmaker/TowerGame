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
    
    context->hub.destroyed_entities.append({.entity = entity, .timer = 0}, &context->allocator);
}

void remember_killed_enemy(Context *context, Entity *entity) {
    auto allowed_flags = ENEMY;
    if (!(entity->flags & allowed_flags)) {
        return;
    }

    auto enemy = entity->union_enemy;
    
    if (!enemy->dead_man && !entity->will_be_destroyed) {
        log("On remembering killed enemy enemy was not, in fact, dead man and was not marked as will_be_destroyed. That's an error.", LOG_ERROR);
        return;
    }
    
    if (entity->flags & CENTIPEDE_SEGMENT) {
        // Currently we want to only revive only centipede segments, because they're marked as dead man and will not be destroyed  
        // until centipede itself is dead. 
        //
        // In case if this segment itself is marked as destroyed - we assume that centipede head was destroyed and also we assume that 
        // we will find this exact segment in array of enemies that we want to revive and we want to remove that segment from there,
        // because it will be created again when centipede will be copied.
        if (entity->will_be_destroyed) {
            bool found = false;
            For (&context->hub.enemies_to_revive) {
                if (it->entity == entity) {
                    found = true;
                    context->hub.enemies_to_revive.remove(i);
                    break;
                }
            }
            
            if (!found) {
                log("On recording destroyed entity centipede segment that is currently marked destroyed was not found in array of remembered dead enemies for revival. That should not happen because segment should be destroyed only when centipede itself is destroyed and that should happen only when every segment is dead.", LOG_ERROR);
            }
        } else {
            context->hub.enemies_to_revive.append({.entity = entity, .timer = 0}, &context->allocator);
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
