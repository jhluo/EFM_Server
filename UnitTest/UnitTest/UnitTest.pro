TEMPLATE = subdirs

SUBDIRS += src

CONFIG(debug, debug|release) {
    SUBDIRS += tests
}

SOURCES      += $$PWD/googletest/src/gtest-all.cc \
                $$PWD/googlemock/src/gmock-all.cc

INCLUDEPATH  += $$PWD/googletest
INCLUDEPATH  += $$PWD/googlemock
INCLUDEPATH  += $$PWD/googletest/include
INCLUDEPATH  += $$PWD/googlemock/include
