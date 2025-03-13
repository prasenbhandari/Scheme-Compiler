
#!/bin/bash

# Define input and output filenames
DIR="/home/prasen/Programing/C/Project/Compiler/hash_table"
GPERF_INPUT="$DIR/scheme_keywords.gperf"
HASH_C_OUTPUT="$DIR/scheme_keywords_hash.c"

# Check if gperf is installed
if ! command -v gperf &> /dev/null; then
    echo "Error: gperf is not installed. Install it using: sudo apt install gperf (Debian/Ubuntu) or sudo pacman -S gperf (Arch)."
    exit 1
fi

# Generate scheme_keywords_hash.c
gperf --output-file=$HASH_C_OUTPUT $GPERF_INPUT


# Display success message
echo "Generated $HASH_C_OUTPUT and $HASH_H_OUTPUT successfully!"
