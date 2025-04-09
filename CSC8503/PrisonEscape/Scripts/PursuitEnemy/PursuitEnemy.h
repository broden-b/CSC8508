#pragma once
#include "GameObject.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "Ray.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "../Player/Player.h"
#include "RenderObject.h"

namespace NCL {
    namespace CSC8503 {
        class PursuitEnemy : public GameObject {
        public:
            PursuitEnemy(GameWorld* world, const std::string& name);
            ~PursuitEnemy();

            void UpdateGame(float dt);
            void SetPatrolPoints(const std::vector<Vector3>& points);
            void SetPlayerObject(Player* player);

            void OnCollisionBegin(GameObject* otherObject) override {
                if (otherObject == playerObject) {
                    currentState = PATROL;
                    pursuitTimer = 0.0f;
                    patrolCounter = 0;

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
            float pursuitTimer;


            const float MAX_PURSUIT_TIME = 5.0f;
            const float VISION_RANGE = 15.0f;

            enum AIState {
                PATROL,
                PURSUIT
            };
            AIState currentState;

        private:
            bool clientControlled = false;
        };
    }
}
