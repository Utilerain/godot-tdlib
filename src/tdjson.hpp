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

private:
    int session_id;
};
}