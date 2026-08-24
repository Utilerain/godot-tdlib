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
	var client = TdJson.new()
	client.set_max_verbosity_level(4)
	client.set_verbosity_level(0)
	check(client != null, "TdJson class is registered")
	if client == null:
		return
	check(client.get_client_id() > 0, "client id is allocated")
	check(client.is_running() == false, "polling is stopped by default")

	var version := client.get_tdlib_version()
	check(not version.is_empty(), "TDLib version is available through execute")

	var option := client.execute({"@type": "getOption", "name": "version"})
	check(option.get("@type", "") == "optionValueString", "execute returns a parsed TDLib object")
	check(not String(option.get("value", "")).is_empty(), "execute preserves the response value")

	client.request_received.connect(on_response)
	client.send({"@type": "getOption", "name": "version", "@extra": "send-test"})
	var response := client.receive(2.0)
	check(response.get("@type", "") == "optionValueString", "send and receive round-trip a request")
	check(response.get("@extra", "") == "send-test", "receive preserves @extra")
	await get_tree().process_frame
	check(received_count == 1, "receive emits request_received once")
	check(last_response.get("@extra", "") == "send-test", "signal carries the received response")

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
	client = null
