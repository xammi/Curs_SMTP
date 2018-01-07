TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/server.c \
    main.c \
    src/smtp.c

HEADERS += \
    inc/server.h \
    inc/smtp.h

DISTFILES += \
    userinfo.txt \
    README.md
