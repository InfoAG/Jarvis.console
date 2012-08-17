#-------------------------------------------------
#
# Project created by QtCreator 2012-07-31T11:54:06
#
#-------------------------------------------------

QT       += core network

QT       -= gui
QMAKE_CXXFLAGS += -std=c++11
TARGET = Jarvis-console
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
INCLUDEPATH += ../Frontend
LIBS += -L$$PWD/../Frontend/debug/ -lJarvis-Frontend

SOURCES += main.cpp \
    TerminalPrinter.cpp \

HEADERS += \
    TerminalPrinter.h \
    InputWorker.h \
