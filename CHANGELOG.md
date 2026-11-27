# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with the tag scheme `<wrapper-version>-<tdlib-version>`.

## [Unreleased]

### Breaking

- **Renamed `TdJsonClient.close_and_destroy()` to `close()`** — update call sites and signal connections.
- Reworked response flow: `TdJsonManager.receive()` is now the single entry point for TDLib updates, and client signals are emitted through it (removed the internal signal choreography).

### Added

- `TdJsonManager.set_log_callback()` — custom callable for TDLib log messages, with an editable max verbosity level.
- `poll_started` and `poll_stopped` signals on `TdJsonManager`.

### Changed

- `set_tdlib_parameters()` no longer sends the request immediately: parameters are stored and sent automatically (once) when a client reaches `authorizationStateWaitTdlibParameters`. Re-calling the method re-arms the auto-send.
- All responses and signals (`request_received`, `response_received`) are now emitted on the main thread via `call_deferred`.
- Reduced polling timeout from 10 s to 1 s.
- Updated TDLib from 1.8.70 to 1.8.71 (submodule, binaries and bundled headers).

### Fixed

- Connection leak: TDLib parameters are now applied to every newly created client, not just the first one.
- Data race on the client registry in `receive()` — snapshot is taken under mutex before dispatching responses.
- `execute()` and JSON parse failures now report errors via `push_error` instead of failing silently.
- Inverted boolean arguments in `set_tdlib_parameters()` binding.
- `.gdextension` entry files for example/tests.

### CI

- Unified TDLib cache keys across build jobs so they actually match: keys now track `build.ps1`/`build.sh`/`build_android.sh` and the TDLib submodule ref (recorded into `.td-ref.txt`).
- Fixed the Linux cache-hit guard to accept any `libtdjson.so.*` version.
