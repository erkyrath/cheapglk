# Unix Makefile for CheapGlk library

# This generates two files. One, of course, is libcheapglk.a -- the library
# itself. The other is Make.cheapglk; this is a snippet of Makefile code
# which locates the cheapglk library.
#
# When you install cheapglk, you must put libcheapglk.a in the lib directory,
# and glk.h, glkstart.h, and Make.cheapglk in the include directory.

# Pick a C compiler.
#CC = cc
CC = gcc -ansi

OPTIONS = -O

CFLAGS = $(OPTIONS) $(INCLUDEDIRS)

GLKLIB = libcheapglk.a

CHEAPGLK_OBJS =  \
  cgfref.o cggestal.o cgmisc.o cgstream.o cgstyle.o cgwindow.o main.o \
  gi_dispa.o cgblorb.o

CHEAPGLK_HEADERS = cheapglk.h gi_dispa.h

all: $(GLKLIB) Make.cheapglk

$(GLKLIB): $(CHEAPGLK_OBJS)
	ar r $(GLKLIB) $(CHEAPGLK_OBJS)
	ranlib $(GLKLIB)

Make.cheapglk:
	echo LINKLIBS = $(LIBDIRS) $(LIBS) > Make.cheapglk
	echo GLKLIB = -lcheapglk >> Make.cheapglk

$(CHEAPGLK_OBJS): glk.h $(CHEAPGLK_HEADERS)

clean:
	rm -f *~ *.o $(GLKLIB) Make.cheapglk
