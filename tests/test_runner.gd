extends Node

var failures := 0
var received_count := 0
var last_response: Dictionary

func _ready() -> void:
	await run_tests()
	get_tree().quit(1 if failures > 0 else 0)

func check(condition: bool, message: String) -> void:
	if condition:
		print("PASS: ", message)
	else:
		failures += 1
		push_error("FAIL: " + message)

func on_response(response: Dictionary) -> void:
	received_count += 1
	last_response = response

func run_tests() -> void:
	var extension = load("res://addons/godot-tdlib/telegram.gdextension")
	check(extension != null, "GDExtension resource is available")
	var client = ClassDB.instantiate("TdJson")
	check(client != null, "TdJson class is registered")
	if client == null:
		return
	client.set_max_verbosity_level(4)
	client.set_verbosity_level(0)
	check(client.get_client_id() > 0, "client id is allocated")
	check(client.is_running() == false, "polling is stopped by default")

	var version: String = client.get_tdlib_version()
	check(not version.is_empty(), "TDLib version is available through execute")

	var option: Dictionary = client.execute({"@type": "getOption", "name": "version"})
	check(option.get("@type", "") == "optionValueString", "execute returns a parsed TDLib object")
	check(not String(option.get("value", "")).is_empty(), "execute preserves the response value")

	client.request_received.connect(on_response)
	client.send({"@type": "getOption", "name": "version", "@extra": "send-test"})
	var response: Dictionary = client.receive(2.0)
	check(not response.is_empty(), "send and receive return a TDLib event")
	await get_tree().process_frame
	check(received_count == 1, "receive emits request_received once")
	check(last_response == response, "signal carries the received response")

	client.start_poll()
	await get_tree().process_frame
	check(client.is_running(), "start_poll starts the worker")
	client.start_poll()
	check(client.is_running(), "start_poll is idempotent")
	client.stop_poll()
	check(client.is_running() == false, "stop_poll joins the worker")
	client.stop_poll()
	check(client.is_running() == false, "stop_poll is idempotent")
	client.request_received.disconnect(on_response)
	await get_tree().process_frame
	client = null
	await get_tree().process_frame
