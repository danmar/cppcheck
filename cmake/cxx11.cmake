macro(use_cxx11)
  # some GitHub Action Windows runners randomly fail with a complaint that Qt6 requires a C++17 compiler
  if(MSVC AND USE_QT6)
    # CMAKE_CXX_STANDARD 17 was added in CMake 3.8
    set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to use")
  elseif(USE_BOOST AND USE_BOOST_INT128)
    # Boost.Math requires C++14
    set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to use")
  else()
    set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ standard to use")
  endif()
endmacro()
