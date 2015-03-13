include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)

if (${CMAKE_BUILD_TYPE} STREQUAL Release)
    file(GLOB originalSrcs *.cpp)

    # Process each file (individually) with matchcompiler script. Detects changes in files to run
    # again the script.
    foreach(file_in ${originalSrcs})
        get_filename_component(file_out ${file_in} NAME)
        set(file_out "${PROJECT_BINARY_DIR}/lib/${file_out}")
        add_custom_command(OUTPUT ${file_out}
            COMMAND python tools/matchcompiler.py -i ${file_in} -b ${PROJECT_BINARY_DIR}/lib
            DEPENDS ${file_in}
            COMMENT "Updating file ${file_in} with matchcompiler script"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
        set(srcs ${srcs} ${file_out})
    endforeach(file_in)

else()
    file(GLOB srcs *.cpp)
endif()

file(GLOB hdrs *.h)

add_library(CppCheckLib ${srcs} ${hdrs})

if (HAVE_RULES)
    target_link_libraries(CppCheckLib ${PCRE})
endif()

install(TARGETS CppCheckLib
   RUNTIME DESTINATION bin COMPONENT libraries
   LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT libraries
   ARCHIVE DESTINATION lib COMPONENT libraries)

install(FILES ${hdrs}
    DESTINATION include/CppCheckLib/
    COMPONENT headers)

