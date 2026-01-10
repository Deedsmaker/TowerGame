#pragma once

void player_validate_stats_by_game_save(Player *player) {
    auto save = &global_data.game_save;
    
    player->max_sword_charges = save->sword_charges_collected;
    player->sword_charges = player->max_sword_charges;
    
    player->max_ammo = save->ammo_collected + player->BASE_AMMO;
    player->ammo = player->max_ammo;
}

void add_blood_amount(Player *player, f32 added) {
    player->blood_amount += added;
    clamp(&player->blood_amount, 0, player->max_blood_amount);
    player->blood_progress = player->blood_amount / player->max_blood_amount;
}

// void set_sword_velocity(f32 value) {
//     player_data->sword_angular_velocity = value;
//     player_data->sword_spin_direction = normalized(player_data->sword_angular_velocity);
//     f32 sword_max_spin_speed = 5000;
//     player_data->sword_spin_progress = clamp01(abs(player_data->sword_angular_velocity) / sword_max_spin_speed);
// }

inline void add_player_ammo(i32 amount) {
    player_data->ammo += amount;
    
    player_data->ammo = clamp(player_data->ammo, 0, 3333);
    
    if (player_data->ammo == 0 && amount < 0) {
        player_data->timers.last_bullet_shot_time = current_context->game_time;
    }
}

inline b32 is_sword_can_damage() {
    return player_data->sword_mode == SWORD_MODE && !is_player_in_stun(current_context->player_entity);
}

inline b32 can_damage_blocker(Entity *blocker_entity) {
    assert(blocker_entity->union_enemy);
    return is_sword_can_damage() && !blocker_entity->union_enemy->blocker_immortal && (blocker_entity->union_enemy->blocker_clockwise ? player_data->sword_spin_direction > 0 : player_data->sword_spin_direction < 0);
}

inline b32 can_damage_sword_size_required_enemy(Entity *enemy_entity) {
    assert(enemy_entity->union_enemy);
    return is_sword_can_damage() && player_data->sword_mode == SWORD_MODE == enemy_entity->union_enemy->big_sword_killable;
}

inline b32 can_sword_damage_enemy(Entity *enemy_entity) {
    assert(enemy_entity->union_enemy);

    b32 sword_can_damage = is_sword_can_damage();
    b32 is_blocker_damageble = true;
    if (enemy_entity->flags & BLOCKER) {
        is_blocker_damageble = can_damage_blocker(enemy_entity);
    }
    b32 is_sword_size_required_damageble = true;
    if (enemy_entity->flags & SWORD_SIZE_REQUIRED) {
        is_sword_size_required_damageble = can_damage_sword_size_required_enemy(enemy_entity);
    }
    
    return sword_can_damage && ((is_blocker_damageble && is_sword_size_required_damageble) || enemy_entity->union_enemy->dead_man);
}

void sword_kill_enemy(Entity *enemy_entity, Vector2 *enemy_velocity) {
    assert(enemy_entity->union_enemy);

    Player *player_data = &current_context->player;

    Entity *sword = get_entity(player_data->connected_entities_ids.sword_entity_id);
    enemy_velocity->y = fmaxf(100.0f, 100.0f + enemy_velocity->y);
    enemy_velocity->x = player_data->sword_spin_direction * 50 + enemy_velocity->x;
    
    if (!enemy_entity->union_enemy->dead_man) {
        add_hitstop(0.1f);
    }
    
    Vector2 particles_direction = enemy_entity->up;
    if (sword_tip_ground_emitter_index != -1) {
        particles_direction = get_particle_emitter(sword_tip_ground_emitter_index)->direction;
    }
    // Should just do a enemy flag for serious enemies instead of picking everyone individualy.
    if (enemy_entity->flags & JUMP_SHOOTER) {
        kill_enemy(enemy_entity, sword->position + sword->up * sword->scale.y * sword->pivot.y, particles_direction, 1.0f);
        add_explosion_light(enemy_entity->position, 75, 0.03f, 0.1f, ColorBrightness(RED, 0.4f));
    } else {
        // Vector2 kill_position = sword->position + sword->up * sword->scale.y * sword->pivot.y;
        stun_enemy(enemy_entity, enemy_entity->position, particles_direction, true);
    }
}

// void start_sword_mode(Player *player_data) {
//     player_data->sword_mode_change_time = current_context->game_time;    
//     // player_data->max_speed_multiplier = 2.0f;
//     player_data->sword_mode = SWORD_MODE;
    
//     play_sound("SwordSwingBig", 0.9f, 1.0f, 0.05f);
    
//     assert(player_data->sword_charges > 0 && player_data->sword_charges <= player_data->max_sword_charges);
//     player_data->sword_charges -= 1;
    
//     remove_flag(&player_data->state_flags, HIT_CENTIPEDE_THIS_SPIN);
// }

void stop_sword_mode(Player *player_data) {
    // player_data->max_speed_multiplier = 1.0f;
    if (player_data->sword_mode != RIFLE_MODE ) {
        play_sound("SwordSwing", 0.9f, 1.5f, 0.05f);
    }
    player_data->sword_mode = RIFLE_MODE;
    player_data->to_spin_angle_amount = 0;
    
    remove_flag(&player_data->state_flags, SWORD_ATTACKING);
}

void flick_to_enemy(Entity *player_entity, Player *player, Entity *enemy_entity) {
    if (!enemy_entity->union_enemy) {
        log("On flick_to_enemy: enemy_entity does not have union_enemy on him.", LOG_ERROR);
        return;
    }
    
    if (enemy_entity->union_enemy->dead_man) {
        return;
    }
    
    make_line(player_entity->position, enemy_entity->position, 2.0f, Fade(RED, 0.5f), 0.5f);
    
    add_hitstop(0.5f);
    
    player_entity->position = enemy_entity->position;
    
    if (enemy_entity->flags & HIT_BOOSTER) {
        player->velocity = enemy_entity->up * 200;
        if (enemy_entity->up.x * player->sword_spin_direction < 0) {
            player->sword_spin_direction *= -1;
        }
    } else {
        player->velocity.y = 100;
    }
    
    player->last_flicked_enemy_flags = enemy_entity->flags;
    
    player->since_flick_timer = 0;
    
    if (player->sword_mode == SWORD_MODE) {
        player->this_attack_killed_count += 1;
        player->to_spin_angle_amount += 1000;
    }
}

void player_start_killing_centipede(Entity *segment_entity, Player *player) {
    assert(segment_entity->flags & CENTIPEDE_SEGMENT);
    
    // That would mean that we've hit the top segment, so no need to going in KILLING_CENTIPEDE state.
    if (segment_entity->centipede_segment->previous->flags & CENTIPEDE) { 
        return;
    }
    
    if (player->state_flags & KILLING_CENTIPEDE) {
        return;
    }
    
    if (player->state_flags & HIT_CENTIPEDE_THIS_SPIN) {
        return;
    }
    
    player->killing_centipede_timer = 0;
    
    flick_to_enemy(segment_entity->context->player_entity, player, segment_entity);

    player->state_flags |= HIT_CENTIPEDE_THIS_SPIN;
    
    player->state_flags |= KILLING_CENTIPEDE;
    player->state_flags |= PLAYER_INVINCIBLE;
    player->last_segment_kill_time = current_context->game_time;
    player->last_killed_segment = segment_entity;
}

void player_stop_killing_centipede(Player *player) {
    assert(player->state_flags & KILLING_CENTIPEDE);
    
    remove_flag(&player->state_flags, KILLING_CENTIPEDE);
    remove_flag(&player->state_flags, PLAYER_INVINCIBLE);
    
    player->killing_centipede_timer = 0;
    
    player->velocity = Vector2_up * 50;
    
    stop_sword_mode(player);
}

b32 try_sword_damage_enemy(Entity *enemy_entity, Vector2 hit_position) {
    if (!can_sword_damage_enemy(enemy_entity)) {
        return false;
    }
    
    if (!enemy_entity->union_enemy) {
        log("Tried to try_sword_damage_enemy, but there's no union_enemy on enemy_entity.", LOG_ERROR);
        return false;
    }
    
    Entity *player_entity = enemy_entity->context->player_entity;
    Player *player = &enemy_entity->context->player;

    b32 killed_enemy = false;
    if (is_sword_can_damage() && !is_player_in_stun(player_entity) && is_enemy_can_take_damage(enemy_entity)) {
                
        Enemy *enemy = enemy_entity->union_enemy;
                
        if (enemy_entity->flags & AMMO_PACK) {
            add_player_ammo(1);
        }
        add_blood_amount(player, 10);
    
        b32 was_alive_before_hit = !enemy->dead_man;
        f32 hitstop_add = 0;
        
        Vector2 particles_direction = enemy_entity->up;
        if (sword_tip_ground_emitter_index != -1) {
            particles_direction = get_particle_emitter(sword_tip_ground_emitter_index)->direction;
        }
        
        b32 can_kill = true;
        
        // Sword centipede kill.
        if (enemy_entity->flags & CENTIPEDE_SEGMENT) {
            player_start_killing_centipede(enemy_entity, player);
        }
        
        if (can_kill) {
            if (enemy_entity->flags & GIVES_BIG_SWORD_CHARGE) {
                player->sword_charges += 1;                
                clamp(&player->sword_charges, 0, player->max_sword_charges);
            }
            
            if (enemy_entity->flags & HIT_BOOSTER) {
                flick_to_enemy(player_entity, player, enemy_entity);
                kill_enemy(enemy_entity, hit_position, particles_direction, false, 1.0f);
            } else if (enemy_entity->flags & BIRD_ENEMY) {
                flick_to_enemy(player_entity, player, enemy_entity);
                sword_kill_enemy(enemy_entity, &enemy_entity->bird_enemy->velocity);
            } else if (enemy_entity->flags & JUMP_SHOOTER) {
                flick_to_enemy(player_entity, player, enemy_entity);
                sword_kill_enemy(enemy_entity, &enemy_entity->jump_shooter->velocity);
            } else if (enemy_entity->flags & WIN_BLOCK) {
                flick_to_enemy(player_entity, player, enemy_entity);
                kill_enemy(enemy_entity, hit_position, particles_direction, false, 1.0f);
            } else {
                kill_enemy(enemy_entity, hit_position, particles_direction, false, lerp(1.0f, 1.5f, 1));
            }
            
            killed_enemy = true;
        }
        // player->sword_angular_velocity += player->sword_spin_direction * 1400;
        
        // We also set this last hit variable in kill_enemy and stun_enemy, but as we see now we don't want to kill or stun 
        // every enemy that take hit, so it have sense to set it where actual hit is delivered.
        enemy->last_hit_time = current_context->game_time;
        
        // f32 max_speed_boost = 6 * player->sword_spin_direction * enemy->sword_kill_speed_modifier;
        // f32 max_vertical_speed_boost = player->grounded ? 0 : 20;
        // if (player->velocity.y > 0) {
        //     max_vertical_speed_boost *= 0.3f;   
        // }
        
        // if (!player->grounded) {
        //     player->velocity += Vector2_up * max_vertical_speed_boost + Vector2_right * max_speed_boost; 
        // }
                         
        if (was_alive_before_hit) {
            add_hitstop(0.01f + hitstop_add);
            shake_camera(0.1f);
        }
        
        if (enemy_entity->flags & AMMO_PACK) {
            play_sound("AmmoCollect", hit_position, 0.5f, 1.1f, 0.1f);  
        } else if (enemy_entity->flags & BIRD_ENEMY && was_alive_before_hit) {
            play_sound("SwordHit33", hit_position, 0.5f, 1.1f, 0.1f);
        } else if (enemy_entity->flags & JUMP_SHOOTER && was_alive_before_hit) {
            play_sound("SwordHit2222", hit_position, 0.5f, 0.7f, 0.1f);
        } else {
            play_sound("SwordKill", hit_position, 0.5f);
        }
        
        // sword hitmarks
        {
            Color hitmark_color = WHITE;
            f32 hitmark_scale = 1;
            b32 is_hitmark_follow = false;
            
            if (enemy_entity->flags & (CENTIPEDE_SEGMENT | TRIGGER | BIRD_ENEMY)) {
                is_hitmark_follow = true;
            }
            
            if (enemy_entity->flags & EXPLOSIVE) {
                hitmark_scale += 2;
                hitmark_color = Fade(ColorBrightness(ORANGE, 0.3f), 0.8f);
            }
            
            if (enemy_entity->flags & HIT_BOOSTER) {
                hitmark_scale += 1.5f;
                hitmark_color = ColorBrightness(RED, 0.3f);
            }
            
            add_hitmark(enemy_entity, is_hitmark_follow, hitmark_scale, hitmark_color); 
        }
    }
    
    return killed_enemy;
}

void calculate_sword_collisions(Entity *sword, Entity *player_entity) {
    fill_collisions(sword, &collisions_buffer, GROUND | ENEMY | WIN_BLOCK | CENTIPEDE_SEGMENT | PLATFORM | BLOCK_ROPE, &default_allocator);
    
    assert(player_entity->player_data);
    Player *player_data = player_entity->player_data;
    
    for (i32 i = 0; i < collisions_buffer.count; i++) {
        Collision col = collisions_buffer.get_value(i);
        Entity *other = col.other_entity;
        
        // blocker block
        if ((other->flags & BLOCKER || other->flags & SWORD_SIZE_REQUIRED) && !is_player_in_stun(player_entity)) {
            if (is_sword_can_damage() && !can_sword_damage_enemy(other)) {
                player_data->velocity = player_data->velocity * -0.5f;
                emit_particles(&rifle_bullet_emitter, col.point, col.normal, 3, 5);
                // set_sword_velocity(normalized(-player_data->sword_angular_velocity) * 150);
                
                stop_sword_mode(player_data);
                
                player_data->weak_recoil_stun_start_time = current_context->game_time;
                add_hitstop(0.1f);
                shake_camera(0.7f);
                // changed pitch from 0.5f and changed sound from 0.4f
                play_sound("SwordBlock", col.point, 0.3f, 0.75f, 0.1f);
                emit_particles(&sparks_emitter_copy, sword->position + sword->up * sword->scale.y * sword->pivot.y, col.normal, 1.5f, 1.0f);
                emit_particles(&shockwave_emitter_copy, sword->position + sword->up * sword->scale.y * sword->pivot.y, col.normal, 1.0f, 1.0f);
                continue;
            }
        }
        
        if (other->flags & ENEMY) {
            try_sword_damage_enemy(other, sword->position + sword->up * sword->scale.y * sword->pivot.y);
        }
        
        // if (other->flags & BLOCK_ROPE && player_data->sword_spin_progress >= 0.7f) {
        //     // cut rope
        //     cut_rope(other, col.point);
        // }
        
        if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT || other->flags & PLATFORM) {
            player_data->sword_hit_ground = true;
        }
    }
}

void push_player_up(f32 power) {
    Player *player_data = &current_context->player;
    if (player_data->velocity.y < 0) {
        player_data->velocity.y = 0;
    }
    
    player_data->velocity.y += power;
    player_data->timers.since_jump_timer = 0;
    player_data->grounded = false;
}

void push_or_set_player_up(f32 power) {
    Player *player_data = &current_context->player;
    if (player_data->velocity.y > power) {
        power *= 0.25f;
    }

    player_data->velocity.y += power;
    
    player_data->timers.since_jump_timer = 0;
    player_data->grounded = false;
}

inline void update_connected_entities_positions(Entity *player_entity) {
    Player *player_data = player_entity->player_data;
    Entity *ground_checker     = get_entity(player_data->connected_entities_ids.ground_checker_id);
    Entity *left_wall_checker  = get_entity(player_data->connected_entities_ids.left_wall_checker_id);
    Entity *right_wall_checker = get_entity(player_data->connected_entities_ids.right_wall_checker_id);
    Entity *sword = get_entity(player_data->connected_entities_ids.sword_entity_id);
    
    ground_checker->position     = player_entity->position - player_entity->up * player_entity->scale.y * 0.5f;
    left_wall_checker->position  = player_entity->position - player_entity->right * player_entity->scale.x * 1.0f + Vector2_up * player_entity->scale.y * 0.3f;
    right_wall_checker->position = player_entity->position + player_entity->right * player_entity->scale.x * 1.0f + Vector2_up * player_entity->scale.y * 0.3f;
    sword->position = player_entity->position;
    // rifle->position = sword->position + sword->up * sword->scale.y;
}

inline Vector2 get_move_plane(Vector2 normal, f32 move_dir) {
    return get_rotated_vector_90(normal, -normalized(move_dir));
}

inline void player_snap_to_plane(Entity *player_entity, Vector2 normal) {
    Player *player_data = player_entity->player_data;
    player_data->ground_normal = normal;
    player_data->velocity_plane = get_move_plane(player_data->ground_normal, player_data->velocity.x);
    player_data->velocity = player_data->velocity_plane * magnitude(player_data->velocity);
}

inline b32 is_player_in_stun(Entity *entity) {
    f32 max_weak_stun_time = 0.3f;
    f32 in_weak_stun_time  = current_context->game_time - entity->player_data->weak_recoil_stun_start_time;
    return (in_weak_stun_time <= max_weak_stun_time);
}

// void change_sword_spin_progress(Player *player_data, i32 direction, f32 progress) {
//     direction = clamp(direction, -1, 1);

//     player_data->sword_spin_progress = progress;
//     player_data->sword_angular_velocity = direction * progress * player_data->SWORD_SPIN_SPEED;
    
//     player_data->is_sword_accelerating = direction != 0;
//     player_data->sword_spin_direction = normalized(player_data->sword_angular_velocity);
//     assert(player_data->sword_spin_direction == direction);
// }

void spin_sword(Entity *sword, Entity *player_entity, Player *player, f32 spin_amount) {
    static const f32 MAX_ONE_TIME_ROTATION_AMOUNT = 5;

    // Someone could enter sword on previous frame after this update so we'll check for that.
    rotate(sword, -1.0f * 0.5f * MAX_ONE_TIME_ROTATION_AMOUNT * player->sword_spin_direction);         
    calculate_sword_collisions(sword, player_entity);
    
    rotate(sword, 0.5f * MAX_ONE_TIME_ROTATION_AMOUNT * player->sword_spin_direction);         
    calculate_sword_collisions(sword, player_entity);
    
    while(spin_amount > MAX_ONE_TIME_ROTATION_AMOUNT) {
        rotate(sword, MAX_ONE_TIME_ROTATION_AMOUNT * player->sword_spin_direction);
        calculate_sword_collisions(sword, player_entity);
        spin_amount -= MAX_ONE_TIME_ROTATION_AMOUNT;
    }
    rotate(sword, spin_amount);
    calculate_sword_collisions(sword, player_entity);
}

void update_sword(Entity *entity, Player *player, Input input, f32 dt) {
    if (player->dead_man) {
        return;
    }
    
    Vector2 input_direction = input.sum_direction;
    
    Entity *sword = get_entity(player->connected_entities_ids.sword_entity_id);
    
    // Changing mode and managing size.
    {
        if (player->state_flags & KILLING_CENTIPEDE) {
            goto killing_centipede_sword_case;
        }
    
        if (player->sword_mode == RIFLE_MODE) {
            if (input.press_flags & SPIN && player->sword_charges > 0) {            
                player->sword_mode = SWORD_MODE;
                player->sword_prepare_timer = 0;
                
                play_sound("SwordSwingBig", 0.9f, 1.0f, 0.05f);
                
                assert(player->sword_charges > 0 && player->sword_charges <= player->max_sword_charges);
                
                remove_flag(&player->state_flags, HIT_CENTIPEDE_THIS_SPIN);
                player->sword_charges -= 1;
                
                player->sword_attack_start_move_direction = input_direction.x != 0 ? input_direction.x : 1;
                player->sword_attack_start_angle = sword->rotation;
                
                player->state_flags |= PREPARING_SWORD;
                
                player->this_attack_killed_count = 0;
            }
        }
            
        if (player->sword_mode == SWORD_MODE) {
            // Preparing sword for attack.
            if (player->sword_prepare_timer < player->SWORD_PREPARE_TIME) { 
                player->sword_prepare_timer += dt;
                
                f32 t = clamp01(player->sword_prepare_timer / player->SWORD_PREPARE_TIME);
                
                f32 start = player->sword_attack_start_angle;
                f32 end = player->sword_attack_start_move_direction > 0 ? 359.99f : 0;
                
                f32 rotation = lerp(start, end, EaseInOutQuad(t));
                
                rotate_to(sword, rotation);
                
                if (player->sword_prepare_timer >= player->SWORD_PREPARE_TIME) {
                    // player->sword_spin_timer = 0;
                    player->sword_spin_direction = input.last_non_zero_x;
                    
                    assert(player->state_flags & PREPARING_SWORD);
                    player->state_flags ^= PREPARING_SWORD;
                    
                    player->state_flags |= SWORD_ATTACKING;
                    player->state_flags |= JUST_ENDED_PREPARING_ATTACK;
                    
                    player->to_spin_angle_amount = 1440;
                    
                    player->velocity.x = 200 * player->sword_spin_direction;
                    player->velocity.y = 80;
                }
            } 
            
            if (player->state_flags & SWORD_ATTACKING) {
                if (player->state_flags & JUST_ENDED_PREPARING_ATTACK) {
                    // Fo the future.
                    remove_flag(&player->state_flags, JUST_ENDED_PREPARING_ATTACK);
                } else {
                }
                                
                bool last_enemy_blocks_change_direction = player->last_flicked_enemy_flags & HIT_BOOSTER;
                static const f32 AFTER_FLICK_DIRECTION_BUFFER = 0.2f;
                if (player->since_flick_timer <= AFTER_FLICK_DIRECTION_BUFFER && !last_enemy_blocks_change_direction) {
                    player->sword_spin_direction = input.last_non_zero_x;
                }
                                
                f32 spin_velocity = player->SWORD_SPIN_SPEED * player->sword_spin_direction;
                f32 spin_amount = abs(spin_velocity) * dt;
                
                spin_sword(sword, entity, player, spin_velocity * dt);
                
                player->to_spin_angle_amount -= spin_amount;
                
                if (player->to_spin_angle_amount < 0) {
                    stop_sword_mode(player);
                }
            }
        }
        
        {
            Vector2 sword_target_scale = player->rifle_scale;
            
            if (player->sword_mode == SWORD_MODE)    sword_target_scale = player->sword_scale;
            
            change_scale(sword, lerp(sword->scale, sword_target_scale, dt * 5));
        }
        
        killing_centipede_sword_case:
    
        change_color(sword, player->sword_mode == SWORD_MODE ? ColorBrightness(RED, 0.1f) : ColorBrightness(SKYBLUE, 0.3f));
        
        if (player->state_flags & KILLING_CENTIPEDE) {
            static const f32 KILLING_CENTIPEDE_BUFF_TIME = 0.15f;
            player->killing_centipede_timer += dt;
            
            Vector2 target_scale = player->rifle_scale;
            f32 spin_speed_multiplier = 1.0f;
        
            if (player->killing_centipede_timer <= KILLING_CENTIPEDE_BUFF_TIME) {
                target_scale = player->sword_scale * 1.5f;
                spin_speed_multiplier = 2.0f;
            } 
            
            change_scale(sword, lerp(sword->scale, target_scale, dt * 25));
            
            f32 spin_velocity = player->SWORD_SPIN_SPEED * spin_speed_multiplier * player->sword_spin_direction;
            spin_sword(sword, entity, player, spin_velocity * dt);
        }
    }
    
    // Calculating required spin.
    // if (0) {
    // } else if (player->state_flags & KILLING_CENTIPEDE) {
    //     change_scale(sword, lerp(sword->scale, player->sword_ground_mode_scale, dt * 15));
    //     change_sword_spin_progress(player, -1, 1);
    //     spin_sword(sword, entity, dt);
    // } else if (player->sword_mode != RIFLE_MODE) {
    //     f32 sword_max_spin_speed = player->sword_mode == SWORD_MODE ? player->SWORD_SPIN_SPEED : player->SWORD_SPIN_SPEED;
        
    //     b32 can_sword_spin = !is_player_in_stun(entity);
    //     if (can_sword_spin) {
    //         f32 sword_spin_sense = player->sword_mode == SWORD_MODE ? 40 : 10; 
            
    //         f32 wish_angular_velocity = input_direction.x * sword_max_spin_speed;
            
    //         if (player->sword_mode != SWORD_MODE && player->grounded) {
    //             wish_angular_velocity *= 2;
    //         }
            
    //         if (core.time.time_scale < 1) {
    //             sword_spin_sense /= core.time.time_scale;
    //             sword_spin_sense = fminf(sword_spin_sense, 60);
    //         }
    //         player->sword_angular_velocity = lerp(player->sword_angular_velocity, wish_angular_velocity, dt * sword_spin_sense);
    //     }
    //     player->is_sword_accelerating = input_direction.x != 0;
        
    //     player->sword_spin_progress = clamp01(abs(player->sword_angular_velocity) / sword_max_spin_speed);
    //     player->sword_spin_direction = normalized(player->sword_angular_velocity);
        
    //     spin_sword(sword, entity, dt);
    // } // Sword spin end.
    
    // Sword effects.
    {
        Vector2 sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
    
        Particle_Emitter *sword_tip_emitter       = get_particle_emitter(blood_trail_emitter_index);
        if (sword_tip_emitter) {
            enable_emitter(sword_tip_emitter);
            sword_tip_emitter->position = sword_tip;
        }
        Particle_Emitter *sword_tip_ground_emitter = get_particle_emitter(sword_tip_ground_emitter_index);
        if (sword_tip_ground_emitter) {
            sword_tip_ground_emitter->position = sword_tip;
        }
        
        if (player->sword_hit_ground) {
            player->sword_hit_ground = false;
            enable_emitter(sword_tip_ground_emitter);
            f32 t = 0.6f;
            sword_tip_ground_emitter->speed_multiplier = lerp(0.7f, 4.0f, t * t * t);
            sword_tip_ground_emitter->count_multiplier = lerp(0.45f, 1.0f, t * t * t);
        } else {
            sword_tip_ground_emitter->enabled = false;
        }

    } // End sword effects.
    
    player->since_flick_timer += dt;
} // End update player sword.

void update_rifle(Entity *entity, Player *player_data, Input input, f32 dt) {
    if (player_data->dead_man) {
        return;
    }
    
    Entity *sword = get_entity(player_data->connected_entities_ids.sword_entity_id);
    
    Vector2 sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
    
    b32 can_shoot = player_data->sword_mode == RIFLE_MODE && !(player_data->state_flags & KILLING_CENTIPEDE);
    
    // Rifle shoot.
    if (can_shoot) {
        i32 shoots_queued = 0;
        local_persist f32 shoot_press_time = -12;
        local_persist b32 rifle_in_machinegun_mode = false;
        if (input.press_flags & SHOOT) {
            shoot_press_time = current_context->game_time;
            shoots_queued += 1;
        }
        
        if (input.hold_flags & SHOOT_DOWN && shoot_press_time > 0) {
            f32 hold_time = current_context->game_time - shoot_press_time;
            
            if (!rifle_in_machinegun_mode && hold_time >= 0.2f) {
                rifle_in_machinegun_mode = true;
                shoots_queued += 1;
                shoot_press_time = current_context->game_time;
                hold_time -= 0.2f;
            }
            
            f32 machinegun_shoots_delay = 0.03f;
            if (rifle_in_machinegun_mode) {
                while (hold_time >= machinegun_shoots_delay) {
                    hold_time -= machinegun_shoots_delay;
                    shoots_queued += 1;
                    shoot_press_time = current_context->game_time;
                }
            }
        }
        
        if (input.press_flags & SHOOT_RELEASED) {
            shoot_press_time = -12;
            rifle_in_machinegun_mode = false;
        }
        
        if (input.press_flags & SHOOT) {
            if ((player_data->ammo <= 0 && !debug.infinite_ammo)) {
                play_sound("FailedRifleActivation", 0.4f);
            }
            
            player_data->timers.rifle_shake_start_time = current_context->game_time;
            emit_particles(&gunpowder_emitter, sword_tip, sword->up);
        }
        
        // player_data->sword_angular_velocity = 0;
        player_data->is_sword_accelerating = 0;
        // player_data->sword_spin_progress = 0;
    
        Vector2 sword_vec_to_mouse = input.mouse_position - sword->position;
        Vector2 sword_to_mouse = normalized(sword_vec_to_mouse);
        change_up(sword, sword_to_mouse);
        // player shoot
        b32 can_shoot_rifle = (player_data->ammo > 0 || debug.infinite_ammo) && state_context.shoot_stopers_count == 0;
        
        while (shoots_queued > 0) {
            if (can_shoot_rifle) {
                sword_tip = sword->position + sword->up * sword->scale.y * sword->pivot.y;
                
                Vector2 sword_tip_vec_to_mouse = input.mouse_position - sword->position;
                Vector2 sword_tip_to_mouse = normalized(sword_tip_vec_to_mouse);
                
                
                Vector2 shoot_direction = sword_tip_to_mouse;
                
                if (rifle_in_machinegun_mode) {
                    f32 max_spread = 20;
                    f32 spread_angle = rnd(-max_spread * 0.5f, max_spread * 0.5f);
                    
                    shoot_direction = get_rotated_vector(shoot_direction, spread_angle);
                }
                
                f32 rifle_projectile_speed = 1400;
                add_rifle_projectile(sword_tip, shoot_direction * rifle_projectile_speed);
                add_player_ammo(-1);
                
                add_explosion_light(sword_tip, 50, 0.03f, 0.05f, ColorBrightness(ORANGE, 0.3f));
                
                if (!player_data->grounded) {
                    push_or_set_player_up(rifle_in_machinegun_mode ? 5 : 20);
                }
                
                shake_camera(0.1f);
                play_sound("RifleShot", sword_tip, 0.3f);
                player_data->timers.rifle_shake_start_time = current_context->game_time;
                player_data->timers.rifle_shoot_time = current_context->game_time;
                
                enable_emitter(player_data->rifle_trail_emitter_index);
                
            } else if (input.press_flags & SHOOT) {
                player_data->timers.rifle_shake_start_time = current_context->game_time;
                emit_particles(&gunpowder_emitter, sword_tip, sword->up);
                
                // shoot stoper blocked
                if (state_context.shoot_stopers_count > 0) {
                    local_persist i32 contiguous_failed_shots_count = 0;
                    f32 time_since_last_failed_shot = core.time.app_time - state_context.timers.last_shoot_stoper_failed_shot_app_time;
                    
                    if (time_since_last_failed_shot <= 0.4f) {
                        contiguous_failed_shots_count += 1;
                    } else {
                        contiguous_failed_shots_count = 0;
                    }
                    
                    state_context.timers.last_shoot_stoper_failed_shot_app_time = core.time.app_time;
                    
                    if (contiguous_failed_shots_count <= 5) {
                        ForEntities(entity, SHOOT_STOPER) {
                            assert(entity->union_enemy);
                            if (entity->union_enemy->in_agro) {
                            
                                Entity *sticky_line = add_entity(entity->position, {1,1}, {0.5f,0.5f}, 0, STICKY_TEXTURE);
                                sticky_line->sticky_texture->draw_line = true;
                                sticky_line->sticky_texture->line_color = ColorBrightness(VIOLET, 0.1f);
                                sticky_line->sticky_texture->line_width = contiguous_failed_shots_count * 0.5f;
                                sticky_line->sticky_texture->follow_id = entity->id;
                                sticky_line->sticky_texture->need_to_follow = true;
                                sticky_line->position = get_shoot_stoper_cross_position(entity);
                                sticky_line->sticky_texture->birth_time = current_context->game_time;
                                sticky_line->sticky_texture->max_distance = 0;
                                sticky_line->draw_order = 1;
                                shake_camera(0.1f);
                            }
                        }
                    }
                    
                    play_sound("FailedRifleActivation", 0.4f, 0.5f);
                }
            }
            shoots_queued -= 1;
        } // Shots queued loop end.
        
        f32 time_since_shoot = current_context->game_time - player_data->timers.rifle_shoot_time;
        if (time_since_shoot >= 0.5f && current_context->game_time > 1) {
            disable_emitter(player_data->rifle_trail_emitter_index);
        } else {
        }

    } // Rifle shoot end.
    
    // Rifle effects.
    {
        Particle_Emitter *rifle_trail_emitter = get_particle_emitter(player_data->rifle_trail_emitter_index);
        if (rifle_trail_emitter) {
            rifle_trail_emitter->position = sword_tip;
            rifle_trail_emitter->direction = sword->up;
        }
    } // Rifle effects end.
} // End update player rifle.

void player_accelerate(Entity *entity, Vector2 dir, f32 wish_speed, f32 acceleration, f32 dt) {
    f32 speed_in_wish_direction = dot(player_data->velocity, dir);
    
    f32 speed_difference = wish_speed - speed_in_wish_direction;        
    
    //means we above max speed
    if (speed_difference <= 0) {
        return;
    }
    
    f32 acceleration_speed = acceleration * speed_difference * dt;
    if (acceleration_speed > speed_difference) {
        acceleration_speed = speed_difference;
    }
    
    player_data->velocity.x += dir.x * acceleration_speed;
}

void player_ground_move(Entity *entity, Player *player, f32 dt) {
    // f32 walk_speed = player_data->sword_mode == SWORD_MODE ? player_data->big_sword_ground_walk_speed : player_data->ground_walk_speed;
    f32 walk_speed = player_data->ground_walk_speed;
    
    Vector2 input_direction = input.sum_direction;
    
    if (player->state_flags & SWORD_ATTACKING) {
        if (input_direction.x != player->sword_spin_direction && input_direction.x != 0) {
            input_direction.x = player->sword_spin_direction;
        }
    }
    
    b32 wanna_stop = input_direction.x == 0 || player_data->on_no_move_block;
    
    Vector2 wish_walking_plane = get_rotated_vector_90(player_data->ground_normal, -input_direction.x);
    
    if (wanna_stop) {
        f32 stopping_deceleration = player_data->ground_deceleration;
        
        Vector2 deceleration_plane = get_rotated_vector_90(player_data->ground_normal, normalized(player_data->velocity.x));
        
        f32 speed_in_wish_plane = dot(deceleration_plane, player_data->velocity);
        f32 speed_change        = fminf(stopping_deceleration * dt, -speed_in_wish_plane);
        player_data->velocity  += deceleration_plane * speed_change;
    } else {
        f32 walking_acceleration = player_data->ground_acceleration;
        
        b32 walking_same_direction = input_direction.x != 0 && dot(wish_walking_plane, player_data->velocity_plane) > 0;
        if (!walking_same_direction) {
            walking_acceleration *= 3;
        }
        
        f32 speed_in_wish_plane = dot(wish_walking_plane, player_data->velocity);
        f32 max_allowed_speed_difference = walk_speed - speed_in_wish_plane;
        
        if (speed_in_wish_plane >= walk_speed && input_direction.x * player_data->velocity.x > 0) {
            max_allowed_speed_difference = speed_in_wish_plane - walk_speed;
            
            wish_walking_plane *= -1;
            
            walking_acceleration *= 0.1f;
        }
        
        f32 speed_change = fminf(walking_acceleration * dt, max_allowed_speed_difference);
        
        player_data->velocity += wish_walking_plane * speed_change;
    }
}

void player_air_move(Entity *entity, Player *player, f32 dt) {
    // f32 walk_speed = player_data->sword_mode == SWORD_MODE ? player_data->big_sword_air_walk_speed : player_data->air_walk_speed;
    f32 walk_speed = player_data->air_walk_speed;
    
    Vector2 input_direction = input.sum_direction;
    
    if (player->state_flags & SWORD_ATTACKING) {
        if (input_direction.x != player->sword_spin_direction && input_direction.x != 0) {
            input_direction.x = player->sword_spin_direction;
        }
    }
    
    b32 wanna_stop = input_direction.x == 0;
    
    f32 wish_direction = input_direction.x;
    
    if (wanna_stop) {
        f32 stopping_deceleration = player_data->air_deceleration;
        
        f32 deceleration_direction = -normalized(player_data->velocity.x);
        
        f32 speed_in_wish_direction = deceleration_direction * player_data->velocity.x;
        f32 speed_change = fminf(stopping_deceleration * dt, -speed_in_wish_direction);
        player_data->velocity.x += deceleration_direction * speed_change;
    } else {
        f32 walking_acceleration = player_data->air_acceleration;
        
        b32 walking_same_direction = input_direction.x != 0 && (wish_direction * player_data->velocity.x) > 0;
        if (!walking_same_direction) {
            walking_acceleration *= 3;
        }
        
        f32 speed_in_wish_direction = wish_direction * player_data->velocity.x;
        f32 max_allowed_speed_difference = walk_speed - speed_in_wish_direction;
        
        if (speed_in_wish_direction >= walk_speed && input_direction.x * player_data->velocity.x > 0) {
            max_allowed_speed_difference = speed_in_wish_direction - walk_speed;
            wish_direction *= -1;
            
            f32 overspeed = speed_in_wish_direction - walk_speed;
            f32 overspeed_t = clamp01(overspeed / 100.0f);
            walking_acceleration = lerp(walking_acceleration * 0.01f, walking_acceleration, overspeed_t);
        }
        
        f32 speed_change = fminf(walking_acceleration * dt, max_allowed_speed_difference);
        
        player_data->velocity.x += wish_direction * speed_change;
    }
}

b32 in_coyote_time(Player *player) {
    static const f32 COYOTE_TIME = 0.08f;
    b32 result = player->timers.since_airborn_timer <= COYOTE_TIME && player->timers.since_jump_timer > COYOTE_TIME;
    return result;
}

void update_movement(Entity *entity, Player *player, Input input, f32 dt) {
    if (player->dead_man) {
        return;
    }
    
    auto context = entity->context;
    
    Entity *ground_checker     = get_entity(player->connected_entities_ids.ground_checker_id);
    Entity *left_wall_checker  = get_entity(player->connected_entities_ids.left_wall_checker_id);
    Entity *right_wall_checker = get_entity(player->connected_entities_ids.right_wall_checker_id);
    
    update_connected_entities_positions(entity);    
    
    // f32 since_hit_booster = current_context->game_time - player->timers.hit_booster_time;
    // b32 player_in_hit_booster = since_hit_booster <= HIT_BOOSTER_BOOST_TIME; // @TODO: Remove.
    
    Vector2 input_direction = input.sum_direction;
    
    bool last_flicked_hit_booster = player->last_flicked_enemy_flags & HIT_BOOSTER;
    bool in_hit_boost = last_flicked_hit_booster && player->since_flick_timer < 0.3f;
    
    // Player moving calculations.
    if (0) {
    } else if (player->state_flags & PREPARING_SWORD) {
        // player->velocity.x = move_towards(player->velocity.x, 0.0f, 400, dt);
        // player->velocity.y = move_towards(player->velocity.y, 0.0f, 100, dt);
        player->velocity = move_towards(player->velocity, Vector2_zero, 300, dt);
    } else if (player->state_flags & KILLING_CENTIPEDE) {
        assert(player->last_killed_segment);
        
        // player->sword_spin_progress = 1;
        Entity *current = player->last_killed_segment;
        if (current->centipede_segment->head->will_be_destroyed) {
            player_stop_killing_centipede(player);
        } else {
            f32 time_since_last_kill = current_context->game_time - player->last_segment_kill_time;
            
            if (time_since_last_kill >= player->SEGMENTS_KILL_DELAY) {
                player->last_segment_kill_time += player->SEGMENTS_KILL_DELAY;
                
                Entity *next    = current->centipede_segment->previous;
                while (next->centipede_segment->dead_man) {
                    next = next->centipede_segment->previous;
                    if (next->flags & CENTIPEDE) {
                        next = NULL;
                        player_stop_killing_centipede(player);
                        break;
                    }
                    player->last_segment_kill_time -= player->SEGMENTS_KILL_DELAY * 0.2f; // That's for giving player more time to notice that he's skipped some segments.
                }
                
                if (next) {
                    try_sword_damage_enemy(next, next->position);
                    add_hitstop(0.01f);
                    player->last_killed_segment = next;
                    make_line(entity->position, next->position, 2.0f, Fade(RED, 0.5f), 0.7f);
                }
            }
            
            entity->position = player->last_killed_segment->position;
            player->velocity = Vector2_zero;
        }
        // Player ground move.
    } else if (in_hit_boost) {
        
    } else if (player->grounded && !is_player_in_stun(entity) && !player->on_propeller) {
        player_ground_move(entity, player, dt);
        
        player_snap_to_plane(entity, player->ground_normal);
        
        entity->position.y -= dt;
        player->velocity -= player->ground_normal * dt;
        
        player->timers.since_airborn_timer = 0;

    } else {
        // Player air move.
        f32 max_downwards_speed = -150;
    
        if (player->velocity.y > 10/* && player->timers.since_jump_timer <= 0.3f*/) { //so we make jump gravity
            f32 t = player->velocity.y / 100.0f;
            player->gravity_mult = lerp(1.0f, 3.0f, sqrtf(t));
        } else {
            if (input.sum_direction.y < 0 && !is_player_in_stun(entity)) {
                player->gravity_mult = 6;
                max_downwards_speed = -150;
            } else {
                // player->gravity_mult = lerp(1.0f, 0.5f, player->sword_spin_progress * player->sword_spin_progress);
                player->gravity_mult = 3;
                if (player->velocity.y > 0) {
                    f32 up_velocity_t = clamp01(player->velocity.y / 200.0f);
                    f32 additional_gravity = lerp(0.0f, 2.0f, up_velocity_t * up_velocity_t);
                    player->gravity_mult += additional_gravity;
                }
            }
        }
        
        if (!is_player_in_stun(entity)) {
            player_air_move(entity, player, dt);
        }
        
        // In air and in big sword mode we keeping velocity mostly horizontal for more control.
        // This on wall check needs so that our wall boost system worked nice.
        if (player->sword_mode == SWORD_MODE && !player->on_wall) {
            // player->velocity.y = lerp(player->velocity.y, 0.0f, dt * 10);            
            
        } 
        
        b32 should_apply_gravity = !in_coyote_time(player);
        if (should_apply_gravity) {
            player->velocity.y -= player->gravity * player->gravity_mult * dt;
        }
        
        if (player->velocity.y < max_downwards_speed) {
            player->velocity.y = lerp(player->velocity.y, max_downwards_speed, 30 * dt);
        }
         
        player->timers.since_airborn_timer += dt;
    }
    
    // Player jump.
    {
        if (input.press_flags & JUMP) {
        
        // This thing tells about button press time, not about real act of jumping.
        player->timers.jump_press_time = current_context->game_time;
            
        if (!player->grounded) {
                player->timers.air_jump_press_time = current_context->game_time;
            }
        }
        
        f32 time_since_jump_press = current_context->game_time - player->timers.jump_press_time;
        f32 time_since_air_jump_press = current_context->game_time - player->timers.air_jump_press_time;
        
        player->timers.since_jump_timer += dt;
        
        b32 need_jump = (input.press_flags & JUMP && player->grounded)
                     || (player->grounded && time_since_air_jump_press <= player->jump_buffer_time) 
                     || (input.press_flags & JUMP && in_coyote_time(player));
        
        // Player jump.
        if (need_jump) {
            push_player_up(player->jump_force);
        }

    } // End player jump.
    
    // Actually moving player.
    Vector2 next_pos = {entity->position.x + player->velocity.x * dt, entity->position.y + player->velocity.y * dt};
    
    entity->position = next_pos;
    
    // Player collisions.
    {
        f32 found_ground = false;
        f32 just_grounded = false;
        
        f32 wall_acceleration = 400;
        
        f32 player_speed = magnitude(player->velocity);
        
        b32 hit_a_wall = false;
        
        f32 wall_vertical_boost = 100;
        
        local_persist f32 timer_since_on_wall = 0;
        f32 allowed_time_on_wall_without_pushing_back = 0.5f;
        
        // We're giving a vertical boost on entering wall collision for nicer movement. 
        // After some time on wall we're starting to push player away from wall because with current movement player could just climb
        // any inclined wall without that.
        
        u64 collision_flags = GROUND | ENEMY_BARRIER | PROPELLER | CENTIPEDE_SEGMENT | PLATFORM | NO_MOVE_BLOCK | TURRET | PLAYER_TOUCH_TIMER | HIT_BOOSTER | WIN_BLOCK | LEVEL_LOADER;
        
        // Player left wall collisions.
        fill_collisions(left_wall_checker, &collisions_buffer, collision_flags, &default_allocator);
        for (i32 i = 0; i < collisions_buffer.count && !is_player_in_stun(entity); i++) {
            Collision col = collisions_buffer.get_value(i);
            
            if (!(player->state_flags & BOOSTED_LEFT_WALL) && player->velocity.y < wall_vertical_boost && player->velocity.y != 0 && (input_direction.x * col.normal.x < 0) && !player->grounded) {
                player->velocity.y = wall_vertical_boost;
                player->state_flags |= BOOSTED_LEFT_WALL;
            } else if (timer_since_on_wall >= allowed_time_on_wall_without_pushing_back) {
                // Here 90 is straight wall.
                f32 wall_angle = fangle(col.normal, Vector2_up);
                // That's looks a bit complicated so here's explanation:
                //
                // Angles that we might consider "wall" is 45..90, so part with "-0.5f" and "* 2" is for 
                // remaping angle to 0..1, where 1 will be straight wall. Next we do "1 - x", so now angle 45 will be maximum power
                // and 90 is no additional down power at all.
                // Then there's sqrtf for more rapid increase in power and "* 5" for max down power when angle is 45.
                f32 push_down_power = sqrtf(1.0f - clamp01((((wall_angle / 90.0f) - 0.5f) * 2))) * 5;
                player->velocity += (col.normal - get_rotated_vector_90(col.normal, 1)) * push_down_power;
            }
            hit_a_wall = true;
            
            if (col.other_entity->flags & CENTIPEDE_SEGMENT && input_direction.x < 0) {
                Vector2 climb_plane = get_rotated_vector(col.normal, -player->sword_spin_direction * -120);
                f32 max_wall_speed = 150;
                f32 wall_acceleration = 400;
                if (dot(player->velocity, climb_plane) < max_wall_speed) {
                    player->velocity += climb_plane * wall_acceleration * dt;
                }
            }
            break;
        }
        
        // Player right wall collision.
        fill_collisions(right_wall_checker, &collisions_buffer, collision_flags, &default_allocator);
        for (i32 i = 0; i < collisions_buffer.count && !is_player_in_stun(entity); i++) {
            Collision col = collisions_buffer.get_value(i);
            
            if (!(player->state_flags & BOOSTED_RIGHT_WALL) && player->velocity.y < wall_vertical_boost && player->velocity.y != 0 && (input_direction.x * col.normal.x < 0) && !player->grounded) {
                player->velocity.y = wall_vertical_boost;
                player->state_flags |= BOOSTED_RIGHT_WALL;
            } else if (timer_since_on_wall >= allowed_time_on_wall_without_pushing_back) {
                // Here 90 is straight wall.
                f32 wall_angle = fangle(col.normal, Vector2_up);
                // Explanation of this "formula" is on left wall collision code.
                f32 push_down_power = sqrtf(1.0f - clamp01((((wall_angle / 90.0f) - 0.5f) * 2))) * 5;
                player->velocity += (col.normal - get_rotated_vector_90(col.normal, -1)) * push_down_power;
            }
            hit_a_wall = true;
            
            if (col.other_entity->flags & CENTIPEDE_SEGMENT && input_direction.x > 0) {
                Vector2 climb_plane = get_rotated_vector(col.normal, -player->sword_spin_direction * -120);
                f32 max_wall_speed = 150;
                f32 wall_acceleration = 400;
                if (dot(player->velocity, climb_plane) < max_wall_speed) {
                    player->velocity += climb_plane * wall_acceleration * dt;
                }
            }
            break;
        }
        
        player->on_wall = hit_a_wall;
        
        f32 wall_timer_modifier = hit_a_wall ? 1 : -1;
        timer_since_on_wall = clamp(timer_since_on_wall + dt * wall_timer_modifier, 0.0f, allowed_time_on_wall_without_pushing_back);
        
        Vector2 last_collision_point = Vector2_zero;
        Vector2 last_collision_normal = Vector2_one;
        
        player->standing_on_entities.clear();
        
        b32 moving_object_detected = false;
        // Player ground checker.
        FLAGS player_ground_collision_flags = collision_flags;
        fill_collisions(ground_checker, &collisions_buffer, player_ground_collision_flags, &default_allocator);
        b32 is_ground_huge_collision_speed = false;
        b32 found_no_move_block = false;
        for (i32 i = 0; i < collisions_buffer.count && !is_player_in_stun(entity); i++) {
            Collision col = collisions_buffer.get_value(i);
            Entity *other = col.other_entity;
            assert(col.collided);
            
            f32 dot_velocity = dot(col.normal, player->velocity);
            if (dot_velocity >= 0) {
                continue;
            }
            
            if (other->flags & PLATFORM && dot(entity->position - other->position, other->up) < 0) {
                continue;
            }
            
            //now we don't want to stand on projectiles
            if ((other->flags & BLOCKER | SHOOT_BLOCKER) && other->flags & PROJECTILE) {
                continue;
            }
            
            if ((other->flags & SHOOT_BLOCKER) && !(other->flags & BLOCKER) && !other->union_enemy->shoot_blocker_immortal) {
                continue;
            }
            
            if (other->flags & ENEMY && can_sword_damage_enemy(other) && !(other->flags & CENTIPEDE_SEGMENT)) {
                if (try_sword_damage_enemy(other, col.point)) {
                    continue;
                }
            }
            
            player->standing_on_entities.append(other, entity->allocator);
            
            if (other->flags & NO_MOVE_BLOCK) {
                found_no_move_block = true;
            }
            
            if (other->flags & CENTIPEDE_SEGMENT) {
                if (other->centipede_segment->head->centipede->spikes_on_right && other->centipede_segment->head->centipede->spikes_on_left) {
                    kill_player();
                    return;
                } else if (!other->centipede_segment->head->centipede->spikes_on_right && !other->centipede_segment->head->centipede->spikes_on_left) {
                    
                } else {
                    Vector2 side = other->centipede_segment->head->centipede->spikes_on_right ? other->right : (other->right * -1.0f);
                    f32 side_dot = dot(side, entity->position - other->position);
                    // so we on side of the centipede segments where are SPIKES
                    if (side_dot > 0.1f) {
                        kill_player();
                        return;
                    }
                }
            }
            
            last_collision_point = col.point;
            last_collision_normal = col.normal;
            
            Vector2 velocity_direction = normalized(player->velocity);
            f32 before_speed = magnitude(player->velocity);
            
            if (before_speed > 200) {
                is_ground_huge_collision_speed = true;
            } 
        
            f32 collision_force_multiplier = 1;
            
            if (dot((cast(Vector2) {0, 1}), col.normal) > 0.5f) {
                player->velocity -= col.normal * dot(player->velocity, col.normal);
            }
            
            if (other->flags & MOVE_SEQUENCE && other->move_sequence->moving) {
                entity->position += other->move_sequence->moved_last_frame;
            }
            
            f32 angle = fangle(col.normal, entity->up);
            
            if (angle <= player->max_ground_angle) {
                b32 ceiling_too_close = raycast(entity->position + Vector2_up * entity->scale.y * 0.5f, Vector2_up, 2.0f, GROUND, 2.0f, entity->id).collided;
                if (!ceiling_too_close) {
                    entity->position.y += col.overlap;
                } 
                
                found_ground = true;
                player->ground_normal = col.normal;
                player->ground_point = col.point;
                
                if (!player->grounded && !just_grounded) {
                    player_snap_to_plane(entity, player->ground_normal);
                    // player->velocity_plane = get_rotated_vector_90(player->ground_normal, -normalized(player->velocity.x));
                    // player->velocity = player->velocity_plane * magnitude(player->velocity);
                    just_grounded = true;
                    
                    //heavy landing
                    if (before_speed > 200 && magnitude(player->velocity) < 100) {
                        player->heavy_collision_time = current_context->game_time;
                        player->heavy_collision_velocity = player->velocity;
                        emit_particles(&ground_splash_emitter, col.point, col.normal, 1, 1.5f);
                        shake_camera(0.7f);
                        
                        play_sound("HeavyLanding", col.point, 1.5f);
                    }
                }
            } else { // If ground angle is too high.
                // We move player in this case separately because could happen that even though we've stepped on too steap of a ground,
                // but our wall detectors did not noticed that. But we could think of a better way than that.
                // I think that eventually we'll rewwrite movement compeltely to be a lot less physics-based.
                player->velocity.y -= 800 * dt;
            }
        } // player ground checker end
        
        player->on_no_move_block = found_no_move_block;
        
        if (!moving_object_detected && player->on_moving_object) {
            if (dot(player->moving_object_velocity, player->velocity) > magnitude(player->velocity)) {
            } else if (dot(player->moving_object_velocity, player->velocity) > 0) {
                player->velocity += player->moving_object_velocity;   
            }
            player->on_moving_object = false;
        }
        
        Particle_Emitter *tires_emitter = get_particle_emitter(player->tires_emitter_index);
        if (is_ground_huge_collision_speed) {
            tires_volume = lerp(tires_volume, 0.5f, core.time.real_dt * 6.0f);
            SetMusicVolume(tires_theme, tires_volume);
        } else {
            tires_volume = lerp(tires_volume, 0.0f, core.time.real_dt * 15.0f);
            SetMusicVolume(tires_theme, tires_volume);
        }
        
        // Player body collision.
        fill_collisions(entity, &collisions_buffer, collision_flags, &default_allocator);
        
        b32 is_body_huge_collision_speed = false;
        b32 on_propeller = false;
        for (i32 i = 0; i < collisions_buffer.count; i++) {
            Collision col = collisions_buffer.get_value(i);
            Entity *other = col.other_entity;
            assert(col.collided);
            
            b32 should_not_collide = (other->flags & BLOCKER | SHOOT_BLOCKER) && other->flags & PROJECTILE
                                  || (other->flags & SHOOT_BLOCKER) && !(other->flags & BLOCKER) && !other->union_enemy->shoot_blocker_immortal;
            if (should_not_collide) {
                continue;
            }
            
            if (other->flags & PROPELLER) {
                // Update propeller
                
                // We're keeping propellers to push player and keeping him in claws for now, because that
                // gives us some room for some things to *make* player do and if player can just leave propeller - this thing literally 
                // don't do anything interesting.
                on_propeller = true;
                Vector2 acceleration_dir = other->up;
                Vector2 deceleration_plane = other->right;
                
                Vector2 to_player = entity->position - other->position;
                
                f32 deceleration_sign = dot(to_player, deceleration_plane) > 0 ? -1 : 1;
                
                player->velocity = lerp(player->velocity, (other->up + deceleration_plane * deceleration_sign * 0.1f) * other->propeller->power, dt * 40);
                continue; 
            }
    
            
            f32 dot_velocity = dot(col.normal, player->velocity);
            if (dot_velocity >= 0) {
                continue;
            }
            
            if (other->flags & PLATFORM && dot(player->velocity, other->up) > 0) {
                continue;
            }
            
            if (other->flags & ENEMY && can_sword_damage_enemy(other) && !(other->flags & CENTIPEDE_SEGMENT)) {
                if (try_sword_damage_enemy(other, col.point))
                {
                    continue;
                }
            }
            
            if (other->flags & HIT_BOOSTER) {
                // entity->position = other->position; // @TODO: Will need to do small delay before boost and actually smoothly snap to booster position.
                // player->velocity = other->up * other->union_enemy->hit_booster.boost;
                // player->timers.hit_booster_time = current_context->game_time;
                continue; // Only detecting collision. Don't want to actually physically collide with it.
            }
    
            
            if (other->flags & CENTIPEDE_SEGMENT) {
                if (other->centipede_segment->head->centipede->spikes_on_right && other->centipede_segment->head->centipede->spikes_on_left) {
                    kill_player();
                    return;
                } else if (!other->centipede_segment->head->centipede->spikes_on_right && !other->centipede_segment->head->centipede->spikes_on_left) {
                    
                } else {
                    Vector2 side = other->centipede_segment->head->centipede->spikes_on_right ? other->right : (other->right * -1.0f);
                    f32 side_dot = dot(side, entity->position - other->position);
                    // So we on side of the centipede segments where are SPIKES.
                    if (side_dot > 0) {
                        kill_player();
                        return;
                    }
                }
            }
            
            resolve_collision(entity, col);
            
            Vector2 velocity_direction = normalized(player->velocity);
            
            f32 before_speed = magnitude(player->velocity);
            
            if (before_speed > 200) {
                is_body_huge_collision_speed = true;
            }
            
            last_collision_point = col.point;
            last_collision_normal = col.normal;
            
            if (is_player_in_stun(entity)) {
                player->velocity = reflected_vector(player->velocity * 0.5f, col.normal);
                shake_camera(0.2f);
                continue;
            }
    
            f32 collision_force_multiplier = 1;
            
            clamp(&collision_force_multiplier, 0, 1.0f);
            
            player->velocity -= col.normal * dot(player->velocity, col.normal) * collision_force_multiplier;
            
            // Heavy collision.
            if (before_speed > 200 && magnitude(player->velocity) < 100) {
                player->heavy_collision_time = current_context->game_time;
                player->heavy_collision_velocity = player->velocity;
                emit_particles(&ground_splash_emitter, col.point, col.normal, 1, 1.5f);
                shake_camera(0.7f);
                play_sound("HeavyLanding", col.point, 1.5f);
            }
        } // End player body collisions.
        
        player->on_propeller = on_propeller;
        
        if (is_body_huge_collision_speed || is_ground_huge_collision_speed) {
            if (tires_emitter) {
                tires_emitter->position = last_collision_point;
                tires_emitter->direction = last_collision_normal;
                tires_emitter->count_multiplier = 0.2f;
                enable_emitter(tires_emitter);
            }
        } else {
            disable_emitter(tires_emitter);
        }
        
        b32 just_lost_ground_below_my_feet = player->grounded && !found_ground && player->timers.since_jump_timer >= 0.5f;
        if (just_lost_ground_below_my_feet && !on_propeller) {
            Collision col = raycast(entity->position + normalized(player->velocity), Vector2_up * -1, 10, player_ground_collision_flags, 0.2f, entity->id);
            // That situation for snapping to surface while moving on different normal ground.
            if (col.collided) {
                Vector2 next_velocity_plane = get_move_plane(col.normal, player->velocity.x);
                f32 angle_difference = fangle(col.normal, player->ground_normal);
                if (col.normal != player->ground_normal && angle_difference < 35) {
                    found_ground = true;
                    player_snap_to_plane(entity, col.normal);
                    
                    // This thing just kills player in centipede case, but we're probably should first of all think of a better way
                    // for centipede walking and secondly make better centipede segments collision.
                    if (!(col.other_entity->flags & CENTIPEDE_SEGMENT)) {
                        entity->position -= col.normal;
                    }
                }
            } else if (input_direction.x == 0 && abs(player->velocity.x) < player->ground_walk_speed * 0.4f) {
                // That for stopping and not falling when on edge and player not holding key forward.
                player->velocity = player->velocity * -0.8f;;
            }
        }
        player->grounded = found_ground;
    } // End player collisions.
    
    if (player->grounded) {
        remove_flag(&player->state_flags, BOOSTED_LEFT_WALL);
        remove_flag(&player->state_flags, BOOSTED_RIGHT_WALL);
    }
    
    update_connected_entities_positions(entity);
    
    // Player effects.
    {
        if (!is_player_in_stun(entity)) {
            disable_emitter(player->stun_emitter_index);
        } else {
            enable_emitter(player->stun_emitter_index);
        }
        
        f32 wind_t = clamp01(magnitude(player->velocity) / 300.0f);
        SetMusicVolume(wind_theme, lerp(0.0f, 1.0f, wind_t * wind_t));
    } // End player effects.
    
    
} // End update player movement.


void update_player(Entity *entity, Player *player_data, Input input, f32 dt) {    
    update_movement(entity, player_data, input, dt);
    
    update_sword(entity, player_data, input, dt);
    update_rifle(entity, player_data, input, dt);
}

