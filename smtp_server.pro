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
    conf/userinfo.txt \
    README.md \
    conf/settings.cfg

macx: LIBS += -L/usr/local/Cellar/libconfig/1.7.2/lib/ -lconfig.11

INCLUDEPATH += /usr/local/Cellar/libconfig/1.7.2/include
DEPENDPATH += /usr/local/Cellar/libconfig/1.7.2/include
