#include <godot_cpp/classes/ref_counted.hpp>
#include <thread>
#include <atomic>

namespace godot
{
    class TdJson : public RefCounted
    {
        GDCLASS(TdJson, RefCounted)
    protected:
        static void _bind_methods();

    public:
        TdJson();
        ~TdJson();
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
            String database_directory = String("user://tdlib_data"),
            bool use_test_dc = false,
            String files_directory = String(""),
            bool use_file_database = true,
            bool use_message_database = true,
            bool use_secret_chats = true,
            String system_language_code = String(""),
            String system_version = String(""));
        String get_tdlib_version();
        void start_poll();
        void stop_poll();
        bool is_running();
        void set_bot_token(String bot_token);

    private:
        int client_id;
        void _set_log_message_callback();
        int max_verbosity_level = 4;
        void _set_tdlib_parameters(Dictionary p_response, Dictionary p_parameters);
        void _thread_poll();
        void _set_bot_token(Dictionary p_response, Dictionary p_parameters);
        static Callable *log_callback;
        std::thread worker_thread;
        std::atomic<bool> _is_running{false};
    };
}