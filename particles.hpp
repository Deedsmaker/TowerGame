#pragma once

enum Particle_Shape{
    RECT
};

struct Particle{
    Particle_Shape shape = RECT;
    Vector2 position;
    Vector2 scale = {1, 1};
    Vector2 velocity;
    Vector2 original_scale;
    f32 lifetime;
    f32 max_lifetime;
    
    //b32 colliding;
    
    Color color = YELLOW;
};

struct Particle_Emitter{
    Particle_Shape shape = RECT;
    
    b32 enabled = true;
    
    Vector2 position = {0, 0};
    Vector2 last_emitted_position = {0, 0};
    Vector2 direction = Vector2_up;
    
    b32 emitting;
    f32 emitting_timer;
    f32 over_time;
    f32 over_distance;
    u32 count_min = 10;
    u32 count_max = 50;
    f32 speed_min = 10;
    f32 speed_max = 50;  
    f32 scale_min = 0.1f;
    f32 scale_max = 0.5f;
    f32 spread = 0.2f;
    f32 lifetime_min = 0.5f;
    f32 lifetime_max = 2;
    
    //f32 colliding_chance = 1.0f;
    
    Color color = YELLOW;
};


void shoot_particle(Particle_Emitter emitter, Vector2 position, Vector2 direction, f32 speed_multiplier){
    Particle particle = {};
    particle.position = position;
    
    particle.shape = emitter.shape;
    
    f32 scale = rnd(emitter.scale_min, emitter.scale_max);
    particle.scale = {scale, scale};
    particle.original_scale = {scale, scale};
    
    f32 x_direction = rnd(direction.x - emitter.spread, direction.x + emitter.spread);
    f32 y_direction = rnd(direction.y - 0.2f, direction.y + 0.2f);
    Vector2 randomized_direction = {x_direction, y_direction};
                                    
    f32 randomized_speed = rnd(emitter.speed_min * speed_multiplier, emitter.speed_max * speed_multiplier);
    
    f32 lifetime = rnd(emitter.lifetime_min, emitter.lifetime_max);
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

void update_overtime_emitter(Particle_Emitter *emitter, f32 count_multiplier, f32 speed_multiplier){
    emitter->emitting_timer += dt;
    f32 emit_delay = 1.0f / (emitter->over_time * count_multiplier);
    while (emitter->emitting_timer >= emit_delay){
        emitter->emitting_timer -= emit_delay;
        shoot_particle(*emitter, emitter->position, emitter->direction, speed_multiplier);
    }
}

void update_overdistance_emitter(Particle_Emitter *emitter, f32 count_multiplier, f32 speed_multiplier){
    Vector2 current_emitter_position = emitter->position;

    Vector2 moved_vector = emitter->position - emitter->last_emitted_position;
    Vector2 moved_direction = normalized(moved_vector);
    f32 moved_distance = magnitude(moved_vector);
    
    f32 distance_to_emit = 1.0f / emitter->over_distance;
    
    while (moved_distance > distance_to_emit){
        moved_distance -= distance_to_emit;
        emitter->last_emitted_position += moved_direction * distance_to_emit;
        emitter->position = emitter->last_emitted_position;
        shoot_particle(*emitter, emitter->position, emitter->direction, speed_multiplier);
    }
    
    emitter->position = current_emitter_position;
}

void update_emitters(){
    for (int i = 0; i < context.emitters.count; i++){
        Particle_Emitter *emitter = context.emitters.get_ptr(i);
        
        if (!emitter->enabled){
            continue;
        }
        
        if (emitter->over_time > 0){
            update_overtime_emitter(emitter, 1, 1);
        }
        
        if (emitter->over_distance > 0){
            update_overdistance_emitter(emitter, 1, 1);
        }
    }
}


void update_particles(){
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
