#!/bin/zsh

# Project .xcodeproj path (modify as needed)
PROJECT_PATH="$$.xcodeproj"
PBXPROJ_FILE="$PROJECT_PATH/project.pbxproj"

if [[ ! -f "$PBXPROJ_FILE" ]]; then
  echo "Error: Could not find project.pbxproj at $PBXPROJ_FILE"
  exit 1
fi

echo "Creating backup of project.pbxproj..."
cp "$PBXPROJ_FILE" "${PBXPROJ_FILE}.bak"
echo "Backup created at ${PBXPROJ_FILE}.bak"

# Remove old external volume paths and replace with local Homebrew paths
echo "Updating project.pbxproj with local Homebrew paths..."

EXT_VOLUME=""
HOMEBREW_INCLUDE="/opt/homebrew/include"
HOMEBREW_LIB="/opt/homebrew/lib"

/usr/bin/sed -i '' "s|${EXT_VOLUME}/flint/lib/dll|$HOMEBREW_LIB|g" "$PBXPROJ_FILE"
/usr/bin/sed -i '' "s|${EXT_VOLUME}/flint/lib|$HOMEBREW_LIB|g" "$PBXPROJ_FILE"
/usr/bin/sed -i '' "s|${EXT_VOLUME}|.|g" "$PBXPROJ_FILE"

# Ensure header search paths set to Homebrew include
echo "Setting header search path to Homebrew include..."
/usr/bin/grep -q "$HOMEBREW_INCLUDE" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
  /usr/bin/sed -i '' "/HEADER_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$HOMEBREW_INCLUDE\",|" "$PBXPROJ_FILE"
  echo "Added $HOMEBREW_INCLUDE to Header Search Paths"
else
  echo "Header Search Path already includes $HOMEBREW_INCLUDE"
fi

# Ensure library search paths set to Homebrew lib
echo "Setting library search path to Homebrew lib..."
/usr/bin/grep -q "$HOMEBREW_LIB" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
  /usr/bin/sed -i '' "/LIBRARY_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$HOMEBREW_LIB\",|" "$PBXPROJ_FILE"
  echo "Added $HOMEBREW_LIB to Library Search Paths"
else
  echo "Library Search Path already includes $HOMEBREW_LIB"
fi

# Add required linker flags if missing
function add_flag_if_missing() {
  local flag=$1
  /usr/bin/grep -q "\\$flag" "$PBXPROJ_FILE"
  if [[ $? -ne 0 ]]; then
    /usr/bin/sed -i '' "/OTHER_LDFLAGS = (/ s|(|(\n\t\t\t\t\"$flag\",|" "$PBXPROJ_FILE"
    echo "Added linker flag $flag"
  else
    echo "Linker flag $flag already present"
  fi
}

echo "Ensuring linker flags are set..."
add_flag_if_missing "-lflint"
add_flag_if_missing "-lmpfr"
add_flag_if_missing "-lgmp"
add_flag_if_missing "-lpthread"

# Clean DerivedData directory for all projects
DERIVED_DATA="$HOME/Library/Developer/Xcode/DerivedData"

echo "Deleting DerivedData directory at $DERIVED_DATA..."
rm -rf "$DERIVED_DATA"
echo "DerivedData cleared."

# Optional: clear Xcode's ModuleCache if needed
MODULE_CACHE="$HOME/Library/Developer/Xcode/DerivedData/ModuleCache"
if [[ -d "$MODULE_CACHE" ]]; then
  echo "Clearing ModuleCache..."
  rm -rf "$MODULE_CACHE"
fi

echo "Xcode build folder cleanup and project setup complete."
echo "Please open Xcode, clean build folder (Cmd + Shift + Option + K), then build your project again."

