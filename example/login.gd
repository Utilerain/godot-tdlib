extends Control

@onready var phone_btn := $LoginContainer/PhoneContainer/Phone
@onready var code_btn := $LoginContainer/CodeContainer/Code
@onready var pass_btn := $LoginContainer/PassContainer/Password
@onready var _phone := $LoginContainer/PhoneContainer/PhoneEdit
@onready var _code := $LoginContainer/CodeContainer/CodeEdit
@onready var _passw := $LoginContainer/PassContainer/PasswordEdit
@onready var login_label := $LoginContainer/Label

func _ready() -> void:
	TdlibSingleton.wait_for_phone_number.connect(_phone_number_wait)
	TdlibSingleton.wait_for_password.connect(_pass_wait)
	TdlibSingleton.wait_for_auth_code.connect(_code_wait)
	TdlibSingleton.login_completed.connect(_login_completed)
	
	phone_btn.pressed.connect(_phone_number_receive)
	code_btn.pressed.connect(_code_receive)
	pass_btn.pressed.connect(_pass_receive)

func _login_completed():
	login_label.show()
	login_label.text = "Login completed!"
	get_tree().change_scene_to_file("res://main_scene.tscn")
	

func _phone_number_receive():
	TdlibSingleton.phone_number_received.emit(_phone.text)

func _code_receive():
	TdlibSingleton.auth_code_received.emit(_code.text)

func _pass_receive():
	TdlibSingleton.password_received.emit(_passw.text)

func _phone_number_wait():
	$LoginContainer/PhoneContainer.show()

func _code_wait():
	$LoginContainer/CodeContainer.show()
	
func _pass_wait():
	$LoginContainer/PassContainer.show()
