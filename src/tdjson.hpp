#include <godot_cpp/classes/ref_counted.hpp>

namespace godot 
{
    class TdJson : public RefCounted
    {
    GDCLASS(TdJson, RefCounted)
protected:
    static void _bind_methods();
public:
    TdJson();
    void send(String request);
    String execute(String request);
    String receive(double timeout);
    void set_max_verbosity_level(int verbosity_level);
    int get_client_id();

private:
    int session_id;
    void set_log_message_callback();
    int max_verbosity_level = 4;
};
}