# Godot-tdlib
GDExtension C++ wrapper for working with [TdLib](https://github.com/tdlib/td) (JSON interface) on Godot Engine.

### Realized Functions

1. `TdJson.send` (td_send from [td_json_client.h](https://core.telegram.org/tdlib/docs/td__json__client_8h.html))
2. `TdJson.receive` (td_receive from [td_json_client.h](https://core.telegram.org/tdlib/docs/td__json__client_8h.html))
3. `TdJson.execute` (td_execute from [td_json_client.h](https://core.telegram.org/tdlib/docs/td__json__client_8h.html))
4. `TdJson.set_verbosity_level` (custom function for easier adjustment of verbosity level)
5. `TdJson.set_max_verbosity_level` (custom function for set maximum verbosity level before Tdlib starts)
6. `TdJson.set_log_callback` (create custom callback for tracking changes in TdLib)

## Usage

This wrapper can be used to create games or applications for integration or create alternative Telegram clients through Telegram Database Api. Look **[example](./example)** for discover.
General pattern of usage:
```gdscript
var client := TdJson.new()
while true:
	var result = client.receive(10.0)
	if result:
		# parse the result as a JSON object and process it as an incoming update or the answer to a previously sent request
		pass
```


## Build (Linux, Windows)

Follow the instructions for complete build:
* Download and install [python 3.9+](https://www.python.org/)
* Install **scons** with pip or other package manager
* Build TdLib packages and its dependencies (check [README.md](./thirdparty/README.md))

After that, open terminal and write this:
```
scons -j XXX
```
Where XXX - CPU cores for build parallelism.

## Build (Android)

The easiest way for build is using Docker:
* Install [Docker](https://www.docker.com/products/docker-desktop/)
* Build TdLib packages and its dependencies (check [README.md](./thirdparty/README.md))
* Build [Dockerfile](./Dockerfile) image
* Run image (check [Dockerfile](./Dockerfile) instruction)

Otherwise, if you want to do it manually, do this (linux):

* Download and install build-essential, python3, scons, unzip, wget, git using your package manager
* Build TdLib packages and its dependencies (check [README.md](./thirdparty/README.md))
* Write this on your terminal:
```
mkdir -p /opt/android/ndk/28.1.13356709
wget -q https://dl.google.com/android/repository/android-ndk-r28-linux.zip -O /tmp/ndk.zip
unzip -q /tmp/ndk.zip -d /tmp
cp -r /tmp/android-ndk-r28/. /opt/android/ndk/28.1.13356709/
rm -rf /tmp/ndk.zip /tmp/android-ndk-r28
scons platform=android ANDROID_HOME=/opt/android
```

## Supported platforms
| Platform | Support |
|-----------|:------------:|
|   Windows     | ✅ supported
|   Linux       | ✅ supported
|   Android     | ✅ supported
|   macOS       | ❌ not supported
|   iOS       | ❌ not supported

