TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    server.c \
    main.c \
    smtp.c

HEADERS += \
    server.h \
    smtp.h
