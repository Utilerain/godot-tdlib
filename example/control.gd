extends Control

var client: TdJson

var reqversion := {"@type": "getOption", "name": "version"}
var thrd = Thread.new()


func _init() -> void:
	client = TdJson.new()
	client.request_received.connect(receive_signal)
	client.send(JSON.stringify(reqversion))


func receive_signal(response):
	print(response)

func _await_response():
	client.receive(10.0)
