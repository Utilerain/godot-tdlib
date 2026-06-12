cd tdlib_android
sudo apt-get install -y unzip
commit_hash=d6debbb2aae29a39e280f86e25ec0e54960dc838
docker build --build-arg TDLIB_INTERFACE=JSON --build-arg ANDROID_STL=c++_shared --build-arg ANDROID_NDK_VERSION=28.1.13356709 --build-arg COMMIT_HASH=$commit_hash --output builds .
cd builds
unzip tdlib.zip
