#-------------------------------------------------
#
# Project created by QtCreator 2012-11-24T14:15:15
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = sotedit
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    amf3parser.cpp \
    sotfile.cpp \
    amfparser.cpp \
    variable.cpp \
    amf3vars.cpp

HEADERS += \
    amf3parser.h \
    Serializable.h \
    sotfile.h \
    amfparser.h \
    variable.h \
    readexception.h \
    parser.h \
    amf3vars.h
