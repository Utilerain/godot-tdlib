
# Used from https://tdlib.github.io/td/build.html (Language: Other, OS: Windows, Architecture: x86-64, Terminal: PowerShell)

$ErrorActionPreference = "Stop"
$ThirdpartyRoot = $PSScriptRoot
$TdRoot = Join-Path $ThirdpartyRoot "td"
$VcpkgRoot = Join-Path $ThirdpartyRoot "vcpkg"
$VcpkgExe = Join-Path $VcpkgRoot "vcpkg.exe"

if (-not (Test-Path (Join-Path $VcpkgRoot ".git"))) {
	throw "vcpkg was not found at $VcpkgRoot"
}

if (-not (Test-Path $VcpkgExe)) {
	Push-Location $VcpkgRoot
	try {
		.\bootstrap-vcpkg.bat
		if ($LASTEXITCODE -ne 0) {
			throw "vcpkg bootstrap failed with exit code $LASTEXITCODE"
		}
	}
	finally {
		Pop-Location
	}
}

$ZlibRoot = Join-Path $VcpkgRoot "installed\x64-windows"
& $VcpkgExe install gperf:x64-windows openssl:x64-windows zlib:x64-windows
if ($LASTEXITCODE -ne 0) {
	throw "vcpkg dependency installation failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path (Join-Path $ZlibRoot "include\zlib.h")) -or
	-not (Test-Path (Join-Path $ZlibRoot "lib\z.lib"))) {
	throw "vcpkg did not install zlib for x64-windows"
}

$BuildRoot = Join-Path $TdRoot "build"
New-Item -ItemType Directory -Path $BuildRoot -Force | Out-Null
Push-Location $BuildRoot
try {
	cmake -A x64 `
	-DCMAKE_INSTALL_PREFIX:PATH=../../tdlib_win_x86_64 `
	-DCMAKE_TOOLCHAIN_FILE:FILEPATH="$VcpkgRoot/scripts/buildsystems/vcpkg.cmake" `
	-DVCPKG_TARGET_TRIPLET=x64-windows `
	-DZLIB_INCLUDE_DIR:PATH="$ZlibRoot/include" `
	-DZLIB_LIBRARY:FILEPATH="$ZlibRoot/lib/z.lib" $TdRoot
	if ($LASTEXITCODE -ne 0) {
		throw "TDLib CMake configuration failed with exit code $LASTEXITCODE"
	}
	cmake --build . --target install --config Release --parallel
	if ($LASTEXITCODE -ne 0) {
		throw "TDLib build failed with exit code $LASTEXITCODE"
	}
}
finally {
	Pop-Location
}

