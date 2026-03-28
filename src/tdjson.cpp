#include "tdjson.hpp"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/json.hpp>
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
    const char* response = td_execute(_req.utf8().get_data());
    
    return Dictionary(JSON::parse_string(String(response != nullptr ? response : "")));
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
    const char* response = td_receive(timeout);

    if (response != nullptr) {
        emit_signal("request_received", Dictionary(JSON::parse_string(String(response))));
        return Dictionary(JSON::parse_string(String(response)));
    }
    return Dictionary();
}

// log message output for godot console
void TdJson::set_log_message_callback() 
{
    td_set_log_message_callback(max_verbosity_level, [](int verbosity_level, const char *message) {
    
    if (verbosity_level > 0) {
        print_line(String("[TDLib] ") + String(message));
    }
    else {
        print_error(String("[TDLib] [FATAL!!!] ") + String(message), __FILE__, __LINE__);
    }
    });
}

Callable *TdJson::log_callback;
// You can add your own callback using this function
void TdJson::set_log_callback(Callable p_callback)
{
    if (!log_callback) {
        log_callback = memnew(Callable);
    }
    *log_callback = p_callback;

    td_set_log_message_callback(max_verbosity_level, [](int verbosity_level, const char *message) {
        if (log_callback && log_callback->is_valid()) {
            log_callback->call_deferred(Variant(verbosity_level), Variant(String(message)));
        }
    });
}

// Set the maximum verbosity level for TDLib log messages. Can be called from any thread.
void TdJson::set_max_verbosity_level(int verbosity_level)
{
    max_verbosity_level = verbosity_level;
    set_log_message_callback();
}

// Set the verbosity level for TDLib log messages. Can be called from any thread.
void TdJson::set_verbosity_level(int new_verbosity_level)
{
    String _req = UtilityFunctions::str("{ \"@type\":\"setLogVerbosityLevel\", \"new_verbosity_level\": ", new_verbosity_level, " }");
    td_send(client_id, _req.utf8().get_data());
}

TdJson::TdJson() 
{
    client_id = td_create_client_id();
    td_send(client_id, "{\"@type\":\"setLogStream\", \"log_stream\": {\"@type\": \"logStreamEmpty\"}}");
    set_log_message_callback();
}

int TdJson::get_client_id() 
{
    return client_id;
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

    ADD_SIGNAL(MethodInfo("request_received", PropertyInfo(Variant::DICTIONARY, "response")));
}