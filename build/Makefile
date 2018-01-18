#############################################################################
# Makefile for building: smtp_server
#############################################################################

####### Compiler, tools and options

CC	    = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
CXX	   = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
CFLAGS	= -pipe -g -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk -mmacosx-version-min=10.7 -Wall -W -fPIC
CXXFLAGS      = -pipe -stdlib=libc++ -g -std=gnu++11 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk -mmacosx-version-min=10.7 -Wall -W -fPIC
INCPATH       = -I../ -I/usr/local/Cellar/libconfig/1.7.2/include
DEL_FILE      = rm -f
CHK_DIR_EXISTS= test -d
MKDIR	 = mkdir -p
COPY	  = cp -f
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
INSTALL_FILE  = install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   = cp -f -R
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE	  = mv -f
TAR	   = tar -cf
COMPRESS      = gzip -9f
DISTNAME      = smtp_server1.0.0
DESTDIR       = ../build
LINK	  = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
LFLAGS	= -headerpad_max_install_names -stdlib=libc++ -Wl,-syslibroot,/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk -mmacosx-version-min=10.7
LIBS	  = $(SUBLIBS) -L/usr/local/Cellar/libconfig/1.7.2/lib/ -lconfig.11
AR	    = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar cq
RANLIB	= /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib -s
SED	   = sed
STRIP	 =

####### Output directory

OBJECTS_DIR   = $(DESTDIR)

####### Files

SOURCES       = ../src/server.c \
		../main.c \
		../src/smtp.c \
		../src/utils.c \
		../src/logger.c 
OBJECTS       = server.o \
		main.o \
		smtp.o \
		utils.o \
		logger.o
DIST	  = ../conf/userinfo.txt \
		../conf/settings.cfg
TARGET	= smtp_server


first: all

####### Build rules

all: $(TARGET)

$(TARGET):  dest $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

dest: destdir FORCE
	cd $(DESTDIR)

destdir: FORCE
	@test -d $(DESTDIR) || mkdir -p $(DESTDIR)
	$(COPY_FILE) -p $(DIST) $(DESTDIR)/

clean: FORCE
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) $(TARGET)
	


####### Compile

server.o: ../src/server.c ../inc/server.h \
		../inc/smtp.h \
		../inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o server.o ../src/server.c

main.o: ../main.c ../inc/server.h \
		../inc/smtp.h \
		../inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h \
		../inc/logger.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o main.o ../main.c

smtp.o: ../src/smtp.c ../inc/smtp.h \
		../inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o smtp.o ../src/smtp.c

utils.o: ../src/utils.c ../inc/utils.h \
		/usr/local/Cellar/libconfig/1.7.2/include/libconfig.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o utils.o ../src/utils.c

logger.o: ../src/logger.c ../inc/logger.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o logger.o ../src/logger.c

####### Install

install:  FORCE

uninstall:  FORCE

FORCE:

