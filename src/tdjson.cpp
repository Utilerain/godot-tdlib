#include "tdjson.hpp"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/json.hpp>
#include <td/telegram/td_json_client.h>
#include <td/telegram/Log.h>

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
 * \attention This function will crash your project without creating thread 
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

// Set the maximum verbosity level for TDLib log messages. Can be called from any thread.
void godot::TdJson::set_max_verbosity_level(int verbosity_level)
{
    max_verbosity_level = verbosity_level;
    set_log_message_callback();
}

// Constructor
TdJson::TdJson() 
{
    client_id = td_create_client_id();
    set_log_message_callback();
}

// Return the session id
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

    ADD_SIGNAL(MethodInfo("request_received", PropertyInfo(Variant::DICTIONARY, "response")));
}