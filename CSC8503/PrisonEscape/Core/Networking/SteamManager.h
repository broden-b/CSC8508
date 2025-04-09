#pragma once

// Only include Steam API if ENABLE_STEAM is defined
#ifdef ENABLE_STEAM
#include <steam/steam_api.h>
#endif

#include "GameWorld.h"

namespace NCL {
    namespace CSC8503 {

        class SteamManager {
        public:
            static SteamManager* GetInstance();
            ~SteamManager();

            bool Initialize();
            void Shutdown();
            bool IsInitialized() const { return m_bInitialized; }
            void Update();

            // Friend invite functionality
            void SendGameInvite(uint64_t steamID);
            void SetJoinGameCallback(std::function<void(uint64_t)> callback);
            std::string GetSteamUserName();
            uint64_t GetSteamID();

            // Get the list of friends
            std::vector<std::pair<std::string, uint64_t>> GetFriendsList();
            bool IsFriendOnline(uint64_t steamID);
            bool DoesFriendOwnGame(uint64_t steamID);
            bool IsFriendInGame(uint64_t steamID);

            // Game server functionality
            bool CreateLobby(int maxPlayers);
            bool JoinLobby(uint64_t lobbyID);
            void LeaveLobby();
            bool IsInLobby() const { return m_CurrentLobbyID != 0; }
            bool IsLobbyOwner() const;
            uint64_t GetLobbyOwner();
            uint64_t GetCurrentLobbyID() const { return m_CurrentLobbyID; }

            // Position update handling
            void SendPlayerPosition(const Vector3& position);
            void SetPlayerUpdateCallback(std::function<void(uint64_t, const Vector3&)> callback);

        private:
            SteamManager();
            static SteamManager* s_Instance;

            bool m_bInitialized;
            std::function<void(uint64_t)> m_JoinGameCallback;
            std::function<void(uint64_t, const Vector3&)> m_PositionUpdateCallback;
            uint64_t m_CurrentLobbyID;

            // Helper methods for polling-based callbacks
            void PollSteamCallbacks();
            void CheckForGameInvites();
            void CheckForLobbyUpdates();
            void CheckForOverlayState();

            CCallResult<SteamManager, LobbyCreated_t> m_CallResultLobbyCreated;
            void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
        };
    }
}