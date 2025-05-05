if(BUILD_GUI)
    list(APPEND qt_components Core Gui Widgets PrintSupport LinguistTools Help Network)
    if(WITH_QCHART)
        list(APPEND qt_components Charts)
    endif()
    if(BUILD_TESTS)
        list(APPEND qt_components Test)
    endif()
    if(USE_QT6)
        find_package(Qt6 COMPONENTS ${qt_components} REQUIRED)
        set(QT_VERSION "${Qt6Core_VERSION}")
        if(NOT QT_VERSION)
            # TODO: remove fallback
            message(WARNING "'Qt6Core_VERSION' is not set - using 6.0.0 as fallback")
            set(QT_VERSION "6.0.0")
        endif()
        if(MSVC)
            # disable Visual Studio C++ memory leak detection since it causes compiler errors with Qt 6
            # D:\a\cppcheck\Qt\6.2.4\msvc2019_64\include\QtCore/qhash.h(179,15): warning C4003: not enough arguments for function-like macro invocation 'free' [D:\a\cppcheck\cppcheck\build\gui\cppcheck-gui.vcxproj]
            # D:\a\cppcheck\Qt\6.2.4\msvc2019_64\include\QtCore/qhash.h(179,15): error C2059: syntax error: ',' [D:\a\cppcheck\cppcheck\build\gui\cppcheck-gui.vcxproj]
            # this is supposed to be fixed according to the following tickets but it still happens
            # https://bugreports.qt.io/browse/QTBUG-40575
            # https://bugreports.qt.io/browse/QTBUG-86395
            set(DISABLE_CRTDBG_MAP_ALLOC ON)
        endif()
    else()
        message(WARNING "Building with Qt5 is deprecated (it went EOL in May 2023) and will be removed in Cppcheck 2.19 - please use Qt6 instead")
        find_package(Qt5 COMPONENTS ${qt_components} REQUIRED)
        set(QT_VERSION "${Qt5Core_VERSION_STRING}")
    endif()

    if(BUILD_ONLINE_HELP)
        find_program(QHELPGENERATOR qhelpgenerator)
        if(NOT QHELPGENERATOR)
            # TODO: how to properly get the Qt binary folder?
            # piggy-back off Qt::qmake for now as it should be in the same folder as the binary we are looking for
            get_target_property(_qmake_executable Qt::qmake IMPORTED_LOCATION)
            get_filename_component(_qt_bin_dir ${_qmake_executable} DIRECTORY)
            message(STATUS "qhelpgenerator not found in PATH - trying ${_qt_bin_dir}")
            # cannot be mandatory since qhelpgenerator is missing from the official qttools Linux package - https://bugreports.qt.io/browse/QTBUG-116168
            find_program(QHELPGENERATOR qhelpgenerator HINTS ${_qt_bin_dir} REQUIRED)
        endif()
    endif()
endif()

if(HAVE_RULES)
    find_path(PCRE_INCLUDE pcre.h)
    find_library(PCRE_LIBRARY NAMES pcre pcred)
    if(NOT PCRE_LIBRARY OR NOT PCRE_INCLUDE)
        message(FATAL_ERROR "pcre dependency for RULES has not been found")
    endif()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Python COMPONENTS Interpreter)

if(NOT Python_Interpreter_FOUND)
    if(NOT USE_MATCHCOMPILER_OPT STREQUAL "Off")
        message(WARNING "No python interpreter found - disabling matchcompiler.")
        set(USE_MATCHCOMPILER_OPT "Off")
    endif()
else()
    if(${Python_VERSION} VERSION_LESS 3.7)
        message(FATAL_ERROR "The minimum supported Python version is 3.7 - found ${Python_VERSION}")
    endif()
endif()

if(NOT USE_BUNDLED_TINYXML2)
    find_package(tinyxml2 QUIET)
    if(TARGET tinyxml2::tinyxml2)
        set(tinyxml2_LIBRARIES "tinyxml2::tinyxml2")
        set(tinyxml2_INCLUDE_DIRS $<TARGET_PROPERTY:tinyxml2::tinyxml2,INTERFACE_INCLUDE_DIRECTORIES>)
    else()
        find_library(tinyxml2_LIBRARIES tinyxml2)
        find_path(tinyxml2_INCLUDE_DIRS tinyxml2.h)
        if(NOT tinyxml2_LIBRARIES AND NOT tinyxml2_INCLUDE_DIRS)
            message(FATAL_ERROR "tinyxml2 has not been found")
        else()
            set(tinyxml2_FOUND 1)
        endif()
    endif()
endif()

find_package(Threads REQUIRED)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
    # avoid legacy warning about Boost lookup in CMake
    cmake_policy(SET CMP0167 NEW)
endif()

# we are only using the header-only "container" component so we can unconditionally search for it
if(USE_BOOST)
    find_package(Boost REQUIRED)
else()
    find_package(Boost)
endif()

find_program(LIBXML2_XMLLINT_EXECUTABLE xmllint)
