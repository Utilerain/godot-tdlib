extends Node

var client: TdJson
var reqversion := {"@type": "getOption", "name": "version"}
#region Taken from https://github.com/tdlib/td/blob/master/example/python/tdjson_example.py
#	You should obtain your own api_id and api_hash at https://my.telegram.org
var api_hash := "a3406de8d171bb422bb6ddf3bbd800e2"
var api_id := 94575 
#endregion

var response: Dictionary

var USR_PATH = OS.get_user_data_dir()
signal wait_for_phone_number
signal wait_for_auth_code
signal wait_for_password
signal login_completed
signal state_changed


func _ready() -> void:
	client = TdJson.new()
	client.set_max_verbosity_level(4)
	client.set_verbosity_level(2)
	client.request_received.connect(receive_signal)
	client.set_tdlib_parameters(self.api_id, 
		self.api_hash, 
		"1.0.0",
		"Desktop")
	client.start_poll()
		

func receive_signal(_response: Dictionary): 
	response = _response
	update_state.call_deferred()
	
func update_state():
	if not response.has("@type"):
		return
	var event_type = response["@type"]
	state_changed.emit()
	
	if event_type == "updateAuthorizationState":
		var auth_state = response["authorization_state"]
		var auth_type = auth_state["@type"]
		
		if auth_type == "authorizationStateClosed":
			return
		
		# Deprecated: use TdJson.set_tdlib_parameters() instead
		#elif auth_type == "authorizationStateWaitTdlibParameters": 
			#client.send(
			#{
				#"@type": "setTdlibParameters",
				#"database_directory": USR_PATH+"/tdlib_data",
				#"use_message_database": true,
				#"use_secret_chats": true,
				#"api_id": self.api_id,
				#"api_hash": self.api_hash,
				#"system_language_code": OS.get_locale_language(),
				#"device_model": "Desktop",
				#"application_version": "1.0",
			#}
			#)
		
		elif auth_type == "authorizationStateWaitPhoneNumber":
			wait_for_phone_number.emit()
		
		elif auth_type == "authorizationStateWaitCode":
			wait_for_auth_code.emit()
		
		elif auth_type == "authorizationStateWaitPassword":
			wait_for_password.emit()
		
		elif auth_type == "authorizationStateReady":
			login_completed.emit()

func send_phone_number(phone):
	client.send(
		{
			"@type": "setAuthenticationPhoneNumber",
			"phone_number": phone
		}
	)

func send_code(code):
	client.send(
		{
			"@type": "checkAuthenticationCode",
			"code": code
		}
	)

func send_password(password):
	client.send(
		{
			"@type": "checkAuthenticationPassword", 
			"password": password
		}
	)

func _exit_tree() -> void:
	client.stop_poll()
	print(client.is_running())

func print_json(data):
	print(JSON.stringify(data, "\t"))

func search_for_state(event_type: String, attempts: int = 10) -> Dictionary:
	while TdlibSingleton.response["@type"] != event_type and attempts > 0:
		await TdlibSingleton.state_changed
		attempts -= 1
		
	if attempts == 0:
		return {}
	return TdlibSingleton.response
