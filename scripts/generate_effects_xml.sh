#!/bin/bash

# generate_effects_xml.sh
# Converts wildcard <Graphics> elements to explicit <Graphic> elements in Effects.xml

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_DIR="${SCRIPT_DIR}/../config"
GRAPHICS_DIR="${SCRIPT_DIR}/../graphics/Effects"
INPUT_FILE="${CONFIG_DIR}/Effects.xml"
OUTPUT_FILE="${CONFIG_DIR}/Effects_new.xml"

echo "Generating explicit Effects.xml..."
echo "Input: ${INPUT_FILE}"
echo "Output: ${OUTPUT_FILE}"

# Check if input file exists
if [[ ! -f "${INPUT_FILE}" ]]; then
    echo "Error: Input file not found: ${INPUT_FILE}"
    exit 1
fi

# Start building the output file with XML header
cat > "${OUTPUT_FILE}" << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<Effects>
EOF

# Read the input file and process it (skip first 2 lines: XML declaration and root element)
tail -n +3 "${INPUT_FILE}" | while IFS= read -r line; do
    # Check if this is a Graphics line with wildcard
    if [[ "$line" =~ ^[[:space:]]*\<Graphics\>([^/]+)/\*\.tga\</Graphics\>[[:space:]]*$ ]]; then
        dir_name="${BASH_REMATCH[1]}"
        full_dir="${GRAPHICS_DIR}/${dir_name}"
        
        echo "Processing wildcard: ${dir_name}/*.tga"
        
        # Check if directory exists
        if [[ -d "${full_dir}" ]]; then
            # List all .tga files, sort them, and output as Graphic elements
            files=$(ls -1 "${full_dir}"/*.tga 2>/dev/null | sort)
            if [[ -n "$files" ]]; then
                for file_path in $files; do
                    file_name=$(basename "$file_path")
                    # Preserve the original indentation (tabs)
                    printf '\t\t<Graphic>%s/%s</Graphic>\n' "${dir_name}" "${file_name}" >> "${OUTPUT_FILE}"
                done
            else
                echo "Warning: No .tga files found in ${full_dir}"
                # Keep the original line as a comment
                printf '\t\t<!-- No files found: %s -->\n' "$line" >> "${OUTPUT_FILE}"
            fi
        else
            echo "Warning: Directory not found: ${full_dir}"
            # Keep the original line as a comment
            printf '\t\t<!-- Directory not found: %s -->\n' "$line" >> "${OUTPUT_FILE}"
        fi
    else
        # Copy line as-is
        echo "$line" >> "${OUTPUT_FILE}"
    fi
done

# Append the closing root element
echo "</Effects>" >> "${OUTPUT_FILE}"

echo ""
echo "Generated: ${OUTPUT_FILE}"
echo ""

# Count the wildcards that were replaced
wildcard_count=$(grep -c '<Graphics>[^<]*/\*\.tga</Graphics>' "${INPUT_FILE}")
graphic_count=$(grep -c '<Graphic>[^<]*\.tga</Graphic>' "${OUTPUT_FILE}")

echo "Statistics:"
echo "  - Replaced ${wildcard_count} wildcard <Graphics> elements"
echo "  - Generated ${graphic_count} explicit <Graphic> elements"
echo ""
echo "To apply the changes:"
echo "  mv ${OUTPUT_FILE} ${INPUT_FILE}"
