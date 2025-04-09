#include "SampleLevel.h"

SampleLevel::SampleLevel()
{

}

SampleLevel::~SampleLevel()
{

}

void SampleLevel::Init()
{
	Level::Init();
}

void SampleLevel::Update(float dt)
{
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().UpdateCamera(dt);
	Level::Update(dt);

}
//GameObject* SampleLevel::CreateRandomColorCube(Vector3 position) {
//    GameObject* cube = new GameObject("RandomColorCube");
//
//    Vector4 colors[] = {
//        Vector4(1, 0, 0, 1), // Red
//        Vector4(0, 0, 1, 1), // Blue
//        Vector4(0, 1, 0, 1), // Green
//        Vector4(1, 1, 1, 1), // White
//        Vector4(0, 0, 0, 1), // Black
//        Vector4(1, 1, 0, 1)  // Yellow
//    };
//
//    int randomIndex = rand() % 6; // Select a random color
//    cube->SetColor(colors[randomIndex]);
//
//    cube->GetTransform().SetScale(Vector3(1, 1, 1)).SetPosition(position);
//    cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, basicTex, basicShader));
//
//    cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));
//
//    world->AddGameObject(cube);
//    return cube;
//}