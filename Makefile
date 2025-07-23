dependdir := $(shell ls .depend >/dev/null 2>&1)
ifneq ($(dependdir), .depend)
    dependdir := $(shell touch .depend)
endif

INSTALL_PATH=/data/qmfish/by_ctrl/libs/
INSTALL_INTERFACE_PATH=/data/qmfish/by_ctrl/interface/
CC=g++
GCC=gcc
COMP_PARA=-g -Wall -c -D__MULITI_THREAD__ -D__DEBUG__ -DNDEBUG -D_LINUX -D_mt
LINK_PARA=-mt #-pg

INCS=-I/usr/local/include -I/usr/include/  -I../interface/

LIBS=-L/usr/local/lib   -lpthread -lz
CSRCS   = $(wildcard *.c) 
CPPSRCS = $(wildcard *.cpp) 
COBJS   = $(patsubst %.c, %.o, $(CSRCS))
CPPOBJS = $(patsubst %.cpp, %.o, $(CPPSRCS))
OBJS = $(CPPOBJS) $(COBJS)

TARGET=../libs/libby_ctrl.a
INTERFACE_TARGET=../interface/*.h

all : $(TARGET)

$(TARGET): $(OBJS)
	ar -r  $(TARGET) *.o	
ifeq ("${USER}","by")
	@-cp -rf $(INTERFACE_TARGET) $(INSTALL_INTERFACE_PATH)
	@-echo "copy to $(INSTALL_INTERFACE_PATH) done!"
	@-cp $(TARGET) $(INSTALL_PATH)
	@-echo "copy to $(INSTALL_PATH) done!"
	#@-svn add $(INSTALL_INTERFACE_PATH)*.h
	@-svn commit -m "auto make update" $(INSTALL_INTERFACE_PATH)*.h
	#@-svn add $(INSTALL_PATH)*.a
	@-svn commit -m "auto commit make" $(INSTALL_PATH)*.a
	@-echo "svn commit $(INSTALL_PATH) $(INSTALL_INTERFACE_PATH) done!"
endif

.cpp.o :
	${CC} ${COMP_PARA} $< ${INCS}

.c.o :
	${GCC} ${COMP_PARA} $< ${INCS}

clean :
	@-rm -f *.o *.so.* *.so *.a $(TARGET)
	@-rm -f ../libs/*.a

depend:
	gcc -E -c $(CFLAGS) $(INCS) -MM *.c >.depend
	g++ -E -c $(CPPFLAGS) $(INCS) -MM *.cpp >>.depend

include .depend
