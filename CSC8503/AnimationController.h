#pragma once
#include "GameWorld.h"
#include "MeshAnimation.h"
#include "RenderObject.h"
#include "GameWorld.h"
#include <map>


namespace NCL {
	namespace CSC8503
	{
		class AnimationObject;
		class Player;
		class PatrolEnemy;
		class AnimationController {
		public:
			AnimationController(GameWorld& world, std::map<std::string, MeshAnimation*>& preLoadedAnimationList);
			~AnimationController();

			void Clear();
			void Update(float dt, vector<GameObject*> updatableObjects);
			void UpdateAllAnimations(float dt, vector<GameObject*> updatableObjects);
			void UpdateCurrentFrames(float dt);

			void SetObjectList(vector<GameObject*> UpdatableObjects);
			void SetAnimationState(GameObject* obj, GameObject::AnimationState state);

			std::map<std::string, MeshAnimation*>& GetPreLoadedAnimationList() { return mPreLoadedAnimationList; }


		protected:
			GameWorld& mGameWorld;
			vector<AnimationObject*> mAnimationList;
			vector<Player*> mPlayersList;
			vector<PatrolEnemy*> mPatrolEnemyList;

			Mesh* mMesh;
			MeshAnimation* mAnim;

			GameObject::AnimationState mPlayerState;
			std::map<std::string, MeshAnimation*>& mPreLoadedAnimationList;

			std::map<GameObject::AnimationState, std::string> mPlayerAnimationMap;
			std::map<GameObject::AnimationState, std::string> mPatrolEnemyAnimationMap;

			void InitPlayerAnimMap();
			void InitPatrolEnemyAnimMap();

		};
	}
}