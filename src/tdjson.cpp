/*
 * Most of the comments are taken from include\td\telegram\td_json_client.h
 */

#include "tdjson.hpp"

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <td/telegram/td_json_client.h>

using namespace godot;

/**
 * Sends request to the TDLib client. May be called from any thread.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 */
void TdJson::send(Dictionary request)
{
    String _req = JSON::stringify(request);
    td_send(client_id, _req.utf8().get_data());
}

/**
 * Synchronously executes a TDLib request.
 * A request can be executed synchronously, only if it is documented with "Can be called synchronously".
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 * \return JSON-serialized null-terminated request response.
 */
Dictionary TdJson::execute(Dictionary request)
{
    String _req = JSON::stringify(request);
    const char *response = td_execute(_req.utf8().get_data());

    if (!response) {
        return Dictionary();
    }

    Variant parsed_response = JSON::parse_string(String(response));
    if (parsed_response.get_type() != Variant::DICTIONARY) {
        return Dictionary();
    }

    return parsed_response;
}

/**
 * Receives incoming updates and request responses. Must not be called simultaneously from two different threads.
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] timeout The maximum number of seconds allowed for this function to wait for new data.
 * \return JSON-serialized null-terminated incoming update or request response. May be NULL if the timeout expires.
 * \attention This function will crash your program without creating thread. So you should use function Thread.start()
 */
Dictionary TdJson::receive(double timeout)
{
    const char *response = td_receive(timeout);

    if (response != nullptr) {
        emit_signal("request_received", Dictionary(JSON::parse_string(String(response))));
        return Dictionary(JSON::parse_string(String(response)));
    }

    return Dictionary();
}

// Logs message output for godot console
void TdJson::_set_log_message_callback()
{
    td_set_log_message_callback(max_verbosity_level, [](int verbosity_level, const char *message) {
        if (verbosity_level > 0) {
            print_line(String("[TDLib] ") + String(message));
        } else {
            print_error(String("[TDLib] [FATAL!!!] ") + String(message), __FILE__, __LINE__);
        }
    });
}

Callable *TdJson::log_callback;

// Sets the callback that will be called when a message is added to the internal TDLib log.
// None of the TDLib methods can be called from the callback. By default the callback is set in set_log_message_callback
void TdJson::set_log_callback(Callable p_callback)
{
    if (log_callback) {
        memdelete(log_callback);
    }
    log_callback = memnew(Callable);
    *log_callback = p_callback;

    td_set_log_message_callback(max_verbosity_level, [](int verbosity_level, const char *message) {
        if (log_callback && log_callback->is_valid()) {
            log_callback->call_deferred(Variant(verbosity_level), Variant(String(message)));
        }
    });
}

// Sets the maximum verbosity level for TDLib log messages. Can be called from any thread.
void TdJson::set_max_verbosity_level(int verbosity_level)
{
    max_verbosity_level = verbosity_level;
    _set_log_message_callback();
}

// Sets the verbosity level for TDLib log messages. Can be called from any thread.
void TdJson::set_verbosity_level(int new_verbosity_level)
{
    Dictionary _dict;
    _dict["@type"] = "setLogVerbosityLevel";
    _dict["new_verbosity_level"] = new_verbosity_level;
    String _req = JSON::stringify(_dict);
    td_execute(_req.utf8().get_data());
}

TdJson::TdJson()
{
    client_id = td_create_client_id();
    _set_log_message_callback();
}

int TdJson::get_client_id()
{
    return client_id;
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
 */
void godot::TdJson::set_tdlib_parameters(
    int api_id,
    String api_hash,
    String application_version,
    String device_model,
    String database_directory,
    bool use_test_dc,
    String files_directory,
    bool use_file_database,
    bool use_message_database,
    bool use_secret_chats,
    String system_language_code,
    String system_version)
{
    Dictionary _req;
    _req["@type"] = "setTdlibParameters";
    _req["api_id"] = api_id;
    _req["api_hash"] = api_hash;
    _req["application_version"] = application_version;
    _req["device_model"] = device_model;
    _req["database_directory"] = ProjectSettings::get_singleton()->globalize_path(String(database_directory));

    _req["use_test_dc"] = use_test_dc;

    if (files_directory != String("")) {
        _req["files_directory"] = files_directory;
    }

    _req["use_file_database"] = use_file_database;
    _req["use_message_database"] = use_message_database;
    _req["use_secret_chats"] = use_secret_chats;

    if (system_language_code != String("")) {
        _req["system_language_code"] = system_language_code;
    } else {
        _req["system_language_code"] = OS::get_singleton()->get_locale_language();
    }

    if (system_version != String("")) {
        _req["system_version"] = system_version;
    }

    connect("request_received", Callable(this, "_send_tdlib_parameters").bind(_req));
}

// \return Current version of the tdlib
String godot::TdJson::get_tdlib_version()
{
    Dictionary _req;
    _req["@type"] = "getOption";
    _req["name"] = "version";
    return String(execute(_req).get("value", ""));
}

void TdJson::_send_tdlib_parameters(Dictionary p_response, Dictionary p_parameters)
{
    String type = p_response.get("@type", "");
    if (type != "updateAuthorizationState") {
        return;
    }

    Dictionary auth_state = p_response.get("authorization_state", Dictionary());
    String auth_type = auth_state.get("@type", "");

    if (auth_type != "authorizationStateWaitTdlibParameters") {
        return;
    }

    send(p_parameters);
    disconnect("request_received", Callable(this, "_send_tdlib_parameters"));
}

// Starts the TDLib client. Unnecessary if you've used `send()` before
void TdJson::run()
{
    Dictionary _req;
    _req["@type"] = "getOption";
    _req["name"] = "version";
    send(_req);
}

// Bindings for godot
void TdJson::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("send", "request"), &TdJson::send);
    ClassDB::bind_method(D_METHOD("receive", "timeout"), &TdJson::receive);
    ClassDB::bind_method(D_METHOD("execute", "request"), &TdJson::execute);
    ClassDB::bind_method(D_METHOD("set_max_verbosity_level", "verbosity_level"), &TdJson::set_max_verbosity_level);
    ClassDB::bind_method(D_METHOD("get_client_id"), &TdJson::get_client_id);
    ClassDB::bind_method(D_METHOD("set_verbosity_level", "new_verbosity_level"), &TdJson::set_verbosity_level);
    ClassDB::bind_method(D_METHOD("set_log_callback", "callback"), &TdJson::set_log_callback);
    ClassDB::bind_method(D_METHOD("get_tdlib_version"), &TdJson::get_tdlib_version);
    ClassDB::bind_method(D_METHOD("run"), &TdJson::run);

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

    ClassDB::bind_method(D_METHOD("_send_tdlib_parameters", "p_response", "p_parameters"), &TdJson::_send_tdlib_parameters);

    ADD_SIGNAL(MethodInfo("request_received", PropertyInfo(Variant::DICTIONARY, "response")));
}

TdJson::~TdJson()
{
    if (log_callback) {
        memdelete(log_callback);
        log_callback = nullptr;
    }
}