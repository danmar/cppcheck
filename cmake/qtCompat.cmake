if (QT_VERSION VERSION_LESS 5.15)
    # "versionless" Qt is not supported until 5.15 so we need to use wrappers

    function(qt_wrap_ui out)
        qt5_wrap_ui(_uis_hdrs ${ARGN})
        set("${out}" ${_uis_hdrs} PARENT_SCOPE)
    endfunction()

    function(qt_add_resources out)
        qt5_add_resources(_resources ${ARGN})
        set("${out}" ${_resources} PARENT_SCOPE)
    endfunction()

    function(qt_create_translation out)
        qt5_create_translation(_qms ${ARGN})
        set("${out}" ${_qms} PARENT_SCOPE)
    endfunction()

    function(qt_wrap_cpp out)
        qt5_wrap_cpp(_sources ${ARGN})
        set("${out}" ${_sources} PARENT_SCOPE)
    endfunction()

    set(QT_CORE_LIB Qt5::Core)
    set(QT_TEST_LIB Qt5::Test)
    set(QT_WIDGETS_LIB Qt5::Widgets)
    set(QT_GUI_LIB Qt5::Gui)
    set(QT_HELP_LIB Qt5::Help)
    set(QT_PRINTSUPPORT_LIB Qt5::PrintSupport)
    set(QT_CHARTS_LIB Qt5::Charts)
    set(QT_NETWORK_LIB Qt5::Network)
else()
    # use "versionless" targets - no need for wrapper functions

    set(QT_CORE_LIB Qt::Core)
    set(QT_TEST_LIB Qt::Test)
    set(QT_WIDGETS_LIB Qt::Widgets)
    set(QT_GUI_LIB Qt::Gui)
    set(QT_HELP_LIB Qt::Help)
    set(QT_PRINTSUPPORT_LIB Qt::PrintSupport)
    set(QT_CHARTS_LIB Qt::Charts)
    set(QT_NETWORK_LIB Qt::Network)
endif()
