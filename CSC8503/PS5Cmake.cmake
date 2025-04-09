function(Create_PS5_CSC8503_Files)

    message("PS5 CSC8503")

    ################################################################################
    # Source groups
    ################################################################################
    set(Header_Files
        "GameTechRenderer.h"
        "GameTechAGCRenderer.h"
        "NetworkedGame.h"
        "NetworkPlayer.h"
        "StateGameObject.h"
        "TutorialGame.h"
    )
    source_group("Header Files" FILES ${Header_Files})

    set(Prison_Escape_Core
        "PrisonEscape/Core/GameBase.cpp"
        "PrisonEscape/Core/GameBase.h"
        "PrisonEscape/Core/GameConfigManager.cpp"
        "PrisonEscape/Core/GameConfigManager.h"
        "PrisonEscape/Core/GameLevelManager.cpp"
        "PrisonEscape/Core/GameLevelManager.h"
        "PrisonEscape/Core/ImGuiManager.cpp"
        "PrisonEscape/Core/ImGuiManager.h"
        "PrisonEscape/Core/GameSettingManager.h"
        "PrisonEscape/Core/GameSettingManager.cpp"
    )
    source_group("Prison Escape Core" FILES ${Prison_Escape_Core})

    set(Prison_Escape_Levels
        "PrisonEscape/Levels/Level.cpp"
        "PrisonEscape/Levels/Level.h"
        "PrisonEscape/Levels/LevelOne.cpp"
        "PrisonEscape/Levels/LevelOne.h"
        "PrisonEscape/Levels/LevelT.cpp"
        "PrisonEscape/Levels/LevelT.h"
        "PrisonEscape/Levels/SampleLevel.cpp"
        "PrisonEscape/Levels/SampleLevel.h"
    )
    source_group("Prison Escape Levels" FILES ${Prison_Escape_Levels})

    set(Prison_Escape_Scripts
        "PrisonEscape/Scripts/PatrolEnemy/PatrolEnemy.cpp"
        "PrisonEscape/Scripts/PatrolEnemy/PatrolEnemy.h"
	"PrisonEscape/Scripts/PursuitEnemy/PursuitEnemy.cpp"
	"PrisonEscape/Scripts/PursuitEnemy/PursuitEnemy.h"
	"PrisonEscape/Scripts/CameraEnemy/CameraEnemy.cpp"
	"PrisonEscape/Scripts/CameraEnemy/CameraEnemy.h"
        "PrisonEscape/Scripts/Player/Player.cpp"
        "PrisonEscape/Scripts/Player/Player.h"
        "PrisonEscape/Scripts/puzzle/Button.cpp"
        "PrisonEscape/Scripts/puzzle/Button.h"
        "PrisonEscape/Scripts/puzzle/puzzleT.cpp"
        "PrisonEscape/Scripts/puzzle/puzzleT.h"
    )
    source_group("Prison Escape Scripts" FILES ${Prison_Escape_Scripts})

    set(Prison_Escape_States
        "PrisonEscape/States/GameplayState.cpp"
        "PrisonEscape/States/GameplayState.h"
        "PrisonEscape/States/GameState.cpp"
        "PrisonEscape/States/GameState.h"
        "PrisonEscape/States/MenuState.cpp"
        "PrisonEscape/States/MenuState.h"
        "PrisonEscape/States/PauseState.cpp"
        "PrisonEscape/States/PauseState.h"
        "PrisonEscape/States/GameoverState.cpp"
        "PrisonEscape/States/GameoverState.h"
    )
    source_group("Prison Escape States" FILES ${Prison_Escape_States})


    set(Source_Files
        "GameTechRenderer.cpp"
        "GameTechAGCRenderer.cpp"
        "Main.cpp"
        "NetworkedGame.cpp"
        "NetworkPlayer.cpp"
        "StateGameObject.cpp"
        "TutorialGame.cpp"
    )

    source_group("Source Files" FILES ${Source_Files})

    set(ALL_FILES
        ${Header_Files}
        ${Source_Files}
        ${Prison_Escape_Core}
        ${Prison_Escape_Levels}
        ${Prison_Escape_Prefabs}
        ${Prison_Escape_Scripts}
        ${Prison_Escape_States}
    )

    ################################################################################
    # Target
    ################################################################################
    add_executable(${PROJECT_NAME}  ${ALL_FILES})

    set(ROOT_NAMESPACE CSC8503)

    ################################################################################
    # Compile definitions
    ################################################################################
    if(MSVC)
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            "UNICODE;"
            "_UNICODE" 
            "WIN32_LEAN_AND_MEAN"
            "_WINSOCKAPI_"   
            "_WINSOCK2API_"
            "_WINSOCK_DEPRECATED_NO_WARNINGS"
        )
    endif()

    target_precompile_headers(${PROJECT_NAME} PRIVATE
        <vector>
        <map>
        <stack>
        <list>   
        <set>   
        <string>
        <thread>
        <atomic>
        <functional>
        <iostream>
        <chrono>
        <sstream>
        
        "../NCLCoreClasses/Vector.h"
        "../NCLCoreClasses/Quaternion.h"
        "../NCLCoreClasses/Plane.h"
        "../NCLCoreClasses/Matrix.h"
        "../NCLCoreClasses/GameTimer.h"
    )

    ################################################################################
    # Dependencies
    ################################################################################
    if(MSVC)
        target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
    endif()

    include_directories("../NCLCoreClasses/")
    include_directories("../PS5Core/")
    include_directories("../CSC8503CoreClasses/")
    include_directories("../PS5Core")
    include_directories("../FMODCorePS5/includes")  # Ensure this path is correct
    include_directories("../FMODCorePS5/libs")

    target_link_libraries(${PROJECT_NAME} PRIVATE ${NCLCoreClasses})
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC PS5Core)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
    target_link_libraries(MyPS5Game 
        fmodL.prx 
        fmod.prx
    )

endfunction()

