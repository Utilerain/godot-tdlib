# Forking Rules for Contributors

When making changes to the project, follow these naming conventions:

- Function and method arguments must be passed as `p_args`.
- Private variables and internal fields should be named with the `_` prefix, for example: `_param`.
- Private functions and methods should also use the `_` prefix, for example: `_function`.
- All new implementations must match the style of the existing code; see examples in the `tdjson.hpp` and `tdjson.cpp` files.

Example:

```cpp
// public
void function(p_param)
{
    Dictionary _resp;
}

// private
void _function(p_param)
{
    Dictionary _req;
}
```

This rule must be followed to keep the code compatible with the project's current architecture and the style of the existing classes.

Workflow note: contributors may automate the TDLib and GDExtension build process via GitHub Actions if they consider it necessary. This is optional and intended to help with validation or CI, but it does not replace the project's coding conventions and architecture requirements.
