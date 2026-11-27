#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/core/mutex_lock.hpp>

#include <atomic>
#include <unordered_map>

namespace godot
{
    class TdJsonClient;

    class TdJsonManager : public Object
    {
        GDCLASS(TdJsonManager, Object)
    protected:
        static void _bind_methods();

    public:
        TdJsonManager();
        ~TdJsonManager();
        Dictionary execute(Dictionary p_request);
        Dictionary receive(double p_timeout);
        void set_max_verbosity_level(int p_verbosity_level);
        void set_verbosity_level(int p_new_verbosity_level);
        void set_log_callback(Callable p_callback, int p_max_verbosity_level);
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
        static TdJsonManager *get_singleton();
        Ref<TdJsonClient> create_client();
        void remove_client(Ref<TdJsonClient> p_client);
        static Dictionary _build_tdlib_params_request(
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
            String p_system_version);
        void _set_client_params(int p_client_id, const Dictionary &p_params);

    private:
        // TDLib parameters for one client: either explicitly assigned via
        // TdJsonClient::set_tdlib_parameters, or the default parameters that were
        // applied (at most once) when the client reached authorizationStateWaitTdlibParameters.
        struct ClientAuth
        {
            Dictionary params;
            bool has_own = false;
            bool sent = false;
        };

        void _set_log_message_callback();
        int _max_verbosity_level = 4;
        void _try_send_tdlib_parameters(const Dictionary &p_resp, const Ref<TdJsonClient> &p_client);
        void _thread_poll();
        void _send_to_all_clients(Dictionary p_request);
        static Callable *_log_callback;
        Ref<Thread> _worker_thread;
        std::atomic<bool> _is_running{false};
        Dictionary _default_params;
        std::unordered_map<int, ClientAuth> _auth;
        Ref<Mutex> _mutex;
        static inline TdJsonManager *_instance = nullptr;
        std::unordered_map<int, Ref<TdJsonClient>> _clients;
        uint64_t _worker_thread_id = 0;
    };
}