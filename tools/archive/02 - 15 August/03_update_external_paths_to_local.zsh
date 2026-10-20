#!/bin/zsh

# Set your Xcode project path here
PROJECT_PATH="$$.xcodeproj"
PBXPROJ_FILE="$PROJECT_PATH/project.pbxproj"

if [[ ! -f "$PBXPROJ_FILE" ]]; then
    echo "Error: Could not find project.pbxproj at $PBXPROJ_FILE"
    exit 1
fi

# Backup first
cp "$PBXPROJ_FILE" "${PBXPROJ_FILE}.bak"
echo "Backup created at ${PBXPROJ_FILE}.bak"

# External volume path to replace - escape spaces with backslash
EXT_VOLUME="/Volumes/macOS\\ -\\ Beck"

# Homebrew include and lib
HOMEBREW_INCLUDE="/opt/homebrew/include"
HOMEBREW_LIB="/opt/homebrew/lib"

echo "Replacing external volume references with homebrew and relative paths..."

# Remove all externalDrive/flint/lib and dll paths and replace with homebrew lib path
/usr/bin/sed -i '' "s|${EXT_VOLUME}/$$|$HOMEBREW_LIB|g" "$PBXPROJ_FILE"
/usr/bin/sed -i '' "s|${EXT_VOLUME}$$|$HOMEBREW_LIB|g" "$PBXPROJ_FILE"

# Remove any lingering external volume absolute paths replacing with relative project path (.)
# Note: This assumes your source files and project is rooted locally in /Users/nameOfUser/Documents/...
# Adjust if your project layout differs

# Escape spaces for sed
LOCAL_PROJECT_PATH=""

# Replace all external volume absolute project references with relative path '.'
/usr/bin/sed -i '' "s|${EXT_VOLUME}/externalDrive|.|g" "$PBXPROJ_FILE"

# Replace any leftover /Volumes/externalDrive references in other paths with '.'
/usr/bin/sed -i '' "s|${EXT_VOLUME}|.|g" "$PBXPROJ_FILE"

echo "External volume references replaced with local paths and homebrew paths."

echo "Script completed successfully. Please clean your build folder in Xcode and rebuild."

