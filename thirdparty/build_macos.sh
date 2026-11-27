# Used from https://tdlib.github.io/td/build.html (Language: Other, OS: macOS)
# Dependencies: Homebrew, gperf (brew install gperf), openssl@3 (preinstalled on GitHub macOS runners).
# Installs into tdlib_macos_<arch>/ for the machine's native architecture.

set -e

ARCH_DIR="tdlib_macos_$(uname -m)"
OPENSSL_ROOT="$(brew --prefix openssl@3 2>/dev/null || echo /usr/local/opt/openssl@3)"

cd td
if [ ! -d "build_macos" ]; then
  mkdir build_macos
fi
cd build_macos
cmake -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT" -DCMAKE_INSTALL_PREFIX:PATH=../../"${ARCH_DIR}" ..
cmake --build . --target install --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
cd ..
cd ..
