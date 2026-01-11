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
    
    for_array_backwards (i, &context->hub.destroyed_entities) {
        auto it = context->hub.destroyed_entities.get(i);
        
        it->timer += dt;
        static const f32 HUB_ENTITY_RESTORE_TIME = 3.0f;
        if (it->timer >= HUB_ENTITY_RESTORE_TIME) {
            it->entity->enabled = true;
            context->hub.destroyed_entities.remove(i);
        }
    }
}

void copy_and_remember_entity_for_restoration(Context *context, Entity *entity) {
    // NOTE: Currently we don't keep the id of destroyed entity, so any entity that was pointing to that one will be lost.
    // If that will be needed we'll think about it.
    auto remember_me = copy_and_add_entity(entity, context);
    
    remember_me->will_be_destroyed = false;
    remember_me->destroyed = false;
    if (remember_me->union_enemy) {
        remember_me->union_enemy->dead_man = false;
    }
    remember_me->enabled = false;
    
    context->hub.destroyed_entities.append({.entity = remember_me, .timer = 0}, &context->allocator);
}

void remember_destroyed_or_killed_entity(Context *context, Entity *entity) {
    auto enemy = entity->union_enemy;
    if (!enemy) {        
        if (!entity->will_be_destroyed) {
            log("In remember_destroyed_or_killed_entity passed entity was not enemy and will not be destroyed. So that's an error.", LOG_ERROR);
            return;
        }
        // It's not a enemy so we're gonna just record it for restoration later. 
        
        copy_and_remember_entity_for_restoration(context, entity);
        return;
    }
    
    if (!enemy->dead_man) {
        log("On remembering killed enemy enemy was not, in fact, dead man. That's an error.", LOG_ERROR);
        return;
    }
    
    bool should_only_revive_enemy = entity->flags & CENTIPEDE_SEGMENT;
    if (should_only_revive_enemy) {
        // @TODO: 
        return;
    }
    
    // Entity here presumably should be destroyed any second now, so we will copy it and restore when the time comes.
    if (!entity->will_be_destroyed) {
        log("On remembering destroyed enemy entity is actually is not marked as will_be_destroyed, so there's no reason to copy it for restoration.", LOG_ERROR);
        return;
    }
    
    copy_and_remember_entity_for_restoration(context, entity);
}
