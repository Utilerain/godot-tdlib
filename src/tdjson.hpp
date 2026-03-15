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
    void send(Dictionary request);
    Dictionary execute(Dictionary request);
    Dictionary receive(double timeout);
    void set_max_verbosity_level(int verbosity_level);
    void set_verbosity_level(int new_verbosity_level);
    void set_log_callback(Callable callback);
    int get_client_id();

private:
    int client_id;
    void set_log_message_callback();
    int max_verbosity_level = 4;
    static Callable *log_callback;
};
}