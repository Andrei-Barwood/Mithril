#!/bin/zsh

# Mithril Framework Header Configuration Script
# This script helps configure all headers in Mithril_Framework directory as public

echo "🔧 Configuring Mithril Framework headers as public..."

# Set framework directory name
FRAMEWORK_DIR="Mithril_Framework"
PROJECT_NAME="Mithril_Framework"

# Function to find Xcode project file
find_xcode_project() {
    local xcodeproj=$(find . -name "*.xcodeproj" -type d | head -1)
    if [[ -n "$xcodeproj" ]]; then
        echo "$xcodeproj"
    else
        echo ""
    fi
}

# Function to find all header files in framework directory
find_header_files() {
    if [[ -d "$FRAMEWORK_DIR" ]]; then
        find "$FRAMEWORK_DIR" -name "*.h" -type f
    else
        find . -name "*.h" -type f | grep -E "(01_addition|fmpz|Mithril_Framework)\.h"
    fi
}

# Main execution
XCODE_PROJECT=$(find_xcode_project)

if [[ -z "$XCODE_PROJECT" ]]; then
    echo "❌ No Xcode project found in current directory"
    echo "Please run this script from your Xcode project root directory"
    exit 1
fi

echo "✅ Found Xcode project: $XCODE_PROJECT"

# Find all header files
HEADER_FILES=($(find_header_files))

if [[ ${#HEADER_FILES[@]} -eq 0 ]]; then
    echo "❌ No header files found"
    exit 1
fi

echo "📂 Found ${#HEADER_FILES[@]} header files:"
for header in "${HEADER_FILES[@]}"; do
    echo "   - $header"
done

# Method 1: Using xcodeproj gem (if available)
if command -v xcodeproj >/dev/null 2>&1; then
    echo "🚀 Using xcodeproj gem to configure headers..."
    
    # Create Ruby script to modify project
    cat > configure_headers.rb << 'EOF'
#!/usr/bin/env ruby
require 'xcodeproj'

project_path = ARGV[0]
framework_target_name = ARGV[1]
header_files = ARGV[2..-1]

project = Xcodeproj::Project.open(project_path)
target = project.targets.find { |t| t.name == framework_target_name }

if target.nil?
  puts "❌ Framework target '#{framework_target_name}' not found"
  exit 1
end

header_files.each do |header_path|
  next unless File.exist?(header_path)
  
  # Add file to project if not already added
  file_ref = project.reference_for_path(header_path)
  unless file_ref
    file_ref = project.new_file(header_path)
  end
  
  # Add to headers build phase as public
  build_file = target.headers_build_phase.add_file_reference(file_ref, true)
  build_file.settings = { "ATTRIBUTES" => ["Public"] }
  
  puts "✅ Set #{header_path} as public header"
end

project.save
puts "🎉 Project saved successfully"
EOF

    ruby configure_headers.rb "$XCODE_PROJECT" "$PROJECT_NAME" "${HEADER_FILES[@]}"
    rm configure_headers.rb

# Method 2: Using xcodebuild (fallback)
elif command -v xcodebuild >/dev/null 2>&1; then
    echo "🔨 Using xcodebuild approach..."
    echo "Manual steps required:"
    echo "1. Open your Xcode project"
    echo "2. Select your framework target"
    echo "3. Go to Build Phases > Headers"
    echo "4. Move the following headers to 'Public' section:"
    for header in "${HEADER_FILES[@]}"; do
        echo "   - $(basename "$header")"
    done

# Method 3: Manual instructions
else
    echo "📋 Manual configuration required:"
    echo ""
    echo "To configure headers as public in Xcode:"
    echo "1. Open $XCODE_PROJECT in Xcode"
    echo "2. Select your '$PROJECT_NAME' framework target"
    echo "3. Go to 'Build Phases' tab"
    echo "4. Expand 'Headers' section"
    echo "5. Drag the following headers from 'Project' to 'Public':"
    echo ""
    for header in "${HEADER_FILES[@]}"; do
        echo "   ✓ $(basename "$header")"
    done
    echo ""
    echo "Alternative method:"
    echo "1. Select each header file in Project Navigator"
    echo "2. In File Inspector (right panel), under 'Target Membership'"
    echo "3. Change dropdown from 'Project' to 'Public'"
fi

# Create a summary file
cat > header_configuration_summary.txt << EOF
Mithril Framework Header Configuration Summary
==============================================

Project: $XCODE_PROJECT
Framework Target: $PROJECT_NAME
Headers to configure as PUBLIC:

$(for header in "${HEADER_FILES[@]}"; do echo "- $header"; done)

Configuration Steps:
1. Open Xcode project
2. Select framework target: $PROJECT_NAME  
3. Build Phases > Headers
4. Move headers from 'Project' to 'Public' section

Or use File Inspector method:
1. Select header file in Project Navigator
2. File Inspector > Target Membership
3. Change from 'Project' to 'Public'

Generated on: $(date)
EOF

echo ""
echo "📄 Summary saved to: header_configuration_summary.txt"
echo "🎯 Next steps:"
echo "   1. Open your Xcode project"
echo "   2. Follow the instructions in the summary file"
echo "   3. Build your framework to verify configuration"
echo ""
echo "✨ Done!"

