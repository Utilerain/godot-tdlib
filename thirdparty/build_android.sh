cd tdlib_android
sudo apt-get install -y unzip
commit_hash=ea3f69ce11aef458bfbb09869b809ede4447486f
docker build --build-arg TDLIB_INTERFACE=JSON --build-arg ANDROID_STL=c++_shared --build-arg ANDROID_NDK_VERSION=28.1.13356709 --build-arg COMMIT_HASH=$commit_hash --output builds .
cd builds
unzip tdlib.zip
