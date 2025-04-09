#include <iostream>
#include <string>
#include <functional>
#include <vector>

#include "PrisonEscape/Core/Networking/SteamManager.h"

using namespace NCL;
using namespace CSC8503;

SteamManager* SteamManager::s_Instance = nullptr;

SteamManager* SteamManager::GetInstance() 
{
    if (!s_Instance) {
        s_Instance = new SteamManager();
    }
    return s_Instance;
}

SteamManager::SteamManager() :
    m_bInitialized(false),
    m_CurrentLobbyID(0)
{
    
}

SteamManager::~SteamManager() 
{
    Shutdown();
}

bool SteamManager::Initialize() 
{
    #ifdef ENABLE_STEAM
        if (m_bInitialized) return true;

        if (!SteamAPI_Init()) {
            std::cout << "SteamAPI_Init failed. Make sure Steam is running and you are logged in." << std::endl;
            return false;
        }

        if (SteamFriends() == nullptr) {
            std::cout << "SteamFriends interface is NULL. Steam may not be running properly." << std::endl;
            SteamAPI_Shutdown();
            return false;
        }

        if (SteamUser() == nullptr) {
            std::cout << "SteamUser() is NULL." << std::endl;
            return false;
        }

        std::cout << "Steam initialized successfully. User: " << GetSteamUserName() << std::endl;
        m_bInitialized = true;
        return true;
    #else
        std::cout << "Steam support is not enabled in this build." << std::endl;
        return false;
    #endif
}

void SteamManager::Shutdown() 
{
#ifdef ENABLE_STEAM
    if (m_bInitialized) {
        LeaveLobby();
        SteamAPI_Shutdown();
        m_bInitialized = false;
        std::cout << "Steam shutdown." << std::endl;
    }
#endif
}

void SteamManager::Update() 
{
#ifdef ENABLE_STEAM
    if (m_bInitialized) {
        SteamAPI_RunCallbacks();
        PollSteamCallbacks();
    }
#endif
}

void SteamManager::PollSteamCallbacks() 
{
#ifdef ENABLE_STEAM
    CheckForGameInvites();
    CheckForLobbyUpdates();

    uint32 packetSize;
    while (SteamNetworking()->IsP2PPacketAvailable(&packetSize)) {
        if (packetSize == sizeof(float) * 3) { // Position update packet
            char buffer[sizeof(float) * 3];
            CSteamID senderID;
            if (SteamNetworking()->ReadP2PPacket(buffer, sizeof(buffer), &packetSize, &senderID)) {
                // Extract position
                Vector3 position;
                memcpy(&position.x, buffer, sizeof(float));
                memcpy(&position.y, buffer + sizeof(float), sizeof(float));
                memcpy(&position.z, buffer + sizeof(float) * 2, sizeof(float));

                // Call the position update callback
                if (m_PositionUpdateCallback) {
                    m_PositionUpdateCallback(senderID.ConvertToUint64(), position);
                }
            }
        }
    }
#endif
}

void SteamManager::CheckForGameInvites()
{
#ifdef ENABLE_STEAM
    // This would track actual join requests coming from other players,
    // not from the local player
    if (m_JoinGameCallback && SteamUtils()->IsOverlayEnabled()) {
        // Don't process invites if we're already the host of a lobby
        if (IsInLobby() && IsLobbyOwner()) {
            return;
        }

        // Process any pending game join requests
        if (SteamFriends()) {
            // For the prototype, we can check if there are any rich presence
            // join requests that have been accepted
            for (int i = 0; i < SteamFriends()->GetFriendCount(k_EFriendFlagImmediate); i++) {
                CSteamID friendID = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);

                // Skip our own ID
                if (friendID == SteamUser()->GetSteamID()) continue;

                // Check if this friend has sent us an invite
                const char* connectString = SteamFriends()->GetFriendRichPresence(friendID, "connect");
                if (connectString && strlen(connectString) > 0) {
                    std::string connect = connectString;

                    if (connect.find("lobbyid=") != std::string::npos) {
                        size_t pos = connect.find("lobbyid=") + 8;
                        try {
                            uint64_t lobbyID = std::stoull(connect.substr(pos));

                            // Call the callback with the lobby ID
                            if (m_JoinGameCallback) {
                                m_JoinGameCallback(lobbyID);
                            }

                            // Clear the processed invite
                            SteamFriends()->ClearRichPresence();
                            break;
                        }
                        catch (...) {
                            // Handle parsing errors
                        }
                    }
                }
            }
        }
    }
#endif
}

void SteamManager::CheckForLobbyUpdates() 
{
#ifdef ENABLE_STEAM
    static uint64_t lastLobbyID = 0;

    if (m_CurrentLobbyID != lastLobbyID) {
        std::cout << "Lobby state changed. Current lobby: " << m_CurrentLobbyID << std::endl;
        lastLobbyID = m_CurrentLobbyID;
    }
#endif
}

void SteamManager::CheckForOverlayState() 
{
#ifdef ENABLE_STEAM
    // Check if the Steam overlay was activated/deactivated
    static bool lastOverlayState = false;
    bool currentOverlayState = SteamUtils()->BOverlayNeedsPresent();

    if (currentOverlayState != lastOverlayState) {
        std::cout << "Steam overlay " << (currentOverlayState ? "activated" : "deactivated") << std::endl;
    }

    lastOverlayState = currentOverlayState;
#endif
}

void SteamManager::SendGameInvite(uint64_t steamID)
{
#ifdef ENABLE_STEAM
    if (!m_bInitialized || m_CurrentLobbyID == 0) return;

    // Set Rich Presence that will be used when the invite is accepted
    SteamFriends()->SetRichPresence("connect", ("lobbyid=" + std::to_string(m_CurrentLobbyID)).c_str());

    // Send the actual invite
    SteamFriends()->InviteUserToGame(CSteamID(steamID), "Join my game!");
    std::cout << "Game invite sent to friend with ID: " << steamID << std::endl;
#endif
}

void SteamManager::SetJoinGameCallback(std::function<void(uint64_t)> callback) {
    m_JoinGameCallback = callback;
}

std::string SteamManager::GetSteamUserName() 
{
#ifdef ENABLE_STEAM
    if (m_bInitialized) {
        ISteamFriends* steamFriends = SteamFriends();
        if (steamFriends) {
            const char* name = steamFriends->GetPersonaName();
            return name;
        }
        else {
            std::cout << "SteamFriends() returned nullptr" << std::endl;
        }
    }
    else {
        std::cout << "Steam not initialized when GetUserName() was called" << std::endl;
    }
#else
    std::cout << "ENABLE_STEAM not defined" << std::endl;
#endif
    return "Unknown";
}

uint64_t SteamManager::GetSteamID() 
{
#ifdef ENABLE_STEAM
    if (m_bInitialized) {
        return SteamUser()->GetSteamID().ConvertToUint64();
    }
#endif
    return 0;
}

std::vector<std::pair<std::string, uint64_t>> SteamManager::GetFriendsList() 
{
    std::vector<std::pair<std::string, uint64_t>> friends;
#ifdef ENABLE_STEAM
    if (m_bInitialized) {
        int friendCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
        for (int i = 0; i < friendCount; i++) {
            CSteamID friendID = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
            std::string friendName = SteamFriends()->GetFriendPersonaName(friendID);
            friends.push_back(std::make_pair(friendName, friendID.ConvertToUint64()));
        }
    }
#endif
    return friends;
}

bool SteamManager::IsFriendOnline(uint64_t steamID)
{
#ifdef ENABLE_STEAM
    if (m_bInitialized) {
        CSteamID id(steamID);
        EPersonaState state = SteamFriends()->GetFriendPersonaState(id);
        return state != k_EPersonaStateOffline && state != k_EPersonaStateInvisible;
    }
#endif
    return false;
}

bool SteamManager::DoesFriendOwnGame(uint64_t steamID)
{
#ifdef ENABLE_STEAM
    if (m_bInitialized) {
        CSteamID id(steamID);

        uint32 appID = SteamUtils()->GetAppID();

        bool bOwnsGame = false;

        if (SteamFriends()) {
            bOwnsGame = SteamFriends()->HasFriend(id, k_EFriendFlagImmediate);
        }

        // For testing/development purposes, always return true
        bOwnsGame = true;

        return bOwnsGame;
    }
#endif
    return false;
}

bool SteamManager::IsFriendInGame(uint64_t steamID)
{
    #ifdef ENABLE_STEAM
    if (m_bInitialized) {
        CSteamID id(steamID);
        FriendGameInfo_t gameInfo;
        if (SteamFriends()->GetFriendGamePlayed(id, &gameInfo)) {
            // For development, consider any game as "our game"
            return true;
        }
    }
#endif
    return false;
}

bool SteamManager::CreateLobby(int maxPlayers)
{
#ifdef ENABLE_STEAM
    if (!m_bInitialized) return false;

    std::cout << "Creating Steam lobby..." << std::endl;

    SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxPlayers);
    m_CallResultLobbyCreated.Set(hSteamAPICall, this, &SteamManager::OnLobbyCreated);

    return true;
#else
    return false;
#endif
}

void SteamManager::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
{
#ifdef ENABLE_STEAM
    if (bIOFailure || pCallback->m_eResult != k_EResultOK) {
        std::cout << "Failed to create lobby: " << pCallback->m_eResult << std::endl;
        return;
    }

    m_CurrentLobbyID = pCallback->m_ulSteamIDLobby;
    std::cout << "Lobby created successfully. ID: " << m_CurrentLobbyID << std::endl;

    CSteamID lobbyID(m_CurrentLobbyID);
    SteamMatchmaking()->SetLobbyData(lobbyID, "game", "PrisonEscape");
    SteamMatchmaking()->SetLobbyJoinable(lobbyID, true);

    // At this point, you are guaranteed to be the owner of the lobby
    std::cout << "You are the lobby owner. Your Steam ID: " << SteamUser()->GetSteamID().ConvertToUint64() << std::endl;
#endif
}

bool SteamManager::JoinLobby(uint64_t lobbyID) 
{
#ifdef ENABLE_STEAM
    if (!m_bInitialized) return false;

    SteamMatchmaking()->JoinLobby(CSteamID(lobbyID));
    std::cout << "Joining Steam lobby: " << lobbyID << std::endl;

    // Simulate successful join for demonstration
    m_CurrentLobbyID = lobbyID;
    return m_CurrentLobbyID != 0;
#else
    return false;
#endif
}

void SteamManager::LeaveLobby() 
{
#ifdef ENABLE_STEAM
    if (!m_bInitialized || m_CurrentLobbyID == 0) return;

    SteamMatchmaking()->LeaveLobby(CSteamID(m_CurrentLobbyID));
    m_CurrentLobbyID = 0;
    std::cout << "Left Steam lobby" << std::endl;
#endif
}

bool SteamManager::IsLobbyOwner() const
{
#ifdef ENABLE_STEAM
    if (!m_bInitialized) {
        std::cout << "IsLobbyOwner: Steam not initialized" << std::endl;
        return false;
    }

    if (m_CurrentLobbyID == 0) {
        std::cout << "IsLobbyOwner: No active lobby" << std::endl;
        return false;
    }

    CSteamID ownerId = SteamMatchmaking()->GetLobbyOwner(CSteamID(m_CurrentLobbyID));
    CSteamID myId = SteamUser()->GetSteamID();
    return ownerId == myId;
#endif
    return false;
}

uint64_t SteamManager::GetLobbyOwner()
{
#ifdef ENABLE_STEAM
    if (!m_bInitialized || m_CurrentLobbyID == 0) return 0;

    CSteamID ownerID = SteamMatchmaking()->GetLobbyOwner(CSteamID(m_CurrentLobbyID));
    return ownerID.ConvertToUint64();
#else
    return 0;
#endif
}

void SteamManager::SendPlayerPosition(const Vector3& position)
{
#ifdef ENABLE_STEAM
    if (!m_bInitialized || m_CurrentLobbyID == 0) return;
    // Example using Steam's P2P networking:
    // Serialize position data
    char buffer[sizeof(float) * 3];
    memcpy(buffer, &position.x, sizeof(float));
    memcpy(buffer + sizeof(float), &position.y, sizeof(float));
    memcpy(buffer + sizeof(float) * 2, &position.z, sizeof(float));

    // Get the other player's Steam ID (simplified)
    CSteamID targetID;

    // If we're the host, send to all members except ourselves
    if (IsLobbyOwner()) {
        int memberCount = SteamMatchmaking()->GetNumLobbyMembers(CSteamID(m_CurrentLobbyID));
        for (int i = 0; i < memberCount; i++) {
            CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(CSteamID(m_CurrentLobbyID), i);
            if (memberId != SteamUser()->GetSteamID()) {
                targetID = memberId;
                break; // For simplicity, just send to the first non-host player
            }
        }
    }
    else {
        // If we're not the host, send to the host
        targetID = SteamMatchmaking()->GetLobbyOwner(CSteamID(m_CurrentLobbyID));
    }

    // Send the data
    if (targetID.IsValid()) {
        SteamNetworking()->SendP2PPacket(targetID, buffer, sizeof(buffer), k_EP2PSendReliable);
    }
#endif
}

void SteamManager::SetPlayerUpdateCallback(std::function<void(uint64_t, const Vector3&)> callback)
{
    m_PositionUpdateCallback = callback;
}
