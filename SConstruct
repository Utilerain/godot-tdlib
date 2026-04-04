import os
import sys
import shutil

env = SConscript("godot-cpp/SConstruct")
tdlib_include = ""
tdlib_lib_path = ""
target_lib = ""
dest_dir = ""

target_os = env["platform"].replace("dows", "")
target_template = env["target"]
tdlib_include = "thirdparty/tdlib_{}_{}/include".format(target_os,env["arch"])
tdlib_lib_path = "thirdparty/tdlib_{}_{}/lib".format(target_os,env["arch"])
target_lib = "addons/godot-tdlib/bin/godot_tdlib_{}_{}/godot_tdlib".format(target_os,env["arch"])
dest_dir = "addons/godot-tdlib/bin/godot_tdlib_{}_{}".format(target_os,env["arch"])

if target_template == "template_debug":
    target_lib += "_debug"

env.Append(CPPPATH=["src/", tdlib_include])
env.Append(LIBPATH=[tdlib_lib_path])
env.Append(LIBS=["tdjson"])
sources = Glob("src/*.cpp")

library = env.SharedLibrary(
    target=target_lib,
    source=sources,
)

# Installing libraries and dependencies

if env["platform"] == "windows":
    runtime_bin = "thirdparty/tdlib_win_x86_64/bin"
    runtime_dlls = Glob(runtime_bin + '/*.dll')

    inst_addon = env.Install('addons/godot-tdlib/bin', runtime_dlls)

    env.Depends(inst_addon, library)

    for node in runtime_dlls:
        src_path = str(node)
        dest_path = os.path.join(dest_dir, os.path.basename(src_path))
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
        try:
            shutil.copy2(src_path, dest_path)
        except Exception as e:
            print('Warning: could not copy', src_path, '->', dest_path, '-', e)

elif env["platform"] == "linux":
    source_lib = "thirdparty/tdlib_linux_x86_64/lib/libtdjson.so.1.8.63"

    env.Append(CPPPATH=[tdlib_include])

    env.Append(LINKFLAGS=["-Wl,-rpath,'$$ORIGIN'"])

    inst_lib = env.Install(dest_dir, source_lib)
    env.Depends(library, inst_lib)

    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
        
    dest_path = os.path.join(dest_dir, "libtdjson.so.1.8.63")
    try:
        shutil.copy2(source_lib, dest_path)
    except Exception as e:
        print(f'Warning: could not copy {source_lib} -> {dest_path}: {e}')

Default(library)