#!/bin/bash

# Claude Clone APK Builder
# This script builds a ready-to-install APK for Android

set -e

echo "================================"
echo "Claude Clone APK Builder"
echo "================================"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"

if ! command -v node &> /dev/null; then
  echo -e "${RED}Node.js is required but not installed${NC}"
  exit 1
fi

if ! command -v npm &> /dev/null; then
  echo -e "${RED}npm is required but not installed${NC}"
  exit 1
fi

if [ ! -d "mobile/android" ]; then
  echo -e "${RED}React Native Android folder not found${NC}"
  echo "Make sure you're in the claude-clone directory"
  exit 1
fi

BUILD_TYPE="${1:-debug}"

echo -e "${GREEN}✓ Prerequisites OK${NC}"
echo ""

# Setup mobile app
echo -e "${YELLOW}Setting up mobile app...${NC}"
cd mobile

if [ ! -d "node_modules" ]; then
  echo "Installing dependencies..."
  npm install
fi

if [ ! -f ".env" ]; then
  echo "Creating .env from template..."
  cp .env.example .env
fi

echo -e "${GREEN}✓ Mobile app ready${NC}"
echo ""

# Build APK
echo -e "${YELLOW}Building APK ($BUILD_TYPE)...${NC}"
cd android

if [ "$BUILD_TYPE" = "release" ]; then
  echo "Building release APK..."
  if [ ! -f "gradle.properties" ]; then
    echo -e "${RED}Release build requires keystore configuration${NC}"
    echo "See README.md for keystore setup instructions"
    exit 1
  fi
  ./gradlew assembleRelease
  APK_PATH="app/build/outputs/apk/release/app-release.apk"
else
  echo "Building debug APK..."
  ./gradlew assembleDebug
  APK_PATH="app/build/outputs/apk/debug/app-debug.apk"
fi

echo ""

# Check if APK was built successfully
if [ -f "$APK_PATH" ]; then
  APK_SIZE=$(ls -lh "$APK_PATH" | awk '{print $5}')
  echo -e "${GREEN}✓ APK built successfully!${NC}"
  echo ""
  echo "APK Details:"
  echo "  Path: $APK_PATH"
  echo "  Size: $APK_SIZE"
  echo "  Type: $BUILD_TYPE"
  echo ""
  echo "Installation:"
  echo "  adb install -r $APK_PATH"
  echo ""
  echo "To install on connected device:"
  echo "  adb install -r $(pwd)/$APK_PATH"
else
  echo -e "${RED}✗ APK build failed${NC}"
  exit 1
fi

echo ""
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}================================${NC}"
