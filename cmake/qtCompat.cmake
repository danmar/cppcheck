# "versionless" Qt is not supported until 5.15 we need to use wrappers

function(qt_wrap_ui out)
    qt5_wrap_ui(_uis_hdrs ${ARGN})
    set("${out}" ${_uis_hdrs} PARENT_SCOPE)
endfunction()

function(qt_add_resources out)
    qt5_add_resources(_resources ${ARGN})
    set("${out}" ${_resources} PARENT_SCOPE)
endfunction()

function(qt_add_translation out)
    qt5_add_translation(_qms ${ARGN})
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