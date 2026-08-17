#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/os.hpp>

namespace godot
{
    class TdJson : public RefCounted
    {
        GDCLASS(TdJson, RefCounted)
    protected:
        static void _bind_methods();

    public:
        TdJson();
        void send(Dictionary request);
        Dictionary execute(Dictionary request);
        Dictionary receive(double timeout);
        void set_max_verbosity_level(int verbosity_level);
        void set_verbosity_level(int new_verbosity_level);
        void set_log_callback(Callable callback);
        int get_client_id();
        void set_tdlib_parameters(
            int api_id,
            String api_hash,
            String application_version,
            String device_model,
            String database_directory = OS::get_singleton()->get_user_data_dir().path_join(String("tdlib_data")),
            bool use_test_dc = false,
            String files_directory = String(""),
            bool use_file_database = true,
            bool use_message_database = true,
            bool use_secret_chats = true,
            String system_language_code = OS::get_singleton()->get_locale_language(),
            String system_version = String("")
        );
        String get_tdlib_version();

    private:
        int client_id;
        void set_log_message_callback();
        int max_verbosity_level = 4;
        void _send_tdlib_parameters(Dictionary _response, Dictionary parameters);
        static Callable *log_callback;
    };
}