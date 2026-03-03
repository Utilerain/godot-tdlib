#include "register_types.hpp"
#include "tdjson.hpp"
#include <godot_cpp/godot.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>

void initialize_telegram_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    godot::ClassDB::register_class<godot::TdJson>();
}

void uninitialize_telegram_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {

// Initialization.

GDExtensionBool GDE_EXPORT
tdjson_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_telegram_module);
    init_obj.register_terminator(uninitialize_telegram_module);
    init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}