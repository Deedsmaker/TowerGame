#pragma once

global_variable i32 enabled_particles_count = 0;
// i32 last_added_index = -1;

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
    
    particle.gravity_multiplier = emitter->gravity_multiplier;
    
    f32 spread_angle = rnd(-emitter->spread, emitter->spread) * 180.0f;
    Vector2 randomized_direction = get_rotated_vector(direction, spread_angle);
                                    
    f32 randomized_speed = rnd(emitter->speed_min * speed_multiplier, emitter->speed_max * speed_multiplier);
    
    f32 lifetime = rnd(emitter->lifetime_min, emitter->lifetime_max) * emitter->lifetime_multiplier;
    particle.max_lifetime = lifetime;
    
    particle.color = color_fade(emitter->color, rnd(0.5f, 1.0f));
    particle.start_color = particle.color;
    
    particle.velocity = multiply(randomized_direction, randomized_speed);
    
    particle.rotation = rnd(-180.0f, 180.0f);
    
    particle.enabled = true;
    
    
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
            print("WARNING: Could not find emitter on emit_particles");
            return;
        }
        emitter = get_particle_emitter(new_emitter_index);
        emitter->should_extinct = true;
        // emitter->emitter_max_lifetime = 4.0f;
        emitter->enabled = true;
    }

    normalize(&direction);
    i32 count = rnd((int)emitter->count_min, (int)emitter->count_max);
    count = (i32)((f32)count * count_multiplier); 
    
    emitter->spawn_radius *= area_multiplier;
    
    for (i32 i = 0; i < count; i++){
        shoot_particle(emitter, position, direction, speed_multiplier);
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

inline void enable_emitter(Particle_Emitter *emitter){
    if (!emitter->enabled){
        emitter->last_emitted_position = emitter->position;
    }
    emitter->enabled = true;
}

inline void disable_emitter(Particle_Emitter *emitter){
    emitter->enabled = false;
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
            //current_level_context->particles.remove(i);
            particle->enabled = false;
            enabled_particles_count--;
            continue;
        }
        
        emitter->alive_particles_count += 1;
        
        f32 t_lifetime = particle->lifetime / particle->max_lifetime;
        
        if (emitter->shape == PARTICLE_TEXTURE){
            particle->color = lerp(particle->start_color, Fade(particle->start_color, 0), t_lifetime * t_lifetime);            
        }
        
        if (emitter->grow_after_birth && t_lifetime <= 0.2f){
            f32 t = clamp01(t_lifetime / 0.2f);
            particle->scale = lerp(Vector2_zero, particle->original_scale, t);
        }
        
        if (emitter->shrink_before_death && t_lifetime >= 0.3f){
            f32 t = clamp01((t_lifetime - 0.3f) / 0.7f);
            particle->scale = lerp(particle->original_scale, Vector2_zero, t * t);
        }
        
        f32 gravity = -50 * particle->gravity_multiplier;
        particle->velocity.y += gravity * dt;
        
        particle->velocity += frame_on_circle_rnd * 100 * dt;
        
        particle->rotation += emitter->rotation_multiplier * particle->velocity.x * dt;
        
        Vector2 next_position = add(particle->position, multiply(particle->velocity, dt));
        
        particle->position = next_position;
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


void DEPRECATED_update_particles(f32 dt){
    for (int i = 0; i < current_level_context->particles.max_count; i++){
        if (!current_level_context->particles.get(i).enabled){
            continue;
        }
    
        Particle *particle = current_level_context->particles.get_ptr(i);
        dt = game_state == EDITOR ? core.time.real_dt : dt;
        
        if (particle->lifetime <= 0.2f && game_state == GAME){
            dt = core.time.real_dt;
        }
        
        particle->lifetime += dt;
        
        if (particle->lifetime >= particle->max_lifetime){
            //current_level_context->particles.remove(i);
            particle->enabled = false;
            enabled_particles_count--;
            continue;
        }
        
        f32 t_lifetime = particle->lifetime / particle->max_lifetime;
        particle->scale = lerp(particle->original_scale, Vector2_zero, t_lifetime * t_lifetime);
        
        f32 gravity = -50 * particle->gravity_multiplier;
        particle->velocity.y += gravity * dt;
        
        particle->velocity += frame_on_circle_rnd * 100 * dt;
        
        Vector2 next_position = add(particle->position, multiply(particle->velocity, dt));
        
        particle->position = next_position;
    }
}

global_variable Particle_Emitter chainsaw_emitter_copy  = {};
global_variable Particle_Emitter sword_tip_emitter_copy = {};
global_variable Particle_Emitter sword_tip_ground_emitter_copy = {};
global_variable Particle_Emitter blood_emitter_copy = {};

global_variable i32 chainsaw_emitter_index = -1;
global_variable i32 sword_tip_emitter_index = -1;
global_variable i32 sword_tip_ground_emitter_index = -1;
global_variable i32 blood_emitter_index = -1;

global_variable Particle_Emitter big_blood_emitter_copy = {};
global_variable Particle_Emitter rifle_bullet_emitter = {};
global_variable Particle_Emitter air_dust_emitter = {};
global_variable Particle_Emitter tires_emitter_copy = {};
global_variable Particle_Emitter explosion_emitter = {};
global_variable Particle_Emitter fire_emitter = {};
global_variable Particle_Emitter little_fire_emitter = {};
global_variable Particle_Emitter sparks_emitter = {};
global_variable Particle_Emitter big_sparks_emitter = {};
global_variable Particle_Emitter gunpowder_emitter = {};
global_variable Particle_Emitter air_emitter = {};
global_variable Particle_Emitter ground_splash_emitter = {};


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
    
    sword_tip_emitter_copy.spawn_radius = 1.0f;
    sword_tip_emitter_copy.over_distance     = 2;
    sword_tip_emitter_copy.direction_to_move = true;
    sword_tip_emitter_copy.over_time         = 0;
    sword_tip_emitter_copy.speed_min         = 1;
    sword_tip_emitter_copy.speed_max         = 5;
    sword_tip_emitter_copy.scale_min         = 0.1f;
    sword_tip_emitter_copy.scale_max         = 0.6f;
    sword_tip_emitter_copy.lifetime_min      = 0.2f;
    sword_tip_emitter_copy.lifetime_max      = 1.5f;
    sword_tip_emitter_copy.spread            = 0.4f;
    sword_tip_emitter_copy.gravity_multiplier = 0.1f;
    sword_tip_emitter_copy.color             = Fade(RED, 0.5f);
    sword_tip_emitter_copy.enabled           = true;
    str_copy(sword_tip_emitter_copy.tag_16, "sword_tip");
    
    sword_tip_emitter_index = add_particle_emitter(&sword_tip_emitter_copy);
    
    sword_tip_ground_emitter_copy.spawn_radius = 0.3f;
    sword_tip_ground_emitter_copy.over_distance     = 1;
    sword_tip_ground_emitter_copy.direction_to_move = true;
    sword_tip_ground_emitter_copy.over_time         = 0;
    sword_tip_ground_emitter_copy.speed_min         = 20;
    sword_tip_ground_emitter_copy.speed_max         = 40;
    sword_tip_ground_emitter_copy.scale_min         = 0.1f;
    sword_tip_ground_emitter_copy.scale_max         = 0.6f;
    sword_tip_ground_emitter_copy.lifetime_min      = 0.2f;
    sword_tip_ground_emitter_copy.lifetime_max      = 0.8f;
    sword_tip_ground_emitter_copy.spread            = 0.1f;
    sword_tip_ground_emitter_copy.gravity_multiplier = 0.5f;
    sword_tip_ground_emitter_copy.color             = Fade(ColorBrightness(YELLOW, 0.3f), 0.8f);
    sword_tip_ground_emitter_copy.enabled           = false;
    str_copy(sword_tip_ground_emitter_copy.tag_16, "sword_ground_t");
    
    sword_tip_ground_emitter_index = add_particle_emitter(&sword_tip_ground_emitter_copy);

    blood_emitter_copy.spawn_radius      = 2;
    blood_emitter_copy.over_distance     = 0;
    blood_emitter_copy.direction_to_move = 0;
    blood_emitter_copy.over_time         = 0;
    blood_emitter_copy.speed_min         = 5;
    blood_emitter_copy.speed_max         = 10;
    blood_emitter_copy.count_min         = 50;
    blood_emitter_copy.count_max         = 150;
    blood_emitter_copy.scale_min         = 0.2f;
    blood_emitter_copy.scale_max         = 0.8f;
    blood_emitter_copy.lifetime_min      = 0.4f;
    blood_emitter_copy.lifetime_max      = 2.5f;
    blood_emitter_copy.spread            = 0.1f;
    blood_emitter_copy.gravity_multiplier = 0.1f;
    blood_emitter_copy.color             = Fade(RED, 0.7f);
    blood_emitter_copy.enabled           = true;
    str_copy(blood_emitter_copy.tag_16, "blood");
    
    blood_emitter_index = add_particle_emitter(&blood_emitter_copy);
    
    big_blood_emitter_copy.spawn_radius      = 3;
    big_blood_emitter_copy.over_distance     = 0;
    big_blood_emitter_copy.direction_to_move = 0;
    big_blood_emitter_copy.over_time         = 0;
    big_blood_emitter_copy.speed_min         = 10;
    big_blood_emitter_copy.speed_max         = 80;
    big_blood_emitter_copy.count_min         = 40;
    big_blood_emitter_copy.count_max         = 100;
    big_blood_emitter_copy.scale_min         = 1.0f;
    big_blood_emitter_copy.scale_max         = 2.0f;
    big_blood_emitter_copy.lifetime_min      = 0.4f;
    big_blood_emitter_copy.lifetime_max      = 1.5f;
    big_blood_emitter_copy.spread            = 0.5f;
    big_blood_emitter_copy.color             = RED * 0.8f;
    big_blood_emitter_copy.enabled           = true;
    str_copy(big_blood_emitter_copy.tag_16, "big_blood");

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
    air_dust_emitter.texture = get_texture("SmokeParticle1.png");
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
    
    tires_emitter_copy = air_dust_emitter;
    tires_emitter_copy.over_distance = 5.0f;
    tires_emitter_copy.scale_min         = 5.0f;
    tires_emitter_copy.scale_max         = 10.0f;
    tires_emitter_copy.color         = Fade(tires_emitter_copy.color, 0.5f);
    str_copy(tires_emitter_copy.tag_16, "tires");
    
    explosion_emitter.spawn_radius      = 5;
    explosion_emitter.over_distance     = 1;
    explosion_emitter.direction_to_move = 0;
    explosion_emitter.over_time         = 0;
    explosion_emitter.speed_min         = 1;
    explosion_emitter.speed_max         = 100;
    explosion_emitter.count_min         = 20;
    explosion_emitter.count_max         = 30;
    explosion_emitter.scale_min         = 1;
    explosion_emitter.scale_max         = 12;
    explosion_emitter.lifetime_min      = 0.6f;
    explosion_emitter.lifetime_max      = 3.5f;
    explosion_emitter.spread            = 1.0f;
    explosion_emitter.gravity_multiplier = 1;
    explosion_emitter.color             = Fade(ORANGE, 0.7f);
    explosion_emitter.enabled           = false;
    str_copy(explosion_emitter.tag_16, "explosion");
    
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
    
    sparks_emitter.shape = PARTICLE_LINE;
    sparks_emitter.line_length_multiplier = 1.0f;
    sparks_emitter.line_width = 0.5f;
    sparks_emitter.spawn_radius       = 1.5f;
    sparks_emitter.over_distance      = 1;
    sparks_emitter.direction_to_move  = 0;
    sparks_emitter.over_time          = 0;
    sparks_emitter.speed_min          = 1;
    sparks_emitter.speed_max          = 30;
    sparks_emitter.count_min          = 30;
    sparks_emitter.count_max          = 100;
    sparks_emitter.scale_min          = 0.2f;
    sparks_emitter.scale_max          = 0.7f;
    sparks_emitter.lifetime_min       = 0.3f;
    sparks_emitter.lifetime_max       = 2.0f;
    sparks_emitter.spread             = 0.1f;
    sparks_emitter.gravity_multiplier = 0.1f;
    sparks_emitter.color              = Fade(YELLOW, 0.9f);
    sparks_emitter.enabled            = false;
    str_copy(sparks_emitter.tag_16, "sparks");
    
    big_sparks_emitter.spawn_radius       = 3.5f;
    big_sparks_emitter.over_distance      = 0;
    big_sparks_emitter.direction_to_move  = 0;
    big_sparks_emitter.over_time          = 0;
    big_sparks_emitter.speed_min          = 20;
    big_sparks_emitter.speed_max          = 80;
    big_sparks_emitter.count_min          = 30;
    big_sparks_emitter.count_max          = 100;
    big_sparks_emitter.scale_min          = 0.2f;
    big_sparks_emitter.scale_max          = 0.7f;
    big_sparks_emitter.lifetime_min       = 0.1f;
    big_sparks_emitter.lifetime_max       = 0.5f;
    big_sparks_emitter.spread             = 1.0f;
    big_sparks_emitter.gravity_multiplier = 0.5f;
    big_sparks_emitter.color              = Fade(ColorBrightness(YELLOW, 0.2f), 0.9f);
    big_sparks_emitter.enabled            = false;
    str_copy(big_sparks_emitter.tag_16, "big_sparks");

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
    
    air_emitter.spawn_type = BOX;
    air_emitter.spawn_area         = Vector2_one * 10;
    air_emitter.over_distance      = 1;
    air_emitter.direction_to_move  = 0;
    air_emitter.over_time          = 10;
    air_emitter.speed_min          = 5;
    air_emitter.speed_max          = 10;
    air_emitter.count_min          = 10;
    air_emitter.count_max          = 20;
    air_emitter.scale_min          = 0.2f;
    air_emitter.scale_max          = 1.5f;
    air_emitter.lifetime_min       = 1.0f;
    air_emitter.lifetime_max       = 10.0f;
    air_emitter.spread             = 0.0f;
    air_emitter.gravity_multiplier = -0.01f;
    air_emitter.color              = Fade(WHITE, 0.4f);
    air_emitter.enabled            = false;
    str_copy(air_emitter.tag_16, "air");
    
    ground_splash_emitter.spawn_radius             = 4;
    ground_splash_emitter.over_distance      = 0;
    ground_splash_emitter.direction_to_move  = 0;
    ground_splash_emitter.over_time          = 10;
    ground_splash_emitter.speed_min          = 5;
    ground_splash_emitter.speed_max          = 10;
    ground_splash_emitter.count_min          = 10;
    ground_splash_emitter.count_max          = 30;
    ground_splash_emitter.scale_min          = 0.8f;
    ground_splash_emitter.scale_max          = 2.5f;
    ground_splash_emitter.lifetime_min       = 0.5f;
    ground_splash_emitter.lifetime_max       = 3.0f;
    ground_splash_emitter.spread             = 0.4f;
    ground_splash_emitter.gravity_multiplier = 0.1f;
    ground_splash_emitter.color              = Fade(WHITE, 0.3f);
    ground_splash_emitter.enabled            = false;
    str_copy(ground_splash_emitter.tag_16, "ground_splash");
}
