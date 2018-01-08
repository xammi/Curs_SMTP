TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/server.c \
    main.c \
    src/smtp.c \
    src/utils.c

HEADERS += \
    inc/server.h \
    inc/smtp.h \
    inc/utils.h

DISTFILES += \
    userinfo.txt \
    README.md
