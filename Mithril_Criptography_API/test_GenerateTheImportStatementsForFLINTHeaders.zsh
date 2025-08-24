#!/bin/zsh

# Generate umbrella header imports for Mithril Framework

FRAMEWORK_NAME="Mithril_Framework"
UMBRELLA_HEADER="Mithril_Framework.h"
HEADER_DIR="."  # Adjust if headers are in a subdirectory

echo "🔄 Generating import statements for umbrella header..."

# Find all .h files except the umbrella header itself
HEADERS=($(find "$HEADER_DIR" -name "*.h" -not -name "$UMBRELLA_HEADER" | sort))

if [[ ${#HEADERS[@]} -eq 0 ]]; then
    echo "❌ No header files found"
    exit 1
fi

# Create the new umbrella header
cat > "$UMBRELLA_HEADER" << EOF
//
// $UMBRELLA_HEADER
// $FRAMEWORK_NAME
//
// Created by Andres Barbudo on 17-08-25.
//

#import <Foundation/Foundation.h>

// Auto-generated imports - $(date)
EOF

# Add import statements for each header
for header in "${HEADERS[@]}"; do
    header_name=$(basename "$header")
    echo "#import \"$header_name\"" >> "$UMBRELLA_HEADER"
done

# Add the rest of the umbrella header
cat >> "$UMBRELLA_HEADER" << EOF

//! Project version number for $FRAMEWORK_NAME.
FOUNDATION_EXPORT double ${FRAMEWORK_NAME}VersionNumber;

//! Project version string for $FRAMEWORK_NAME.
FOUNDATION_EXPORT const unsigned char ${FRAMEWORK_NAME}VersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <$FRAMEWORK_NAME/PublicHeader.h>
EOF

echo "✅ Generated umbrella header with ${#HEADERS[@]} imports:"
for header in "${HEADERS[@]}"; do
    echo "   - $(basename "$header")"
done

echo "📄 Umbrella header saved as: $UMBRELLA_HEADER"

