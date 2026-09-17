#include "tdjson_client.hpp"

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/godot.hpp>

#include <td/telegram/td_json_client.h>

using namespace godot;

TdJsonClient::TdJsonClient(int p_client_id)
{
    _client_id = p_client_id;
}

/**
 * Sends request to the TDLib client. May be called from any thread.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 */
void TdJsonClient::send(Dictionary p_request)
{
    String _req = JSON::stringify(p_request);
    td_send(_client_id, _req.utf8().get_data());
}


int TdJsonClient::get_client_id()
{
    return _client_id;
}

void TdJsonClient::_on_response(Dictionary p_response)
{
    int _id = p_response.get("client_id", 0);
    if (_client_id == 0 || _id != _client_id)
    {
        return;
    }
    emit_signal("response_received", _client_id, p_response);
}

void TdJsonClient::_set_bot_token(Dictionary p_response, Dictionary p_parameters)
{
    String _type = p_response.get("@type", "");
    if (_type != "updateAuthorizationState")
    {
        return;
    }

    Dictionary _auth_state = p_response.get("authorization_state", Dictionary());
    String _auth_type = _auth_state.get("@type", "");

    if (_auth_type != "authorizationStateWaitPhoneNumber")
    {
        return;
    }

    send(p_parameters);
    disconnect("request_received", Callable(this, "_set_bot_token"));
}

// Sets the bot token for the TDLib client. Can be used instead of user authentication. The bot token can be obtained from @BotFather.
void TdJsonClient::set_bot_token(String bot_token)
{
    Dictionary _req;
    _req["@type"] = "checkAuthenticationBotToken";
    _req["token"] = bot_token;

    connect("request_received", Callable(this, "_set_bot_token").bind(_req));
}

void TdJsonClient::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("send", "request"), &TdJsonClient::send);
    ClassDB::bind_method(D_METHOD("get_client_id"), &TdJsonClient::get_client_id);
    ClassDB::bind_method(D_METHOD("set_bot_token", "bot_token"), &TdJsonClient::set_bot_token);
    ClassDB::bind_method(D_METHOD("_set_bot_token", "p_response", "p_parameters"), &TdJsonClient::_set_bot_token);

    ADD_SIGNAL(MethodInfo("response_received", PropertyInfo(Variant::INT, "client_id"), PropertyInfo(Variant::DICTIONARY, "response")));
}