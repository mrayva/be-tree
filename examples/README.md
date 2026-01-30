# C++ Conversion Examples

This directory contains example C++ conversions demonstrating the migration pattern for converting C modules to modern C++20/23.

## debug.cpp / debug.hpp

An example conversion of `src/debug.c` to idiomatic C++, demonstrating:

- **RAII patterns**: Using `std::string` for automatic memory management
- **Type safety**: Using `static_cast` and `const_cast` instead of C-style casts
- **Modern C++**: Using `nullptr` instead of `NULL`
- **Extern "C" linkage**: Maintaining C API compatibility for public functions
- **STL containers**: Using `std::vector` where appropriate (in function body)

### Why This Example Isn't Active

The `debug.cpp` conversion cannot currently be compiled because shared header files (like `value.h`) use C++ keywords as identifiers (e.g., `namespace` as a field name). These headers are included transitively and cause compilation errors in C++ mode.

### Migration Path

Before modules like `debug` can be converted, the repository needs a preliminary refactoring phase:

1. **Header Cleanup**: Rename C++ keywords used as identifiers
   - `namespace` → `ns` or `namespace_id`
   - Any other reserved C++ keywords

2. **Header C++ Compatibility**: Ensure headers can be included from C++ code
   - Add `extern "C"` guards where needed
   - Avoid C++-incompatible syntax

3. **Module Conversion**: Once headers are compatible, convert modules
   - Follow the pattern shown in `debug.cpp`
   - Update `src/CMakeLists.txt` to include the converted .cpp file
   - Keep extern "C" for functions called from C code

## Using These Examples

These files are reference implementations. To activate a converted module:

1. Ensure prerequisite header refactoring is complete
2. Copy the converted .cpp/.hpp to `src/`
3. Remove or rename the original .c/.h files
4. Update `src/CMakeLists.txt`:
   ```cmake
   set(BETREE_CPP_SOURCES
       debug.cpp  # Uncomment when ready
       # Add other converted modules here
   )
   ```
5. Build with `BUILD_AS_CPP=ON`
6. Test thoroughly

## Conversion Checklist

When converting a module to C++, follow this checklist:

- [ ] Rename `.c` → `.cpp` and `.h` → `.hpp`
- [ ] Add C++ includes (`<string>`, `<memory>`, `<vector>`, etc.)
- [ ] Add `extern "C"` wrapper around includes and public functions
- [ ] Replace `malloc/free` with RAII (std::unique_ptr, std::string, std::vector)
- [ ] Replace `NULL` with `nullptr`
- [ ] Use `static_cast`/`const_cast` instead of C-style casts
- [ ] Use `bool` instead of `int` for boolean values
- [ ] Ensure no implicit `void*` conversions
- [ ] Add proper const-correctness
- [ ] Update CMakeLists.txt to include the new .cpp file
- [ ] Verify all tests pass

## Further Reading

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Modern C++ Best Practices](https://github.com/cpp-best-practices)
- BUILD.md - Migration strategy documentation
