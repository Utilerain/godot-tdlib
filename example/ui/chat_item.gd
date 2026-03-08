extends Button

@export var chat_id: int: set = _chat_id_changed
@onready var chat_name := $VBoxContainer/ChatName
@onready var chat_last_message := $VBoxContainer/ChatMessage


func _chat_id_changed(value):
	chat_id = value
	TdlibSingleton.client.send(
		{
			"@type":"getChat",
			"chat_id":value
		}
	)
	await TdlibSingleton.client.request_received
	print(JSON.stringify(TdlibSingleton.response))
	
