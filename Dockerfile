#
#   Godot Android Builder for godot-tdlib
#   Image uses NDK 28.1.13356709 by default
#
#   How to use:
#   1. Build the Docker image:
#      docker build -t godot-tdlib-android-builder .
#   2. Run the container, mounting the project directory:
#       docker run --rm -v /path/to/your/godot-tdlib:/project godot-tdlib-android-builder
#       (For example (Windows): docker run --rm -v ${PWD}:/build godot-tdlib-android-builder scons -j6 platform=android verbose=true)
#       (For example (Linux): docker run --rm -v $(pwd):/build godot-tdlib-android-builder scons -j6 platform=android verbose=true)
#

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    python3 \
    scons \
    unzip \
    wget \
    git \
    && rm -rf /var/lib/apt/lists/*

ENV ANDROID_HOME=/opt/android
ENV ANDROID_NDK_ROOT=${ANDROID_HOME}/ndk/28.1.13356709
RUN mkdir -p ${ANDROID_NDK_ROOT}

RUN wget -q https://dl.google.com/android/repository/android-ndk-r28-linux.zip -O /tmp/ndk.zip && \
    unzip -q /tmp/ndk.zip -d /tmp && \
    cp -r /tmp/android-ndk-r28/. ${ANDROID_NDK_ROOT}/ && \
    rm -rf /tmp/ndk.zip /tmp/android-ndk-r28

WORKDIR /build

CMD ["scons", "platform=android", "verbose=true"]