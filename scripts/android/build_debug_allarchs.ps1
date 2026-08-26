echo 'Build Dockerfile...'
docker build -t godot-tdlib-android-builder .

echo 'Compile library for android (arch=x86_32)...'
docker run --rm -v ${PWD}:/build godot-tdlib-android-builder scons -j6 platform=android target=template_debug arch=x86_32 debug_symbols=yes
echo 'Compile library for android (arch=x86_64)...'
docker run --rm -v ${PWD}:/build godot-tdlib-android-builder scons -j6 platform=android target=template_debug arch=x86_64 debug_symbols=yes
echo 'Compile library for android (arch=arm32)...'
docker run --rm -v ${PWD}:/build godot-tdlib-android-builder scons -j6 platform=android target=template_debug arch=arm32 debug_symbols=yes
echo 'Compile library for android (arch=arm64)...'
docker run --rm -v ${PWD}:/build godot-tdlib-android-builder scons -j6 platform=android target=template_debug arch=arm64 debug_symbols=yes