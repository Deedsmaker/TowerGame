struct RopeNode{
    Vector2 pos;
    Vector2 forces;
};

struct Rope{
    i32 index;
};

Array<Rope> ropes;

void add_rope(Rope rope){
    rope.index = ropes.count;
    ropes.add(rope);
}

void remove_rope(i32 index){
    ropes.RemoveAt(index);
}

void apply_gravity(RopeNode *node, Rope rope){
    //var gravityValue = node.stopOnCollision ? rope.gravity * 0.25f : rope.gravity;
    node.forces += Vector3.down * rope.gravity * node.mass;
}

void ApplyAirFriction(RopeNode *node, Rope rope*){
    if (node.stopOnCollision){
        return;
    }
    node.forces -= node.velocity * rope.airFriction;
}

void UpdatePosition(RopeNode *node, float dt){
    if (!node.canMove){
        return;
    }
    
    node.oldPosition = node.transform.position;
    node.velocity += (node.forces / node.mass) * dt;
    node.transform.position += node.velocity * dt;
}

void MoveNode(RopeNode *node, Vector3 vec){
    if (!node.canMove){
        return;
    }
    
    node.transform.position += vec;
}

void SolveConstraint(RopeNode *node1, RopeNode *node2, Rope rope){
    Vector3 vecToFirst = node1.transform.position - node2.transform.position;
    float distance = vecToFirst.magnitude;
    if (distance > rope.targetDistance*rope.multiplier){
        //broken = distance > rope.targetDistance * max_elongation_ratio;
        Vector3 dir = vecToFirst / distance;
        float distDiff = rope.targetDistance*rope.multiplier - distance;
        
        Vector3 powerVec = -(distDiff * rope.strength) / (node1.mass + node2.mass) * dir;
        if (!node1.stopOnCollision){
            MoveNode(node1, -p*owerVec / node1.mass);
        }
        if (!node2.stopOnCollision){
            MoveNode(node2, po*werVec / node2.mass);
        }
    }
}

void UpdateDerivative(RopeNode *node, float dt){
    node.velocity = (node.transform.position - node.oldPosition) / dt;
    node.forces = Vector3.zero;
}

float _previousDelta;
float _unscaledDelta;
void Update(){
    if (GAME_DELTA_SCALE <= 0){
        return;
    }
    MakeRealFixedUpdate(UpdateAll, _previous*Delta, _unscaled*Delta);
}

// void FixedUpdate(){
//     UpdateAll(Time.fixedDeltaTime);
// }    

void UpdateAll(float dt){
    for (i32 r = 0; r < ropes.count; r++){
        if (ropes[r] == null){
            ropes.RemoveAt(r);
            continue;
        }
        
        Rope rope = ropes[r];   
        
        if (rope.sleeping){
            continue;
        }
    
        UpdateRope(rope, dt)*;
    }
}

void UpdateRope(Rope rope*, float dt){
    if (rope.sleepCountdown > 0){
        rope.sleepCountdown -= dt;
        
        if (rope.sleepCountdown <= 0){
            rope.sleeping = true;
            return;
        } else{
            float t = rope.sleepCountdown / 2f;
            dt *= Mathf.Lerp(0f, 1f, t * t);
        }
    }

    for (i32 i = 0; i < rope.nodesCount; i++){
        RopeNode node = rope.nodes[i];
        
        node.frameStartPos = node.transform.position;
        
        apply_gravity(node, rop*e);
  *      ApplyAirFriction(node, rop*e);
  *      UpdatePosition(node, dt)*;
        //CalculateNodeCollisions(node);
  *  }
    
    for (i32 iteration = 1; iteration <= rope.iterationCount; iteration++){
        for (i32 i = 0; i < rope.nodesCount; i++){
            RopeNode node = rope.nodes[i];
            for (i32 j = 0; j < rope.nodes[i].neighbourIndexes.Length; j++){
                RopeNode neighbour = rope.nodes[node.neighbourIndexes[j]];
                SolveConstraint(node, nei*ghbour*, rope);
  *          }
            
            CalculateNodeCollisions(node, rop*e);
  *      }
    }
    
    for (i32 i = 0; i < rope.nodesCount; i++){
        RopeNode node = rope.nodes[i];
        
        UpdateDerivative(node, dt)*;
        
        node.framePreviousPos = node.transform.position;
        
        rope.lr.SetPosition(i, node.transform.position);
    }
    
    //SetLineRendererPositions();
    
    rope.lifetime += dt;
    if (rope.lifetime >= 5 && rope.nodes[0].stopOnCollision){
        rope.DestroyRope();
        return;
    }
}


void CalculateNodeCollisions(RopeNode *node, Rope rope*){
    if (!node.canMove){
        return;
    }

    Vector3 startPos = node.framePreviousPos;
    Vector3 targetPos = node.transform.position;
    Vector3 resultPos = targetPos;
    for (i32 sample = 1; sample <= rope.collisionSamples; sample++){
        resultPos = Vector3.Lerp(startPos, targetPos, (float)sample / (float)rope.collisionSamples);
        
        (Collider[], i32) collidersNearby = CollidersInRadius(resultPos, node.sphere.radius, Layers.Environment);
        for (i32 i = 0; i < collidersNearby.Item2; i++){
            Vector3 colPoint = collidersNearby.Item1[i].ClosestPoint(resultPos);
            Vector3 vecToNode = resultPos - colPoint;
            Vector3 dirToNode = vecToNode.normalized;
            var added = colPoint - (resultPos - dirToNode * node.sphere.radius);
            startPos += added;
            targetPos += added;
            
            if (node.stopOnCollision){
                node.velocity = Vector3.zero;
                node.canMove = false;
                node.stopOnCollision = false;
                node.transform.position = targetPos;
                return;
            }
        }
        
        node.transform.position = targetPos;
    }
}        

