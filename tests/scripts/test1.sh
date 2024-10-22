#!/bin/bash

# Define a variable for the directory
#TEMP_DIR="/tmp/my_temp_files"

# Create a directory for temporary files
#mkdir -p TEMP_DIR

# Get the directory of the current script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Test the packet generator and parser functionality
$SCRIPT_DIR/../bin/packet_generator | $SCRIPT_DIR/../bin/packet_parser