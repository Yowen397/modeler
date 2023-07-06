file(REMOVE_RECURSE
  "CMAKE_BINARY_DIR/bin/hello"
  "CMAKE_BINARY_DIR/bin/hello.pdb"
  "CMakeFiles/hello.dir/test.cpp.o"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/hello.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
