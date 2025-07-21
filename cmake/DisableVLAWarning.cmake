# Disable warning for variable-length arrays in C++
if(CLANG OR ICX)
  add_compile_options(-Wno-vla-cxx-extension)
endif()
