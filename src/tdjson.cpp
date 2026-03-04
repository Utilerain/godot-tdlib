#include "tdjson.hpp"
#include <godot_cpp/godot.hpp>
#include <td/telegram/td_json_client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/Log.h>

using namespace godot;


/**
 * Sends request to the TDLib client. May be called from any thread.
 * \param[in] client_id TDLib client identifier.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 */
void TdJson::send(String request) 
{
    td_send(session_id, request.utf8().get_data());
}

/**
 * Synchronously executes a TDLib request.
 * A request can be executed synchronously, only if it is documented with "Can be called synchronously".
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] request JSON-serialized null-terminated request to TDLib.
 * \return JSON-serialized null-terminated request response.
 */
String TdJson::execute(String request)
{
    const char* response = td_execute(request.utf8().get_data());
    return String(response != nullptr ? response : "");
}

/**
 * Receives incoming updates and request responses. Must not be called simultaneously from two different threads.
 * The returned pointer can be used until the next call to td_receive or td_execute, after which it will be deallocated by TDLib.
 * \param[in] timeout The maximum number of seconds allowed for this function to wait for new data.
 * \return JSON-serialized null-terminated incoming update or request response. May be NULL if the timeout expires.
 */
String TdJson::receive(double timeout) 
{
    const char* response = td_receive(timeout);
    if (response != nullptr) {
        emit_signal("request_received", String(response));
        return String(response);
    }
    return String();
}

// Constructor
TdJson::TdJson() 
{
    session_id = td_create_client_id();
    td_set_log_message_callback(4, [](int verbosity_level, const char *message) {
        if (verbosity_level > 0) {
            print_line(String("[TDLib] ") + String(message));
        }
    });
}

// Bindings for godot
void TdJson::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("send", "request"), &TdJson::send);
    ClassDB::bind_method(D_METHOD("receive", "timeout"), &TdJson::receive);
    ClassDB::bind_method(D_METHOD("execute", "request"), &TdJson::execute);

    ADD_SIGNAL(MethodInfo("request_received", PropertyInfo(Variant::STRING, "response")));
}
