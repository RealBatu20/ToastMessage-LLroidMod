#!/usr/bin/env bash
# Local Linux/macOS build + package helper for the Toast Message mod.
#
# Usage:
#   ANDROID_HOME=~/Android/Sdk ./scripts/build.sh [arm64-v8a|armeabi-v7a]
#
# Requires: Android NDK (28.2.13676358 preferred), CMake 3.22+, Ninja.
set -euo pipefail

ABI="${1:-arm64-v8a}"
ANDROID_PLATFORM="${ANDROID_PLATFORM:-android-28}"
BUILD_ROOT="${BUILD_ROOT:-build}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

find_ndk() {
    if [[ -n "${ANDROID_NDK_HOME:-}" && -d "${ANDROID_NDK_HOME}" ]]; then
        echo "${ANDROID_NDK_HOME}"; return
    fi
    if [[ -n "${ANDROID_HOME:-}" ]]; then
        local pinned="${ANDROID_HOME}/ndk/28.2.13676358"
        if [[ -d "${pinned}" ]]; then echo "${pinned}"; return; fi
        local latest
        latest="$(ls -d "${ANDROID_HOME}"/ndk/*/ 2>/dev/null | sort -V | tail -n1 || true)"
        if [[ -n "${latest}" ]]; then echo "${latest%/}"; return; fi
    fi
    echo "ERROR: Android NDK not found. Set ANDROID_NDK_HOME or ANDROID_HOME." >&2
    exit 1
}

NDK="$(find_ndk)"
TOOLCHAIN="${NDK}/build/cmake/android.toolchain.cmake"

# 1) Generate config.json / config.schema.json with the host generator.
CONFIG_BUILD="${REPO_ROOT}/${BUILD_ROOT}-config"
cmake -S "${REPO_ROOT}" -B "${CONFIG_BUILD}" -G Ninja
cmake --build "${CONFIG_BUILD}" --target levi_generate_config
GENERATED_CONFIG="${CONFIG_BUILD}/generated-config"

# 2) Cross-compile the mod and package it.
BUILD_DIR="${REPO_ROOT}/${BUILD_ROOT}-${ABI}"
cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
    -DANDROID_ABI="${ABI}" \
    -DANDROID_PLATFORM="${ANDROID_PLATFORM}" \
    -DANDROID_STL="c++_shared" \
    -DLEVI_PACKAGE_CONFIG_DIR="${GENERATED_CONFIG}"

cmake --build "${BUILD_DIR}" --target levi_package

echo "Done. Artifacts in ${BUILD_DIR}:"
ls -1 "${BUILD_DIR}"/*.levipack 2>/dev/null || true
