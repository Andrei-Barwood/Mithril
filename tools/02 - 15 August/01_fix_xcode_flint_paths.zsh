#!/bin/zsh

# Export a clean PATH to ensure standard tools are found
export PATH="/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"

# Set your Xcode project path here (path to .xcodeproj folder)
PROJECT_PATH="$$.xcodeproj"

PBXPROJ_FILE="$PROJECT_PATH/project.pbxproj"

if [[ ! -f "$PBXPROJ_FILE" ]]; then
    echo "Error: Could not find project.pbxproj at $PBXPROJ_FILE"
    exit 1
fi

# Backup original project file
cp "$PBXPROJ_FILE" "${PBXPROJ_FILE}.bak"
echo "Backup created at ${PBXPROJ_FILE}.bak"

# Define old invalid paths to remove (add any other bad paths if needed)
OLD_PATHS=(
    ""
    ""
)

# Define correct paths
NEW_HEADER_PATH="/usr/local/include"
NEW_LIB_PATH="/usr/local/lib"

echo "Updating Header Search Paths..."
# Remove old header search paths
for path in $OLD_PATHS; do
    /usr/bin/sed -i '' "s|$path||g" "$PBXPROJ_FILE"
done

# Add new header search path if missing
/usr/bin/grep -q "$NEW_HEADER_PATH" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
    /usr/bin/sed -i '' "/HEADER_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$NEW_HEADER_PATH\",|" "$PBXPROJ_FILE"
    echo "Added $NEW_HEADER_PATH to Header Search Paths"
else
    echo "Header Search Path $NEW_HEADER_PATH already present"
fi

echo "Updating Library Search Paths..."
# Remove old library search paths
for path in $OLD_PATHS; do
    /usr/bin/sed -i '' "s|$path||g" "$PBXPROJ_FILE"
done

# Add new library search path if missing
/usr/bin/grep -q "$NEW_LIB_PATH" "$PBXPROJ_FILE"
if [[ $? -ne 0 ]]; then
    /usr/bin/sed -i '' "/LIBRARY_SEARCH_PATHS = (/ s|(|(\n\t\t\t\t\"$NEW_LIB_PATH\",|" "$PBXPROJ_FILE"
    echo "Added $NEW_LIB_PATH to Library Search Paths"
else
    echo "Library Search Path $NEW_LIB_PATH already present"
fi

echo "Fixing Other Linker Flags..."

# Function to add linker flags if missing
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

