#!/bin/zsh

# Set your Xcode project path here (path to .xcodeproj folder)
PROJECT_PATH="$$.xcodeproj/"

PBXPROJ_FILE="$PROJECT_PATH/project.pbxproj"

if [[ ! -f "$PBXPROJ_FILE" ]]; then
    echo "Error: Could not find project.pbxproj at $PBXPROJ_FILE"
    exit 1
fi

# Backup original project file
cp "$PBXPROJ_FILE" "${PBXPROJ_FILE}.bak"
echo "Backup created at ${PBXPROJ_FILE}.bak"

# Define old invalid paths to remove
OLD_PATHS=(
    ""
    ""
)

# Define correct paths
NEW_HEADER_PATH="/usr/local/include"
NEW_LIB_PATH="/usr/local/lib"

echo "Updating Header Search Paths..."
# Remove old header search paths if any (they appear in HEADER_SEARCH_PATHS)
# and add correct path

# We will remove any old path first, then add the new path if missing
for path in $OLD_PATHS; do
    sed -i '' "s|$path||g" "$PBXPROJ_FILE"
done

# Add new header path if not present
grep -q "$NEW_HEADER_PATH" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
    # Add header search path inside the HEADER_SEARCH_PATHS section
    # This part is a bit tricky because pbxproj is structured,
    # so just append it to the first HEADER_SEARCH_PATHS found line for safety

    sed -i '' "/HEADER_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$NEW_HEADER_PATH\",|" "$PBXPROJ_FILE"
    echo "Added $NEW_HEADER_PATH to Header Search Paths"
else
    echo "Header Search Path $NEW_HEADER_PATH already present"
fi

echo "Updating Library Search Paths..."
# Remove old library search paths
for path in $OLD_PATHS; do
    sed -i '' "s|$path||g" "$PBXPROJ_FILE"
done

# Add new library path if not present
grep -q "$NEW_LIB_PATH" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
    sed -i '' "/LIBRARY_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$NEW_LIB_PATH\",|" "$PBXPROJ_FILE"
    echo "Added $NEW_LIB_PATH to Library Search Paths"
else
    echo "Library Search Path $NEW_LIB_PATH already present"
fi

echo "Fixing Other Linker Flags..."

# Check if flint and dependencies are properly linked with -l flags

# This function will add a flag to OTHER_LDFLAGS if missing
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

echo "Changes applied. Please clean build folder in Xcode and rebuild your project."

