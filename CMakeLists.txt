cmake_minimum_required(VERSION 3.22)

project(toast_message LANGUAGES CXX)

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(MOD_ID "toast-message" CACHE STRING "LeviLauncher mod package directory name")
set(MOD_NAME "Toast Message" CACHE STRING "LeviLauncher mod display name")
set(MOD_AUTHOR "kyrocute" CACHE STRING "LeviLauncher mod author")
set(MOD_VERSION "0.1.0" CACHE STRING "LeviLauncher mod version")
set(MOD_LIBRARY_NAME "toast_message" CACHE STRING "Native library name without lib prefix or .so suffix")
set(MOD_MINECRAFT_VERSIONS "[\"1.26.*\"]" CACHE STRING "Raw JSON array for manifest minecraft_versions")
set(MOD_ICON "" CACHE STRING "Optional package-relative icon path, such as icon.png")

include(FetchContent)

# ---- preloader-android -------------------------------------------------
set(LEVI_PRELOADER_ROOT "" CACHE PATH "Optional path to a local preloader-android checkout")
if(LEVI_PRELOADER_ROOT STREQUAL "" AND DEFINED ENV{LEVI_PRELOADER_ROOT} AND NOT "$ENV{LEVI_PRELOADER_ROOT}" STREQUAL "")
    set(LEVI_PRELOADER_ROOT "$ENV{LEVI_PRELOADER_ROOT}")
endif()

if(LEVI_PRELOADER_ROOT STREQUAL "")
    FetchContent_Declare(
        preloader_android
        GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
        GIT_TAG main
    )
    FetchContent_GetProperties(preloader_android)
    if(NOT preloader_android_POPULATED)
        FetchContent_Populate(preloader_android)
    endif()
    set(LEVI_PRELOADER_ROOT "${preloader_android_SOURCE_DIR}")
endif()

if(NOT EXISTS "${LEVI_PRELOADER_ROOT}/src/pl/cpp/Mod.hpp")
    message(FATAL_ERROR "LEVI_PRELOADER_ROOT must point to a preloader-android checkout: ${LEVI_PRELOADER_ROOT}")
endif()

# ---- config / reflection dependencies ---------------------------------
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_Declare(
    boost_pfr
    GIT_REPOSITORY https://github.com/boostorg/pfr.git
    GIT_TAG boost-1.88.0
)
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG v0.9.7
)
FetchContent_MakeAvailable(nlohmann_json boost_pfr magic_enum)

# ---- Host-only config generator ---------------------------------------
if(NOT ANDROID)
    set(LEVI_GENERATED_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated-config")

    add_executable(levi_config_generator
        src/config_generator.cpp
        src/mod/Config.cpp
    )
    target_include_directories(levi_config_generator
        PRIVATE
            src
            "${LEVI_PRELOADER_ROOT}/src"
    )
    target_compile_definitions(levi_config_generator PRIVATE PL_CONFIG_NO_RUNTIME)
    target_link_libraries(levi_config_generator PRIVATE
        nlohmann_json::nlohmann_json
        Boost::pfr
        magic_enum::magic_enum
    )

    add_custom_target(levi_generate_config
        COMMAND levi_config_generator "${LEVI_GENERATED_CONFIG_DIR}"
        DEPENDS levi_config_generator
        COMMENT "Generating config.json and config.schema.json"
        VERBATIM
    )

    return()
endif()

# ---- ImGui (Android GLES3) --------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
    "${imgui_SOURCE_DIR}/misc/cpp"
)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_ES3 IMGUI_DEFINE_MATH_OPERATORS)

# ---- Package config discovery -----------------------------------------
if(NOT DEFINED LEVI_PACKAGE_CONFIG_DIR)
    set(LEVI_PACKAGE_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config" CACHE PATH
        "Directory containing config.json and config.schema.json to package")
endif()
get_filename_component(LEVI_PACKAGE_CONFIG_DIR "${LEVI_PACKAGE_CONFIG_DIR}" ABSOLUTE)
set(LEVI_PACKAGE_CONFIG_JSON "${LEVI_PACKAGE_CONFIG_DIR}/config.json")
set(LEVI_PACKAGE_CONFIG_SCHEMA "${LEVI_PACKAGE_CONFIG_DIR}/config.schema.json")

if(NOT IS_DIRECTORY "${LEVI_PACKAGE_CONFIG_DIR}")
    message(FATAL_ERROR "LEVI_PACKAGE_CONFIG_DIR must be a directory: ${LEVI_PACKAGE_CONFIG_DIR}")
endif()
if(NOT EXISTS "${LEVI_PACKAGE_CONFIG_JSON}")
    message(FATAL_ERROR "LEVI_PACKAGE_CONFIG_DIR must contain config.json: ${LEVI_PACKAGE_CONFIG_JSON}")
endif()
if(NOT EXISTS "${LEVI_PACKAGE_CONFIG_SCHEMA}")
    message(FATAL_ERROR "LEVI_PACKAGE_CONFIG_DIR must contain config.schema.json: ${LEVI_PACKAGE_CONFIG_SCHEMA}")
endif()

if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a" AND
   NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
    message(FATAL_ERROR "Unsupported Android ABI: ${CMAKE_ANDROID_ARCH_ABI}")
endif()

# ---- Native mod library -----------------------------------------------
add_library(${MOD_LIBRARY_NAME} SHARED
    src/main.cpp
    src/mod/Config.cpp
    src/mod/MyMod.cpp
    src/toast/ToastBridge.cpp
    src/ui/Overlay.cpp
    src/ui/ImGuiRenderer.cpp
    src/android/InputBridge.cpp
    third_party/imgui_virtual_keyboard.cpp
)

set_target_properties(${MOD_LIBRARY_NAME} PROPERTIES OUTPUT_NAME "${MOD_LIBRARY_NAME}")

target_include_directories(${MOD_LIBRARY_NAME}
    PRIVATE
        src
        third_party
        "${LEVI_PRELOADER_ROOT}/src"
)

target_compile_options(${MOD_LIBRARY_NAME}
    PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,Clang,GNU>:-fvisibility=hidden -fvisibility-inlines-hidden -O2 -ffunction-sections -fdata-sections>
)

# The preloader runtime (GlossInit/Open/Symbol/Hook/HookDisable,
# GetPreloaderModMenu, GetPreloaderInput, pl_resolve_signature, ...) is provided
# by the launcher's libpreloader.so, already loaded with global scope. Mods
# resolve these symbols at load time, so the mod .so is intentionally linked with
# undefined symbols. The Android NDK toolchain defaults to -Wl,--no-undefined
# (with --fatal-warnings), which would reject them at link time, so we strip that
# and tell the linker to leave unresolved symbols to the runtime loader.
string(REPLACE "-Wl,--no-undefined" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

target_link_options(${MOD_LIBRARY_NAME}
    PRIVATE
        "-Wl,-z,max-page-size=16384"
        "-Wl,--gc-sections"
        "-Wl,--unresolved-symbols=ignore-all"
)

find_library(log-lib log REQUIRED)
find_library(egl-lib EGL REQUIRED)
find_library(gles-lib GLESv3 REQUIRED)
find_library(android-lib android REQUIRED)

target_link_libraries(${MOD_LIBRARY_NAME} PRIVATE
    imgui
    ${log-lib}
    ${egl-lib}
    ${gles-lib}
    ${android-lib}
    nlohmann_json::nlohmann_json
    Boost::pfr
    magic_enum::magic_enum
)

if(NOT CMAKE_STRIP)
    message(FATAL_ERROR "CMAKE_STRIP is required to strip the generated library.")
endif()

add_custom_command(TARGET ${MOD_LIBRARY_NAME} POST_BUILD
    COMMAND "${CMAKE_STRIP}" --strip-unneeded "$<TARGET_FILE:${MOD_LIBRARY_NAME}>"
    COMMENT "Stripping symbols from $<TARGET_FILE_NAME:${MOD_LIBRARY_NAME}>"
    VERBATIM
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    @ONLY
)

# ---- .levipack packaging ----------------------------------------------
set(LEVI_PACKAGE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/package")
set(LEVI_PACKAGE_MOD_DIR "${LEVI_PACKAGE_ROOT}/${MOD_ID}")
set(LEVI_PACKAGE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${MOD_ID}-${MOD_VERSION}-${CMAKE_ANDROID_ARCH_ABI}.levipack")
set(LEVI_PACKAGE_ICON_COMMAND)
set(LEVI_PACKAGE_CONFIG_COMMAND
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${LEVI_PACKAGE_CONFIG_DIR}" "${LEVI_PACKAGE_MOD_DIR}/config")

if(NOT MOD_ICON STREQUAL "")
    if(IS_ABSOLUTE "${MOD_ICON}")
        message(FATAL_ERROR "MOD_ICON must be a package-relative path")
    endif()
    if(MOD_ICON MATCHES "(^|/)\\.\\.(/|$)")
        message(FATAL_ERROR "MOD_ICON must not contain ..")
    endif()
    set(MOD_ICON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${MOD_ICON}")
    if(NOT EXISTS "${MOD_ICON_SOURCE}")
        message(FATAL_ERROR "MOD_ICON does not exist: ${MOD_ICON_SOURCE}")
    endif()
    get_filename_component(MOD_ICON_DIR "${MOD_ICON}" DIRECTORY)
    if(NOT MOD_ICON_DIR STREQUAL "")
        list(APPEND LEVI_PACKAGE_ICON_COMMAND
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${LEVI_PACKAGE_MOD_DIR}/${MOD_ICON_DIR}")
    endif()
    list(APPEND LEVI_PACKAGE_ICON_COMMAND
        COMMAND "${CMAKE_COMMAND}" -E copy "${MOD_ICON_SOURCE}" "${LEVI_PACKAGE_MOD_DIR}/${MOD_ICON}")
endif()

add_custom_target(levi_package
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${LEVI_PACKAGE_ROOT}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${LEVI_PACKAGE_MOD_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${MOD_LIBRARY_NAME}>" "${LEVI_PACKAGE_MOD_DIR}/lib${MOD_LIBRARY_NAME}.so"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/manifest.json" "${LEVI_PACKAGE_MOD_DIR}/manifest.json"
    ${LEVI_PACKAGE_ICON_COMMAND}
    ${LEVI_PACKAGE_CONFIG_COMMAND}
    COMMAND "${CMAKE_COMMAND}" -E chdir "${LEVI_PACKAGE_ROOT}" "${CMAKE_COMMAND}" -E tar "cf" "${LEVI_PACKAGE_FILE}" --format=zip "${MOD_ID}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS ${MOD_LIBRARY_NAME}
    COMMENT "Packaging ${MOD_ID} for LeviLauncher"
    VERBATIM
)
