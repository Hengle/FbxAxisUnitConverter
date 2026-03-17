# FindFBXSDK.cmake
# Locates the Autodesk FBX SDK and creates an IMPORTED target FBXSDK::FBXSDK.
#
# Usage:
#   set(FBXSDK_ROOT "C:/Program Files/Autodesk/FBX/FBX SDK/2020.3.9")
#   find_package(FBXSDK REQUIRED)
#   target_link_libraries(myapp PRIVATE FBXSDK::FBXSDK)
#
# The module searches for the SDK in the following order:
#   1. FBXSDK_ROOT CMake variable
#   2. FBXSDK_ROOT environment variable
#   3. Default install path (Windows)

# --- Determine root path ---------------------------------------------------
if(NOT FBXSDK_ROOT)
    if(DEFINED ENV{FBXSDK_ROOT})
        set(FBXSDK_ROOT "$ENV{FBXSDK_ROOT}")
    else()
        set(FBXSDK_ROOT "C:/Program Files/Autodesk/FBX/FBX SDK/2020.3.9")
    endif()
endif()

# --- Find include directory -------------------------------------------------
find_path(FBXSDK_INCLUDE_DIR
    NAMES fbxsdk.h
    PATHS "${FBXSDK_ROOT}/include"
    NO_DEFAULT_PATH
)

# --- Find libraries (Debug / Release) --------------------------------------
find_library(FBXSDK_LIBRARY_RELEASE
    NAMES libfbxsdk-md
    PATHS "${FBXSDK_ROOT}/lib/x64/release"
    NO_DEFAULT_PATH
)

find_library(FBXSDK_LIBRARY_DEBUG
    NAMES libfbxsdk-md
    PATHS "${FBXSDK_ROOT}/lib/x64/debug"
    NO_DEFAULT_PATH
)

find_library(FBXSDK_XML2_RELEASE
    NAMES libxml2-md
    PATHS "${FBXSDK_ROOT}/lib/x64/release"
    NO_DEFAULT_PATH
)

find_library(FBXSDK_XML2_DEBUG
    NAMES libxml2-md
    PATHS "${FBXSDK_ROOT}/lib/x64/debug"
    NO_DEFAULT_PATH
)

find_library(FBXSDK_ZLIB_RELEASE
    NAMES zlib-md
    PATHS "${FBXSDK_ROOT}/lib/x64/release"
    NO_DEFAULT_PATH
)

find_library(FBXSDK_ZLIB_DEBUG
    NAMES zlib-md
    PATHS "${FBXSDK_ROOT}/lib/x64/debug"
    NO_DEFAULT_PATH
)

# --- Standard find_package handling -----------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FBXSDK
    REQUIRED_VARS
        FBXSDK_INCLUDE_DIR
        FBXSDK_LIBRARY_RELEASE
        FBXSDK_LIBRARY_DEBUG
)

# --- Create IMPORTED target -------------------------------------------------
if(FBXSDK_FOUND AND NOT TARGET FBXSDK::FBXSDK)
    add_library(FBXSDK::FBXSDK INTERFACE IMPORTED)

    set_target_properties(FBXSDK::FBXSDK PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${FBXSDK_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES
            "$<IF:$<CONFIG:Debug>,${FBXSDK_LIBRARY_DEBUG};${FBXSDK_XML2_DEBUG};${FBXSDK_ZLIB_DEBUG},${FBXSDK_LIBRARY_RELEASE};${FBXSDK_XML2_RELEASE};${FBXSDK_ZLIB_RELEASE}>"
    )
endif()

mark_as_advanced(
    FBXSDK_INCLUDE_DIR
    FBXSDK_LIBRARY_RELEASE
    FBXSDK_LIBRARY_DEBUG
    FBXSDK_XML2_RELEASE
    FBXSDK_XML2_DEBUG
    FBXSDK_ZLIB_RELEASE
    FBXSDK_ZLIB_DEBUG
)
