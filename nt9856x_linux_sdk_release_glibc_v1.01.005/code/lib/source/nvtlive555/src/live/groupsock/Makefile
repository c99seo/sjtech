INCLUDES = -Iinclude -I../UsageEnvironment/include
#PREFIX = /usr/local
PREFIX = ../../../release/nvtlive555
LIBDIR = $(PREFIX)/lib
##### Change the following for your environment:
INCLUDE_DIR = $(LIBRARY_DIR)/include
OUTPUT_DIR = $(LIBRARY_DIR)/output
COMPILE_OPTS = $(INCLUDES) -I. -I$(INCLUDE_DIR) -O2 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -fPIC $(PLATFORM_CFLAGS) -D__LINUX520 -D__LINUX -D_NO_NVT_IPC_ -D_NO_NVT_INFO_ -D_NO_NVT_SSENDER_ -DXLOCALE_NOT_USED=1 -fpermissive
C =			c
C_COMPILER =	$(CROSS_COMPILE)gcc
C_FLAGS =		$(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS)
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)gcc
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -DBSD=1 $(CPPFLAGS) $(CXXFLAGS)
OBJ =			o
LINK =			$(CROSS_COMPILE)gcc
LINK_OPTS =		-L$(LIBRARY_DIR) -lstdc++ -lpthread -lm -lnvtlive555 -lnvtinfo
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(CROSS_COMPILE)ar cr
LIBRARY_LINK_OPTS =
NM		  =     $(CROSS_COMPILE)nm
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION =
#_680_TODO -lnvtauth
LIBS_FOR_GUI_APPLICATION =
#EXE =
EXE = .a
NM =
SHARED_LIB = ON
LD_FLAGS	= -fPIC -shared
##### End of variables to change

NAME = libgroupsock
ALL = $(NAME).$(LIB_SUFFIX)
all:	$(ALL)

.$(C).$(OBJ):
	@echo Compiling $<
	@$(C_COMPILER) -c $(C_FLAGS) $<

.$(CPP).$(OBJ):
	@echo Compiling $<
	@$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

GROUPSOCK_LIB_OBJS = GroupsockHelper.$(OBJ) GroupEId.$(OBJ) inet.$(OBJ) Groupsock.$(OBJ) NetInterface.$(OBJ) NetAddress.$(OBJ) IOHandlers.$(OBJ)

GroupsockHelper.$(CPP):	include/GroupsockHelper.hh
include/GroupsockHelper.hh:	include/NetAddress.hh
include/NetAddress.hh:	include/NetCommon.h
GroupEId.$(CPP):	include/GroupEId.hh
include/GroupEId.hh:	include/NetAddress.hh
inet.$(C):		include/NetCommon.h
Groupsock.$(CPP):	include/Groupsock.hh include/GroupsockHelper.hh include/TunnelEncaps.hh
include/Groupsock.hh:	include/groupsock_version.hh include/NetInterface.hh include/GroupEId.hh
include/NetInterface.hh:	include/NetAddress.hh
include/TunnelEncaps.hh:	include/NetAddress.hh
NetInterface.$(CPP):	include/NetInterface.hh include/GroupsockHelper.hh
NetAddress.$(CPP):	include/NetAddress.hh include/GroupsockHelper.hh
IOHandlers.$(CPP):	include/IOHandlers.hh include/TunnelEncaps.hh

libgroupsock.$(LIB_SUFFIX): $(GROUPSOCK_LIB_OBJS) $(PLATFORM_SPECIFIC_LIB_OBJS)
	@echo Creating library $* ...
	@$(LIBRARY_LINK) $@ $(LIBRARY_LINK_OPTS) $(GROUPSOCK_LIB_OBJS)

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: install1 $(INSTALL2)
install1: libgroupsock.$(LIB_SUFFIX)
	  install -d $(DESTDIR)$(LIBDIR)
	  install -m 644 libgroupsock.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)
#	  install -d $(DESTDIR)$(PREFIX)/include/groupsock $(DESTDIR)$(LIBDIR)
#	  install -m 644 include/*.hh include/*.h $(DESTDIR)$(PREFIX)/include/groupsock
#	  install -m 644 libgroupsock.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)
install_shared_libraries: libgroupsock.$(LIB_SUFFIX)
	  ln -s libgroupsock.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/libgroupsock.$(SHORT_LIB_SUFFIX)
	  ln -s libgroupsock.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/libgroupsock.so

##### Any additional, platform-specific rules come here:
