#include <godot_cpp/classes/ref_counted.hpp>

namespace godot
{
    class TdJsonClient : public RefCounted
    {
        GDCLASS(TdJsonClient, RefCounted)

    protected:
        static void _bind_methods();

    public:
        void send(Dictionary p_request);
        int get_client_id();
        void _on_response(Dictionary p_response); // used for getting updates from TdJsonManager
        void set_bot_token(String p_bot_token);
        void set_tdlib_parameters(
            int p_api_id,
            String p_api_hash,
            String p_application_version,
            String p_device_model,
            String p_database_directory = String("user://tdlib_data"),
            bool p_use_test_dc = false,
            String p_files_directory = String(""),
            bool p_use_file_database = true,
            bool p_use_message_database = true,
            bool p_use_secret_chats = true,
            String p_system_language_code = String(""),
            String p_system_version = String(""));
        void close();
        static Ref<TdJsonClient> create();
        void _set_client_id(int p_client_id = 0);

    private:
        int _client_id = 0;
        void _set_bot_token(Dictionary p_response, Dictionary p_parameters);
    };
}