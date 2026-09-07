/*
 * Most of the comments are taken from include\td\telegram\td_json_client.h
 */

#include "tdjson.hpp"

#include <atomic>

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/thread.hpp>

#include <td/telegram/td_json_client.h>


using namespace godot;

/**
 * Sends request to the TDLib client. May be called from any thread.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 */
void TdJson::send(Dictionary p_request)
{
    String _req = JSON::stringify(p_request);
    td_send(_client_id, _req.utf8().get_data());
}

/**
 * Synchronously executes a TDLib request.
 * A request can be executed synchronously, only if it is documented with "Can be called synchronously".
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 * \return JSON-serialized null-terminated request response.
 */
Dictionary TdJson::execute(Dictionary p_request)
{
    String _req = JSON::stringify(p_request);
    const char *_resp = td_execute(_req.utf8().get_data());

    if (!_resp)
    {
        return Dictionary();
    }

    Variant _resp_parsed = JSON::parse_string(String(_resp));
    if (_resp_parsed.get_type() != Variant::DICTIONARY)
    {
        return Dictionary();
    }

    return _resp_parsed;
}

/**
 * Receives incoming updates and request responses. Must not be called simultaneously from two different threads.
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] timeout The maximum number of seconds allowed for this function to wait for new data.
 * \return JSON-serialized null-terminated incoming update or request response. May be NULL if the timeout expires.
 * \attention This function will crash your program without creating thread. So you should use function Thread.start()
 */
Dictionary TdJson::receive(double p_timeout)
{
    _mutex->lock();
    const char *_resp = td_receive(p_timeout);

    if (_resp == nullptr)
    {
        return Dictionary();
    }
    Dictionary _resp_parsed = Dictionary(JSON::parse_string(String(_resp)));
    call_deferred("emit_signal", "request_received", _resp_parsed);
    _mutex->unlock();
    return _resp_parsed;
}

// Logs message output for godot console
void TdJson::_set_log_message_callback()
{
    td_set_log_message_callback(_max_verbosity_level, [](int verbosity_level, const char *message)
                                {
        if (verbosity_level > 0) {
            print_line(String("[TDLib] ") + String(message));
        } else {
            print_error(String("[TDLib] [FATAL!!!] ") + String(message), __FILE__, __LINE__);
        } });
}

Callable *TdJson::_log_callback;

// Sets the callback that will be called when a message is added to the internal TDLib log.
// None of the TDLib methods can be called from the callback. By default the callback is set in set_log_message_callback
void TdJson::set_log_callback(Callable p_callback)
{
    if (_log_callback)
    {
        memdelete(_log_callback);
    }
    _log_callback = memnew(Callable);
    *_log_callback = p_callback;

    td_set_log_message_callback(_max_verbosity_level, [](int verbosity_level, const char *message)
                                {
        if (_log_callback && _log_callback->is_valid()) {
            _log_callback->call_deferred(Variant(verbosity_level), Variant(String(message)));
        } });
}

// Sets the maximum verbosity level for TDLib log messages. Can be called from any thread.
void TdJson::set_max_verbosity_level(int p_verbosity_level)
{
    _max_verbosity_level = p_verbosity_level;
    _set_log_message_callback();
}

// Sets the verbosity level for TDLib log messages. Can be called from any thread.
void TdJson::set_verbosity_level(int p_new_verbosity_level)
{
    Dictionary _dict;
    _dict["@type"] = "setLogVerbosityLevel";
    _dict["new_verbosity_level"] = p_new_verbosity_level;
    String _req = JSON::stringify(_dict);
    td_execute(_req.utf8().get_data());
}

TdJson::TdJson()
{
    _client_id = td_create_client_id();
    _set_log_message_callback();
    _mutex.instantiate();
}

int TdJson::get_client_id()
{
    return _client_id;
}

/**
 * Alias to setTdlibParameters. Sets the parameters for TDLib initialization.
 * \param[in] api_id Application identifier for Telegram API access, which can be obtained at https://my.telegram.org.
 * \param[in] api_hash Application identifier hash for Telegram API access, which can be obtained at https://my.telegram.org.
 * \param[in] application_version Application version; must be non-empty.
 * \param[in] device_model Model of the device the application is being run on; must be non-empty.
 * \param[in] database_directory Path to the directory for the persistent database; by default uses user data directory (`user://tdlib_data/`).
 * \param[in] use_test_dc If true, the Telegram test environment will be used instead of the production environment.
 * \param[in] files_directory Path to the directory for storing files; by default uses database_directory.
 * \param[in] use_file_database If true, information about downloaded and uploaded files will be saved between application restarts.
 * \param[in] use_message_database If true, the local database will be used for storing chats and messages between application restarts.
 * \param[in] use_secret_chats If true, support for secret chats will be enabled.
 * \param[in] system_language_code IETF language tag of the user's operating system language; By default uses locale language of the OS.
 * \param[in] system_version Version of the operating system the application is being run on; by default uses OS version.
 */
void TdJson::set_tdlib_parameters(
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
    Dictionary _req;
    _req["@type"] = "setTdlibParameters";
    _req["api_id"] = p_api_id;
    _req["api_hash"] = p_api_hash;
    _req["application_version"] = p_application_version;
    _req["device_model"] = p_device_model;
    _req["database_directory"] = ProjectSettings::get_singleton()->globalize_path(String(p_database_directory));

    _req["use_test_dc"] = p_use_test_dc;

    if (p_files_directory != String(""))
    {
        _req["files_directory"] = p_files_directory;
    }

    _req["use_file_database"] = p_use_file_database;
    _req["use_message_database"] = p_use_message_database;
    _req["use_secret_chats"] = p_use_secret_chats;

    if (p_system_language_code != String(""))
    {
        _req["system_language_code"] = p_system_language_code;
    }
    else
    {
        _req["system_language_code"] = OS::get_singleton()->get_locale_language();
    }

    if (p_system_version != String(""))
    {
        _req["system_version"] = p_system_version;
    }

    connect("request_received", Callable(this, "_set_tdlib_parameters").bind(_req));
}

// \return Current version of the tdlib
String TdJson::get_tdlib_version()
{
    Dictionary _req;
    _req["@type"] = "getOption";
    _req["name"] = "version";
    return String(execute(_req).get("value", ""));
}

void TdJson::_set_tdlib_parameters(Dictionary p_response, Dictionary p_parameters)
{
    String _type = p_response.get("@type", "");
    if (_type != "updateAuthorizationState")
    {
        return;
    }

    Dictionary _auth_state = p_response.get("authorization_state", Dictionary());
    String _auth_type = _auth_state.get("@type", "");

    if (_auth_type != "authorizationStateWaitTdlibParameters")
    {
        return;
    }

    send(p_parameters);
    disconnect("request_received", Callable(this, "_set_tdlib_parameters"));
}

const double POLL_TIMEOUT = 10.0;

void TdJson::_thread_poll()
{
    bool _closing = false;
    while (_is_running.load() || _closing)
    {
        Dictionary _resp = receive(POLL_TIMEOUT);
        if (_resp.get("@type", "") == "updateAuthorizationState")
        {
            Dictionary _authorization_state = _resp.get("authorization_state", Dictionary());
            String _authorization_type = _authorization_state.get("@type", "");
            if (_authorization_type == "authorizationStateClosing")
            {
                _closing = true;
            }
            else if (_authorization_type == "authorizationStateClosed")
            {
                _closing = false;
            }
        }

        if (!_is_running.load() && !_closing)
        {
            break;
        }
    }
}

void godot::TdJson::_set_bot_token(Dictionary p_response, Dictionary p_parameters)
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

// Starts the TDLib client.
void TdJson::start_poll()
{
    if (_is_running.load()) {
        return;
    }
    if (_worker_thread.is_valid() && _worker_thread->is_started()) {
        return;
    }
    _is_running.store(true);
    Dictionary _req;
    _req["@type"] = "getOption";
    _req["name"] = "version";
    send(_req);

    if (_worker_thread.is_null())
    {
        _worker_thread.instantiate();
    }

    _worker_thread->start(Callable(this, "_thread_poll"));
}

// Stops the TDLib client.
void TdJson::stop_poll()
{
    if (_is_running.load() && !_worker_thread.is_null())
    {
        Dictionary _req;
        _req["@type"] = "close";
        send(_req);
        _is_running.store(false);

        if (_worker_thread->is_alive())
        {
            _worker_thread->wait_to_finish();
        }
    }

    _worker_thread.unref();
}

bool TdJson::is_running()
{
    return _is_running.load();
}

// Sets the bot token for the TDLib client. Can be used instead of user authentication. The bot token can be obtained from @BotFather.
void godot::TdJson::set_bot_token(String bot_token)
{
    Dictionary _req;
    _req["@type"] = "checkAuthenticationBotToken";
    _req["token"] = bot_token;

    connect("request_received", Callable(this, "_set_bot_token").bind(_req));
}

// Bindings for godot
void TdJson::_bind_methods()
{
    // public methods
    ClassDB::bind_method(D_METHOD("send", "request"), &TdJson::send);
    ClassDB::bind_method(D_METHOD("receive", "timeout"), &TdJson::receive);
    ClassDB::bind_method(D_METHOD("execute", "request"), &TdJson::execute);
    ClassDB::bind_method(D_METHOD("set_max_verbosity_level", "verbosity_level"), &TdJson::set_max_verbosity_level);
    ClassDB::bind_method(D_METHOD("get_client_id"), &TdJson::get_client_id);
    ClassDB::bind_method(D_METHOD("set_verbosity_level", "new_verbosity_level"), &TdJson::set_verbosity_level);
    ClassDB::bind_method(D_METHOD("set_log_callback", "callback"), &TdJson::set_log_callback);
    ClassDB::bind_method(D_METHOD("get_tdlib_version"), &TdJson::get_tdlib_version);
    ClassDB::bind_method(D_METHOD("start_poll"), &TdJson::start_poll);
    ClassDB::bind_method(D_METHOD("stop_poll"), &TdJson::stop_poll);
    ClassDB::bind_method(D_METHOD("is_running"), &TdJson::is_running);
    ClassDB::bind_method(D_METHOD("set_bot_token", "bot_token"), &TdJson::set_bot_token);
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
        &TdJson::set_tdlib_parameters,
        DEFVAL(String("user://tdlib_data")),
        DEFVAL(false),
        DEFVAL(String("")),
        DEFVAL(true),
        DEFVAL(true),
        DEFVAL(true),
        DEFVAL(String("")),
        DEFVAL(String("")));

    // private methods
    ClassDB::bind_method(D_METHOD("_set_bot_token", "p_response", "p_parameters"), &TdJson::_set_bot_token);
    ClassDB::bind_method(D_METHOD("_thread_poll"), &TdJson::_thread_poll);
    ClassDB::bind_method(D_METHOD("_set_tdlib_parameters", "p_response", "p_parameters"), &TdJson::_set_tdlib_parameters);
    
    // signals
    ADD_SIGNAL(MethodInfo("request_received", PropertyInfo(Variant::DICTIONARY, "response")));
}

TdJson::~TdJson()
{
    stop_poll();
    td_set_log_message_callback(0, nullptr);
    if (_log_callback)
    {
        memdelete(_log_callback);
        _log_callback = nullptr;
    }
}