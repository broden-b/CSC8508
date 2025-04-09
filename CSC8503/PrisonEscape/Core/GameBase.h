#pragma once

namespace NCL {
    namespace CSC8503 {
        class GameTechRenderer;
        class GameWorld;
        class GameConfigManager;
        class PushdownMachine;
        class GameSettingManager;
        class AudioManager;

        class GameBase {
        public:
            GameBase();
            ~GameBase();
            virtual void UpdateGame(float dt);
            void InitialiseGame();

        protected:
            static GameTechRenderer* renderer;
            static GameWorld* world;
            static GameBase* instance;
            static GameSettingManager* gameSettings;
            static AudioManager* audioManager;
            PushdownMachine* stateMachine;

        private:
            GameConfigManager* gameConfig;

        public:
            static GameTechRenderer* GetRenderer() { return renderer; }
            static GameWorld* GetWorld() { return world; }
            static GameBase* GetGameBase();
            static PushdownMachine* GetStateMachine() { return instance->stateMachine; }
            static GameSettingManager* GetGameSettings() { return gameSettings; }
            static AudioManager* GetAudioManager() { return audioManager; }
            void SetGameConfig(GameConfigManager* config) { gameConfig = config; }
            GameConfigManager* GetGameConfig() const { return gameConfig; }
            void QuitGame();
            void CheckPlayerIdle();
        };
    }
}