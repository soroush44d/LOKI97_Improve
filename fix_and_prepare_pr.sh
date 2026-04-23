#!/usr/bin/env bash
set -euo pipefail

# 1) verify build is healthy
./check_build.sh

# 2) create a ready-to-push branch name
BRANCH_NAME=${1:-fix/loki97-stable-sidechannel}

git checkout -B "$BRANCH_NAME"

echo ""
echo "✅ Branch prepared: $BRANCH_NAME"
echo "Next commands:"
echo "  git push -u origin $BRANCH_NAME"
echo "  # then open PR from $BRANCH_NAME -> main"
