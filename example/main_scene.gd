extends Control

var user := {}
var chats_map := {} 
var is_logged_out := false


func _ready() -> void:
	TdlibSingleton.client.send({
		"@type": "getMe"
	})
	user = await TdlibSingleton.search_for_state("user", 3.0)
	
	%GetMeCommand.pressed.connect(_get_me)
	if not user.is_empty() and user.has("first_name"):
		%Output.text += "Hello, %s! \n" % user["first_name"]
		
	%LogOutCommand.pressed.connect(_logout)
	get_chats_list(10)


func _get_me():
	TdlibSingleton.client.send({
		"@type": "getMe"
	})
	var me_info = await TdlibSingleton.search_for_state("user", 3.0)
	%Output.text += JSON.stringify(me_info, "\t") + "\n"

func get_chats_list(limit: int):
	chats_map.clear()
	
	for child in %ChatList.get_children():
		child.queue_free()

	TdlibSingleton.client.send({
		"@type": "loadChats",
		"limit": limit
	})

	var timer = get_tree().create_timer(2.0)
	while timer.time_left > 0 and chats_map.size() < limit:
		if TdlibSingleton.response.get("@type", "") == "updateNewChat":
			var chat_obj = TdlibSingleton.response.get("chat", {})
			if not chat_obj.is_empty():
				var c_id = chat_obj.get("id")
				if not chats_map.has(c_id):
					chats_map[c_id] = chat_obj
		
		await TdlibSingleton.state_changed

	for c_id in chats_map:
		var chat_data = chats_map[c_id]
		var item = load("res://ui/ChatItem.res").instantiate()
		
		item.chat_id = c_id
		%ChatList.add_child(item)
		
		var title_text = chat_data.get("title", "Chat %d" % c_id)
		if "chat_name" in item and item.chat_name != null:
			item.chat_name.text = title_text
		elif item.has_node("chat_name"):
			item.get_node("chat_name").text = title_text

		item.pressed.connect(_on_item_click.bind(item))

func _on_item_click(sender):
	TdlibSingleton.client.send({
		"@type": "getChat",
		"chat_id": sender.chat_id
	})
	var chat_info = await TdlibSingleton.search_for_state("chat", 3.0)
	%Output.text += JSON.stringify(chat_info, "\t") + "\n"

func _logout():
	%LogOutCommand.disabled = true
	TdlibSingleton.client.send({
		"@type": "logOut"
	})
	
	var timeout_timer = get_tree().create_timer(10.0)
	
	
	var on_logout = func(): 
		self.is_logged_out = true
		
	TdlibSingleton.logout_completed.connect(on_logout, CONNECT_ONE_SHOT)
	
	while not is_logged_out and timeout_timer.time_left > 0:
		await get_tree().process_frame
		
	if is_logged_out:
		get_tree().change_scene_to_file("res://login_scene.tscn")
	else:
		if TdlibSingleton.logout_completed.is_connected(on_logout):
			TdlibSingleton.logout_completed.disconnect(on_logout)
		%LogOutCommand.disabled = false
		%Output.text += "Logout failed or timed out.\n"
