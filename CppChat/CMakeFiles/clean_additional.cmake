# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/CppChat_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/CppChat_autogen.dir/ParseCache.txt"
  "CppChat_autogen"
  )
endif()
