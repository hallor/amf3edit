#-------------------------------------------------
#
# Project created by QtCreator 2012-11-24T14:15:15
#
#-------------------------------------------------

QT       += core gui

TARGET = sotedit
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    amf3parser.cpp \
    sotfile.cpp \
    amfparser.cpp \
    Variable.cpp \
    amf3vars.cpp \
    mainwindow.cpp \
    sotmodel.cpp \
    sotstream.cpp

HEADERS += \
    amf3parser.h \
    Serializable.h \
    sotfile.h \
    amfparser.h \
    Variable.h \
    Parser.h \
    amf3vars.h \
    exception.h \
    Value.h \
    mainwindow.h \
    sotmodel.h \
    sotstream.h

FORMS += \
    mainwindow.ui
