# Tests

The test project exercises the built Windows debug extension and does not contact Telegram.

Run from the repository root:

```powershell
.\gen\godot.exe --path .\tests --headless
```

The process exits with code `0` when all checks pass. The suite covers:

- client creation and unique client id;
- `get_tdlib_version` and synchronous `execute` parsing;
- `send`/`receive` delivery of a TDLib event;
- `request_received` signal payload and delivery;
- `start_poll`/`stop_poll`, including repeated calls.

## Integration scenarios

These should run in a separate environment and never use production data:

1. Configure TDLib with a temporary database directory and `use_test_dc=true`.
2. Verify `set_tdlib_parameters` is sent only after `authorizationStateWaitTdlibParameters`.
3. Verify `set_bot_token` responds only to `authorizationStateWaitPhoneNumber`.
4. Authenticate with a disposable bot, wait for `authorizationStateReady`, call `getMe`, then close the client.
5. Kill the process during polling and verify the next run can reopen the same temporary database.
6. After initialization, send `getOption` with `@extra` and verify the matching response preserves `@extra`.

Required CI variables for the bot scenario should be supplied by the CI secret store, for example `TDLIB_TEST_API_ID`, `TDLIB_TEST_API_HASH`, and `TDLIB_TEST_BOT_TOKEN`. Do not commit them.

## Unit-test boundary

The current C++ class calls `td_execute`, `td_send`, and `td_receive` directly, so the runner above is a native-wrapper integration test. To add pure C++ unit tests, introduce a small injectable transport interface around those TDLib functions and test request serialization, response parsing, callback replacement, and poll lifecycle with a fake transport.
