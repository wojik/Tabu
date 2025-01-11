# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Tabu_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Tabu_autogen.dir\\ParseCache.txt"
  "Tabu_autogen"
  )
endif()
