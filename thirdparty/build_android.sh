#!/bin/bash

if ! command -v unzip &> /dev/null; then
    echo "Warning: 'unzip' is not found." >&2

    if [ "$EUID" -eq 0 ]; then
        echo "Installing unzip..."
        if command -v apt-get &> /dev/null; then
            apt-get update && apt-get install -y unzip
        elif command -v dnf &> /dev/null; then
            dnf install -y unzip
        elif command -v yum &> /dev/null; then
            yum install -y unzip
        elif command -v pacman &> /dev/null; then
            pacman -S --noconfirm unzip
        else
            echo "Error: Could not determine package manager for automatic installation." >&2
            exit 1
        fi
    else
        echo "Error: 'unzip' is required for script execution, but you do not have superuser (root) privileges to install it." >&2
        exit 1
    fi
fi

echo "unzip utility found, continuing script execution..."


cd tdlib_android
commit_hash=23383aef218a3123f02b744bfa91bde21056a814
docker build --build-arg TDLIB_INTERFACE=JSON --build-arg ANDROID_STL=c++_shared --build-arg ANDROID_NDK_VERSION=28.1.13356709 --build-arg COMMIT_HASH=$commit_hash --output builds .
cd builds
unzip tdlib.zip
