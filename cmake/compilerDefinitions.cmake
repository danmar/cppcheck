if (UNIX AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_definitions(-D_GLIBCXX_DEBUG)
endif()
