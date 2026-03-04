import os
import sys

env = SConscript("godot-cpp/SConstruct")

tdlib_include = "thirdparty/tdlib/include"
tdlib_lib_path = "thirdparty/tdlib/lib"

env.Append(CPPPATH=["src/", tdlib_include])
env.Append(LIBPATH=[tdlib_lib_path])
env.Append(LIBS=["tdjson"])
sources = Glob("src/*.cpp")

target_lib = "addons/godot-tdlib/bin/godot_tdlib"

library = env.SharedLibrary(
    target=target_lib,
    source=sources,
)

# Copy runtime dependency DLLs (Windows) next to the built library and example addon
runtime_bin = "thirdparty/tdlib/bin"
runtime_dlls = Glob(runtime_bin + '/*.dll')

inst_addon = env.Install('addons/godot-tdlib/bin', runtime_dlls)

# Ensure DLLs are installed after building the library
env.Depends(inst_addon, library)

import shutil

# As a fallback, also perform an explicit copy of the runtime DLLs so they
# are present next to the built library (helps avoid Windows Error 126).
for node in runtime_dlls:
    src_path = str(node)
    dest_dir = 'addons/godot-tdlib/bin'
    dest_path = os.path.join(dest_dir, os.path.basename(src_path))
    try:
        shutil.copy2(src_path, dest_path)
    except Exception as e:
        print('Warning: could not copy', src_path, '->', dest_path, '-', e)



Default(library)