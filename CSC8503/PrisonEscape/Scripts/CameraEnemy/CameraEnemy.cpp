#include "CameraEnemy.h"

using namespace NCL;
using namespace CSC8503;

CameraEnemy::CameraEnemy(GameWorld* world, const std::string& name) : GameObject() {
    gameWorld = world;
    GameObject::SetName(name);
    currentState = WATCHING;
    playerObject = nullptr;
    power = true;
    warningTimer = 2.0f;
	sleepTimer = 3.0f;
    InitBehaviourTree();
}

CameraEnemy::~CameraEnemy() {
    delete rootSequence;
}

void CameraEnemy::UpdateGame(float dt) {
    if (clientControlled) {
        return;
    }

    std::string stateStr;
    switch (currentState) {
    case WATCHING:  stateStr = "WATCHING"; break;
    case SLEEP: stateStr = "SLEEP"; break;
    case CAUGHT: stateStr = "CAUGHT"; break;
    }
    rootSequence->Execute(dt);
}

void CameraEnemy::SetPlayerObject(Player* player) {
    playerObject = player;
}

bool CameraEnemy::CanSeePlayer() const {
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

void CameraEnemy::InitBehaviourTree() {

    BehaviourAction* patrolAction = new BehaviourAction("WATCHING",
        [&](float dt, BehaviourState state) -> BehaviourState {
            std::cout << "watching\n";
            if (currentState != WATCHING) {
                return Failure;
            }

            if (CanSeePlayer()) {
                currentState = CAUGHT;
                return Success;
            }

            if (!power) {
                currentState = SLEEP;
                sleepTimer = MAX_SLEEP_TIME;
                return Success;
            }
            return Ongoing;
        });

    BehaviourAction* sleepAction = new BehaviourAction("SLEEP",
        [&](float dt, BehaviourState state) -> BehaviourState {
            std::cout << "sleeping\n";
            if (currentState != SLEEP) {
                return Failure;
            }

            sleepTimer -= dt;
            if (sleepTimer <= 0.0f) {
                currentState = WATCHING;
                return Success;
            }
            return Ongoing;
        });

    BehaviourAction* catchAction = new BehaviourAction("CAUGHT",
        [&](float dt, BehaviourState state) -> BehaviourState {
            std::cout << "catching";
            if (currentState != CAUGHT) {
                return Failure;
            }

            if (warningTimer > 0.0f) {
                std::cout << "Warning: " << std::to_string(warningTimer);
                warningTimer -= dt;
                return Ongoing;
            }

            if (!CanSeePlayer()) {
                warningTimer = 2.0f;
                currentState = WATCHING;
                return Success;
            }

            else {
                warningTimer = 2.0f;
				OnCatch(playerObject);
				currentState = WATCHING;
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