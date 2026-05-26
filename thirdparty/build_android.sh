cd tdlib_android
sudo apt-get install -y unzip
docker build --build-arg TDLIB_INTERFACE=JSON --build-arg ANDROID_STL=c++_shared --build-arg ANDROID_NDK_VERSION=28.1.13356709 --output bin .
cd bin
unzip tdlib.zip
