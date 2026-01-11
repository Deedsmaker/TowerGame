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
}
