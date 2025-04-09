#include "AnimationController.h"
#include "Assets.h"
#include "AnimationObject.h"
#include "RenderObject.h"
#include "Mesh.h"
#include <map>
#include <string>

AnimationController::AnimationController(GameWorld& world, std::map<std::string, MeshAnimation*>& preLoadedAnimationList) : mGameWorld(world), mPreLoadedAnimationList(preLoadedAnimationList)
{
	mMesh = nullptr;
	mAnim = nullptr;
	InitPlayerAnimMap();
	InitPatrolEnemyAnimMap();
}

AnimationController::~AnimationController(){}

void AnimationController::Clear() {
	mAnimationList.clear();
	mPlayersList.clear();
	mPatrolEnemyList.clear();
}

void AnimationController::Update(float dt, vector<GameObject*> animationObjects)
{
	UpdateCurrentFrames(dt);
	UpdateAllAnimations(dt, animationObjects);
}


void AnimationController::UpdateAllAnimations(float dt, vector<GameObject*> animatedObjects) {
		for (GameObject* obj : animatedObjects) {
		if (obj->GetRenderObject()->GetAnimObject()) {
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimObject();
			if (animObj != nullptr) {

				int currentFrame = animObj->GetCurrentFrame();
				mMesh = obj->GetRenderObject()->GetMesh();
				mAnim = animObj->GetAnimation();

				const Matrix4* invBindPose = mMesh->GetInverseBindPose().data();
				const Matrix4* frameData = mAnim->GetJointData(currentFrame);

				const int* bindPoseIndices = mMesh->GetBindPoseIndices();
				std::vector<std::vector<Matrix4>> frameMatricesVec;
				for (unsigned int i = 0; i < mMesh->GetSubMeshCount(); ++i) {
					Mesh::SubMeshPoses pose;
					mMesh->GetBindPoseState(i, pose);

					vector<Matrix4> frameMatrices;
					for (unsigned int b = 0; b < pose.count; ++b) {
						int jointID = bindPoseIndices[pose.start + b]; // safe

						Matrix4 mat = frameData[jointID] * invBindPose[pose.start + b]; // all three quaternions


						frameMatrices.emplace_back(mat);
					}

					frameMatricesVec.emplace_back(frameMatrices);
				}

				obj->GetRenderObject()->SetCurrentFrame(currentFrame);
				obj->GetRenderObject()->SetFrameMatricesVector(frameMatricesVec);
				
				frameMatricesVec.clear();
			}
		}
	}
}
void AnimationController::InitPlayerAnimMap() {
	mPlayerAnimationMap = {
		{GameObject::AnimationState::Idle, "PlayerIdle"},
		{GameObject::AnimationState::Walk, "PlayerWalk"},
		{GameObject::AnimationState::Caught, "PlayerCaught"},
	};
}

void AnimationController::InitPatrolEnemyAnimMap() {
	mPatrolEnemyAnimationMap = {
		{GameObject::AnimationState::Idle, "PatrolIdle"},
		{GameObject::AnimationState::Walk, "PatrolWalk"},
		{GameObject::AnimationState::Caught, "PatrolCaught"},
	};
}

void AnimationController::SetAnimationState(GameObject* obj, GameObject::AnimationState state) {

	obj->GetRenderObject()->GetAnimObject()->ResetCurrentFrame();

	AnimationObject::AnimationType animationType = obj->GetRenderObject()->GetAnimObject()->GetAnimationType();
	std::map<GameObject::AnimationState, std::string>& animationMap = animationType == AnimationObject::AnimationType::player ? mPlayerAnimationMap : mPatrolEnemyAnimationMap;

	const std::string& animationName = animationMap[state];

	MeshAnimation* animation = mPreLoadedAnimationList[animationName];

	if (animationName == "PlayerSprint" || animationName == "EnemySprint") {
		obj->GetRenderObject()->GetAnimObject()->SetRate(2.0);
	}
	else {
		obj->GetRenderObject()->GetAnimObject()->SetRate(1.0);
	}
	obj->GetRenderObject()->GetAnimObject()->SetAnimation(animation);
}

void AnimationController::UpdateCurrentFrames(float dt) {
	for (AnimationObject*& anim : mAnimationList) {
		anim->Update(dt);
	}
}

void AnimationController::SetObjectList(vector<GameObject*> animationObjects) {
	std::cout << "Object List Size: " << animationObjects.size() << std::endl; // YOU
	for (auto& obj : animationObjects) {
		if (obj->GetName().find("player") != std::string::npos) {
			mPlayersList.emplace_back((Player*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimObject();
			mAnimationList.emplace_back(animObj);
		}
		else if (obj->GetName() == "PatrolEnemy") {
			mPatrolEnemyList.emplace_back((PatrolEnemy*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimObject();
			mAnimationList.emplace_back(animObj);
		}
	}
}