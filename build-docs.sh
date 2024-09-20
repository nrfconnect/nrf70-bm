#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Function to generate Doxygen XML files
generate_doxygen_xml() {
    echo "Generating Doxygen XML files..."
    doxygen Doxyfile
}

# Function to build Sphinx documentation
build_sphinx_docs() {
    echo "Building Sphinx documentation..."
    make html
}

# Main script execution
main() {
    cd nrf70_bm_lib/docs
    generate_doxygen_xml
    build_sphinx_docs

    echo "Documentation build complete. The HTML files are located in nrf70_bm_lib/docs/build/html."
}

# Run the main function
main