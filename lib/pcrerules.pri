# If HAVE_RULES=yes is passed to qmake, use PCRE and enable rules
contains(HAVE_RULES, [yY][eE][sS]) {
    CONFIG += use_pcre_rules
}

use_pcre_rules {
    DEFINES += HAVE_RULES
    LIBS += -L../externals -lpcre
    INCLUDEPATH += ../externals
    message("Rules enabled - to disable them and remove the dependency on PCRE, pass HAVE_RULES=no to qmake.")
} else {
    message("Rules disabled - to enable them, make PCRE available and pass HAVE_RULES=yes to qmake.")
}
