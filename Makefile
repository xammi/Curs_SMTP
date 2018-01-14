#############################################################################
# Makefile for building: smtp_server
#############################################################################

MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
CXX           = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
DEFINES       = -DQT_QML_DEBUG
CFLAGS        = -pipe -g -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk -mmacosx-version-min=10.7 -Wall -W -fPIC $(DEFINES)
CXXFLAGS      = -pipe -stdlib=libc++ -g -std=gnu++11 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk -mmacosx-version-min=10.7 -Wall -W -fPIC $(DEFINES)
INCPATH       = -I../smtp_server -I. -I/usr/local/Cellar/libconfig/1.7.2/include -I../../Qt/5.6/clang_64/mkspecs/macx-clang
DEL_FILE      = rm -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p
COPY          = cp -f
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
INSTALL_FILE  = install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   = cp -f -R
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
TAR           = tar -cf
COMPRESS      = gzip -9f
DISTNAME      = smtp_server1.0.0
DISTDIR = /Users/maksimkislenko/smtp_env/build-smtp_server-Desktop_Qt_5_6_0_clang_64bit-Debug/.tmp/smtp_server1.0.0
LINK          = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
LFLAGS        = -headerpad_max_install_names -stdlib=libc++ -Wl,-syslibroot,/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk -mmacosx-version-min=10.7
LIBS          = $(SUBLIBS) -L/usr/local/Cellar/libconfig/1.7.2/lib/ -lconfig.11 
AR            = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar cq
RANLIB        = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib -s
SED           = sed
STRIP         = 

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = ../smtp_server/src/server.c \
		../smtp_server/main.c \
		../smtp_server/src/smtp.c \
		../smtp_server/src/utils.c \
		../smtp_server/src/logger.c 
OBJECTS       = server.o \
		main.o \
		smtp.o \
		utils.o \
		logger.o
DIST          = ../smtp_server/conf/userinfo.txt \
		../smtp_server/README.md \
		../smtp_server/conf/settings.cfg \
		../smtp_server/smtp_server.pro inc/server.h \
		inc/smtp.h \
		inc/utils.h \
		inc/logger.h ../smtp_server/src/server.c \
		../smtp_server/main.c \
		../smtp_server/src/smtp.c \
		../smtp_server/src/utils.c \
		../smtp_server/src/logger.c
DESTDIR       = 
TARGET        = smtp_server


first: all

####### Build rules

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

all: $(TARGET)

dist: distdir FORCE
	(cd `dirname $(DISTDIR)` && $(TAR) $(DISTNAME).tar $(DISTNAME) && $(COMPRESS) $(DISTNAME).tar) && $(MOVE) `dirname $(DISTDIR)`/$(DISTNAME).tar.gz . && $(DEL_FILE) -r $(DISTDIR)

distdir: FORCE
	@test -d $(DISTDIR) || mkdir -p $(DISTDIR)
	$(COPY_FILE) --parents $(DIST) $(DISTDIR)/


clean: compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


distclean: clean 
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) .qmake.stash
	-$(DEL_FILE) Makefile


####### Compile

server.o: ../smtp_server/src/server.c ../smtp_server/inc/server.h \
		../smtp_server/inc/smtp.h \
		../smtp_server/inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o server.o ../smtp_server/src/server.c

main.o: ../smtp_server/main.c ../smtp_server/inc/server.h \
		../smtp_server/inc/smtp.h \
		../smtp_server/inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h \
		../smtp_server/inc/logger.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o main.o ../smtp_server/main.c

smtp.o: ../smtp_server/src/smtp.c ../smtp_server/inc/smtp.h \
		../smtp_server/inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o smtp.o ../smtp_server/src/smtp.c

utils.o: ../smtp_server/src/utils.c ../smtp_server/inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o utils.o ../smtp_server/src/utils.c

logger.o: ../smtp_server/src/logger.c ../smtp_server/inc/logger.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o logger.o ../smtp_server/src/logger.c

####### Install

install:  FORCE

uninstall:  FORCE

FORCE:

