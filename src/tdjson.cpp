#include "tdjson.hpp"
#include <godot_cpp/godot.hpp>
#include <td/telegram/td_json_client.h>

using namespace godot;

void TdJson::send(String request) 
{
    td_json_client_send(client, request.utf8().get_data());
}

String TdJson::execute(String request)
{
    td_json_client_send(client, request.utf8().get_data());
    const char* response = td_json_client_receive(client, 1.0);

    return String(response != nullptr ? response : "");
}

String TdJson::receive(double timeout) 
{
    const char* response = td_json_client_receive(client, timeout);
    if (response != nullptr) {
        emit_signal("request_received", String(response));
        return String(response);
    }
    return String();
}
TdJson::TdJson() 
{
    client = td_json_client_create();
}

void TdJson::destroy() 
{
    td_json_client_destroy(client);
}

void TdJson::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("send", "request"), &TdJson::send);
    ClassDB::bind_method(D_METHOD("receive", "timeout"), &TdJson::receive);
    ClassDB::bind_method(D_METHOD("destroy"), &TdJson::destroy);

    ADD_SIGNAL(MethodInfo("request_received", PropertyInfo(Variant::STRING, "response")));
}
