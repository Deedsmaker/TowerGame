#pragma once

void shoot_particle(Particle_Emitter emitter, Vector2 position, Vector2 direction, f32 speed_multiplier){
    Particle particle = {};
    particle.position = position;
    if (emitter.spawn_radius > 0){
        particle.position += rnd_on_circle() * emitter.spawn_radius;
    }
    
    particle.shape = emitter.shape;
    
    f32 scale = rnd(emitter.scale_min, emitter.scale_max);
    particle.scale = {scale, scale};
    particle.original_scale = {scale, scale};
    
    f32 x_direction = rnd(direction.x - emitter.spread, direction.x + emitter.spread);
    f32 y_direction = rnd(direction.y - 0.2f, direction.y + 0.2f);
    Vector2 randomized_direction = {x_direction, y_direction};
                                    
    f32 randomized_speed = rnd(emitter.speed_min * speed_multiplier, emitter.speed_max * speed_multiplier);
    
    f32 lifetime = rnd(emitter.lifetime_min, emitter.lifetime_max) * emitter.lifetime_multiplier;
    particle.max_lifetime = lifetime;
    
    // if (emitter.try_splash){
    //     if (rnd(0.0f, 1.0f) <= emitter.splash_chance){
    //         particle.leave_splash = 1;
    //     } 
    // }
    
    // if (emitter.colliding_chance >= 1.0f){ 
    //     particle.colliding = 1;
    // } else if (emitter.colliding_chance <= 0){
    //     particle.colliding = 0;
    // } else{
    //     if (rnd(0.0f, 1.0f) <= emitter.colliding_chance){
    //         particle.colliding = 1;
    //     } 
    //}
    
    particle.color = emitter.color;
    
    particle.velocity = multiply(randomized_direction, randomized_speed);
    
    context.particles.add(particle);
}

void emit_particles(Particle_Emitter emitter, Vector2 position, Vector2 direction, f32 count_multiplier, f32 speed_multiplier){
    normalize(&direction);
    int count = rnd((int)emitter.count_min, (int)emitter.count_max);
    count *= count_multiplier; 
    
    for (int i = 0; i < count; i++){
        shoot_particle(emitter, position, direction, speed_multiplier);
    }
}

void update_overtime_emitter(Particle_Emitter *emitter){
    emitter->emitting_timer += core.time.dt;
    f32 emit_delay = 1.0f / (emitter->over_time * emitter->count_multiplier);
    while (emitter->emitting_timer >= emit_delay){
        emitter->emitting_timer -= emit_delay;
        shoot_particle(*emitter, emitter->position, emitter->direction, emitter->speed_multiplier);
    }
}

void update_overdistance_emitter(Particle_Emitter *emitter){
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
        shoot_particle(*emitter, emitter->position, emitter->direction, emitter->speed_multiplier);
    }
    
    emitter->position = current_emitter_position;
}

void enable_emitter(Particle_Emitter *emitter){
    emitter->enabled = true;
    emitter->last_emitted_position = emitter->position;
}

void update_emitter(Particle_Emitter *emitter){
    emitter->emitter_lifetime += core.time.dt;
    
    if (emitter->over_time > 0){
        update_overtime_emitter(emitter);
    }
    
    if (emitter->over_distance > 0){
        update_overdistance_emitter(emitter);
    }
}

void update_emitters(){
    for (int i = 0; i < context.emitters.count; i++){
        Particle_Emitter *emitter = context.emitters.get_ptr(i);
        
        if (emitter->destroyed){
        }
        
        if (!emitter->enabled){
            emitter->emitter_lifetime = 0;
            continue;
        }
        update_emitter(emitter);        
    }
}


void update_particles(){
    f32 dt = core.time.dt;

    for (int i = 0; i < context.particles.count; i++){
        Particle *particle = context.particles.get_ptr(i);
        particle->lifetime += dt;
        
        if (particle->lifetime >= particle->max_lifetime){
            context.particles.remove(i);
            continue;
        }
        
        f32 t_lifetime = particle->lifetime / particle->max_lifetime;
        particle->scale = lerp(particle->original_scale, Vector2_zero, t_lifetime * t_lifetime);
        
        f32 gravity = -50;
        particle->velocity.y += gravity * dt;
        
        particle->velocity += frame_on_circle_rnd * 100 * dt;
        
        // if (particle->colliding){
        //     calculate_particle_tilemap_collisions(game, particle, check_tilemap_collisions(game, particle->velocity, particle->entity));
        // }
        
        Vector2 next_position = add(particle->position, multiply(particle->velocity, dt));
        
        particle->position = next_position;
        
        // loop_world_position(game, &particle->entity.position);
        
        // if (particle->leave_splash){
        //     add_splash(game, particle->entity, particle-> splash_color == 0 ? particle->color : particle->splash_color);
        // }
    }
}

global_variable Particle_Emitter *chainsaw_emitter;
global_variable Particle_Emitter *sword_tip_emitter;
global_variable Particle_Emitter *blood_emitter;
global_variable Particle_Emitter rifle_bullet_emitter;

void free_emitter(Particle_Emitter *emitter){
    emitter = NULL;
}

void copy_emitter(Particle_Emitter *dest, Particle_Emitter *src, Vector2 start_position){
    *dest = *src;
    dest->last_emitted_position = start_position;
    dest->position              = start_position;
}

void setup_particles(){
    free_emitter(chainsaw_emitter);
    chainsaw_emitter = add_emitter();
    chainsaw_emitter->spawn_radius = 0.2f;
    chainsaw_emitter->over_distance = 3;
    chainsaw_emitter->over_time     = 0;
    chainsaw_emitter->speed_min     = 5;
    chainsaw_emitter->speed_max     = 20;
    chainsaw_emitter->scale_min     = 0.1f;
    chainsaw_emitter->scale_max     = 0.6f;
    chainsaw_emitter->lifetime_min  = 0.05f;
    chainsaw_emitter->lifetime_max  = 0.3f;
    chainsaw_emitter->spread        = 1;
    chainsaw_emitter->enabled       = false;

    free_emitter(sword_tip_emitter);
    sword_tip_emitter = add_emitter();
    sword_tip_emitter->over_distance     = 3;
    sword_tip_emitter->direction_to_move = true;
    sword_tip_emitter->over_time         = 0;
    sword_tip_emitter->speed_min         = 5;
    sword_tip_emitter->speed_max         = 20;
    sword_tip_emitter->scale_min         = 0.1f;
    sword_tip_emitter->scale_max         = 0.6f;
    sword_tip_emitter->lifetime_min      = 0.05f;
    sword_tip_emitter->lifetime_max      = 0.3f;
    sword_tip_emitter->spread            = 0.4f;
    sword_tip_emitter->color             = RED * 0.9f;
    sword_tip_emitter->enabled           = true;

    free_emitter(blood_emitter);
    blood_emitter = add_emitter();
    blood_emitter->spawn_radius      = 2;
    blood_emitter->over_distance     = 0;
    blood_emitter->direction_to_move = 0;
    blood_emitter->over_time         = 0;
    blood_emitter->speed_min         = 5;
    blood_emitter->speed_max         = 40;
    blood_emitter->count_min         = 10;
    blood_emitter->count_max         = 40;
    blood_emitter->scale_min         = 0.4f;
    blood_emitter->scale_max         = 1.2f;
    blood_emitter->lifetime_min      = 0.1f;
    blood_emitter->lifetime_max      = 0.9f;
    blood_emitter->spread            = 1.0f;
    blood_emitter->color             = RED * 0.7f;
    blood_emitter->enabled           = true;

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
    rifle_bullet_emitter.spread            = 1.0f;
    rifle_bullet_emitter.color             = WHITE * 0.9f;
    rifle_bullet_emitter.enabled           = false;
}
