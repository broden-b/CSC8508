#pragma once
#include "MeshMaterial.h"
#include "MeshAnimation.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		class AnimationObject {
		public:
			enum AnimationType {
				player,
				enemy
			};

			AnimationObject(AnimationType type, MeshAnimation* anim);
			~AnimationObject();

			void Update(float dt);

			void SetAnimation(MeshAnimation* anim) {
				animation = anim;
			}

			MeshAnimation* GetAnimation() { return animation; }

			int GetCurrentFrame() { return currentFrame; }

			void ResetCurrentFrame() { currentFrame = 0; }

			void SetRate(float rate) { frameRate = rate;}

			float GetRate() { return frameRate; }
			
			AnimationType GetAnimationType() { return animationType; }

		protected:
			MeshAnimation* animation;
			AnimationType animationType;

			int currentFrame;
			int nextFrame;
			float frameTime;
			float frameRate;
		};
	}
}