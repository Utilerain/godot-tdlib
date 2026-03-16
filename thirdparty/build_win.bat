@echo off 
REM Used from https://tdlib.github.io/td/build.html (Language: Other, OS: Windows, Architecture: x86-64, Terminal: PowerShell)


cd td 
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
git checkout bc3512a509f9d29b37346a7e7e929f9a26e66c7e

REM IDK why this crashes batch file
start .\bootstrap-vcpkg.bat 

.\vcpkg.exe install gperf:x64-windows openssl:x64-windows zlib:x64-windows
cd ..
Remove-Item build -Force -Recurse -ErrorAction SilentlyContinue
mkdir build
cd build
cmake -A x64 -DCMAKE_INSTALL_PREFIX:PATH=../../tdlib_win_x86_64 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --target install --config Release

