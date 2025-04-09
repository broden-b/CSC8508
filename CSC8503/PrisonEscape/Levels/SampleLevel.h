#pragma once

#include "Vector.h"

#include "Level.h"
#include "../CSC8503/PrisonEscape/Core/GameBase.h"

using namespace NCL;
using namespace CSC8503;

class SampleLevel : public Level
{
public:
	SampleLevel();
	~SampleLevel();

	void Init() override;

	void Update(float dt) override;



	ImFont* mHeaderFont = nullptr;
	GameObject* CreateRandomColorCube(Vector3 position);
};

