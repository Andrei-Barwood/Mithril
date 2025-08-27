#!/bin/zsh

# Export PATH so explicit Homebrew binaries are found if necessary
export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"

# Set your Xcode project path here (to the .xcodeproj folder)
PROJECT_PATH="$$.xcodeproj"
PBXPROJ_FILE="$PROJECT_PATH/project.pbxproj"

if [[ ! -f "$PBXPROJ_FILE" ]]; then
    echo "Error: Could not find project.pbxproj at $PBXPROJ_FILE"
    exit 1
fi

# Backup before modifying
cp "$PBXPROJ_FILE" "${PBXPROJ_FILE}.bak"
echo "Backup created at ${PBXPROJ_FILE}.bak"

# Define old paths to remove from search paths
OLD_PATHS=(
  "/usr/local/include"
  "/usr/local/lib"
)

# Define new Homebrew paths
NEW_HEADER_PATH="/opt/homebrew/include"
NEW_LIB_PATH="/opt/homebrew/lib"

echo "Cleaning old search paths..."
for old_path in $OLD_PATHS; do
    /usr/bin/sed -i '' "s|$old_path||g" "$PBXPROJ_FILE"
done

echo "Adding new header search path if missing..."
/usr/bin/grep -q "$NEW_HEADER_PATH" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
    /usr/bin/sed -i '' "/HEADER_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$NEW_HEADER_PATH\",|" "$PBXPROJ_FILE"
    echo "Added $NEW_HEADER_PATH"
else
    echo "Header search path $NEW_HEADER_PATH already present"
fi

echo "Adding new library search path if missing..."
/usr/bin/grep -q "$NEW_LIB_PATH" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
    /usr/bin/sed -i '' "/LIBRARY_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$NEW_LIB_PATH\",|" "$PBXPROJ_FILE"
    echo "Added $NEW_LIB_PATH"
else
    echo "Library search path $NEW_LIB_PATH already present"
fi

echo "Ensuring required linker flags are present..."
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

add_flag_if_missing "-lflint"
add_flag_if_missing "-lmpfr"
add_flag_if_missing "-lgmp"
add_flag_if_missing "-lpthread"

echo "Done. Please clean build folder in Xcode and rebuild your project."

