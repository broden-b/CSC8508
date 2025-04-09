#include "PatrolEnemy.h"

using namespace NCL;
using namespace CSC8503;

PatrolEnemy::PatrolEnemy(GameWorld* world, const std::string& name) : GameObject() {
    gameWorld = world;
    GameObject::SetName(name);
    currentPatrolPoint = 0;
    patrolCounter = 0;
    currentState = PATROL;
    playerObject = nullptr;
    warningTimer = 2.0f;
	sleepTimer = 3.0f;
    InitBehaviourTree();
}

PatrolEnemy::~PatrolEnemy() {
    delete rootSequence;
}

void PatrolEnemy::UpdateGame(float dt) {
    if (clientControlled) {
        return;
    }

    std::string stateStr;
    switch (currentState) {
    case PATROL:  stateStr = "PATROL"; break;
    case SLEEP: stateStr = "SLEEP"; break;
    case CAUGHT: stateStr = "CAUGHT"; break;
    }
    rootSequence->Execute(dt);

}

void PatrolEnemy::SetPatrolPoints(const std::vector<Vector3>& points) {
    patrolPoints = points;
}

void PatrolEnemy::SetPlayerObject(Player* player) {
    playerObject = player;
}

bool PatrolEnemy::CanSeePlayer() const {
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

void PatrolEnemy::InitBehaviourTree() {

    BehaviourAction* patrolAction = new BehaviourAction("PATROL",
        [&](float dt, BehaviourState state) -> BehaviourState {
            if (currentState != PATROL) {
                return Failure;
            }

            if (CanSeePlayer()) {
                Vector3 direction = playerObject->GetTransform().GetPosition();
                if (direction.Length() > 0.0f) {
                    float angle = atan2(-direction.x, -direction.z);
                    Quaternion targetOrientation = Quaternion::EulerAnglesToQuaternion(0, angle * 180.0f / M_PI, 0);
                    Quaternion currentOrientation = GetTransform().GetOrientation();
                    Quaternion newOrientation = Quaternion::Slerp(currentOrientation, targetOrientation, dt * 5.0f); // Adjust the interpolation speed as needed
                    GetTransform().SetOrientation(newOrientation);
                }
                GetPhysicsObject()->SetLinearVelocity(Vector3());
                currentState = CAUGHT;

                return Success;
            }

            SetObjectAnimationState(Walk);
            Vector3 targetPoint = patrolPoints[currentPatrolPoint];
            Vector3 direction = targetPoint - transform.GetPosition();
           
            float distance = Vector::Length(direction);

            if (distance < 1.0f) {
                currentPatrolPoint = (currentPatrolPoint + 1) % patrolPoints.size();
                patrolCounter++;

                if (patrolCounter >= patrolPoints.size()) {
                    GetPhysicsObject()->SetLinearVelocity(Vector3());
                    currentState = SLEEP;
                    sleepTimer = MAX_SLEEP_TIME;
                    patrolCounter = 0;
                    return Success;
                }
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

    BehaviourAction* sleepAction = new BehaviourAction("SLEEP",
        [&](float dt, BehaviourState state) -> BehaviourState {
            if (currentState != SLEEP) {
                return Failure;
            }

            SetObjectAnimationState(Idle);
            sleepTimer -= dt;
            if (sleepTimer <= 0.0f) {
                currentState = PATROL;
                return Success;
            }
            return Ongoing;
        });

    BehaviourAction* catchAction = new BehaviourAction("CAUGHT",
        [&](float dt, BehaviourState state) -> BehaviourState {
            if (currentState != CAUGHT) {
                return Failure;
            }
            Vector3 direction = playerObject->GetTransform().GetPosition() - transform.GetPosition();;
            if (direction.Length() > 0.0f) {
                float angle = atan2(-direction.x, -direction.z);
                Quaternion targetOrientation = Quaternion::EulerAnglesToQuaternion(0, angle * 180.0f / M_PI, 0);
                Quaternion currentOrientation = GetTransform().GetOrientation();
                Quaternion newOrientation = Quaternion::Slerp(currentOrientation, targetOrientation, dt * 5.0f); // Adjust the interpolation speed as needed
                GetTransform().SetOrientation(newOrientation);
            }
            SetObjectAnimationState(Caught);
            if (warningTimer > 0.0f && playerObject->GetVisible() == true) {
                std::cout << "Warning: " << std::to_string(warningTimer);
                warningTimer -= dt;
                return Ongoing;
            }

            if (!CanSeePlayer()) {
                warningTimer = 2.0f;
                currentState = PATROL;
                return Success;
            }

            else {
                warningTimer = 2.0f;
				OnCatch(playerObject);
				patrolCounter = 0;
				currentState = PATROL;
                return Success;
            }
            return Ongoing;
        });


    modeSelector = new BehaviourSelector("Mode Selector");
    modeSelector->AddChild(patrolAction);
    modeSelector->AddChild(catchAction);
	modeSelector->AddChild(sleepAction);

    rootSequence = new BehaviourSequence("Root");
    rootSequence->AddChild(modeSelector);
}