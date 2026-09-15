#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <atomic>
#include <godot_cpp/classes/mutex.hpp>

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
        String get_tdlib_version();
        void start_poll();
        void stop_poll();
        bool is_running();
        void set_bot_token(String bot_token);

    private:
        int _client_id;
        void _set_log_message_callback();
        int _max_verbosity_level = 4;
        void _set_tdlib_parameters(Dictionary p_response, Dictionary p_parameters);
        void _thread_poll();
        void _set_bot_token(Dictionary p_response, Dictionary p_parameters);
        static Callable *_log_callback;
        Ref<Thread> _worker_thread;
        std::atomic<bool> _is_running{false};
        Ref<Mutex> _mutex;
    };
}