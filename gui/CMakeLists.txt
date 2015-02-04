if (BUILD_GUI)

    set(CMAKE_AUTOMOC ON)
    include(${QT_USE_FILE})
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        add_definitions(-DQT_NO_DEBUG)
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
        add_definitions(-DQT_NO_WARNING_OUTPUT)
    else()
        add_definitions(-DQT_DEBUG)
    endif()
    

    include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)
    include_directories(${PROJECT_SOURCE_DIR}/lib/)

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    file(GLOB uis  "*.ui")
    file(GLOB tss  "*.ts")
    QT4_WRAP_UI(uis_hdrs ${uis})
    QT4_ADD_RESOURCES(resources "gui.qrc")
    QT4_ADD_TRANSLATION(qms ${tss})
    
    link_libraries(${PROJECT_NAME} tinyxml ${QT_LIBRARIES})

    add_executable(cppcheck-gui ${hdrs} ${srcs} ${uis_hdrs} ${resources} ${qms})

    install(TARGETS cppcheck-gui RUNTIME DESTINATION bin COMPONENT applications)
    install(FILES ${qms} DESTINATION bin COMPONENT applications)

    set(CMAKE_AUTOMOC OFF)

endif()
