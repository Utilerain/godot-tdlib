extends Control

var client: TdJson

var reqversion := {"@type": "getOption", "name": "version"}
var thrd = Thread.new()
#region Taken from https://github.com/tdlib/td/blob/master/example/python/tdjson_example.py
#	You should obtain your own api_id and api_hash at https://my.telegram.org
var api_hash := "a3406de8d171bb422bb6ddf3bbd800e2"
var api_id := 94575 
var response: Dictionary
var running = true
#endregion

var USR_PATH = ProjectSettings.globalize_path("usr://")

func _init() -> void:
	client = TdJson.new()
	client.request_received.connect(receive_signal)
	thrd.start(td_background_work)
	

func td_background_work():
	while running:
		client.receive(0.1)

func receive_signal(_response: String):
	self.response = JSON.parse_string(_response)
	print(_response)
	update_state()

func update_state():
	if response["@type"] == "updateAuthorizationState" and response.has("authorization_state"):
		if response["authorization_state"]["@type"] == "authorizationStateWaitTdlibParameters":
			client.send(JSON.stringify(
			{
				"@type": "setTdlibParameters",
				"database_directory": "tdlib_data",
				"use_message_database": true,
				"use_secret_chats": true,
				"api_id": self.api_id,
				"api_hash": self.api_hash,
				"system_language_code": "en",
				"device_model": "Godot TDLib Client",
				"application_version": "0.0.1",
			}
		))
	
func _exit_tree() -> void:
	running = false
	thrd.wait_to_finish()
