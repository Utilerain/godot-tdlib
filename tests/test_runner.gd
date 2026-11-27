extends Node

const TDLIB_VERSION = "1.8.71"

var failures := 0
var received_count := 0
var last_response: Dictionary = {}

func _ready() -> void:
	await run_tests()
	get_tree().quit(1 if failures > 0 else 0)

func check(condition: bool, message: String) -> void:
	if condition:
		print("PASS: ", message)
	else:
		failures += 1
		printerr("FAIL: " + message)

func on_response(arg1, arg2 = null) -> void:
	var response: Dictionary = arg1 if arg1 is Dictionary else (arg2 if arg2 is Dictionary else {})
	if not response.is_empty():
		received_count += 1
		last_response = response

func run_tests() -> void:
	var extension = load("res://addons/godot-tdlib/telegram.gdextension")
	check(extension != null, "GDExtension resource is available")
	
	var manager = ClassDB.instantiate("TdJsonManager") if ClassDB.can_instantiate("TdJsonManager") else Engine.get_singleton("TdJsonManager")

	check(manager != null, "TdJsonManager class/singleton is registered")
	if manager == null:
		return

	manager.set_max_verbosity_level(4)
	manager.set_verbosity_level(1)

	check(manager.is_running() == false, "polling is stopped by default")

	var version: String = manager.get_tdlib_version()
	check(not version.is_empty(), "TDLib version is available through execute")
	check(version == TDLIB_VERSION, "TDLib version is up to date")

	var option: Dictionary = manager.execute({"@type": "getOption", "name": "version"})
	check(option.get("@type", "") == "optionValueString", "execute returns a parsed TDLib object")
	check(not String(option.get("value", "")).is_empty(), "execute preserves the response value")

	var client = manager.create_client()
	check(client != null and client.get_client_id() > 0, "client id is allocated")

	if not client.response_received.is_connected(on_response):
		client.response_received.connect(on_response)

	received_count = 0
	last_response.clear()

	client.send({"@type": "getOption", "name": "version", "@extra": "send-test"})
	var timeout := 2.0
	var target_response: Dictionary = await wait_for_response(timeout, manager)

	check(not target_response.is_empty(), "send and receive return a TDLib event")
	await get_tree().process_frame

	check(received_count >= 1, "receive emits request_received at least once")
	check(last_response.get("@extra", "") == "send-test", "signal carries the received response")

	# Routing isolation: responses are delivered only to their own client
	var client_b = manager.create_client()
	check(client_b != null and client_b.get_client_id() > 0, "second client id is allocated")
	var counts := {"a": 0, "b": 0}
	var on_a := func(_client_id, _response = null): counts["a"] += 1
	var on_b := func(_client_id, _response = null): counts["b"] += 1
	client.response_received.connect(on_a)
	client_b.response_received.connect(on_b)

	client_b.send({"@type": "getOption", "name": "version", "@extra": "route-b"})
	var waited := 0.0
	while waited < 2.0 and counts["b"] == 0:
		manager.receive(0.1)
		await get_tree().process_frame
		waited += 0.1

	check(counts["b"] >= 1, "client B receives its own response")
	check(counts["a"] == 0, "client A does not receive client B responses")

	client.response_received.disconnect(on_a)
	client_b.response_received.disconnect(on_b)
	client_b.close()

	manager.start_poll()
	await get_tree().process_frame
	check(manager.is_running(), "start_poll starts the worker")

	manager.start_poll()
	check(manager.is_running(), "start_poll is idempotent")

	manager.stop_poll()
	check(manager.is_running() == false, "stop_poll joins the worker")

	manager.stop_poll()
	check(manager.is_running() == false, "stop_poll is idempotent")
	
	if client.response_received.is_connected(on_response):
		client.response_received.disconnect(on_response)

	client.close()
	await get_tree().process_frame

	# Free the manager created via ClassDB.instantiate to avoid an ObjectDB leak at exit
	if ClassDB.can_instantiate("TdJsonManager"):
		manager.free()

func wait_for_response(timeout, manager):
	while timeout > 0.0:
		var resp: Dictionary = manager.receive(0.1)
		if resp.get("@extra", "") == "send-test":
			return resp.duplicate()
		timeout -= 0.1
		await get_tree().process_frame
