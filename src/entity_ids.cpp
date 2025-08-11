void notify_destruction_to_connected_entities(Entity *entity) {
    // Notifying entities pointing at me and entities that we're connected to.

    for_array(i, (&entity->entities_pointing_at_me)) {
        // Entity *pointing_entity = get_entity
        // @TODO: Right now it think that we should keep that pointing_at_me thing and store in that array flag about type 
        // of pointing (trigger connected, trigger tracking, kill switch connected etc. basically for every type.
        // Here we will be checking in switch type of pointing where on default could be assert(false) and we will basically
        // know that we did not forget to add something. 
        // That will highly restrict us and maybe will add some friction, but we will know 
        // that at any given moment all entities have correct referring ids and get_entity will never return NULL.
        //
        // Also that means that responsibility to remove id from entity that pointing at me is completely on that function
        // and entities that actually pointing at me will always have valid ids.
        //
        // But don't forget that when entity that pointing at me is destroyed itself it should go to me and remove 
        // it's id from here.
        //
        // Actually right now I realized that we're not exactly *should* have type of pointing, because when entity is 
        // destroyed it should remove reference to itself from every instance of pointing, but type will help us 
        // know that we don't forget to check anything so we should try that.
        //
        // Maybe we could go without type - we will just check all we know and if we did not find anything where this 
        // entity id is stored - we're asserting. Thing that bugs me is that we really will not know how exactly this entity
        // is pointing at us and that maybe will create bugs. But maybe all of this we could catch with asserts.
        
        get_entity(
    }
}

inline i32 register_connected_entity(Entity *entity, i32 connected_id) {
    if (!entity->connected_entities.contains(connected_id)) {
        entity->connected_entities.append(connected_id);
    }
}

void remove_connected_entity(Entity *entity, i32 to_remove) {
    
}
