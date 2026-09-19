extends Node

const TDLIB_VERSION = "1.8.67"
var failures := 0
var received_count := 0
var last_response: Dictionary

func _ready() -> void:
	run_tests()
	get_tree().quit(1 if failures > 0 else 0)

func check(condition: bool, message: String) -> void:
	if condition:
		print("PASS: ", message)
	else:
		failures += 1
		push_error("FAIL: " + message)

func on_response(_id, response: Dictionary) -> void:
	received_count += 1
	last_response = response

func on_response_manager(response: Dictionary) -> void:
	pass

func run_tests() -> void:
	var extension = load("res://addons/godot-tdlib/telegram.gdextension")
	check(extension != null, "GDExtension resource is available")
	var TdJsonManager = ClassDB.instantiate("TdJsonManager")
	check(TdJsonManager != null, "TdJsonManager class is registered")
	if TdJsonManager == null:
		return
	TdJsonManager.set_max_verbosity_level(4)
	TdJsonManager.set_verbosity_level(0)

	var client: TdJsonClient = TdJsonManager.create_client()
	client.response_received.connect(on_response)
	check(client.get_client_id() > 0, "client id is allocated")
	check(TdJsonManager.is_running() == false, "polling is stopped by default")

	var version: String = TdJsonManager.get_tdlib_version()
	check(not version.is_empty(), "TDLib version is available through execute")
	check(version == TDLIB_VERSION, "TDLib version is up to date")

	var option: Dictionary = TdJsonManager.execute({"@type": "getOption", "name": "version"})
	check(option.get("@type", "") == "optionValueString", "execute returns a parsed TDLib object")
	check(not String(option.get("value", "")).is_empty(), "execute preserves the response value")

	client.response_received.connect(on_response)
	client.send({"@type": "getOption", "name": "version", "@extra": "send-test"})
	var response: Dictionary = TdJsonManager.receive(2.0)
	check(not response.is_empty(), "send and receive return a TDLib event")
	await get_tree().process_frame
	check(received_count == 1, "receive emits request_received once")
	check(last_response == response, "signal carries the received response")

	TdJsonManager.start_poll()
	await get_tree().process_frame
	check(TdJsonManager.is_running(), "start_poll starts the worker")
	TdJsonManager.start_poll()
	check(TdJsonManager.is_running(), "start_poll is idempotent")
	TdJsonManager.stop_poll()
	check(TdJsonManager.is_running() == false, "stop_poll joins the worker")
	TdJsonManager.stop_poll()
	check(TdJsonManager.is_running() == false, "stop_poll is idempotent")
	TdJsonManager.request_received.disconnect(on_response)
	await get_tree().process_frame
	TdJsonManager = null
	await get_tree().process_frame
	await get_tree().process_frame
