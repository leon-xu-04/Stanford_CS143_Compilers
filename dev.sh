#!/usr/bin/env bash
#
# dev.sh — open a shell in the Cool course container (edit on host, build/run here).
#
# Usage:
#   ./dev.sh            open an interactive shell in the container
#   ./dev.sh --build    (re)build the image first, then open the shell
#
# The container is disposable (--rm); your real work lives in the bind-mounted
# repo, so nothing is lost when you exit.

set -euo pipefail

# Resolve the repo root = the folder this script sits in, so it works from anywhere.
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE="cool"

# Build the image if asked, or if it doesn't exist yet.
if [[ "${1:-}" == "--build" ]] || ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  echo ">> Building image '$IMAGE' from .devcontainer/Dockerfile ..."
  docker build --platform=linux/amd64 -t "$IMAGE" "$REPO_DIR/.devcontainer"
fi

# Make sure the Docker daemon is up before we try to run.
if ! docker info >/dev/null 2>&1; then
  echo "!! Docker daemon isn't running. Start Docker Desktop (open -a Docker) and retry." >&2
  exit 1
fi

echo ">> Entering container (repo mounted at /workspace, cwd = PA3) ..."
exec docker run --platform=linux/amd64 -it --rm \
  -v "$REPO_DIR:/workspace" \
  -w /workspace/src/PA3 \
  "$IMAGE" \
  bash -c 'export PATH="/workspace/bin:$PATH"; exec bash'
