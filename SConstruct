import os
import sys
import shutil

localEnv = Environment(tools=["default"])
localEnv["build_profile"] = "build_profile.json"
tdlib_version = "1.8.67"

android_archs = {
    "arm64": "arm64-v8a",
    "arm32": "armeabi-v7a",
    "x86_64": "x86_64",
    "x86_32": "x86"
}

env = localEnv.Clone()
if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

env = SConscript("godot-cpp/SConstruct", {"env": env})

target_os = env["platform"].replace("dows", "")
target_template = env["target"]
if target_os == "android":
    tdlib_include = "thirdparty/tdlib_android/include"
    if env["arch"] in android_archs:
        tdlib_lib_path = "thirdparty/tdlib_android/builds/tdlib/libs/{}".format(android_archs[env["arch"]])
else:
    tdlib_include = "thirdparty/tdlib_{}_{}/include".format(target_os,env["arch"])
    tdlib_lib_path = "thirdparty/tdlib_{}_{}/lib".format(target_os,env["arch"])

target_lib = "addons/godot-tdlib/bin/godot_tdlib_{}_{}/godot_tdlib".format(target_os,env["arch"])
dest_dir = "addons/godot-tdlib/bin/godot_tdlib_{}_{}".format(target_os,env["arch"])

env.Append(CPPPATH=["src/", tdlib_include])
env.Append(LIBPATH=[tdlib_lib_path])
env.Append(LIBS=["tdjson"])
sources = Glob("src/*.cpp")

if target_template == "template_debug":
    target_lib += "_debug"

if env["target"] in ["editor", "template_debug"]:
    doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
    sources.append(doc_data)

# Debug symbols
if env["target"] == "template_debug" or env["debug_symbols"] == "yes":
    if env["platform"] == "windows":
        env.Append(CCFLAGS=["/Zi", "/Od"])
        env.Append(LINKFLAGS=["/DEBUG"])
    else:
        env.Append(CCFLAGS=["-g", "-O0"])
        env.Append(LINKFLAGS=["-g"])

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
    source_lib = f"thirdparty/tdlib_linux_x86_64/lib/libtdjson.so.{tdlib_version}"

    env.Append(LINKFLAGS=["-Wl,-rpath,'$$ORIGIN'"])

    inst_lib = env.Install(dest_dir, source_lib)
    env.Depends(library, inst_lib)

    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
        
    dest_path = os.path.join(dest_dir, f"libtdjson.so.{tdlib_version}")
    try:
        shutil.copy2(source_lib, dest_path)
    except Exception as e:
        print(f'Warning: could not copy {source_lib} -> {dest_path}: {e}')

elif env["platform"] == "android":
    source_lib = "thirdparty/tdlib_android/builds/tdlib/libs/{}/libtdjson.so".format(android_archs[env["arch"]])

    env.Append(LINKFLAGS=["-Wl,-rpath,'$$ORIGIN'"])

    inst_bin = env.Install(dest_dir, source_lib)
    env.Depends(library, inst_bin)

    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    
    dest_path = os.path.join(dest_dir, os.path.basename(source_lib))
    try:
        shutil.copy2(source_lib, dest_path)
    except Exception as e:
        print(f'Warning: could not copy {source_lib} -> {dest_path}: {e}')


Default(library)