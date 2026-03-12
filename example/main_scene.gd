extends Control

var user := {}
var chats := []

func _ready() -> void:
	TdlibSingleton.client.send(
		{
			"@type": "getMe"
		}
	)
	user = await TdlibSingleton.search_for_state("user", 100)
	%GetMeCommand.pressed.connect(_get_me)
	%Output.text += "Hello, %s! \n" % user["first_name"]
	%LogOutCommand.pressed.connect(_logout)
	get_chats_list(10)
	
func _get_me():
	TdlibSingleton.client.send(
		{
			"@type": "getMe"
		}
	)
	%Output.text += JSON.stringify(await TdlibSingleton.search_for_state("user", 100), "\t")

func get_chats_list(limit):
	TdlibSingleton.client.send(
		{
			"@type": "loadChats",
			"limit": limit
		}
	)
	for i in range(limit):
		await TdlibSingleton.state_changed
		chats.append(await TdlibSingleton.search_for_state("updateNewChat", 50))
	
	for chat in chats:
		
		var item = load("res://ui/ChatItem.res").instantiate()
		item.chat_id = chat["chat"]["id"]
		%ChatList.add_child(item)
		item.chat_name.text = chat["chat"]["title"]
		item.pressed.connect(_on_item_click.bind(item))

func _on_item_click(sender):
	TdlibSingleton.client.send(
		{
			"@type": "getChat",
			"chat_id": sender.chat_id
		}
	)
	await TdlibSingleton.state_changed
	%Output.text += JSON.stringify(await TdlibSingleton.search_for_state("chat", 100), "\t")

func _logout():
	TdlibSingleton.client.send(
		{
			"@type": "logOut"
		}
	)
	await TdlibSingleton.state_changed
	var resp = await TdlibSingleton.search_for_state("ok")
	if not resp.is_empty():
		get_tree().change_scene_to_file("res://login_scene.tscn")
