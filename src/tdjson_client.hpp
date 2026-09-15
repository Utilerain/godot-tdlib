#include <godot_cpp/classes/ref_counted.hpp>

namespace godot
{
    class TdJsonClient : public RefCounted
    {
        GDCLASS(TdJsonClient, RefCounted)

    protected:
        static void _bind_methods();

    public:
        TdJsonClient(int p_client_id=0);
        void send(Dictionary p_request);
        int get_client_id();
        void _on_response(Dictionary p_response); // used for getting updates from TdJsonManager
        void set_bot_token(String p_bot_token);
    
    private:
        int _client_id;
        void _set_bot_token(Dictionary p_response, Dictionary p_parameters);
    }
}