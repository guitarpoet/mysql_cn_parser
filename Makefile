#############################################################################
#
# The Makefile for building mysql_cn_parser
#
# License: GPL (General Public License)
# Author:  Jack <guitarpoet AT gmail DOT com>
# Date:    2014/09/21 (version 0.1)
#
# Description:
# ------------
#  This is a small parser for support Chinese word tokenizing for MySql's
#  full text search
#
#===========================================================================

# The pre-processor and compiler options.
MY_CFLAGS =

# The linker options.
MY_LIBS   = -lmmseg

# The pre-processor options used by the cpp (man cpp for more).
CPPFLAGS  = -Wall -I/opt/local/include/mysql55/mysql -Wno-deprecated -DMYSQL_DYNAMIC_PLUGIN 

MYSQL_PLUGIN_FOLDER = /opt/local/lib/mysql55/plugin/

# The options used in linking as well as in any direct use of ld.
LDFLAGS   =

SHARED_PLUGIN_FLAGS := -shared -fPIC

# The directories in which source files reside.
# If not specified, only the current directory will be serached.
SRCDIRS   = src

# The executable file name.
# If not specified, current directory name or `a.out' will be used.
PROGRAM   = mysql_cn_parser

PLUGIN    = mysql_cn_parser.so

## Implicit Section: change the following only when necessary.
##==========================================================================

# The source file types (headers excluded).
# .c indicates C source files, and others C++ ones.
SRCEXTS = .c .C .cc .cpp .CPP .c++ .cxx .cp

# The header file types.
HDREXTS = .h .H .hh .hpp .HPP .h++ .hxx .hp

# The pre-processor and compiler options.
# Users can override those variables from the command line.
CFLAGS  = -g -O2
CXXFLAGS= -g -O2

# The C program compiler.
CC     = gcc

# The C++ program compiler.
CXX    = g++

# Un-comment the following line to compile C programs as C++ ones.
CC     = $(CXX)

# The command used to delete file.
RM     = rm -f

ETAGS = etags
ETAGSFLAGS =

CTAGS = ctags
CTAGSFLAGS =

## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
SHELL   = /bin/sh
EMPTY   =
SPACE   = $(EMPTY) $(EMPTY)
ifeq ($(PROGRAM),)
  CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))
  PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))
  ifeq ($(PROGRAM),)
    PROGRAM = a.out
  endif
endif
ifeq ($(SRCDIRS),)
  SRCDIRS = .
endif
SOURCES = $(shell ls src/*.c*)
HEADERS = $(shell ls src/*.h)
SRC_CXX = $(filter-out %.c,$(SOURCES))
OBJS    = $(addsuffix .o, $(basename $(SOURCES)))
DEPS    = $(OBJS:.o=.d)

## Define some useful variables.
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
                  echo "-MM -MP"; else echo "-M"; fi )
DEPEND      = $(CC)  $(DEP_OPT)  $(MY_CFLAGS) $(CFLAGS) $(CPPFLAGS)
DEPEND.d    = $(subst -g ,,$(DEPEND))
COMPILE.c   = $(CC)  $(MY_CFLAGS) $(CFLAGS)   $(CPPFLAGS) -c
COMPILE.cxx = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c
LINK.c      = $(CC)  $(MY_CFLAGS) $(CFLAGS)   $(CPPFLAGS) $(LDFLAGS)
LINK.cxx    = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

.PHONY: all objs tags ctags clean distclean help show

# Delete the default suffixes
.SUFFIXES:

run: all
	@./${PROGRAM}

install: ${PLUGIN}
	@sudo cp ${PLUGIN} ${MYSQL_PLUGIN_FOLDER}

plugin_test:
	@sudo /opt/local/share/mysql5/mysql/mysql.server restart
	@mysql -u root test < test/test.sql
test_sql:
	@mysql -u root test -e 'select match(text) against("其他 -其他" in boolean mode), text from fulltext_test'

all: $(PROGRAM)

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

%.o:%.c
	$(COMPILE.c) $< -o $@

%.o:%.C
	$(COMPILE.cxx) $< -o $@

%.o:%.cc
	$(COMPILE.cxx) $< -o $@

%.o:%.cpp
	$(COMPILE.cxx) $< -o $@

%.o:%.CPP
	$(COMPILE.cxx) $< -o $@

%.o:%.c++
	$(COMPILE.cxx) $< -o $@

%.o:%.cp
	$(COMPILE.cxx) $< -o $@

%.o:%.cxx
	$(COMPILE.cxx) $< -o $@

# Rules for generating the tags.
#-------------------------------------
tags: $(HEADERS) $(SOURCES)
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)

ctags: $(HEADERS) $(SOURCES)
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)

# Rules for generating the executable.
#-------------------------------------
$(PROGRAM):${HEADERS} ${SOURCES} ${OBJS}
	${COMPILE.cxx} ${SOURCES}
	${LINK.cxx} ${OBJS} -o ${PROGRAM} ${MY_LIBS}

$(PLUGIN):${HEADERS} ${SOURCES} ${OBJS}
	${COMPILE.cxx} ${SOURCES}
	${LINK.cxx} ${OBJS} ${SHARED_PLUGIN_FLAGS} -o ${PLUGIN} ${MY_LIBS}

clean:
	$(RM) $(OBJS) ${PLUGIN} $(PROGRAM) $(PROGRAM).exe

distclean: clean
	$(RM) $(DEPS) TAGS

## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##
#############################################################################
