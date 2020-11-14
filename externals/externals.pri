INCLUDEPATH += $${PWD} \
               $${PWD}/picojson \
               $${PWD}/simplecpp \
               $${PWD}/tinyxml

HEADERS += $${PWD}/picojson/picojson.h \
           $${PWD}/simplecpp/simplecpp.h \
           $${PWD}/tinyxml/tinyxml2.h

SOURCES += $${PWD}/simplecpp/simplecpp.cpp \
           $${PWD}/tinyxml/tinyxml2.cpp
