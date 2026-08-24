cd tdlib_android
sudo apt-get install -y unzip
commit_hash=d1085f9cebc5a62379991ae1652673954f229c1f
docker build --build-arg TDLIB_INTERFACE=JSON --build-arg ANDROID_STL=c++_shared --build-arg ANDROID_NDK_VERSION=28.1.13356709 --build-arg COMMIT_HASH=$commit_hash --output builds .
cd builds
unzip tdlib.zip
