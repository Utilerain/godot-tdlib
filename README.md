# Godot-tdlib
GDExtension C++ library for working with [TdLib](https://github.com/tdlib/td) (JSON interface) on Godot Engine.

### Realized Functions
1. `TdJson.send` (td_send from [td_json_client.h](https://core.telegram.org/tdlib/docs/td__json__client_8h.html))
2. `TdJson.receive` (td_receive from [td_json_client.h](https://core.telegram.org/tdlib/docs/td__json__client_8h.html))
3. `TdJson.execute` (td_execute from [td_json_client.h](https://core.telegram.org/tdlib/docs/td__json__client_8h.html))
4. `TdJson.set_verbosity_level` (custom function for easier adjustment of verbosity level)
5. `TdJson.set_max_verbosity_level` (custom function for set maximum verbosity level before Tdlib starts)
6. `TdJson.set_log_callback`
## Usage
This library can be used to create games or applications for integration or create alternative Telegram clients through Telegram Database Api.

## Build

Follow the instructions for complete build:
* Download and install [python 3.7+](https://www.python.org/)
* Install **scons** with pip (you can install for this repository using **requirements.txt**)
* Build TdLib packages (check [README.md](./thirdparty/README.md))

After that, open terminal and write this:
```
scons -j XXX
```
Where XXX - CPU cores for build parallelism.

## Supported platforms
| Platform | Support |
|-----------|:------------:|
|   Windows     | ✅ supported
|   Linux       | 📝 planned
|   Android     | 📝 planned
|   macOS       | ❌ not supported
|   iOS       | ❌ not supported

