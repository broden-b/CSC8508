#pragma once
#include "GameObject.h"
#include "Vector.h"
#include "CollisionVolume.h"
#include "CollisionDetection.h"
#include "PrisonEscape/Scripts/player/Player.h"
namespace NCL {
    namespace CSC8503 {
        class HidingArea : public GameObject {
        public:
            HidingArea(const Vector3& position, const Vector3& size);
            ~HidingArea();

            void Update(float dt);


            void OnCollisionBegin(GameObject* otherObject) override;
            

        private:
            Vector3 hidingAreaPosition;
            Vector3 hidingAreaSize;
        };
    }
}