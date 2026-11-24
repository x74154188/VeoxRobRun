#!/usr/bin/env sh
set -eu
TARGET="${1:-artifacts/MODIFIED_FILE}"
BACKUP="${TARGET}.backup"
cp "$TARGET" "$BACKUP"
git show HEAD:VeoxDMA/Misc/Localization.h > "$TARGET"
if cmp -s "$TARGET" "$BACKUP"; then
  echo "rollback failed: content unchanged"
  exit 1
fi
printf 'rollback restored %s from HEAD\n' "$TARGET"