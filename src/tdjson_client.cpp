#include "tdjson_client.hpp"
#include "tdjson_manager.hpp"

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/godot.hpp>

#include <td/telegram/td_json_client.h>

using namespace godot;

/**
 * Sends request to the TDLib client. May be called from any thread.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 */
void TdJsonClient::send(Dictionary p_request)
{
    godot::UtilityFunctions::print_verbose("TdJsonClient::send: ", p_request);
    String _req = JSON::stringify(p_request);
    td_send(_client_id, _req.utf8().get_data());
}

int TdJsonClient::get_client_id()
{
    return _client_id;
}

void TdJsonClient::_on_response(Dictionary p_response)
{
    godot::UtilityFunctions::print_verbose("TdJsonClient::_on_response: ", p_response);
    int _id = p_response.get("@client_id", 0);
    if (_client_id == 0 || _id != _client_id)
    {
        return;
    }
    call_deferred("emit_signal", "response_received", _client_id, p_response);
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
    disconnect("response_received", Callable(this, "_set_bot_token"));
}

// Sets the bot token for the TDLib client. Can be used instead of user authentication. The bot token can be obtained from @BotFather.
void TdJsonClient::set_bot_token(String p_bot_token)
{
    Dictionary _req;
    _req["@type"] = "checkAuthenticationBotToken";
    _req["token"] = p_bot_token;

    connect("response_received", Callable(this, "_set_bot_token").bind(_req));
}

// Sets the TDLib parameters for this client. The request is stored and sent automatically (once)
// when this client reaches authorizationStateWaitTdlibParameters. Without explicit parameters the
// client falls back to the default parameters set via TdJsonManager::set_tdlib_parameters.
void TdJsonClient::set_tdlib_parameters(
    int p_api_id,
    String p_api_hash,
    String p_application_version,
    String p_device_model,
    String p_database_directory,
    bool p_use_test_dc,
    String p_files_directory,
    bool p_use_file_database,
    bool p_use_message_database,
    bool p_use_secret_chats,
    String p_system_language_code,
    String p_system_version)
{
    TdJsonManager *_manager = TdJsonManager::get_singleton();
    if (!_manager)
    {
        godot::UtilityFunctions::push_error("TdJsonClient::set_tdlib_parameters: TdJsonManager is not available");
        return;
    }
    Dictionary _req = TdJsonManager::_build_tdlib_params_request(p_api_id, p_api_hash, p_application_version, p_device_model, p_database_directory, p_use_test_dc, p_files_directory, p_use_file_database, p_use_message_database, p_use_secret_chats, p_system_language_code, p_system_version);
    _manager->_set_client_params(_client_id, _req);
}

void TdJsonClient::close()
{
    godot::UtilityFunctions::print_verbose("TdJsonClient::close: ", _client_id);
    TdJsonManager::get_singleton()->remove_client(this);
}

Ref<TdJsonClient> TdJsonClient::create()
{
    godot::UtilityFunctions::print_verbose("TdJsonClient::create");
    return TdJsonManager::get_singleton()->create_client();
}

void TdJsonClient::_set_client_id(int p_client_id)
{
    _client_id = p_client_id;
}

void TdJsonClient::_bind_methods()
{
    // public methods
    ClassDB::bind_method(D_METHOD("send", "request"), &TdJsonClient::send);
    ClassDB::bind_method(D_METHOD("get_client_id"), &TdJsonClient::get_client_id);
    ClassDB::bind_method(D_METHOD("set_bot_token", "bot_token"), &TdJsonClient::set_bot_token);
    ClassDB::bind_method(
        D_METHOD("set_tdlib_parameters",
                 "api_id",
                 "api_hash",
                 "application_version",
                 "device_model",
                 "database_directory",
                 "use_test_dc",
                 "files_directory",
                 "use_file_database",
                 "use_message_database",
                 "use_secret_chats",
                 "system_language_code",
                 "system_version"),
        &TdJsonClient::set_tdlib_parameters,
        DEFVAL(String("user://tdlib_data")),
        DEFVAL(false),
        DEFVAL(String("")),
        DEFVAL(true),
        DEFVAL(true),
        DEFVAL(true),
        DEFVAL(String("")),
        DEFVAL(String("")));
    ClassDB::bind_method(D_METHOD("close"), &TdJsonClient::close);

    // private methods
    ClassDB::bind_method(D_METHOD("_set_bot_token", "p_response", "p_parameters"), &TdJsonClient::_set_bot_token);

    // static methods
    ClassDB::bind_static_method("TdJsonClient", D_METHOD("create"), &TdJsonClient::create);

    // signals
    ADD_SIGNAL(MethodInfo("response_received", PropertyInfo(Variant::INT, "client_id"), PropertyInfo(Variant::DICTIONARY, "response")));
}