import os
import sys

env = SConscript("godot-cpp/SConstruct")
tdlib_include = ""
tdlib_lib_path = ""
target_lib = ""
dest_dir = ""

if env["platform"] == "windows":
    tdlib_include = "thirdparty/tdlib_win_x86_64/include"
    tdlib_lib_path = "thirdparty/tdlib_win_x86_64/lib"
    target_lib = "addons/godot-tdlib/bin/godot_tdlib_win_x86_64/godot_tdlib"
    dest_dir = "addons/godot-tdlib/bin/godot_tdlib_win_x86_64"

elif env["platform"] == "linux":
    tdlib_include = "thirdparty/tdlib_linux/include"
    tdlib_lib_path = "thirdparty/tdlib_linux/lib"
    target_lib = "addons/godot-tdlib/bin/godot_tdlib_linux/godot_tdlib"
    dest_dir = "addons/godot-tdlib/bin/godot_tdlib_linux"


env.Append(CPPPATH=["src/", tdlib_include])
env.Append(LIBPATH=[tdlib_lib_path])
env.Append(LIBS=["tdjson"])
sources = Glob("src/*.cpp")

library = env.SharedLibrary(
    target=target_lib,
    source=sources,
)

if env["platform"] == "windows":
    runtime_bin = "thirdparty/tdlib_win_x86_64/bin"

elif env["platform"] == "linux":
    runtime_bin = "thirdparty/tdlib_linux/bin"


runtime_dlls = Glob(runtime_bin + '/*.dll')

inst_addon = env.Install('addons/godot-tdlib/bin', runtime_dlls)

# Ensure DLLs are installed after building the library
env.Depends(inst_addon, library)

import shutil

# As a fallback, also perform an explicit copy of the runtime DLLs so they
# are present next to the built library (helps avoid Windows Error 126).
for node in runtime_dlls:
    src_path = str(node)
    dest_path = os.path.join(dest_dir, os.path.basename(src_path))
    try:
        shutil.copy2(src_path, dest_path)
    except Exception as e:
        print('Warning: could not copy', src_path, '->', dest_path, '-', e)



Default(library)