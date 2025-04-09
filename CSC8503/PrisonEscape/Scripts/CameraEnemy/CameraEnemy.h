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
        class CameraEnemy : public GameObject {
        public:
            CameraEnemy(GameWorld* world, const std::string& name);
            ~CameraEnemy();

            void UpdateGame(float dt);
            void SetPlayerObject(Player* player);

            void OnCatch(Player* otherObject) {
                if (otherObject == playerObject) {
                    playerObject->SetHealth(playerObject->GetHealth() - 1);
                    playerObject->GetRenderObject()->GetTransform()->SetPosition(playerObject->GetSpawn());
                }
            }

            void SetClientControlled(bool isClient) {
                clientControlled = isClient;
            }

            bool GetPower() { return power; }
            void SetPower(bool p) { power = p; }

        protected:
            void InitBehaviourTree();
            bool CanSeePlayer() const;

            BehaviourSequence* rootSequence;
            BehaviourSelector* modeSelector;

            GameWorld* gameWorld;
            Player* playerObject;

            float warningTimer;
            float sleepTimer;
            bool power;

            const float VISION_RANGE = 15.0f;
			const float MAX_SLEEP_TIME = 3.0f;

            enum AIState {
                WATCHING,
                SLEEP,
                CAUGHT
            };
            AIState currentState;

        private:
            bool clientControlled = false;
        };


    }
}
