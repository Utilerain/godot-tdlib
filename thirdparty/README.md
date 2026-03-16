# TdLib

Pre-built packages for working with the TdLib library. The build is performed in a standard way.

## Build

### Windows-x86-64

Here is complete instruction for TDLib binaries building:

* Note that Windows Subsystem for Linux (WSL) and Cygwin are not Windows environments, so you need to use instructions for Linux for them instead.
* Download and install [Microsoft Visual Studio](https://visualstudio.microsoft.com/vs/community/). Enable C++ support while installing.
* Download and install [CMake](https://cmake.org/download/); choose "Add CMake to the system PATH" option while installing.
* Download and install [Git](https://git-scm.com/install/windows).
* Download and unpack [PHP](https://windows.php.net/download). Add the path to php.exe to the PATH environment variable.
* Close and re-open **PowerShell** if the PATH environment variable was changed.

After that you can start building with script:
```powershell
.\build_win.bat
```

## Documentation

For more information, visit the [TdLib GitHub repository](https://github.com/tdlib/td).