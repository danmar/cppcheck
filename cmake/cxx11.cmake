macro(use_cxx11)
  if(USE_BOOST AND USE_BOOST_INT128)
    # Boost.Math requires C++14
    set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to use")
  else()
    set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ standard to use")
  endif()
endmacro()
