INCLUDEPATH += $${PWD} \
               $${PWD}/picojson \
               $${PWD}/valijson \
               $${PWD}/simplecpp \
               $${PWD}/tinyxml2

HEADERS += $${PWD}/picojson/picojson.h \
           $${PWD}/valijson/valijson_picojson_bundled.hpp \
           $${PWD}/simplecpp/simplecpp.h \
           $${PWD}/tinyxml2/tinyxml2.h

SOURCES += $${PWD}/simplecpp/simplecpp.cpp \
           $${PWD}/tinyxml2/tinyxml2.cpp
