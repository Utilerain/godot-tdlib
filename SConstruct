import os
import sys
import shutil

localEnv = Environment(tools=["default"])
localEnv["build_profile"] = "build_profile.json"
tdlib_version = "1.8.71"

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
tdlib_arch = env["arch"]
if target_os == "android":
    tdlib_include = "thirdparty/tdlib_android/include"
    if env["arch"] in android_archs:
        tdlib_lib_path = "thirdparty/tdlib_android/builds/tdlib/libs/{}".format(android_archs[env["arch"]])
elif target_os == "macos":
    # godot-cpp defaults macOS builds to "universal", but the prebuilt TDLib libraries are
    # single-arch (CI builds the native arm64 variant). Fall back to arm64 when no
    # universal TDLib build is present, and name the output directory after the TDLib arch.
    if tdlib_arch == "universal" and not os.path.isdir("thirdparty/tdlib_macos_universal/include"):
        tdlib_arch = "arm64"
        print("Note: no universal TDLib build found, linking the arm64 TDLib libraries.")
    tdlib_include = "thirdparty/tdlib_macos_{}/include".format(tdlib_arch)
    tdlib_lib_path = "thirdparty/tdlib_macos_{}/lib".format(tdlib_arch)
else:
    tdlib_include = "thirdparty/tdlib_{}_{}/include".format(target_os,env["arch"])
    tdlib_lib_path = "thirdparty/tdlib_{}_{}/lib".format(target_os,env["arch"])

output_arch = tdlib_arch if target_os == "macos" else env["arch"]
target_lib = "addons/godot-tdlib/bin/godot_tdlib_{}_{}/godot_tdlib".format(target_os,output_arch)
dest_dir = "addons/godot-tdlib/bin/godot_tdlib_{}_{}".format(target_os,output_arch)

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
if env["debug_symbols"] == "yes":
    if env["platform"] == "windows":
        env.Append(CCFLAGS=["/Zi", "/Od", "/FS"])
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

elif env["platform"] == "macos":
    env.Append(LINKFLAGS=["-Wl,-rpath,@loader_path"])

    dylibs = Glob(tdlib_lib_path + "/libtdjson*.dylib")

    inst_lib = env.Install(dest_dir, dylibs)
    env.Depends(library, inst_lib)

    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    for node in dylibs:
        src_path = str(node)
        dest_path = os.path.join(dest_dir, os.path.basename(src_path))
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

    env.Append(LINKFLAGS=["-Wl,-rpath,'$$ORIGIN'", "-Wl,-z,max-page-size=65536", "-Wl,-z,common-page-size=16384"])

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