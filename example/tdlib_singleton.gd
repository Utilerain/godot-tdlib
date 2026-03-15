extends Node

var client: TdJson
var reqversion := {"@type": "getOption", "name": "version"}
var thrd = Thread.new()
#region Taken from https://github.com/tdlib/td/blob/master/example/python/tdjson_example.py
#	You should obtain your own api_id and api_hash at https://my.telegram.org
var api_hash := "a3406de8d171bb422bb6ddf3bbd800e2"
var api_id := 94575 
#endregion

var response: Dictionary
var running = true

var USR_PATH = OS.get_user_data_dir()
signal wait_for_phone_number
signal phone_number_received(phone_number: String)
signal wait_for_auth_code
signal auth_code_received(auth_code: String)
signal wait_for_password
signal password_received(password: String)
signal login_completed
signal state_changed


func _ready() -> void:
	client = TdJson.new()
	client.set_max_verbosity_level(4)
	client.set_verbosity_level(2)
	client.send(reqversion)
	client.request_received.connect(receive_signal)
	phone_number_received.connect(_phone_number_received)
	auth_code_received.connect(_code_received)
	password_received.connect(_pass_received)
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
	state_changed.emit()
	
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
				"system_language_code": "ru",
				"device_model": "Desktop",
				"application_version": "1.0",
			}
		)
		
		elif auth_type == "authorizationStateWaitPhoneNumber":
			wait_for_phone_number.emit()
			await phone_number_received
		
		elif auth_type == "authorizationStateWaitCode":
			wait_for_auth_code.emit()
			await auth_code_received
		
		elif auth_type == "authorizationStateWaitPassword":
			wait_for_password.emit()
			await password_received
		
		elif auth_type == "authorizationStateReady":
			login_completed.emit()

func _phone_number_received(phone):
	client.send(
		{
			"@type": "setAuthenticationPhoneNumber",
			"phone_number": phone
		}
	)

func _code_received(code):
	client.send(
		{
			"@type": "checkAuthenticationCode",
			"code": code
		}
	)

func _pass_received(password):
	client.send(
		{
			"@type": "checkAuthenticationPassword", 
			"password": password
		}
	)

func _exit_tree() -> void:
	running = false
	thrd.wait_to_finish.call_deferred()

func print_json(data):
	print(JSON.stringify(data, "\t"))

func search_for_state(event_type: String, attempts: int = 10) -> Dictionary:
	while TdlibSingleton.response["@type"] != event_type and attempts > 0:
		await TdlibSingleton.state_changed
		attempts -= 1
		
	if attempts == 0:
		return {}
	return TdlibSingleton.response
