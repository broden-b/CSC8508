set(PROJECT_NAME CSC8503)

include("PCCMake.cmake")
include("PS5CMake.cmake")

# PC CMake file
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    Create_PC_CSC8503_Files()
endif()

# PS5 Cmake file
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Prospero")
    Create_PS5_CSC8503_Files()
endif()