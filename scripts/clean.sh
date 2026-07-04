#!/usr/bin/env bash
# Remove all local build directories produced by build.sh / package.ps1.
set -euo pipefail
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}"
rm -rf build build-*
echo "Cleaned build directories."
