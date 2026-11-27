/*
 * Most of the comments are taken from include\td\telegram\td_json_client.h
 */

#include "tdjson_manager.hpp"
#include "tdjson_client.hpp"

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/core/mutex_lock.hpp>

#include <td/telegram/td_json_client.h>

#include <atomic>
#include <unordered_map>

using namespace godot;

/**
 * Synchronously executes a TDLib request.
 * A request can be executed synchronously, only if it is documented with "Can be called synchronously".
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 * \return JSON-serialized null-terminated request response.
 */
Dictionary TdJsonManager::execute(Dictionary p_request)
{
    godot::UtilityFunctions::print_verbose("TdJsonManager::execute: ", p_request);
    String _req = JSON::stringify(p_request);
    const char *_resp = td_execute(_req.utf8().get_data());

    if (!_resp)
    {
        UtilityFunctions::push_error("TdJsonManager::execute: request rejected by TDLib: ", p_request);
        return _make_wrapper_error(400, "Request rejected by TDLib: the request is not executable synchronously or is malformed.");
    }

    Variant _resp_parsed = JSON::parse_string(String(_resp));
    if (_resp_parsed.get_type() != Variant::DICTIONARY)
    {
        UtilityFunctions::push_error("TdJsonManager::execute: failed to parse response for request: ", p_request);
        return _make_wrapper_error(500, "Failed to parse the TDLib response.");
    }

    return _resp_parsed;
}

// Builds an error object in the wrapper's own format, distinct from TDLib's own "error" responses:
// {"@type": "godot-tdlib-error", "code": ..., "message": ...}
Dictionary TdJsonManager::_make_wrapper_error(int p_code, const String &p_message)
{
    Dictionary _error;
    _error["@type"] = "godot-tdlib-error";
    _error["code"] = p_code;
    _error["message"] = p_message;
    return _error;
}

/**
 * Receives incoming updates and request responses. Must not be called simultaneously from two different threads.
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] timeout The maximum number of seconds allowed for this function to wait for new data.
 * \return JSON-serialized null-terminated incoming update or request response. May be NULL if the timeout expires.
 * \attention This function will crash your program without creating thread. So you should use function Thread.start()
 */
Dictionary TdJsonManager::receive(double p_timeout)
{

    if (is_running() && _worker_thread_id != godot::OS::get_singleton()->get_thread_caller_id())
    {
        godot::UtilityFunctions::push_error("TdJsonManager::receive() should not be called from multiple threads simultaneously. Did you use TdJsonManager::start_poll() and TdJsonManager::receive() at the same time?");
        return Dictionary();
    }
 
    const char *_resp = nullptr;
    {
        MutexLock lock(*_mutex.ptr());
        _resp = td_receive(p_timeout);
        if (!_resp) return Dictionary();
    }

    Dictionary _parsed = Dictionary(JSON::parse_string(String(_resp)));
    godot::UtilityFunctions::print_verbose("TdJsonManager::receive: ", _parsed);

    int _resp_client_id = (int)_parsed.get("@client_id", 0);

    if ((String)_parsed.get("@type", "") == "error" && String(_parsed.get("@extra", "")).begins_with("set_params_"))
    {
        UtilityFunctions::push_error("TdJsonManager: setTdlibParameters failed for client ", _resp_client_id, ": ", (String)_parsed.get("message", ""));
    }

    Ref<TdJsonClient> _target = nullptr;
    {
        MutexLock lock(*_mutex.ptr());
        auto it = _clients.find(_resp_client_id);
        if (it != _clients.end()) 
        {
            _target = it->second;
        }
    }

    if (_target.is_valid())
    {
        _target->_on_response(_parsed);
    }
    else
    {
        UtilityFunctions::print_verbose("TdJsonManager::receive: No client found for response with @client_id: ", _resp_client_id);
    }

    _try_send_tdlib_parameters(_parsed, _target);
    call_deferred("emit_signal", "request_received", _parsed);

    return _parsed;
}

// Logs message output for godot console
void TdJsonManager::_set_log_message_callback()
{
    td_set_log_message_callback(_max_verbosity_level, [](int verbosity_level, const char *message)
                                {
        if (verbosity_level > 0) {
            print_line(String("[TDLib] ") + String(message));
        } else {
            print_error(String("[TDLib] [FATAL!!!] ") + String(message));
        } });
}

Callable *TdJsonManager::_log_callback;

// Sets the callback that will be called when a message is added to the internal TDLib log.
// None of the TDLib methods can be called from the callback. By default the callback is set in set_log_message_callback
void TdJsonManager::set_log_callback(Callable p_callback, int p_max_verbosity_level = 4)
{
    if (_log_callback)
    {
        memdelete(_log_callback);
    }
    _log_callback = memnew(Callable);
    *_log_callback = p_callback;

    td_set_log_message_callback(p_max_verbosity_level, [](int verbosity_level, const char *message)
                                {
        if (_log_callback && _log_callback->is_valid()) {
            _log_callback->call_deferred(Variant(verbosity_level), Variant(String(message)));
        } });
}

// Sets the maximum verbosity level for TDLib log messages. Can be called from any thread.
void TdJsonManager::set_max_verbosity_level(int p_verbosity_level)
{
    _max_verbosity_level = p_verbosity_level;
    _set_log_message_callback();
}

// Sets the verbosity level for TDLib log messages. Can be called from any thread.
void TdJsonManager::set_verbosity_level(int p_new_verbosity_level)
{
    Dictionary _dict;
    _dict["@type"] = "setLogVerbosityLevel";
    _dict["new_verbosity_level"] = p_new_verbosity_level;
    String _req = JSON::stringify(_dict);
    td_execute(_req.utf8().get_data());
}

TdJsonManager::TdJsonManager()
{
    _instance = this;
    _set_log_message_callback();
    _mutex.instantiate();
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
void TdJsonManager::set_tdlib_parameters(
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
    Dictionary _req = _build_tdlib_params_request(p_api_id, p_api_hash, p_application_version, p_device_model, p_database_directory, p_use_test_dc, p_files_directory, p_use_file_database, p_use_message_database, p_use_secret_chats, p_system_language_code, p_system_version);

    {
        MutexLock lock(*_mutex.ptr());
        _default_params = _req;
        // Re-arm the clients that rely on the default parameters.
        for (auto &client_auth : _auth)
        {
            if (!client_auth.second.has_own)
            {
                client_auth.second.sent = false;
            }
        }
    }
}

// Builds the setTdlibParameters request. Shared by TdJsonManager::set_tdlib_parameters (default
// parameters) and TdJsonClient::set_tdlib_parameters (per-client parameters).
Dictionary TdJsonManager::_build_tdlib_params_request(
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
        _req["files_directory"] = ProjectSettings::get_singleton()->globalize_path(String(p_files_directory));
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

    return _req;
}

// Stores per-client TDLib parameters (TdJsonClient::set_tdlib_parameters).
// Warns if another client is already configured with the same database/files directory:
// TDLib holds an exclusive sqlite lock on the database directory.
void TdJsonManager::_set_client_params(int p_client_id, const Dictionary &p_params)
{
    if (p_client_id == 0)
    {
        UtilityFunctions::push_error("TdJsonManager::set_client_params: the client has no id, create it via create_client()");
        return;
    }

    String _db_dir = (String)p_params.get("database_directory", "");
    String _files_dir = p_params.has("files_directory") ? (String)p_params.get("files_directory", "") : _db_dir;

    {
        MutexLock lock(*_mutex.ptr());
        ClientAuth &_rec = _auth[p_client_id];
        _rec.params = p_params;
        _rec.has_own = true;
        _rec.sent = false;

        for (const auto &client_auth : _auth)
        {
            if (client_auth.first == p_client_id || !client_auth.second.has_own)
            {
                continue;
            }
            const Dictionary &_other = client_auth.second.params;
            String _other_db = (String)_other.get("database_directory", "");
            String _other_files = _other.has("files_directory") ? (String)_other.get("files_directory", "") : _other_db;
            if (_db_dir == _other_db || _files_dir == _other_files)
            {
                UtilityFunctions::push_warning("TdJsonManager: clients ", p_client_id, " and ", client_auth.first, " share a database/files directory. TDLib requires an exclusive sqlite lock per directory, one of the clients will fail to start.");
            }
        }
    }
}

// \return Current version of the tdlib
String TdJsonManager::get_tdlib_version()
{
    Dictionary _req;
    _req["@type"] = "getOption";
    _req["name"] = "version";
    return String(execute(_req).get("value", ""));
}

// Sends the stored TDLib parameters to the client that reached authorizationStateWaitTdlibParameters.
// Explicit per-client parameters take priority; otherwise the default parameters are used, at most once per client.
void TdJsonManager::_try_send_tdlib_parameters(const Dictionary &p_resp, const Ref<TdJsonClient> &p_client)
{
    if (p_client.is_null() || p_client->get_client_id() == 0)
    {
        return;
    }
    if ((String)p_resp.get("@type", "") != "updateAuthorizationState")
    {
        return;
    }

    Dictionary _auth_state = p_resp.get("authorization_state", Dictionary());
    String _auth_type = _auth_state.get("@type", "");

    if (_auth_type != "authorizationStateWaitTdlibParameters") return;

    int _client_id = p_client->get_client_id();
    Dictionary _params;
    bool _send = false;
    {
        MutexLock lock(*_mutex.ptr());
        ClientAuth &_rec = _auth[_client_id];
        if (_rec.has_own)
        {
            if (_rec.sent)
            {
                return;
            }
            _rec.sent = true;
            _params = _rec.params;
            _send = true;
        }
        else if (!_default_params.is_empty() && !_rec.sent)
        {
            _rec.sent = true;
            _params = _default_params;
            _send = true;
        }
    }

    if (!_send)
    {
        UtilityFunctions::push_warning("TdJsonManager: client ", _client_id, " reached authorizationStateWaitTdlibParameters without TDLib parameters, authorization will stall. Call set_tdlib_parameters().");
        return;
    }

    // Tag the request so its ok/error response can be attributed back to setTdlibParameters.
    _params["@extra"] = "set_params_" + String::num_int64(_client_id);
    p_client->send(_params);
}

const double POLL_TIMEOUT = 1.0;

void TdJsonManager::_thread_poll()
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

void TdJsonManager::_send_to_all_clients(Dictionary p_request)
{
    MutexLock lock(*_mutex.ptr());
    for (auto &[client_id, client] : _clients)
    {
        client->send(p_request);
    }
}

// Starts the TDLib client.
void TdJsonManager::start_poll()
{
    if (_is_running.load())
    {
        return;
    }
    if (_worker_thread.is_valid() && _worker_thread->is_started())
    {
        return;
    }
    _is_running.store(true);
    Dictionary _req;
    _req["@type"] = "getOption";
    _req["name"] = "version";
    _send_to_all_clients(_req);

    if (_worker_thread.is_null())
    {
        _worker_thread.instantiate();
    }

    _worker_thread->start(Callable(this, "_thread_poll"));
    _worker_thread_id = _worker_thread->get_id().to_int();
    emit_signal("poll_started");
}

// Stops the TDLib client.
void TdJsonManager::stop_poll()
{
    if (_is_running.load() && !_worker_thread.is_null())
    {
        Dictionary _req;
        _req["@type"] = "close";
        _send_to_all_clients(_req);
        _is_running.store(false);

        if (_worker_thread->is_alive())
        {
            _worker_thread->wait_to_finish();
        }
    }

    _worker_thread_id = 0;
    _worker_thread.unref();
    emit_signal("poll_stopped");
}

bool TdJsonManager::is_running()
{
    return _is_running.load();
}

TdJsonManager *TdJsonManager::get_singleton()
{
    return _instance;
}

Ref<TdJsonClient> TdJsonManager::create_client()
{
    godot::UtilityFunctions::print_verbose("TdJsonManager::create_client");
    Ref<TdJsonClient> client = memnew(TdJsonClient);
    client->_set_client_id(td_create_client_id());

    {
        MutexLock lock(*_mutex.ptr());
        _clients[client->get_client_id()] = client;
    }

    if (is_running())
    {
        Dictionary _req;
        _req["@type"] = "getOption";
        _req["name"] = "version";
        client->send(_req);
    }
    return client;
}

void TdJsonManager::remove_client(Ref<TdJsonClient> p_client)
{
    godot::UtilityFunctions::print_verbose("TdJsonManager::remove_client: ", p_client->get_client_id());
    int _id = p_client->get_client_id();
    if (_id == 0)
    {
        return;
    }
    {
        MutexLock lock(*_mutex.ptr());
        if (_clients.find(_id) == _clients.end())
        {
            return;
        }
        _clients.erase(_id);
        _auth.erase(_id);
    }
    Dictionary _req;
    _req["@type"] = "close";
    p_client->send(_req);
    p_client->_set_client_id(0);
}

// Bindings for godot
void TdJsonManager::_bind_methods()
{
    // public methods
    ClassDB::bind_method(D_METHOD("receive", "timeout"), &TdJsonManager::receive);
    ClassDB::bind_method(D_METHOD("execute", "request"), &TdJsonManager::execute);
    ClassDB::bind_method(D_METHOD("set_max_verbosity_level", "verbosity_level"), &TdJsonManager::set_max_verbosity_level);
    ClassDB::bind_method(D_METHOD("set_verbosity_level", "new_verbosity_level"), &TdJsonManager::set_verbosity_level);
    ClassDB::bind_method(D_METHOD("set_log_callback", "callback", "max_verbosity_level"), &TdJsonManager::set_log_callback, DEFVAL(4));
    ClassDB::bind_method(D_METHOD("get_tdlib_version"), &TdJsonManager::get_tdlib_version);
    ClassDB::bind_method(D_METHOD("start_poll"), &TdJsonManager::start_poll);
    ClassDB::bind_method(D_METHOD("stop_poll"), &TdJsonManager::stop_poll);
    ClassDB::bind_method(D_METHOD("is_running"), &TdJsonManager::is_running);
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
        &TdJsonManager::set_tdlib_parameters,
        DEFVAL(String("user://tdlib_data")),
        DEFVAL(false),
        DEFVAL(String("")),
        DEFVAL(true),
        DEFVAL(true),
        DEFVAL(true),
        DEFVAL(String("")),
        DEFVAL(String("")));
    ClassDB::bind_method(D_METHOD("create_client"), &TdJsonManager::create_client);
    ClassDB::bind_method(D_METHOD("remove_client", "client"), &TdJsonManager::remove_client);

    // private methods
    ClassDB::bind_method(D_METHOD("_thread_poll"), &TdJsonManager::_thread_poll);

    // signals
    ADD_SIGNAL(MethodInfo("request_received", PropertyInfo(Variant::DICTIONARY, "response")));
    ADD_SIGNAL(MethodInfo("poll_started"));
    ADD_SIGNAL(MethodInfo("poll_stopped"));
}

TdJsonManager::~TdJsonManager()
{
    stop_poll();
    td_set_log_message_callback(0, nullptr);
    if (_log_callback)
    {
        memdelete(_log_callback);
        _log_callback = nullptr;
    }
    if (TdJsonManager::_instance == this)
    {
        TdJsonManager::_instance = nullptr;
    }
}