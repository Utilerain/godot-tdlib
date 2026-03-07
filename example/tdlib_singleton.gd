extends Node

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
var USR_PATH = OS.get_user_data_dir()

signal wait_for_phone_number
signal phone_number_received(phone_number: String)
signal wait_for_auth_code
signal auth_code_received(auth_code: String)
signal wait_for_password
signal password_received(password: String)

func _ready() -> void:
	client = TdJson.new()
	client.set_max_verbosity_level(2)
	client.send(reqversion)
	client.request_received.connect(receive_signal)
	thrd.start(_wait_response)

func _wait_response():
	while running:
		client.receive(1.0)

func receive_signal(_response: Dictionary): 
	_handle_update.call_deferred(_response)
	
func _handle_update(data):
	response = data
	update_state()
	
func update_state():
	var event_type = response["@type"]
	
	if event_type == "updateAuthorizationState":
		var auth_state = response["authorization_state"]
		var auth_type = auth_state["@type"]
		
		if auth_type == "authorizationStateClosed":
			return
		
		elif auth_type == "authorizationStateWaitTdlibParameters":
			client.send(
			{
				"@type": "setTdlibParameters",
				"database_directory": USR_PATH+"/tdlib_data",
				"use_message_database": true,
				"use_secret_chats": true,
				"api_id": self.api_id,
				"api_hash": self.api_hash,
				"system_language_code": "en",
				"device_model": "Godot TDLib Client",
				"application_version": "0.0.1",
			}
		)
		
		elif auth_type == "authorizationStateWaitPhoneNumber":
			wait_for_phone_number.emit()
			var phone_number = await phone_number_received
			client.send(
				{
					"@type": "setAuthenticationPhoneNumber",
					"phone_number": phone_number
				}
			)
		
		elif auth_type == "authorizationStateWaitCode":
			wait_for_auth_code.emit()
			var auth_code = await auth_code_received
			client.send(
				{
					"@type": "checkAuthenticationCode",
					"code": auth_code
				}
			)
		
		elif auth_type == "authorizationStateWaitPassword":
			wait_for_password.emit()
			var password = await password_received
			client.send(
				{
					"@type": "checkAuthenticationPassword", "password": password
				}
			)
			

func _exit_tree() -> void:
	running = false
	thrd.wait_to_finish.call_deferred()
