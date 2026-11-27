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

- Per-client TDLib parameters: new `TdJsonClient.set_tdlib_parameters()` — the request is sent automatically (at most once) when that client reaches `authorizationStateWaitTdlibParameters`. Clients without explicit parameters fall back to the manager-level defaults; re-calling the method re-arms the auto-send.
- Warning when a client reaches `authorizationStateWaitTdlibParameters` without any TDLib parameters configured (previously authorization stalled silently).
- Warning when two clients are configured with the same `database_directory`/`files_directory` — TDLib holds an exclusive sqlite lock per directory.
- Errors from `setTdlibParameters` are now surfaced via `push_error` (requests are correlated with a `@extra` tag).
- `TdJsonManager.set_log_callback()` — custom callable for TDLib log messages, with an editable max verbosity level.
- `poll_started` and `poll_stopped` signals on `TdJsonManager`.

### Changed

- `TdJsonManager.set_tdlib_parameters()` now sets the *default* parameters applied to clients without explicit ones; per-client parameters are set via `TdJsonClient.set_tdlib_parameters()`.
- `files_directory` is now globalized like `database_directory` (a `user://` path passed there previously stayed unresolved).
- All responses and signals (`request_received`, `response_received`) are now emitted on the main thread via `call_deferred`.
- Reduced polling timeout from 10 s to 1 s.
- Updated TDLib from 1.8.70 to 1.8.71 (submodule, binaries and bundled headers).

### Fixed

- Responses are routed to their own client via TDLib's `@client_id` tag — previously every registered client received every client's updates and responses.
- Connection leak: TDLib parameters are now applied to every newly created client, not just the first one.
- Data race on the client registry — reads snapshot under mutex in `receive()`, and all registry mutations (`create_client`, `remove_client`, broadcasts) are performed under the mutex as well.
- `execute()` and JSON parse failures now report errors via `push_error` instead of failing silently.
- Inverted boolean arguments in `set_tdlib_parameters()` binding.
- `.gdextension` entry files for example/tests.

### CI

- Unified TDLib cache keys across build jobs so they actually match: keys now track `build.ps1`/`build.sh`/`build_android.sh` and the TDLib submodule ref (recorded into `.td-ref.txt`).
- Fixed the Linux cache-hit guard to accept any `libtdjson.so.*` version.
