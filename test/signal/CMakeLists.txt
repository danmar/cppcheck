if (CMAKE_VERSION VERSION_EQUAL "3.13" OR CMAKE_VERSION VERSION_GREATER "3.13")
    # target_link_options requires CMake 3.13

    add_executable(test-signalhandler
            test-signalhandler.cpp
            ${PROJECT_SOURCE_DIR}/cli/signalhandler.cpp
            ${PROJECT_SOURCE_DIR}/cli/stacktrace.cpp)
    target_include_directories(test-signalhandler PRIVATE ${PROJECT_SOURCE_DIR}/cli ${PROJECT_SOURCE_DIR}/lib)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # names for static functions are omitted from trace
        target_compile_options_safe(test-signalhandler -Wno-missing-declarations)
        target_compile_options_safe(test-signalhandler -Wno-missing-prototypes)
        # required for backtrace() to produce function names
        target_link_options(test-signalhandler PRIVATE -rdynamic)
    endif()

    add_executable(test-stacktrace
            test-stacktrace.cpp
            ${PROJECT_SOURCE_DIR}/cli/stacktrace.cpp)
    target_include_directories(test-stacktrace PRIVATE ${PROJECT_SOURCE_DIR}/cli ${PROJECT_SOURCE_DIR}/lib)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # names for static functions are omitted from trace
        target_compile_options_safe(test-stacktrace -Wno-missing-declarations)
        target_compile_options_safe(test-stacktrace -Wno-missing-prototypes)
        # required for backtrace() to produce function names
        target_link_options(test-stacktrace PRIVATE -rdynamic)
    endif()
endif()