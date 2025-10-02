# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "ButtonExample_autogen"
  "CMakeFiles\\ButtonExample_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ButtonExample_autogen.dir\\ParseCache.txt"
  )
endif()
