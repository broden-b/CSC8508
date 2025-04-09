#include "Player.h"
#include "PhysicsObject.h"
#include "RenderObject.h"

#include "PrisonEscape/Core/GameBase.h"
#include "PrisonEscape/Core/GameConfigManager.h"
#include "PrisonEscape/Core/ImGuiManager.h"
#include "PrisonEscape/Core/AudioManager.h"


Player::Player(GameWorld* world, const std::string& name) : GameObject()
{
	useGravity = true;
	cameraAttached = true;

    state = Default;
	GameObject::SetName(name);
    mName = name;

    health = 3;
    visible = true;
    score = 0;
    
    audioManager = new AudioManager(); 
    audioManager->Initialize();         
    audioManager->LoadSoundsXtension();
}

Player::~Player()
{
    delete audioManager;
}

void Player::UpdateGame(float dt)
{   
    UpdatePlayerMovement(dt);    
}

void Player::OnCollisionBegin(GameObject* other) {
    if (other->GetName() == "Coin") {
        SetScore(GetScore() + 10);
        GameBase::GetGameBase()->GetWorld()->RemoveGameObject(other);
        std::cout << "Score is: " << GetScore() << std::endl;
    }

    if (other->GetName() == "HidingPlace") {
        SetVisible(false);
    }
}


void Player::UpdatePlayerMovement(float dt)
{
    if (!controller) {
        return;
    }


    if (!renderObject) {
        return;
    }
    isIdle = true;

    GameConfigManager* config = GameBase::GetGameBase()->GetGameConfig();

    // Only process inputs for the local player
    if (config && config->networkConfig.isMultiplayer) {
        std::string playerName = GetName();

        // For the server, control "playerOne"; for the client, control "playerTwo"
        bool shouldControlLocally = (config->networkConfig.isServer && playerName == "playerOne") || (!config->networkConfig.isServer && playerName == "playerTwo");

        if (!shouldControlLocally) {
            // This player is controlled remotely - don't process local inputs
            return;
        }
    }

    Vector3 playerPosition = GetTransform().GetPosition();
    Camera& mainCamera = GameBase::GetGameBase()->GetWorld()->GetMainCamera();

    Vector3 cameraOffset(5.f, 40, 50.0f);
    mainCamera.SetPosition(playerPosition + cameraOffset);

    float forward = controller->GetAxis(2);
    float sidestep = -controller->GetAxis(0);

    Vector3 currentVelocity = GetPhysicsObject()->GetLinearVelocity();

    Vector3 forwardVec(0, 0, 1);
    Vector3 rightVec(1, 0, 0);

    if (forward != 0.0f)
    {
        forwardVec = -forwardVec;
        
        isIdle = false;
    }
    if (sidestep != 0.0f)
    {
        sidestep = -sidestep;
        isIdle = false;
    }

    
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::E)) {
        SetVisible(true);
        SetActive(true);
    }
    sprintMultiplier = 1.0f;
    Vector3 movement(0, 0, 0);
    
    if (Player::isActive) {
        if (forward != 0.0f) {
            movement += forwardVec * forward * GetPlayerSpeed() * sprintMultiplier;
        }

        if (forward != 0.0f) {
            movement += forwardVec * forward * GetPlayerSpeed() * sprintMultiplier;
        }
        if (sidestep != 0.0f) {
            movement += rightVec * sidestep * GetPlayerSpeed() * sprintMultiplier;
        }
    }
    if (movement.Length() > 0.0f) {
        float angle = atan2(-movement.x, -movement.z);
        Quaternion targetOrientation = Quaternion::EulerAnglesToQuaternion(0, angle * 180.0f / M_PI, 0);
        Quaternion currentOrientation = GetTransform().GetOrientation();
        Quaternion newOrientation = Quaternion::Slerp(currentOrientation, targetOrientation, dt * 5.0f); // Adjust the interpolation speed as needed
        GetTransform().SetOrientation(newOrientation);
    }

    if (useGravity)
    {
        if (fabs(currentVelocity.y) < 1.0f) {
            static float lastJumpTime = -2.2f;
            float currentTime = Window::GetTimer().GetTotalTimeSeconds();

            if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE) && (currentTime - lastJumpTime >= 2.2f) && Player::isActive)
            {
                movement.y += 8.0f;
                currentVelocity.y = 20.0f;
                lastJumpTime = currentTime;
                isJumping = true;
                
            }
        }
        
        currentVelocity.y -= 25.0f * dt;
        GetPhysicsObject()->SetLinearVelocity(Vector3(movement.x, currentVelocity.y, movement.z));
    }
    else
    {
        GetPhysicsObject()->SetLinearVelocity(movement);
    }

    if (isIdle) {
        SetObjectAnimationState(Idle);
        if (!audioManager->IsPlaying(audioManager->soundFile11)) { // Ensure sound isn't already playing
            audioManager->StopSound(audioManager->soundFile8);
            audioManager->PlaySound(audioManager->soundFile11);
        }

    }
    else if (!isIdle) {
        SetObjectAnimationState(Walk);
        if (!audioManager->IsPlaying(audioManager->soundFile8)) { // Ensure sound isn't already playing
            audioManager->PlaySound(audioManager->soundFile8);
            audioManager->StopSound(audioManager->soundFile11);
        }
    }
    
}

void Player::InitializeController()
{
    controller = new KeyboardMouseController(*Window::GetKeyboard(), *Window::GetMouse());

    GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetController(*controller);

    controller->MapAxis(0, "Sidestep");
    controller->MapAxis(1, "UpDown");
    controller->MapAxis(2, "Forward");
}