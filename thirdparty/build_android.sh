cd tdlib_android
docker build --build-arg TDLIB_INTERFACE=JSON --build-arg ANDROID_STL=c++_shared --output builds .
cd builds
rm -rf tdlib_debug.zip
unzip tdlib.zip
