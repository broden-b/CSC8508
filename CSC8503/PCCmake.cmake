function(Create_PC_CSC8503_Files)

    message("PC CSC8503")

    ################################################################################
    # Source groups
    ################################################################################
    set(Header_Files
        "GameTechRenderer.h"
        "NetworkedGame.h"
        "NetworkPlayer.h"
        "StateGameObject.h"
        "TutorialGame.h"
        "AnimationController.h"
        "AnimationObject.h"
        "jsonParser.h"
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
        "PrisonEscape/Core/AudioManager.cpp"
        "PrisonEscape/Core/AudioManager.h"
        "PrisonEscape/Core/Networking/SteamManager.h"
        "PrisonEscape/Core/Networking/SteamManager.cpp"
    )
    source_group("Prison Escape Core" FILES ${Prison_Escape_Core})

    set(Prison_Escape_Levels
        "PrisonEscape/Levels/Level.cpp"
        "PrisonEscape/Levels/Level.h"
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
	"PrisonEscape/Scripts/Collectables/Coin.cpp"
	"PrisonEscape/Scripts/Collectables/Coin.h"
        "PrisonEscape/Scripts/puzzle/HidingArea.h"
        "PrisonEscape/Scripts/puzzle/HidingArea.cpp"
    )
    source_group("Prison Escape Scripts" FILES ${Prison_Escape_Scripts})

    set(Prison_Escape_States
        "PrisonEscape/States/GameplayState.cpp"
        "PrisonEscape/States/GameplayState.h"
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
        "Main.cpp"
        "NetworkedGame.cpp"
        "NetworkPlayer.cpp"
        "StateGameObject.cpp"
        "TutorialGame.cpp"
        "AnimationController.cpp"
        "AnimationObject.cpp"
        "json.hpp"
        "jsonParser.cpp"
    )

    if(USE_VULKAN)
        file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)
    else()
        list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.h)
        list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.cpp)
    endif()

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

    if(USE_VULKAN)
        set(ALL_FILES 
            ${Header_Files}
            ${Source_Files}
            ${SHADER_FILES}
        )

        foreach (file ${SHADER_FILES})
            get_filename_component(file_name ${file} NAME)
            get_filename_component(file_ext ${file} EXT)
            
            if(file_ext STREQUAL ".h" OR file_ext STREQUAL ".cpp")
                continue()
            endif()
                
            if( file_ext STREQUAL  ".vert" OR
                file_ext STREQUAL  ".frag" OR
                file_ext STREQUAL  ".comp" OR
                file_ext STREQUAL  ".geom" OR
                file_ext STREQUAL  ".tesc" OR
                file_ext STREQUAL  ".tese" OR
                file_ext STREQUAL  ".rgen" OR
                file_ext STREQUAL  ".rint" OR
                file_ext STREQUAL  ".rahit" OR
                file_ext STREQUAL  ".rchit" OR
                file_ext STREQUAL  ".rmiss" OR
                file_ext STREQUAL  ".rcall" OR
                file_ext STREQUAL  ".task" OR
                file_ext STREQUAL  ".mesh"
            )
                message("Adding custom command to ${file}")
                get_filename_component(file_dir ${file} ABSOLUTE)
                set(SPIRV_OUTPUT ${file_name}.spv)
                set(SPIRV_ABS_INPUT ${file_dir})
                set(SPIRV_ABS_OUTPUT ${file_dir}.spv)
                
                add_custom_command(
                    OUTPUT ${SPIRV_ABS_OUTPUT}
                    
                    COMMENT "Compiling GLSL shader:"
                    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V  ${SPIRV_ABS_INPUT} -o ${SPIRV_ABS_OUTPUT}
                    DEPENDS ${file}
                    VERBATIM
                )
                list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})   
            endif()
        endforeach()
    endif()

    ################################################################################
    # Target
    ################################################################################
    add_executable(${PROJECT_NAME}  ${ALL_FILES})

    #use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
    set(ROOT_NAMESPACE CSC8503)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_GLOBAL_KEYWORD "Win32Proj"
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )

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
    
    if(USE_VULKAN)
        target_precompile_headers(${PROJECT_NAME} PRIVATE
            <vulkan/vulkan.hpp>
            <SmartTypes.h>
            <VulkanRenderer.h>
            <VulkanShader.h>
            <VulkanShaderBuilder.h>
            <VulkanTexture.h>
            <VulkanMesh.h>
            <VulkanPipelineBuilder.h>
            <VulkanDynamicRenderBuilder.h>
            <VulkanTextureBuilder.h>
            <VulkanDescriptorSetLayoutBuilder.h>
            <VulkanRenderPassBuilder.h>
            <VulkanCompute.h>
            <VulkanComputePipelineBuilder.h>
            <VulkanBufferBuilder.h>
        )
    endif()

    ################################################################################
    # Compile and link options
    ################################################################################
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++latest;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
        )
    endif()

    ################################################################################
    # Dependencies
    ################################################################################
    if(MSVC)
        target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
    endif()

    include_directories("../OpenGLRendering/")
    include_directories("../NCLCoreClasses/")
    include_directories("../CSC8503CoreClasses/")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
    include_directories("../FMODCoreAPI/includes")  # Ensure this path is correct
    include_directories("../FMODCoreAPI/dlls")

    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC OpenGLRendering)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "../FMODCoreAPI/libs/fmod_vc")  # Ensure this path is correct
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "../FMODCoreAPI/libs/fmodL_vc")  # Ensure this path is correct

    file(GLOB DLLS "../FMODCoreAPI/dlls/*.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "../FMODCoreAPI/dlls/fmod.dll"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "../FMODCoreAPI/dlls/fmodL.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "../FMODCoreAPI/dlls/fmod.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
 

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "../FMODCoreAPI/dlls/fmodL.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

    option(ENABLE_STEAM "Enable Steam integration" ON)
    if(ENABLE_STEAM)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

        # Steam paths
        set(STEAMWORKS_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/steam")
        include_directories("${STEAMWORKS_SDK_PATH}/public")

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/steam_appid.txt"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    
        # Directly link to the Steam library
        if(WIN32)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                # 64-bit
                target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "${STEAMWORKS_SDK_PATH}/redistributable_bin/win64/steam_api64.lib")
            else()
                # 32-bit
                target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "${STEAMWORKS_SDK_PATH}/redistributable_bin/steam_api.lib")
            endif()
            add_compile_definitions(ENABLE_STEAM)
        endif()

        if(WIN32)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                # 64-bit
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${STEAMWORKS_SDK_PATH}/redistributable_bin/win64/steam_api64.dll"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
            else()
                # 32-bit
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${STEAMWORKS_SDK_PATH}/redistributable_bin/steam_api.dll"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
            endif()
        endif()
    endif()

endfunction()
