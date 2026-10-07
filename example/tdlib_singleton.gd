extends Node

var client: TdJsonClient
var reqversion := {"@type": "getOption", "name": "version"}
#region Taken from https://github.com/tdlib/td/blob/master/example/python/tdjson_example.py
#	You should obtain your own api_id and api_hash at https://my.telegram.org
var api_hash := "a3406de8d171bb422bb6ddf3bbd800e2"
var api_id := 94575 
#endregion
var bot_token := "abcdefg"

var response: Dictionary
var current_auth_state := {}

var USR_PATH = OS.get_user_data_dir()
signal wait_for_phone_number
signal wait_for_auth_code
signal wait_for_password
signal login_completed
signal state_changed
signal logout_completed

func _ready() -> void:
	TdJsonManager.set_max_verbosity_level(4)
	TdJsonManager.set_verbosity_level(2)
	TdJsonManager.request_received.connect(receive_signal)
	TdJsonManager.set_tdlib_parameters(self.api_id, 
		self.api_hash, 
		"1.0.0",
		"Desktop", "user://tdlib_test_data", true)
	# client.set_bot_token(bot_token)
	client = TdJsonClient.create()
	TdJsonManager.start_poll()
	
func receive_signal(_response: Dictionary): 
	if not _response.has("@type"):
		return
	response = _response
	update_state(response)
	
func update_state(response):
	var event_type = response["@type"]
	state_changed.emit()
	
	if event_type == "updateAuthorizationState":
		var auth_state = response["authorization_state"]
		var auth_type = auth_state["@type"]
		current_auth_state = auth_state

		if auth_type == "authorizationStateClosed":
			reset_client()
			logout_completed.emit()
		
		# Deprecated: use TdJsonManager.set_tdlib_parameters() instead
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
			# before here you can set bot token in line 32
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
	TdJsonManager.stop_poll()

func print_json(data):
	print(JSON.stringify(data, "\t"))

func search_for_state(event_type: String, timeout_sec: float = 3.0) -> Dictionary:
	var timer := get_tree().create_timer(timeout_sec)
	
	while timer.time_left > 0:
		if TdlibSingleton.response.get("@type", "") == event_type:
			return TdlibSingleton.response
		
		await TdlibSingleton.state_changed
	
	return {}

func reset_client() -> void:
	if client:
		# client.close_and_destroy()
		client = null
	
	client = TdJsonClient.create()
