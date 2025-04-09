#include "PursuitEnemy.h"

using namespace NCL;
using namespace CSC8503;

PursuitEnemy::PursuitEnemy(GameWorld* world, const std::string& name) : GameObject() {
    gameWorld = world;
    GameObject::SetName(name);
    currentPatrolPoint = 0;
    patrolCounter = 0;
    pursuitTimer = 0.0f;
    currentState = PATROL;
    playerObject = nullptr;
    InitBehaviourTree();
}

PursuitEnemy::~PursuitEnemy() {
    delete rootSequence;
}

void PursuitEnemy::UpdateGame(float dt) {
    if (clientControlled) {
        return;
    }

    std::string stateStr;
    switch (currentState) {
    case PATROL:  stateStr = "PATROL"; break;
    case PURSUIT: stateStr = "PURSUIT"; break;
    }
    Vector3 targetPoint = patrolPoints[currentPatrolPoint];
    
    rootSequence->Execute(dt);
}

void PursuitEnemy::SetPatrolPoints(const std::vector<Vector3>& points) {
    patrolPoints = points;
}

void PursuitEnemy::SetPlayerObject(Player* player) {
    playerObject = player;
}

bool PursuitEnemy::CanSeePlayer() const {
    if (!playerObject) return false;

    if (!playerObject->GetVisible()) return false;

    Vector3 direction = playerObject->GetTransform().GetPosition() - transform.GetPosition();
    
    if (Vector::Length(direction) > VISION_RANGE) return false;

    Ray ray(transform.GetPosition(), Vector::Normalise(direction));
    RayCollision closestCollision;

    if (gameWorld->Raycast(ray, closestCollision, true, (GameObject*)this)) {
        if (closestCollision.node == playerObject) {
            return true;
        }
    }

    return false;
}

void PursuitEnemy::InitBehaviourTree() {
    BehaviourAction* patrolAction = new BehaviourAction("Patrol",
        [&](float dt, BehaviourState state) -> BehaviourState {
            if (currentState != PATROL) {
                return Failure;
            }

            if (CanSeePlayer()) {
                currentState = PURSUIT;
                pursuitTimer = MAX_PURSUIT_TIME;
                return Success;
            }

            Vector3 targetPoint = patrolPoints[currentPatrolPoint];
            Vector3 direction = targetPoint - transform.GetPosition();
            float distance = Vector::Length(direction);

            if (distance < 1.0f) {
                currentPatrolPoint = (currentPatrolPoint + 1) % patrolPoints.size();
                patrolCounter++;
            }

            Vector3 force = Vector::Normalise(direction) * 100.0f;
            GetPhysicsObject()->AddForce(force);

            Vector3 currentVel = GetPhysicsObject()->GetLinearVelocity();
            GetPhysicsObject()->AddForce(-currentVel * 10.0f);
            if (direction.Length() > 0.0f) {
                float angle = atan2(-direction.x, -direction.z);
                Quaternion targetOrientation = Quaternion::EulerAnglesToQuaternion(0, angle * 180.0f / M_PI, 0);
                Quaternion currentOrientation = GetTransform().GetOrientation();
                Quaternion newOrientation = Quaternion::Slerp(currentOrientation, targetOrientation, dt * 5.0f); // Adjust the interpolation speed as needed
                GetTransform().SetOrientation(newOrientation);
            }
            return Ongoing;
        });

    BehaviourAction* pursuitAction = new BehaviourAction("Pursuit",
        [&](float dt, BehaviourState state) -> BehaviourState {
            if (currentState != PURSUIT) {
                return Failure;
            }
            Vector3 direction = playerObject->GetTransform().GetPosition() - transform.GetPosition();
            if (direction.Length() > 0.0f) {
                float angle = atan2(-direction.x, -direction.z);
                Quaternion targetOrientation = Quaternion::EulerAnglesToQuaternion(0, angle * 180.0f / M_PI, 0);
                Quaternion currentOrientation = GetTransform().GetOrientation();
                Quaternion newOrientation = Quaternion::Slerp(currentOrientation, targetOrientation, dt * 5.0f); // Adjust the interpolation speed as needed
                GetTransform().SetOrientation(newOrientation);
            }
            pursuitTimer -= dt;
            if (pursuitTimer <= 0.0f || !playerObject->GetVisible()) {
                GetPhysicsObject()->SetLinearVelocity(Vector3());
                currentState = PATROL;
                return Success;
            }

            if (playerObject) {
                Vector3 direction = playerObject->GetTransform().GetPosition() - transform.GetPosition();
                Vector3 force = Vector::Normalise(direction) * 100.0f;
                GetPhysicsObject()->AddForce(force);

                Vector3 currentVel = GetPhysicsObject()->GetLinearVelocity();
                GetPhysicsObject()->AddForce(-currentVel * 10.0f);
            }


            return Ongoing;
        });

    modeSelector = new BehaviourSelector("Mode Selector");
    modeSelector->AddChild(pursuitAction);
    modeSelector->AddChild(patrolAction);

    rootSequence = new BehaviourSequence("Root");
    rootSequence->AddChild(modeSelector);
}