#!/bin/bash

# Check if a project name was provided
if [ $# -eq 0 ]; then
    echo "Error: Please provide a project name"
    echo "Usage: $0 <project-name>"
    exit 1
fi

# Store the project name
PROJECT_NAME=$1

# Template repository URL - adjust this to your preferred template
TEMPLATE_REPO="https://github.com/raylib-extras/raylib-quickstart.git"

# Clone the template repository
cd src
git clone "$TEMPLATE_REPO" temp_clone

# Check if clone was successful
if [ $? -ne 0 ]; then
    echo "Error: Failed to clone template repository"
    exit 1
fi

# Rename the cloned directory to the project name
mv temp_clone "$PROJECT_NAME"

# Initialize new git repository
cd "$PROJECT_NAME"
rm -rf .git

# remove other files i don't need
rm -rf .vscode
rm -rf src/application.rc
rm -rf build-MinGW-W64.bat
rm -rf build-VisualStudio2022.bat

# rename main.c to main.cpp
mv src/main.c src/main.cpp

echo "Project '$PROJECT_NAME' created successfully!"
