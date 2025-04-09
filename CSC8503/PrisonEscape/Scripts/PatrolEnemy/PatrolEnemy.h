#pragma once
#include "GameObject.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "Ray.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "../Player/Player.h"

namespace NCL {
    namespace CSC8503 {
        class PatrolEnemy : public GameObject {
        public:
            PatrolEnemy(GameWorld* world, const std::string& name);
            ~PatrolEnemy();

            void UpdateGame(float dt);
            void SetPatrolPoints(const std::vector<Vector3>& points);
            void SetPlayerObject(Player* player);

            void OnCatch(Player* otherObject) {
                if (otherObject == playerObject) {
                    playerObject->SetObjectAnimationState(Caught);
                    playerObject->SetHealth(playerObject->GetHealth() - 1);
                    playerObject->GetRenderObject()->GetTransform()->SetPosition(playerObject->GetSpawn());
                }
            }

            void SetClientControlled(bool isClient) {
                clientControlled = isClient;
            }

        protected:
            void InitBehaviourTree();
            bool CanSeePlayer() const;

            BehaviourSequence* rootSequence;
            BehaviourSelector* modeSelector;

            GameWorld* gameWorld;
            Player* playerObject;
            std::vector<Vector3> patrolPoints;

            int currentPatrolPoint;
            int patrolCounter;
            float warningTimer;
            float sleepTimer;

            const float VISION_RANGE = 15.0f;
			const float MAX_SLEEP_TIME = 3.0f;

            enum AIState {
                PATROL,
                SLEEP,
                CAUGHT
            };
            AIState currentState;

        private:
            bool clientControlled = false;
        };
    }
}
