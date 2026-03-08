extends Control

var chats: Array

func _ready() -> void:
	%Output.text += "Welcome to Console, %s!" % TdlibSingleton.user["first_name"]
	for i in range(100):
		_get_chats()
		await TdlibSingleton.state_changed
		if TdlibSingleton.response["@type"] in ["updateNewChat", "updateSupergroup"]:
			chats.append(TdlibSingleton.response)
	TdlibSingleton.print_json(chats)
	
func _get_chats():
	TdlibSingleton.client.send(
		{
			"@type": "getChats",
			"chat_list": {
			"@type": "chatListMain"
			},
			"limit": 100
		}
	)
