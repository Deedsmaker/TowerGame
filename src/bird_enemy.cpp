#pragma once

void bird_clear_formation(Bird_Enemy *bird) {
    if (bird->slot_index != -1) {
        current_context->bird_slots[bird->slot_index].occupied = false;
        bird->slot_index = -1;
    }
}

void respond_bird_collision(Entity *bird_entity, Collision col) {
    assert(bird_entity->flags & BIRD_ENEMY);

    Bird_Enemy *bird = bird_entity->bird_enemy;
    Enemy *enemy = bird_entity->bird_enemy; // @TODO CHANGE.
    Entity *other = col.other_entity;
    f32 bird_speed = magnitude(bird->velocity);
    f32 bird_speed_t = clamp01(bird_speed / 300.0f);
    
    b32 is_high_velocity = bird_speed > 100;
    
    b32 should_respond = true;
    if (other->flags & GROUND || other->flags & CENTIPEDE_SEGMENT || other->flags & ENEMY_BARRIER) {
        resolve_collision(bird_entity, col);
        
        if (bird->attacking && other->flags & BIRD_ENEMY) {
            should_respond = false;            
        }
        
        b32 exploded = false;
        if (bird->attacking && bird_entity->flags & EXPLOSIVE && !(bird_entity->flags & BLOCKER) && !(other->flags & BIRD_ENEMY)) {
            kill_enemy(bird_entity, col.point, col.normal);
            exploded = true;
        }
        
        if (should_respond) {
            if (enemy->dead_man) {
                emit_particles(&fire_emitter, bird_entity->position, col.normal, 2, 3);
                play_sound("Explosion", bird_entity->position, 0.3f);
                
                add_explosion_light(bird_entity->position, rnd(75.0f, 200.0f), 0.1f, 0.3f, ColorBrightness(ORANGE, 0.5f));
                
                mark_entity_destroyed(bird_entity);
                bird_entity->enabled = false;
                shake_camera(0.6f);
                return;
            }
            
            bird->velocity = reflected_vector(bird->velocity * 0.9f, col.normal);
            if (bird->attacking) {
                bird->attacking = false;
                disable_emitter(bird->attack_emitter_index);
                disable_emitter(bird->alarm_emitter_index);
                bird->roaming = true;
                bird->attacked_time = current_context->game_time;
                bird->roam_start_time = current_context->game_time;
            }
        }
        
        emit_particles(bird->collision_emitter_index, col.point, normalized(bird->velocity), lerp(0.5f, 2.0f, bird_speed_t * bird_speed_t), lerp(5, 20, bird_speed_t * bird_speed_t));
        
        if (is_high_velocity) {
            play_sound("BirdToGround", col.point, 0.5f);
        }
    }
    
    if (other->flags & BIRD_ENEMY && !bird->attacking) {
        resolve_collision(bird_entity, col);
        
        if (enemy->dead_man) {
            emit_particles(&fire_emitter, bird_entity->position, col.normal, 2, 3);
            stun_enemy(other, other->position, col.normal);
        }
        
        bird->velocity              = reflected_vector(bird->velocity * 0.8f, col.normal);
        other->bird_enemy->velocity += reflected_vector(bird->velocity * 0.3f, col.normal * -1);
        
        emit_particles(bird->collision_emitter_index, col.point, normalized(bird->velocity), 0.5f, 1);
        
        if (is_high_velocity) {
            play_sound("BirdToBird", col.point, 0.5f);
        }
    }
    
    if (other->flags & PLAYER) {
        b32 should_kill_player = !player_data->dead_man && bird->attacking && !enemy->dead_man && !(bird_entity->flags & EXPLOSIVE);
        if (should_kill_player) {
            kill_player();
            if (bird_entity->flags & EXPLOSIVE) {
                kill_enemy(bird_entity, col.point, col.normal);
            }
        }
    }
} // End of respond_bird_collision.

void update_bird_enemy(Entity *entity, f32 dt) {
    assert(entity->flags & BIRD_ENEMY);
    assert(entity->flags & ENEMY);
    assert(entity->bird_enemy);
    
    Context *context = entity->context;
    
    Entity *player_entity = context->player_entity;
    Player *player_data = &context->player;
    
    
    Bird_Enemy *bird = entity->bird_enemy;
    Enemy *enemy = entity->bird_enemy;
    
    if (!bird->in_agro && !enemy->dead_man) {
        // return;
    } else if (bird->just_awake) {
        bird->just_awake = false;
        bird->roaming = true;
        bird->roam_start_time = context->game_time;
        enemy->birth_time = context->game_time;
    }
    
    if (entity->flags & MOVE_SEQUENCE) {
        entity->move_sequence->moving = false;
    }
    
    Vector2 vec_to_player = player_entity->position - entity->position;
    Vector2 dir_to_player = normalized(vec_to_player);
    f32    distance_to_player = magnitude(vec_to_player);
    
    if (enemy->dead_man) {
        bird->velocity.y -= player_data->gravity * dt;
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
        rotate(entity, bird->velocity.x);
        bird_clear_formation(bird);
        
        f32 since_died_time = context->game_time - enemy->died_time;
        if (since_died_time >= 15 && sqr_magnitude(entity->position - player_entity->position) >= 50000) {
            kill_enemy(entity, entity->position, entity->up);
        }
        return;
    }

    f32 in_stun_time = context->game_time - enemy->stun_start_time;
    
    if (context->game_time > 3 && in_stun_time <= enemy->max_stun_time) {
        rotate(entity, 0.2f * bird->velocity.x);
        bird->velocity = move_towards(bird->velocity, Vector2_zero, magnitude(bird->velocity) * 1.0f, dt);
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
    
        bird->roaming = true;
        bird->roam_start_time = context->game_time;
        bird->charging = false;
        bird->attacking = false;
        disable_emitter(bird->attack_emitter_index);
        disable_emitter(bird->alarm_emitter_index);
        bird_clear_formation(bird);
        return;
    }
    
    // Update bird states.
    if (bird->roaming && bird->in_agro) {
        f32 roam_time = context->game_time - bird->roam_start_time;
        f32 max_roam_time = bird->max_roam_time;
        
        if (bird->slot_index != -1) {
            max_roam_time *= 0.5f;
        }
        
        if (roam_time >= max_roam_time) {
            bird->roaming = false;
            bird->charging = true;
            bird->charging_start_time = context->game_time;
        }
    }
    
    if (bird->charging) {
        f32 charging_time = context->game_time - bird->charging_start_time;
        if (charging_time >= bird->max_charging_time) {
            f32 time_since_last_bird_attacked = context->game_time - state_context.timers.last_bird_attack_time;
            
            if (time_since_last_bird_attacked >= 0.4f) {
                //bird start attack
                state_context.timers.last_bird_attack_time = context->game_time;
                change_scale(entity, bird->original_scale);
            
                change_up(entity, dir_to_player);         
                bird->charging = false;
                bird->attacking = true;
                bird->attack_start_time = context->game_time;
                
                f32 bird_attack_speed = 300;
                bird->velocity = dir_to_player * bird_attack_speed;
                
                emit_particles(&attack_sparks_emitter, entity->position, entity->up, 2, 3);
                emit_particles(bird->alarm_emitter_index, entity->position, entity->up);
                
                enable_emitter(bird->attack_emitter_index);
                enable_emitter(bird->alarm_emitter_index);
                
                play_sound("BirdAttack", entity->position);
            }
        } 
    }
    
    if (bird->attacking) {
        f32 attacking_time = context->game_time - bird->attack_start_time;
        
        if (attacking_time >= bird->max_attack_time) {
            bird->attacking = false;
            bird->roaming = true;
            bird->roam_start_time = context->game_time;
            disable_emitter(bird->attack_emitter_index);
            disable_emitter(bird->alarm_emitter_index);
            bird->attacked_time = context->game_time;
        } 
    }
    
    f32 bird_speed = magnitude(bird->velocity);
    
    f32 time_since_attacked = context->game_time - bird->attacked_time;
    
    //update bird
    if (bird->roaming) {
        f32 roam_time = context->game_time - bird->roam_start_time;
        f32 roam_t = roam_time / bird->max_roam_time;
    
        if (roam_t <= 0.2f && time_since_attacked < 4) {
            rotate(entity, bird_speed * 0.2f * normalized(bird->velocity.x));
            bird->velocity = move_towards(bird->velocity, Vector2_zero, bird_speed * 0.8f, dt);
        } else {
            f32 distance_t = clamp01(distance_to_player / 300.0f);
            f32 acceleration = lerp(bird->roam_acceleration * 0.5f, bird->roam_acceleration, distance_t * distance_t);
            f32 max_speed = lerp(bird->max_roam_speed * 0.5f, bird->max_roam_speed, distance_t);
        
            Vector2 target_position = {0};
            if (bird->in_agro) {
                Vector2 target_position = player_entity->position + Vector2_up * 120;        
                if (bird->slot_index == -1) {
                    for (i32 i = 0; i < MAX_BIRD_POSITIONS; i++) {
                        Bird_Slot *slot = &context->bird_slots[i];
                        
                        if (!slot->occupied) {
                            slot->occupied = true;
                            bird->slot_index = i;
                            break;
                        }
                    }
                }
                
                if (bird->slot_index != -1) {
                    target_position = player_entity->position + bird_formation_positions[bird->slot_index];
                } else {
                    // If bird could not find slot - it will not attack and wait in roaming until slot is freed
                    bird->roam_start_time = context->game_time;
                    roam_time = 0;
                }
            } else { // Bird not in agro and just roaming.
                Vector2 rnd_pos = {.x = perlin_noise3(context->game_time, entity->id, 1), .y = perlin_noise3(entity->id, context->game_time, 2)};
                rnd_pos *= 50;
                target_position = bird->initial_position + rnd_pos;
            }
        
            bird->target_position = target_position;
            f32 damping = 1.0f;
            bird->velocity *= 1.0f - (damping * dt);
            bird->velocity += (bird->target_position - entity->position) * acceleration * dt;
            clamp_magnitude(&bird->velocity, max_speed);
            change_up(entity, move_towards(entity->up, bird->velocity, bird_speed, dt));         
        }
        
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
    } else if (bird->charging) {
        // bird_clear_formation(bird);
        
        f32 charging_time = context->game_time - bird->charging_start_time;
        f32 t = clamp01(charging_time / bird->max_charging_time);
        
        change_scale(entity, lerp(bird->original_scale, {bird->original_scale.x * 1.2f, bird->original_scale.y * 2.0f}, t * t));
        
        bird->velocity = move_towards(bird->velocity, Vector2_zero, bird_speed * 0.8f, dt);
        if (t < 0.4f) {
            rotate(entity, bird_speed * 0.2f * normalized(bird->velocity.x));
        } else {
            change_up(entity, move_towards(entity->up, dir_to_player, lerp(0.0f, 30.0f, t * t), dt));         
            f32 charging_back_movement_amount = 3.0f;
            entity->position -= entity->up * charging_back_movement_amount * dt;
        }
        
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
    } else if (bird->attacking) {
        f32 attacking_time = context->game_time - bird->attack_start_time;
        
        bird_clear_formation(bird);
    
        f32 speed = magnitude(bird->velocity);
        change_up(entity, move_towards(entity->up, dir_to_player, 2, dt));
        bird->velocity = entity->up * speed;
        move_by_velocity_with_collisions(entity, bird->velocity, entity->scale.y * 0.8f, &respond_bird_collision, dt);
        
        if (is_enemy_should_trigger_death_instinct(entity, bird->velocity, dir_to_player, distance_to_player, true)) {
            start_death_instinct(entity, ENEMY_ATTACKING);          
        }
        
        f32 attack_line_t = clamp01(attacking_time / 0.5f);
    } else {
        assert(false);
        //what a state
    }
    
    Particle_Emitter *trail_emitter = get_particle_emitter(bird->trail_emitter_index);
    if (trail_emitter) {
        trail_emitter->direction = entity->up * -1;
    }
} // End of update_bird_enemy().

