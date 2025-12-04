macro(use_cxx11)
  # enable unconditionally for Visual Studio since it is the lowest possible standard to select
  if(MSVC OR (USE_BOOST AND USE_INT128 STREQUAL "Boost"))
    # Boost.Math requires C++14
    set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to use")
  else()
    set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ standard to use")
  endif()
endmacro()
