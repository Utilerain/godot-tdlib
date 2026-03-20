# Used from https://tdlib.github.io/td/build.html (Language: Other, OS: Linux, Architecture: x86-64)
# Please be sure that you have installed all dependencies: Git, g++ >= 4.9.2, make, CMake >= 3.10, OpenSSL-dev, zlib-dev, gperf, PHP
# You can install those dependencies using package manager.

cd td
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=../../tdlib_linux_x86_64 ..
cmake --build . --target install
cd ..
cd ..