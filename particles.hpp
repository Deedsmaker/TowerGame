#pragma once

global_variable Particle_Emitter chainsaw_emitter_copy  = {};
// global_variable Particle_Emitter sword_tip_emitter_copy = {};
global_variable Particle_Emitter sword_tip_ground_emitter_copy = {};
global_variable Particle_Emitter blood_pop_emitter_copy = {};
global_variable Particle_Emitter sword_kill_medium_emitter_copy = {};
global_variable Particle_Emitter sword_kill_big_emitter_copy = {};
global_variable Particle_Emitter blood_trail_emitter_copy = {};

global_variable i32 chainsaw_emitter_index = -1;
global_variable i32 sword_tip_emitter_index = -1;
global_variable i32 sword_tip_ground_emitter_index = -1;
global_variable i32 blood_trail_emitter_index = -1;

global_variable Particle_Emitter big_blood_emitter_copy = {};
global_variable Particle_Emitter rifle_bullet_emitter = {};
global_variable Particle_Emitter small_air_dust_trail_emitter_copy = {};
global_variable Particle_Emitter alarm_smoke_emitter_copy = {};
global_variable Particle_Emitter air_dust_emitter = {};
global_variable Particle_Emitter tires_emitter_copy = {};
global_variable Particle_Emitter explosion_emitter_copy = {};
global_variable Particle_Emitter big_explosion_emitter_copy = {};
global_variable Particle_Emitter shockwave_emitter_copy = {};
global_variable Particle_Emitter big_shockwave_emitter_copy = {};
global_variable Particle_Emitter ultra_small_shockwave_emitter_copy = {};
global_variable Particle_Emitter small_shockwave_emitter_copy = {};
global_variable Particle_Emitter fire_emitter = {};
global_variable Particle_Emitter little_fire_emitter = {};
global_variable Particle_Emitter attack_sparks_emitter = {};
global_variable Particle_Emitter sparks_emitter_copy = {};
global_variable Particle_Emitter white_sparks_emitter_copy = {};
global_variable Particle_Emitter blood_sparks_emitter_copy = {};
global_variable Particle_Emitter sword_sparks_emitter_copy = {};
global_variable Particle_Emitter big_sword_sparks_emitter_copy = {};
global_variable Particle_Emitter bullet_hit_emitter_copy = {};
global_variable Particle_Emitter bullet_strong_hit_emitter_copy = {};
global_variable Particle_Emitter magical_trails_emitter_copy = {};
global_variable Particle_Emitter magical_sparks_emitter_copy = {};
global_variable Particle_Emitter gunpowder_emitter = {};
global_variable Particle_Emitter air_emitter_copy = {};
global_variable Particle_Emitter ground_splash_emitter = {};
global_variable Particle_Emitter metal_debris_emitter_copy = {};
global_variable Particle_Emitter medium_metal_debris_emitter_copy = {};
global_variable Particle_Emitter smoke_fire_emitter_copy = {};
global_variable Particle_Emitter smoke_explosion_emitter_copy = {};
global_variable Particle_Emitter bullet_trail_emitter_copy = {};

global_variable i32 enabled_particles_count = 0;
// i32 last_added_index = -1;

void add_line_trail_position(Line_Trail *line_trail, Vector2 point){
    Vector2 *spot = NULL;
    if (line_trail->positions.count < LINE_TRAIL_MAX_POINTS){
        spot = line_trail->positions.add({});
    } else{
        spot = line_trail->positions.get_ptr(line_trail->start_index);    
        line_trail->start_index = (line_trail->start_index + 1) % LINE_TRAIL_MAX_POINTS;
    }
    
    *spot = point;
    line_trail->last_added_position = point;
}

i32 add_line_trail(Vector2 start_position){
    for (i32 i = 0; i < current_level_context->line_trails.max_count; i++){
        Line_Trail *line_trail = current_level_context->line_trails.get_ptr(i);
        if (!line_trail->occupied){
            line_trail->occupied = true;            
            // line_trail->positions.add(start_position);
            add_line_trail_position(line_trail, start_position);
            return i;
        }
    }
    
    print("L_WARNING: Could not add line trail, no space left");
    return -1;
}

inline Line_Trail *get_line_trail(i32 index){
    if (index < 0 || index >= MAX_LINE_TRAILS){
        printf("WARNING: Tried to get line trail with bad index %d\n", index);
        return NULL;
    }
    
    Line_Trail *line_trail = current_level_context->line_trails.get_ptr(index);
    if (!line_trail->occupied){
        printf("WARNING: Trying to get line trail that wasn't occupied with index %d\n", index);
        line_trail = NULL;
    }
    
    return line_trail;
}

inline void free_line_trail(i32 index){
    if (index < 0 || index >= MAX_LINE_TRAILS){
        printf("WARNING: Tried to free line trail with bad index %d\n", index);
        return;
    }
    
    Line_Trail *line_trail = get_line_trail(index);
    if (line_trail){
        *line_trail = {};
    }
}

void update_line_trail(i32 index, Vector2 current_position){
    Line_Trail *line_trail = get_line_trail(index);
    if (!line_trail){
        return;
    }
    
    Vector2 moved = current_position - line_trail->last_added_position;
    if (sqr_magnitude(moved) >= 4.0f * 4.0f){
        add_line_trail_position(line_trail, current_position);
    }
}

inline void enable_emitter(Particle_Emitter *emitter){
    if (emitter && !emitter->enabled){
        emitter->last_emitted_position = emitter->position;
        emitter->enabled = true;
    }
}

inline void enable_emitter(Particle_Emitter *emitter, Vector2 position){
    if (emitter && !emitter->enabled){   
        // That's so next enable_emitter set last_emitted_position to this position and we don't have particle trail across map.
        emitter->position = position;
        enable_emitter(emitter);
    }
}

inline void enable_emitter(i32 index){
    Particle_Emitter *emitter = get_particle_emitter(index);
    if (emitter){
        enable_emitter(emitter);
    }
}

inline void enable_emitter(i32 index, Vector2 position){
    Particle_Emitter *emitter = get_particle_emitter(index);
    if (emitter){
        enable_emitter(emitter, position);
    }
}

inline void disable_emitter(Particle_Emitter *emitter){
    if (emitter){
        emitter->enabled = false;
    }
}

inline void disable_emitter(i32 index){
    Particle_Emitter *emitter = get_particle_emitter(index);
    if (emitter){
        disable_emitter(emitter);
    }
}

inline void shoot_particle(Particle_Emitter *emitter, Vector2 position, Vector2 direction, f32 speed_multiplier){
    Particle particle = {};
    particle.position = position + emitter->spawn_offset;
    
    switch (emitter->spawn_type){
        case CIRCLE:{
            if (emitter->spawn_radius > 0){
                particle.position += rnd_in_circle() * emitter->spawn_radius;
            }
        } break;
        
        case BOX:{
            particle.position += rnd_in_box(emitter->spawn_area);
        } break;
    }
    particle.shape = emitter->shape;
    
    f32 scale = rnd(emitter->scale_min, emitter->scale_max) * emitter->size_multiplier;
    particle.scale = {scale, scale};
    particle.original_scale = {scale, scale};
    
    f32 spread_angle = rnd(-emitter->spread, emitter->spread) * 180.0f;
    Vector2 randomized_direction = get_rotated_vector(direction, spread_angle);
                                    
    f32 randomized_speed = rnd(emitter->speed_min * speed_multiplier, emitter->speed_max * speed_multiplier);
    
    f32 lifetime = rnd(emitter->lifetime_min, emitter->lifetime_max) * emitter->lifetime_multiplier;
    particle.max_lifetime = lifetime;
    
    particle.color = emitter->color;;
    if (emitter->color.a < 240){
        particle.color = color_fade(particle.color, rnd(0.5f, 1.0f));
    }
    particle.start_color = particle.color;
    
    particle.velocity = multiply(randomized_direction, randomized_speed);
    
    particle.rotation = rnd(-180.0f, 180.0f);
    
    particle.enabled = true;
    
    if (emitter->particle_trail_emitter){
        particle.trail_emitter_index = add_particle_emitter(emitter->particle_trail_emitter);
        Particle_Emitter *em = get_particle_emitter(particle.trail_emitter_index);
        if (em){
            em->position = particle.position;
            enable_emitter(em);
        }
    }
    
    if (emitter->particle_line_trail){
        particle.line_trail_index = add_line_trail(position);
    }
    
    if (!current_level_context->particles.get(emitter->last_added_index).enabled){
        enabled_particles_count++;
    }
    
    *current_level_context->particles.get_ptr(emitter->last_added_index) = particle;
    
    // emitter->last_added_index = (last_added_index + 1) % current_level_context->particles.max_count;
    emitter->last_added_index += 1;
    if (emitter->last_added_index >= emitter->particles_max_index){
        emitter->last_added_index = emitter->particles_start_index;
    }
}

void emit_particles(Particle_Emitter *emitter, Vector2 position, Vector2 direction = Vector2_up, f32 count_multiplier = 1, f32 speed_multiplier = 1, f32 area_multiplier = 1){
    // There we could receive emitter that actually already connected to some entity and we just want emit it. 
    // Also we can receive just setuped emitter and in that case we want to create new emitter that will be updated.
    if (!emitter->occupied){ // Means that's just arbitrary emitter.
        i32 new_emitter_index = add_particle_emitter(emitter);
        if (new_emitter_index < 0){
            // print("WARNING: Could not find emitter on emit_particles");
            return;
        }
        emitter = get_particle_emitter(new_emitter_index);
        emitter->should_extinct = true;
        // emitter->emitter_max_lifetime = 4.0f;
        emitter->enabled = true;
    }

    normalize(&direction);
    i32 count = rnd((int)emitter->count_min, (int)emitter->count_max);
    if (count >= 10){
        count = (i32)((f32)count * count_multiplier); 
    }
    
    emitter->spawn_radius *= area_multiplier;
    
    Vector2 emitter_direction = direction;
    if (emitter->gravity_multiplier > 2){
        emitter->direction = normalized(direction + Vector2_up);
    }
    
    for (i32 i = 0; i < count; i++){
        shoot_particle(emitter, position, direction, speed_multiplier);
    }
    
    for (i32 i = 0; i < emitter->additional_emitters.count; i++){
        emit_particles(emitter->additional_emitters.get(i), position, direction, count_multiplier, speed_multiplier, area_multiplier);
    }
}

inline void emit_particles(i32 emitter_index, Vector2 position, Vector2 direction = Vector2_up, f32 count_multiplier = 1, f32 speed_multiplier = 1, f32 area_multiplier = 1){
    if (emitter_index == -1){
        print("WARNING: Trying to emit particles with emitter index -1");
        return;
    }

    emit_particles(get_particle_emitter(emitter_index), position, direction, count_multiplier, speed_multiplier, area_multiplier);
}

void update_overtime_emitter(Particle_Emitter *emitter, f32 dt){
    emitter->emitting_timer += dt;
    f32 emit_delay = 1.0f / (emitter->over_time * emitter->count_multiplier);
    while (emitter->emitting_timer >= emit_delay){
        emitter->emitting_timer -= emit_delay;
        shoot_particle(emitter, emitter->position, emitter->direction, emitter->speed_multiplier);
    }
}

void update_overdistance_emitter(Particle_Emitter *emitter){
    if (emitter->just_born){
        emitter->last_emitted_position = emitter->position;
        emitter->just_born = false;
        return;
    }

    Vector2 current_emitter_position = emitter->position;

    Vector2 moved_vector = emitter->position - emitter->last_emitted_position;
    Vector2 moved_direction = normalized(moved_vector);
    
    if (emitter->direction_to_move){
        emitter->direction = moved_direction;
    }
    
    f32 moved_distance = magnitude(moved_vector);
    
    f32 distance_to_emit = 1.0f / (emitter->over_distance * emitter->count_multiplier);
    
    while (moved_distance > distance_to_emit){
        moved_distance -= distance_to_emit;
        emitter->last_emitted_position += moved_direction * distance_to_emit;
        emitter->position = emitter->last_emitted_position;
        shoot_particle(emitter, emitter->position, emitter->direction, emitter->speed_multiplier);
    }
    
    emitter->position = current_emitter_position;
}

inline void disable_particle(Particle *particle){
    particle->enabled = false;
    enabled_particles_count--;
    
    if (particle->trail_emitter_index != -1){
        Particle_Emitter *em = get_particle_emitter(particle->trail_emitter_index);
        if (em){
            em->should_extinct = true;
        }
    }
    
    if (particle->line_trail_index >= 0){    
        free_line_trail(particle->line_trail_index);   
    }
}

internal inline void update_emitter_particles(Particle_Emitter *emitter, f32 dt){
    emitter->alive_particles_count = 0;
    for (i32 i = emitter->particles_start_index; i < emitter->particles_max_index; i++){
        if (!current_level_context->particles.get(i).enabled){
            continue;
        }
    
        Particle *particle = current_level_context->particles.get_ptr(i);
        // dt = game_state == EDITOR ? core.time.real_dt : dt;
        
        if (particle->lifetime <= 0.2f && game_state == GAME){
            // dt = core.time.real_dt;
        }
        
        particle->lifetime += dt;
        
        if (particle->lifetime >= particle->max_lifetime){
            disable_particle(particle);
            continue;
        }
        
        emitter->alive_particles_count += 1;
        
        f32 t_lifetime = particle->lifetime / particle->max_lifetime;
        
        if (emitter->shape == PARTICLE_TEXTURE || emitter->fade_till_death){
            if (emitter->grow_till_death){
                particle->color = lerp(particle->start_color, Fade(particle->start_color, 0), sqrtf(t_lifetime));            
            } else{
                particle->color = lerp(particle->start_color, Fade(particle->start_color, 0), t_lifetime * t_lifetime);            
            }
        }
        
        if (emitter->grow_till_death){
            particle->scale = lerp(Vector2_zero, particle->original_scale, sqrtf(t_lifetime));
        } else{
            if (emitter->grow_after_birth && t_lifetime <= 0.2f){
                f32 t = clamp01(t_lifetime / 0.2f);
                particle->scale = lerp(Vector2_zero, particle->original_scale, t);
            }
            
            if (emitter->shrink_before_death && t_lifetime >= 0.3f){
                f32 t = clamp01((t_lifetime - 0.3f) / 0.7f);
                particle->scale = lerp(particle->original_scale, Vector2_zero, t * t);
            }
        }
        
        f32 gravity = -50 * emitter->gravity_multiplier;
        particle->velocity.y += gravity * dt;
        
        if (emitter->individual_noise_movement){
            particle->velocity += get_perlin_in_circle(emitter->noise_speed, particle->position.x, particle->position.y) * emitter->noise_power * dt; 
        } else if (emitter->random_movement){
            particle->velocity += frame_on_circle_rnd * 100 * dt;
        }
        
        if (emitter->stop_before_death && t_lifetime > emitter->lifetime_t_to_start_stopping){
            particle->velocity *= 1.0f - dt * (t_lifetime * 0.5f + 1.0f);
        }
        
        particle->rotation += emitter->rotation_multiplier * particle->velocity.x * dt;
        
        Vector2 next_position = add(particle->position, multiply(particle->velocity, dt));
        
        particle->position = next_position;
        
        if (emitter->should_collide){
            local_persist Array<Vector2, MAX_VERTICES> vertices = Array<Vector2, MAX_VERTICES>();
            add_rect_vertices(&vertices, {0.5f, 0.5f});
            local_persist Bounds bounds = get_bounds(vertices, {0.5f, 0.5f});
            
            fill_collisions(particle->position, vertices, bounds, {0.5f, 0.5f}, &collisions_buffer, GROUND);
            if (collisions_buffer.count > 0){
                Collision col = collisions_buffer.get(0);
                emit_particles(&air_dust_emitter, col.point, col.normal);
                disable_particle(particle);
                continue;
            }
        }   
        
        if (particle->line_trail_index != -1){
            update_line_trail(particle->line_trail_index, particle->position);
        }
        
        if (particle->trail_emitter_index != -1){
            Particle_Emitter *em = get_particle_emitter(particle->trail_emitter_index);
            if (em){
                em->position = particle->position;
            }
        }   
    }
}

inline void update_emitter(Particle_Emitter *emitter, f32 dt){
    emitter->emitter_lifetime += dt;
    
    // We don't want to spawn new particles when emitter is preparing to die.
    if (emitter->enabled && !emitter->should_extinct){
        if (emitter->over_time > 0){
            update_overtime_emitter(emitter, dt);
        }
        
        if (emitter->over_distance > 0){
            update_overdistance_emitter(emitter);
        }
    } else if (!emitter->enabled){
        emitter->emitter_lifetime = 0;
    }
    
    update_emitter_particles(emitter, dt);
    
    if (emitter->should_extinct && emitter->alive_particles_count == 0){
        free_particle_emitter(emitter->index);
    }
}

void update_particle_emitters(f32 dt){
    i32 emitters_count = 0;
    for (int i = 0; i < current_level_context->particle_emitters.max_count; i++){
        Particle_Emitter *emitter = current_level_context->particle_emitters.get_ptr(i);
        if (!emitter->occupied){
            continue;              
        }
        emitters_count++;
        if (emitter->destroyed){
        }
        
        update_emitter(emitter, dt);        
    }
}

void free_emitter(Particle_Emitter *emitter){
    emitter = NULL;
}

void copy_emitter(Particle_Emitter *dest, Particle_Emitter *src, Vector2 start_position){
    *dest = *src;
    dest->last_emitted_position = start_position;
    dest->position              = start_position;
}

void setup_particles(){
    // free_emitter(chainsaw_emitter);
    // chainsaw_emitter = add_emitter();
    chainsaw_emitter_copy.count_type = BIG_PARTICLE_COUNT;
    chainsaw_emitter_copy.spawn_radius = 0.2f;
    chainsaw_emitter_copy.over_distance = 3;
    chainsaw_emitter_copy.over_time     = 0;
    chainsaw_emitter_copy.speed_min     = 5;
    chainsaw_emitter_copy.speed_max     = 20;
    chainsaw_emitter_copy.scale_min     = 0.1f;
    chainsaw_emitter_copy.scale_max     = 0.6f;
    chainsaw_emitter_copy.lifetime_min  = 0.05f;
    chainsaw_emitter_copy.lifetime_max  = 0.3f;
    chainsaw_emitter_copy.spread        = 1;
    chainsaw_emitter_copy.enabled       = false;
    str_copy(chainsaw_emitter_copy.tag_16, "chainsaw1");
    
    chainsaw_emitter_index = add_particle_emitter(&chainsaw_emitter_copy);
    
    // sword_tip_emitter_copy.spawn_radius = 1.0f;
    // sword_tip_emitter_copy.over_distance     = 2;
    // sword_tip_emitter_copy.direction_to_move = true;
    // sword_tip_emitter_copy.over_time         = 0;
    // sword_tip_emitter_copy.speed_min         = 1;
    // sword_tip_emitter_copy.speed_max         = 5;
    // sword_tip_emitter_copy.scale_min         = 0.1f;
    // sword_tip_emitter_copy.scale_max         = 0.6f;
    // sword_tip_emitter_copy.lifetime_min      = 0.2f;
    // sword_tip_emitter_copy.lifetime_max      = 1.5f;
    // sword_tip_emitter_copy.spread            = 0.4f;
    // sword_tip_emitter_copy.gravity_multiplier = 0.1f;
    // sword_tip_emitter_copy.color             = Fade(RED, 0.5f);
    // sword_tip_emitter_copy.enabled           = true;
    // str_copy(sword_tip_emitter_copy.tag_16, "sword_tip");
    
    // sword_tip_emitter_index = add_particle_emitter(&sword_tip_emitter_copy);
    
    sword_tip_ground_emitter_copy.shape = PARTICLE_LINE;
    sword_tip_ground_emitter_copy.count_type = BIG_PARTICLE_COUNT;
    sword_tip_ground_emitter_copy.spawn_radius = 0.3f;
    sword_tip_ground_emitter_copy.over_distance     = 1;
    sword_tip_ground_emitter_copy.direction_to_move = true;
    sword_tip_ground_emitter_copy.over_time         = 0;
    sword_tip_ground_emitter_copy.speed_min         = 20;
    sword_tip_ground_emitter_copy.speed_max         = 40;
    sword_tip_ground_emitter_copy.scale_min         = 0.1f;
    sword_tip_ground_emitter_copy.scale_max         = 0.6f;
    sword_tip_ground_emitter_copy.lifetime_min      = 0.1f;
    sword_tip_ground_emitter_copy.lifetime_max      = 0.6f;
    sword_tip_ground_emitter_copy.spread            = 0.1f;
    sword_tip_ground_emitter_copy.gravity_multiplier = 0.5f;
    sword_tip_ground_emitter_copy.color             = Fade(ColorBrightness(YELLOW, 0.5f), 0.8f);
    sword_tip_ground_emitter_copy.enabled           = false;
    str_copy(sword_tip_ground_emitter_copy.tag_16, "sword_ground_t");
    
    sword_tip_ground_emitter_index = add_particle_emitter(&sword_tip_ground_emitter_copy);

    blood_pop_emitter_copy.shape = PARTICLE_TEXTURE;
    blood_pop_emitter_copy.grow_after_birth = true;
    blood_pop_emitter_copy.shrink_before_death = false;
    blood_pop_emitter_copy.texture = get_texture("SmokeParticle1");
    blood_pop_emitter_copy.spawn_radius      = 4;
    blood_pop_emitter_copy.over_distance     = 0;
    blood_pop_emitter_copy.direction_to_move = 0;
    blood_pop_emitter_copy.over_time         = 0;
    blood_pop_emitter_copy.speed_min         = 5;
    blood_pop_emitter_copy.speed_max         = 15;
    blood_pop_emitter_copy.count_min         = 15;
    blood_pop_emitter_copy.count_max         = 35;
    blood_pop_emitter_copy.scale_min         = 4.0f;
    blood_pop_emitter_copy.scale_max         = 12.0f;
    blood_pop_emitter_copy.lifetime_min      = 0.4f;
    blood_pop_emitter_copy.lifetime_max      = 1.5f;
    blood_pop_emitter_copy.spread            = 0.1f;
    blood_pop_emitter_copy.gravity_multiplier = 0.1f;
    blood_pop_emitter_copy.color             = Fade(RED, 0.2f);
    blood_pop_emitter_copy.enabled           = true;
    blood_pop_emitter_copy.additional_emitters.add(&sword_sparks_emitter_copy);
    str_copy(blood_pop_emitter_copy.tag_16, "blood");
    
    sword_kill_medium_emitter_copy = blood_pop_emitter_copy;
    sword_kill_medium_emitter_copy.additional_emitters.clear();
    sword_kill_medium_emitter_copy.additional_emitters.add(&sparks_emitter_copy);
    sword_kill_medium_emitter_copy.additional_emitters.add(&ultra_small_shockwave_emitter_copy);
    sword_kill_medium_emitter_copy.additional_emitters.add(&metal_debris_emitter_copy);
    sword_kill_medium_emitter_copy.color             = Fade(ColorBrightness(RED, 0.3f), 0.4f);
    sword_kill_medium_emitter_copy.speed_min         = 2;
    sword_kill_medium_emitter_copy.speed_max         = 30;
    sword_kill_medium_emitter_copy.count_min         = 90;
    sword_kill_medium_emitter_copy.count_max         = 90;
    sword_kill_medium_emitter_copy.scale_min         = 6.0f;
    sword_kill_medium_emitter_copy.scale_max         = 16.0f;
    sword_kill_medium_emitter_copy.lifetime_min      = 0.6f;
    sword_kill_medium_emitter_copy.lifetime_max      = 1.5f;
    str_copy(sword_kill_medium_emitter_copy.tag_16, "sw_kill_medium");
    
    sword_kill_big_emitter_copy = sword_kill_medium_emitter_copy;
    sword_kill_big_emitter_copy.additional_emitters.clear();
    sword_kill_big_emitter_copy.additional_emitters.add(&sparks_emitter_copy);
    sword_kill_big_emitter_copy.additional_emitters.add(&small_shockwave_emitter_copy);
    sword_kill_big_emitter_copy.additional_emitters.add(&medium_metal_debris_emitter_copy);
    sword_kill_big_emitter_copy.additional_emitters.add(&blood_sparks_emitter_copy);
    sword_kill_big_emitter_copy.additional_emitters.add(&big_sword_sparks_emitter_copy);
    sword_kill_big_emitter_copy.color             = Fade(ColorBrightness(RED, 0.3f), 0.7f);
    sword_kill_big_emitter_copy.speed_min         = 5;
    sword_kill_big_emitter_copy.speed_max         = 50;
    sword_kill_big_emitter_copy.count_min         = 120;
    sword_kill_big_emitter_copy.count_max         = 120;
    sword_kill_big_emitter_copy.scale_min         = 6.0f;
    sword_kill_big_emitter_copy.scale_max         = 20.0f;
    sword_kill_big_emitter_copy.lifetime_min      = 1.2f;
    sword_kill_big_emitter_copy.lifetime_max      = 2.8f;
    sword_kill_big_emitter_copy.spread      = 0.4f;
    str_copy(sword_kill_medium_emitter_copy.tag_16, "sw_kill_medium");
    
    blood_trail_emitter_copy = blood_pop_emitter_copy;
    blood_trail_emitter_copy.direction_to_move = true;
    blood_trail_emitter_copy.count_type = BIG_PARTICLE_COUNT;    
    blood_trail_emitter_copy.over_distance = 0.5f;    
    blood_trail_emitter_copy.scale_min = 2.0f;    
    blood_trail_emitter_copy.scale_max = 4.0f;    
    blood_trail_emitter_copy.speed_min = 0.0f;    
    blood_trail_emitter_copy.speed_max = 1.0f;    
    blood_trail_emitter_copy.lifetime_min = 0.2f;    
    blood_trail_emitter_copy.lifetime_max = 0.8f;    
    blood_trail_emitter_copy.color     = Fade(ColorBrightness(RED, 0.4f), 0.4f);
    
    blood_trail_emitter_index = add_particle_emitter(&blood_trail_emitter_copy);
    
    //free_emitter(rifle_bullet_emitter);
    //rifle_bullet_emitter = add_emitter();
    rifle_bullet_emitter.spawn_radius      = 0.5f;
    rifle_bullet_emitter.over_distance     = 3;
    rifle_bullet_emitter.direction_to_move = 0;
    rifle_bullet_emitter.over_time         = 0;
    rifle_bullet_emitter.speed_min         = 1;
    rifle_bullet_emitter.speed_max         = 5;
    rifle_bullet_emitter.count_min         = 10;
    rifle_bullet_emitter.count_max         = 40;
    rifle_bullet_emitter.scale_min         = 0.3f;
    rifle_bullet_emitter.scale_max         = 0.8f;
    rifle_bullet_emitter.lifetime_min      = 0.3f;
    rifle_bullet_emitter.lifetime_max      = 0.9f;
    rifle_bullet_emitter.spread            = 0.1f;
    rifle_bullet_emitter.gravity_multiplier = 0;
    rifle_bullet_emitter.color             = WHITE * 0.9f;
    rifle_bullet_emitter.enabled           = false;
    str_copy(rifle_bullet_emitter.tag_16, "rifle_bullet");
    
    air_dust_emitter.shape = PARTICLE_TEXTURE;
    air_dust_emitter.count_type = MEDIUM_PARTICLE_COUNT;
    air_dust_emitter.grow_after_birth = true;
    air_dust_emitter.shrink_before_death = false;
    air_dust_emitter.texture = get_texture("SmokeParticle1");
    air_dust_emitter.spawn_radius      = 0.5f;
    air_dust_emitter.over_distance     = 0.5f;
    air_dust_emitter.direction_to_move = 0;
    air_dust_emitter.over_time         = 0;
    air_dust_emitter.speed_min         = 1;
    air_dust_emitter.speed_max         = 10;
    air_dust_emitter.count_min         = 10;
    air_dust_emitter.count_max         = 40;
    air_dust_emitter.scale_min         = 6.0f;
    air_dust_emitter.scale_max         = 12.0f;
    air_dust_emitter.lifetime_min      = 0.6f;
    air_dust_emitter.lifetime_max      = 1.5f;
    air_dust_emitter.spread            = 0.1f;
    air_dust_emitter.gravity_multiplier = 0;
    air_dust_emitter.color             = Fade(WHITE, 0.2f);
    air_dust_emitter.enabled           = false;
    str_copy(air_dust_emitter.tag_16, "air_dust");
    
    small_air_dust_trail_emitter_copy = rifle_bullet_emitter;
    small_air_dust_trail_emitter_copy.count_type = MEDIUM_PARTICLE_COUNT;
    small_air_dust_trail_emitter_copy.over_distance = 1;
    small_air_dust_trail_emitter_copy.over_time     = 0.5f;
    small_air_dust_trail_emitter_copy.speed_min     = 1;
    small_air_dust_trail_emitter_copy.speed_max     = 5;
    small_air_dust_trail_emitter_copy.count_min     = 10;
    small_air_dust_trail_emitter_copy.count_max     = 40;
    small_air_dust_trail_emitter_copy.scale_min     = 0.5f;
    small_air_dust_trail_emitter_copy.scale_max     = 1.0f;
    small_air_dust_trail_emitter_copy.color         = WHITE;
    str_copy(small_air_dust_trail_emitter_copy.tag_16, "small_dust");
    
    alarm_smoke_emitter_copy = {};
    alarm_smoke_emitter_copy.count_type = SMALL_PARTICLE_COUNT;
    alarm_smoke_emitter_copy.over_distance = 0.1f;
    alarm_smoke_emitter_copy.over_time     = 20.0f;
    alarm_smoke_emitter_copy.speed_min     = 1;
    alarm_smoke_emitter_copy.speed_max     = 10;
    alarm_smoke_emitter_copy.count_min     = 30;
    alarm_smoke_emitter_copy.count_max     = 40;
    alarm_smoke_emitter_copy.scale_min     = 0.5f;
    alarm_smoke_emitter_copy.scale_max     = 1.0f;
    alarm_smoke_emitter_copy.lifetime_min     = 0.5f;
    alarm_smoke_emitter_copy.lifetime_max     = 2.0f;
    alarm_smoke_emitter_copy.gravity_multiplier     = -0.5f;
    alarm_smoke_emitter_copy.color         = Fade(ColorBrightness(RED, 0.15f), 0.9f);
    str_copy(alarm_smoke_emitter_copy.tag_16, "alarm_smoke");

    
    smoke_fire_emitter_copy = air_dust_emitter;
    smoke_fire_emitter_copy.over_distance = 0.3f;
    smoke_fire_emitter_copy.lifetime_min = 0.5f;
    smoke_fire_emitter_copy.lifetime_max = 1.2f;
    smoke_fire_emitter_copy.count_min = 8.0f;
    smoke_fire_emitter_copy.count_max = 10.0f;
    smoke_fire_emitter_copy.color = Fade(ColorBrightness(GRAY, -0.3f), 0.3f);
    str_copy(smoke_fire_emitter_copy.tag_16, "smoke_fire");
    
    smoke_explosion_emitter_copy = smoke_fire_emitter_copy;
    smoke_explosion_emitter_copy.spawn_radius = 20.0f;
    smoke_explosion_emitter_copy.over_time = 2.0f;
    smoke_explosion_emitter_copy.over_distance = 0;
    smoke_explosion_emitter_copy.lifetime_min = 0.5f;
    smoke_explosion_emitter_copy.lifetime_max = 2.2f;
    smoke_explosion_emitter_copy.speed_min     = 10;
    smoke_explosion_emitter_copy.speed_max     = 50;
    smoke_explosion_emitter_copy.count_min     = 40;
    smoke_explosion_emitter_copy.count_max     = 50;
    smoke_explosion_emitter_copy.scale_min     = 20.0f;
    smoke_explosion_emitter_copy.scale_max     = 40.0f;
    smoke_explosion_emitter_copy.spread     = 1.0f;
    smoke_explosion_emitter_copy.gravity_multiplier     = -1.0f;
    str_copy(smoke_explosion_emitter_copy.tag_16, "smoke_explos");
    
    tires_emitter_copy = air_dust_emitter;
    tires_emitter_copy.over_distance = 5.0f;
    tires_emitter_copy.scale_min         = 5.0f;
    tires_emitter_copy.scale_max         = 10.0f;
    tires_emitter_copy.color         = Fade(tires_emitter_copy.color, 0.5f);
    str_copy(tires_emitter_copy.tag_16, "tires");
    
    metal_debris_emitter_copy.spawn_radius      = 5;
    metal_debris_emitter_copy.speed_min         = 60;
    metal_debris_emitter_copy.speed_max         = 105;
    metal_debris_emitter_copy.count_min         = 3;
    metal_debris_emitter_copy.count_max         = 4;
    metal_debris_emitter_copy.scale_min         = 1.5f;
    metal_debris_emitter_copy.scale_max         = 3.5f;
    metal_debris_emitter_copy.lifetime_min      = 20.0f;
    metal_debris_emitter_copy.lifetime_max      = 20.0f;
    metal_debris_emitter_copy.should_collide    = true;
    metal_debris_emitter_copy.spread            = 0.5f;
    metal_debris_emitter_copy.gravity_multiplier = 4;
    metal_debris_emitter_copy.rotation_multiplier = 10;
    metal_debris_emitter_copy.color             = ColorBrightness(GRAY, -0.15f);
    metal_debris_emitter_copy.particle_trail_emitter = &smoke_fire_emitter_copy;
    str_copy(metal_debris_emitter_copy.tag_16, "metal_debris");
    
    medium_metal_debris_emitter_copy = metal_debris_emitter_copy;
    medium_metal_debris_emitter_copy.speed_min         = 50;
    medium_metal_debris_emitter_copy.speed_max         = 145;
    medium_metal_debris_emitter_copy.count_min         = 6;
    medium_metal_debris_emitter_copy.count_max         = 12;
    medium_metal_debris_emitter_copy.scale_min         = 1.5f;
    medium_metal_debris_emitter_copy.scale_max         = 5.5f;
    str_copy(medium_metal_debris_emitter_copy.tag_16, "m_metal_debris");
    
    // explosion_emitter_copy.additional_emitters.add(&smoke_explosion_emitter_copy);
    explosion_emitter_copy.spawn_radius       = 5;
    explosion_emitter_copy.over_distance      = 1;
    explosion_emitter_copy.direction_to_move  = 0;
    explosion_emitter_copy.over_time          = 0;
    explosion_emitter_copy.speed_min          = 1;
    explosion_emitter_copy.speed_max          = 100;
    explosion_emitter_copy.count_min          = 20;
    explosion_emitter_copy.count_max          = 30;
    explosion_emitter_copy.scale_min          = 1;
    explosion_emitter_copy.scale_max          = 12;
    explosion_emitter_copy.lifetime_min       = 0.6f;
    explosion_emitter_copy.lifetime_max       = 3.5f;
    explosion_emitter_copy.spread             = 1.0f;
    explosion_emitter_copy.gravity_multiplier = 1;
    explosion_emitter_copy.color              = Fade(ORANGE, 0.7f);
    explosion_emitter_copy.enabled            = false;
    explosion_emitter_copy.additional_emitters.add(&shockwave_emitter_copy);
    str_copy(explosion_emitter_copy.tag_16, "explosion");
    
    shockwave_emitter_copy.shape              = PARTICLE_TEXTURE;
    shockwave_emitter_copy.texture            = get_texture("Shockwave1");
    shockwave_emitter_copy.grow_till_death    = true;
    shockwave_emitter_copy.random_movement    = false;
    shockwave_emitter_copy.gravity_multiplier = 0;
    shockwave_emitter_copy.speed_min          = 0;
    shockwave_emitter_copy.speed_max          = 0;
    shockwave_emitter_copy.count_min          = 1;
    shockwave_emitter_copy.count_max          = 1;
    shockwave_emitter_copy.scale_min          = 200;
    shockwave_emitter_copy.scale_max          = 200;
    shockwave_emitter_copy.lifetime_min       = 0.4f;
    shockwave_emitter_copy.lifetime_max       = 0.5f;
    shockwave_emitter_copy.gravity_multiplier = 0;
    shockwave_emitter_copy.color              = Fade(WHITE, 0.35f);
    shockwave_emitter_copy.enabled            = false;
    str_copy(shockwave_emitter_copy.tag_16, "shockwave");
    
    big_shockwave_emitter_copy           = shockwave_emitter_copy;
    big_shockwave_emitter_copy.scale_min = 1000;
    big_shockwave_emitter_copy.scale_max = 1000;
    big_shockwave_emitter_copy.color     = Fade(WHITE, 0.1f);
    str_copy(big_shockwave_emitter_copy.tag_16, "big_shockw");
    
    ultra_small_shockwave_emitter_copy           = shockwave_emitter_copy;
    ultra_small_shockwave_emitter_copy.scale_min = 50;
    ultra_small_shockwave_emitter_copy.scale_max = 50;
    ultra_small_shockwave_emitter_copy.color     = Fade(WHITE, 0.99f);
    str_copy(ultra_small_shockwave_emitter_copy.tag_16, "usmol_shockw");
    
    small_shockwave_emitter_copy              = ultra_small_shockwave_emitter_copy;
    small_shockwave_emitter_copy.scale_min    = 90;
    small_shockwave_emitter_copy.scale_max    = 90;
    small_shockwave_emitter_copy.lifetime_min = 0.5f;
    small_shockwave_emitter_copy.lifetime_max = 1.0f;
    small_shockwave_emitter_copy.color        = Fade(WHITE, 0.99f);
    str_copy(small_shockwave_emitter_copy.tag_16, "smol_shockw");
    
    big_explosion_emitter_copy = explosion_emitter_copy;
    big_explosion_emitter_copy.additional_emitters.clear();
    big_explosion_emitter_copy.additional_emitters.add(&big_shockwave_emitter_copy);
    str_copy(big_explosion_emitter_copy.tag_16, "big_explos");
    
    fire_emitter.additional_emitters.add(&smoke_fire_emitter_copy);
    fire_emitter.spawn_radius       = 1.5f;
    fire_emitter.over_distance      = 1;
    fire_emitter.direction_to_move  = 0;
    fire_emitter.over_time          = 0;
    fire_emitter.speed_min          = 5;
    fire_emitter.speed_max          = 30;
    fire_emitter.count_min          = 10;
    fire_emitter.count_max          = 50;
    fire_emitter.scale_min          = 0.5f;
    fire_emitter.scale_max          = 3;
    fire_emitter.lifetime_min       = 0.3f;
    fire_emitter.lifetime_max       = 1.5f;
    fire_emitter.spread             = 0.7f;
    fire_emitter.gravity_multiplier = -1;
    fire_emitter.color              = Fade(ORANGE, 0.7f);
    fire_emitter.enabled            = false;
    str_copy(fire_emitter.tag_16, "fire");
    
    little_fire_emitter.spawn_radius       = 0.5f;
    little_fire_emitter.over_distance      = 1;
    little_fire_emitter.direction_to_move  = 0;
    little_fire_emitter.over_time          = 0;
    little_fire_emitter.speed_min          = 10;
    little_fire_emitter.speed_max          = 40;
    little_fire_emitter.count_min          = 10;
    little_fire_emitter.count_max          = 50;
    little_fire_emitter.scale_min          = 0.3f;
    little_fire_emitter.scale_max          = 1.0;
    little_fire_emitter.lifetime_min       = 0.6f;
    little_fire_emitter.lifetime_max       = 1.5f;
    little_fire_emitter.spread             = 0.1f;
    little_fire_emitter.gravity_multiplier = -1;
    little_fire_emitter.color              = Fade(ORANGE, 0.4f);
    little_fire_emitter.enabled            = false;
    str_copy(little_fire_emitter.tag_16, "little_fire");
    
    attack_sparks_emitter.shape = PARTICLE_LINE;
    attack_sparks_emitter.line_length_multiplier = 1.0f;
    attack_sparks_emitter.line_width             = 0.5f;
    attack_sparks_emitter.spawn_radius           = 1.5f;
    attack_sparks_emitter.over_distance          = 1;
    attack_sparks_emitter.direction_to_move      = 0;
    attack_sparks_emitter.over_time              = 0;
    attack_sparks_emitter.speed_min              = 20;
    attack_sparks_emitter.speed_max              = 40;
    attack_sparks_emitter.count_min              = 30;
    attack_sparks_emitter.count_max              = 40;
    attack_sparks_emitter.scale_min              = 0.2f;
    attack_sparks_emitter.scale_max              = 0.7f;
    attack_sparks_emitter.lifetime_min           = 0.3f;
    attack_sparks_emitter.lifetime_max           = 1.0f;
    attack_sparks_emitter.spread                 = 0.1f;
    attack_sparks_emitter.gravity_multiplier     = 0.1f;
    attack_sparks_emitter.color                  = Fade(ColorBrightness(YELLOW, 0.4f), 0.9f);
    attack_sparks_emitter.enabled                = false;
    str_copy(attack_sparks_emitter.tag_16, "att_sparks");
    
    sparks_emitter_copy = attack_sparks_emitter;
    sparks_emitter_copy.gravity_multiplier = 4.0f;
    sparks_emitter_copy.speed_min          = 80;
    sparks_emitter_copy.speed_max          = 150;
    sparks_emitter_copy.count_min          = 50;
    sparks_emitter_copy.count_max          = 90;
    sparks_emitter_copy.scale_min          = 0.2f;
    sparks_emitter_copy.scale_max          = 0.8f;
    sparks_emitter_copy.lifetime_min       = 0.1f;
    sparks_emitter_copy.lifetime_max       = 1.0f;
    sparks_emitter_copy.spread             = 0.1f;
    sparks_emitter_copy.color              = Fade(ColorBrightness(YELLOW, 0.4f), 0.6f);
    str_copy(sparks_emitter_copy.tag_16, "sparks");
    
    white_sparks_emitter_copy                    = sparks_emitter_copy;
    white_sparks_emitter_copy.gravity_multiplier = 2.0f;
    white_sparks_emitter_copy.count_type         = MEDIUM_PARTICLE_COUNT;
    white_sparks_emitter_copy.over_distance      = 0.5f;
    white_sparks_emitter_copy.speed_min          = 60;
    white_sparks_emitter_copy.speed_max          = 100;
    white_sparks_emitter_copy.count_min          = 50;
    white_sparks_emitter_copy.count_max          = 90;
    white_sparks_emitter_copy.scale_min          = 0.2f;
    white_sparks_emitter_copy.scale_max          = 0.8f;
    white_sparks_emitter_copy.lifetime_min       = 0.1f;
    white_sparks_emitter_copy.lifetime_max       = 0.5f;
    white_sparks_emitter_copy.spread             = 0.1f;
    white_sparks_emitter_copy.color              = Fade(ColorBrightness(WHITE, 0.4f), 0.6f);
    str_copy(white_sparks_emitter_copy.tag_16, "white_sparks");

    
    blood_sparks_emitter_copy           = sparks_emitter_copy;
    blood_sparks_emitter_copy.color     = ColorBrightness(RED, 0.1f);
    blood_sparks_emitter_copy.speed_min = 80;
    blood_sparks_emitter_copy.speed_max = 180;
    blood_sparks_emitter_copy.count_min = 40;
    blood_sparks_emitter_copy.count_max = 80;
    blood_sparks_emitter_copy.spread    = 0.2f;
    str_copy(blood_sparks_emitter_copy.tag_16, "blood_sparks");
    
    sword_sparks_emitter_copy                        = attack_sparks_emitter;
    sword_sparks_emitter_copy.line_length_multiplier = 5.0f;
    sword_sparks_emitter_copy.over_distance          = 10.0f;
    sword_sparks_emitter_copy.spread                 = 1;
    sword_sparks_emitter_copy.spawn_radius           = 0.5f;
    sword_sparks_emitter_copy.lifetime_min           = 0.05f;
    sword_sparks_emitter_copy.lifetime_max           = 0.3f;
    sword_sparks_emitter_copy.count_min              = 30;
    sword_sparks_emitter_copy.count_max              = 50;
    sword_sparks_emitter_copy.speed_min              = 5.0f;
    sword_sparks_emitter_copy.speed_max              = 10.0f;
    str_copy(sword_sparks_emitter_copy.tag_16, "sword_sparks");
    
    big_sword_sparks_emitter_copy = sword_sparks_emitter_copy;
    big_sword_sparks_emitter_copy.line_length_multiplier = 1.5f;
    big_sword_sparks_emitter_copy.lifetime_min = 0.05f;
    big_sword_sparks_emitter_copy.lifetime_max = 0.4f;
    big_sword_sparks_emitter_copy.count_min = 50;
    big_sword_sparks_emitter_copy.count_max = 60;
    big_sword_sparks_emitter_copy.speed_min = 15.0f;
    big_sword_sparks_emitter_copy.speed_max = 35.0f;
    str_copy(big_sword_sparks_emitter_copy.tag_16, "b_sword_sparks");
    
    bullet_hit_emitter_copy.spawn_radius       = 3.5f;
    bullet_hit_emitter_copy.over_distance      = 0;
    bullet_hit_emitter_copy.direction_to_move  = 0;
    bullet_hit_emitter_copy.over_time          = 0;
    bullet_hit_emitter_copy.speed_min          = 20;
    bullet_hit_emitter_copy.speed_max          = 80;
    bullet_hit_emitter_copy.count_min          = 30;
    bullet_hit_emitter_copy.count_max          = 100;
    bullet_hit_emitter_copy.scale_min          = 0.2f;
    bullet_hit_emitter_copy.scale_max          = 0.7f;
    bullet_hit_emitter_copy.lifetime_min       = 0.1f;
    bullet_hit_emitter_copy.lifetime_max       = 0.5f;
    bullet_hit_emitter_copy.spread             = 1.0f;
    bullet_hit_emitter_copy.gravity_multiplier = 0.5f;
    bullet_hit_emitter_copy.color              = Fade(ColorBrightness(YELLOW, 0.2f), 0.9f);
    bullet_hit_emitter_copy.enabled            = false;
    str_copy(bullet_hit_emitter_copy.tag_16, "bullet_hit");
    
    bullet_strong_hit_emitter_copy = bullet_hit_emitter_copy;
    bullet_strong_hit_emitter_copy.additional_emitters.clear();
    bullet_strong_hit_emitter_copy.additional_emitters.add(&magical_trails_emitter_copy);
    str_copy(bullet_hit_emitter_copy.tag_16, "bullet_str_hit");
    
    magical_trails_emitter_copy.spawn_radius                 = 6.5f;
    magical_trails_emitter_copy.speed_min                    = 60;
    magical_trails_emitter_copy.speed_max                    = 240;
    magical_trails_emitter_copy.count_min                    = 4;
    magical_trails_emitter_copy.count_max                    = 9;
    magical_trails_emitter_copy.scale_min                    = 0.2f;
    magical_trails_emitter_copy.scale_max                    = 0.7f;
    magical_trails_emitter_copy.lifetime_min                 = 3.5f;
    magical_trails_emitter_copy.lifetime_max                 = 9.5f;
    magical_trails_emitter_copy.spread                       = 0.2f;
    magical_trails_emitter_copy.gravity_multiplier           = 0.0f;
    magical_trails_emitter_copy.individual_noise_movement    = true;
    magical_trails_emitter_copy.noise_speed                  = 2.0f;
    magical_trails_emitter_copy.noise_power                  = 500.0f;
    magical_trails_emitter_copy.fade_till_death              = true;
    magical_trails_emitter_copy.color                        = Fade(ColorBrightness(SKYBLUE, 0.4f), 0.4f);
    magical_trails_emitter_copy.particle_line_trail          = true;
    magical_trails_emitter_copy.stop_before_death            = true;
    magical_trails_emitter_copy.lifetime_t_to_start_stopping = 0.0f;
    str_copy(magical_trails_emitter_copy.tag_16, "magic_trails");
    
    big_blood_emitter_copy = magical_trails_emitter_copy;
    big_blood_emitter_copy.count_type = MEDIUM_PARTICLE_COUNT;
    big_blood_emitter_copy.speed_min         = 10;
    big_blood_emitter_copy.speed_max         = 300;
    big_blood_emitter_copy.count_min         = 240;
    big_blood_emitter_copy.count_max         = 240;
    big_blood_emitter_copy.scale_min         = 0.2f;
    big_blood_emitter_copy.scale_max         = 1.0f;
    big_blood_emitter_copy.lifetime_min      = 5.0f;
    big_blood_emitter_copy.lifetime_max      = 40.0f;
    big_blood_emitter_copy.spread            = 1.0f;
    big_blood_emitter_copy.particle_line_trail = true;
    big_blood_emitter_copy.color             = Fade(ColorBrightness(SKYBLUE, 0.3f), 0.8f);
    str_copy(big_blood_emitter_copy.tag_16, "big_blood");

    
    magical_sparks_emitter_copy.spawn_radius                 = 3.5f;
    magical_sparks_emitter_copy.over_time                    = 0.05f;
    magical_sparks_emitter_copy.over_distance                = 0.05f;
    magical_sparks_emitter_copy.speed_min                    = 4;
    magical_sparks_emitter_copy.speed_max                    = 10;
    magical_sparks_emitter_copy.count_min                    = 10;
    magical_sparks_emitter_copy.count_max                    = 30;
    magical_sparks_emitter_copy.scale_min                    = 0.2f;
    magical_sparks_emitter_copy.scale_max                    = 1.0f;
    magical_sparks_emitter_copy.lifetime_min                 = 2.5f;
    magical_sparks_emitter_copy.lifetime_max                 = 5.5f;
    magical_sparks_emitter_copy.spread                       = 0.5f;
    magical_sparks_emitter_copy.gravity_multiplier           = 0.0f;
    magical_sparks_emitter_copy.individual_noise_movement    = true;
    magical_sparks_emitter_copy.noise_speed                  = 3.0f;
    magical_sparks_emitter_copy.noise_power                  = 50.0f;
    magical_sparks_emitter_copy.fade_till_death              = true;
    magical_sparks_emitter_copy.color                        = Fade(ColorBrightness(ORANGE, 0.4f), 1.0f);
    str_copy(magical_sparks_emitter_copy.tag_16, "magic_sparks");


    bullet_trail_emitter_copy = air_dust_emitter;
    bullet_trail_emitter_copy.count_type = MEDIUM_PARTICLE_COUNT;
    bullet_trail_emitter_copy.spawn_radius      = 1.5f;
    bullet_trail_emitter_copy.over_distance      = 0.3f;
    bullet_trail_emitter_copy.speed_min          = 1;
    bullet_trail_emitter_copy.speed_max          = 5;
    bullet_trail_emitter_copy.lifetime_min       = 0.3f;
    bullet_trail_emitter_copy.lifetime_max       = 1.0f;
    bullet_trail_emitter_copy.scale_min          = 1.0f;
    bullet_trail_emitter_copy.scale_max          = 4.0f;
    bullet_trail_emitter_copy.color          = Fade(WHITE, 0.8f);
    bullet_trail_emitter_copy.spread             = 0.2f;
    str_copy(bullet_trail_emitter_copy.tag_16, "bullet_trail");

    gunpowder_emitter.spawn_radius       = 1.0f;
    gunpowder_emitter.over_distance      = 1;
    gunpowder_emitter.direction_to_move  = 0;
    gunpowder_emitter.over_time          = 0;
    gunpowder_emitter.speed_min          = 5;
    gunpowder_emitter.speed_max          = 10;
    gunpowder_emitter.count_min          = 10;
    gunpowder_emitter.count_max          = 20;
    gunpowder_emitter.scale_min          = 0.2f;
    gunpowder_emitter.scale_max          = 0.5f;
    gunpowder_emitter.lifetime_min       = 1.0f;
    gunpowder_emitter.lifetime_max       = 4.0f;
    gunpowder_emitter.spread             = 0.2f;
    gunpowder_emitter.gravity_multiplier = 0.1f;
    gunpowder_emitter.color              = Fade(BLACK, 0.4f);
    gunpowder_emitter.enabled            = false;
    str_copy(gunpowder_emitter.tag_16, "gunpowder");
    
    air_emitter_copy.spawn_type = BOX;
    air_emitter_copy.shape = PARTICLE_LINE;
    air_emitter_copy.spawn_area         = Vector2_one * 10;
    air_emitter_copy.over_distance      = 1;
    air_emitter_copy.direction_to_move  = 0;
    air_emitter_copy.over_time          = 2;
    air_emitter_copy.speed_min          = 5;
    air_emitter_copy.speed_max          = 10;
    air_emitter_copy.count_min          = 10;
    air_emitter_copy.count_max          = 20;
    air_emitter_copy.scale_min          = 0.2f;
    air_emitter_copy.scale_max          = 1.5f;
    air_emitter_copy.lifetime_min       = 1.0f;
    air_emitter_copy.lifetime_max       = 10.0f;
    air_emitter_copy.spread             = 0.0f;
    air_emitter_copy.gravity_multiplier = -0.01f;
    air_emitter_copy.color              = Fade(WHITE, 0.4f);
    air_emitter_copy.enabled            = false;
    str_copy(air_emitter_copy.tag_16, "air");
    
    ground_splash_emitter.shape = PARTICLE_TEXTURE;
    ground_splash_emitter.grow_after_birth = true;
    ground_splash_emitter.shrink_before_death = false;
    ground_splash_emitter.texture = get_texture("SmokeParticle1");
    ground_splash_emitter.spawn_radius             = 4;
    ground_splash_emitter.over_distance      = 0;
    ground_splash_emitter.direction_to_move  = 0;
    ground_splash_emitter.over_time          = 10;
    ground_splash_emitter.speed_min          = 5;
    ground_splash_emitter.speed_max          = 10;
    ground_splash_emitter.count_min          = 50;
    ground_splash_emitter.count_max          = 100;
    ground_splash_emitter.scale_min          = 5.0f;
    ground_splash_emitter.scale_max          = 20.0f;
    ground_splash_emitter.lifetime_min       = 0.5f;
    ground_splash_emitter.lifetime_max       = 3.0f;
    ground_splash_emitter.spread             = 0.4f;
    ground_splash_emitter.gravity_multiplier = 0.1f;
    ground_splash_emitter.color              = Fade(WHITE, 0.1f);
    ground_splash_emitter.enabled            = false;
    str_copy(ground_splash_emitter.tag_16, "ground_splash");
}
