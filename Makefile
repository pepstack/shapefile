#######################################################################
# Makefile
#   Build libshapefile
#
# Author: 350137278@qq.com
#
# Update: 2021-06-22
#
# Show all predefinitions of gcc:
#
#   https://blog.csdn.net/10km/article/details/49023471
#
#   $ gcc -posix -E -dM - < /dev/null
#
#######################################################################
# Linux, CYGWIN_NT, MSYS_NT, ...
shuname="$(shell uname)"
OSARCH=$(shell echo $(shuname)|awk -F '-' '{ print $$1 }')

# debug | release (default)
RELEASE = 1
BUILDCFG = release

# 32 | 64 (default)
BITS = 64

# Is MINGW(1) or not(0)
MINGW_FLAG = 0

###########################################################
# Compiler Specific Configuration

CC = gcc

# for gcc-8+
# -Wno-unused-const-variable
CFLAGS += -std=gnu11 -D_GNU_SOURCE -fPIC -Wall -Wno-unused-function -Wno-unused-variable
#......

LDFLAGS += -lm
#......


###########################################################
# Architecture Configuration

ifeq ($(RELEASE), 0)
	# debug: make RELEASE=0
	CFLAGS += -D_DEBUG -g
	BUILDCFG = debug
else
	# release: make RELEASE=1
	CFLAGS += -DNDEBUG -O3
	BUILDCFG = release
endif

ifeq ($(BITS), 32)
	# 32bits: make BITS=32
	CFLAGS += -m32
	LDFLAGS += -m32
else
	ifeq ($(BITS), 64)
		# 64bits: make BITS=64
		CFLAGS += -m64
		LDFLAGS += -m64
	endif
endif


ifeq ($(OSARCH), MSYS_NT)
	MINGW_FLAG = 1
	ifeq ($(BITS), 64)
		CFLAGS += -D__MINGW64__
	else
		CFLAGS += -D__MINGW32__
	endif
else ifeq ($(OSARCH), MINGW64_NT)
	MINGW_FLAG = 1
	ifeq ($(BITS), 64)
		CFLAGS += -D__MINGW64__
	else
		CFLAGS += -D__MINGW32__
	endif
else ifeq ($(OSARCH), MINGW32_NT)
	MINGW_FLAG = 1
	ifeq ($(BITS), 64)
		CFLAGS += -D__MINGW64__
	else
		CFLAGS += -D__MINGW32__
	endif
endif


ifeq ($(OSARCH), CYGWIN_NT)
	ifeq ($(BITS), 64)
		CFLAGS += -D__CYGWIN64__ -D__CYGWIN__
	else
		CFLAGS += -D__CYGWIN32__ -D__CYGWIN__
	endif
endif


###########################################################
# Project Specific Configuration
PREFIX = .
DISTROOT = $(PREFIX)/dist

# Given dirs for all source (*.c) files
SRC_DIR = $(PREFIX)/source
COMMON_DIR = $(SRC_DIR)/common

#----------------------------------------------------------
# shapefile

SHAPEFILE_DIR = $(SRC_DIR)/shapefile
SHAPEFILE_VERSION_FILE = $(SHAPEFILE_DIR)/VERSION
SHAPEFILE_VERSION = $(shell cat $(SHAPEFILE_VERSION_FILE))

SHAPEFILE_STATIC_LIB = libshapefile.a
SHAPEFILE_DYNAMIC_LIB = libshapefile.so.$(SHAPEFILE_VERSION)

SHAPEFILE_DISTROOT = $(DISTROOT)/libshapefile-$(SHAPEFILE_VERSION)
SHAPEFILE_DIST_LIBDIR=$(SHAPEFILE_DISTROOT)/lib/$(OSARCH)/$(BITS)/$(BUILDCFG)
#----------------------------------------------------------


# add other projects here:
#...


# Set all dirs for C source: './source/a ./source/b'
ALLCDIRS += $(SRCDIR) \
	$(COMMON_DIR) \
	$(SHAPEFILE_DIR)
#...


# Get pathfiles for C source files: './source/a/1.c ./source/b/2.c'
CSRCS := $(foreach cdir, $(ALLCDIRS), $(wildcard $(cdir)/*.c))

# Get names of object files: '1.o 2.o'
COBJS = $(patsubst %.c, %.o, $(notdir $(CSRCS)))


# Given dirs for all header (*.h) files
INCDIRS += -I$(PREFIX) \
	-I$(SRC_DIR) \
	-I$(COMMON_DIR) \
	-I$(SHAPEFILE_DIR)
#...


ifeq ($(MINGW_FLAG), 1)
	###MINGW_CSRCS = $(COMMON_DIR)/win32/syslog-client.c
	MINGW_LINKS = -lws2_32
else
	MINGW_CSRCS =
	MINGW_LINKS = -lrt
endif

MINGW_COBJS = $(patsubst %.c, %.o, $(notdir $(MINGW_CSRCS)))

###########################################################
# Build Target Configuration
.PHONY: all apps clean cleanall dist


all: $(SHAPEFILE_DYNAMIC_LIB).$(OSARCH) $(SHAPEFILE_STATIC_LIB).$(OSARCH)

#...


#----------------------------------------------------------
# http://www.gnu.org/software/make/manual/make.html#Eval-Function

define COBJS_template =
$(basename $(notdir $(1))).o: $(1)
	$(CC) $(CFLAGS) -c $(1) $(INCDIRS) -o $(basename $(notdir $(1))).o
endef
#----------------------------------------------------------


$(foreach src,$(CSRCS),$(eval $(call COBJS_template,$(src))))

$(foreach src,$(MINGW_CSRCS),$(eval $(call COBJS_template,$(src))))


help:
	@echo
	@echo "Build all libs and apps as the following"
	@echo
	@echo "Build 64 bits release (default):"
	@echo "    $$ make clean && make"
	@echo
	@echo "Build 32 bits debug:"
	@echo "    $$ make clean && make RELEASE=0 BITS=32"
	@echo
	@echo "Dist target into default path:"
	@echo "    $$ make clean && make dist"
	@echo
	@echo "Dist target into given path:"
	@echo "    $$ make SHAPEFILE_DISTROOT=/path/to/YourInstallDir dist"
	@echo
	@echo "Build apps with all libs:"
	@echo "    $$ make clean && make apps"
	@echo
	@echo "Show make options:"
	@echo "    $$ make help"


#----------------------------------------------------------
$(SHAPEFILE_STATIC_LIB).$(OSARCH): $(COBJS) $(MINGW_COBJS)
	rm -f $@
	rm -f $(SHAPEFILE_STATIC_LIB)
	ar cr $@ $^
	ln -s $@ $(SHAPEFILE_STATIC_LIB)

$(SHAPEFILE_DYNAMIC_LIB).$(OSARCH): $(COBJS) $(MINGW_COBJS)
	$(CC) $(CFLAGS) -shared \
		-Wl,--soname=$(SHAPEFILE_DYNAMIC_LIB) \
		-Wl,--rpath='$(PREFIX):$(PREFIX)/lib:$(PREFIX)/libs:$(PREFIX)/libs/lib' \
		-o $@ \
		$^ \
		$(LDFLAGS) \
		$(MINGW_LINKS)
	ln -s $@ $(SHAPEFILE_DYNAMIC_LIB)
#----------------------------------------------------------

dist: all
	@mkdir -p $(SHAPEFILE_DISTROOT)/include/common
	@mkdir -p $(SHAPEFILE_DISTROOT)/include/shapefile
	@mkdir -p $(SHAPEFILE_DIST_LIBDIR)
	@cp $(SHAPEFILE_DIR)/shapefile_api.h $(SHAPEFILE_DISTROOT)/include/shapefile/
	@cp $(SHAPEFILE_DIR)/shapefile_def.h $(SHAPEFILE_DISTROOT)/include/shapefile/
	@cp $(PREFIX)/$(SHAPEFILE_STATIC_LIB).$(OSARCH) $(SHAPEFILE_DIST_LIBDIR)/
	@cp $(PREFIX)/$(SHAPEFILE_DYNAMIC_LIB).$(OSARCH) $(SHAPEFILE_DIST_LIBDIR)/
	@cd $(SHAPEFILE_DIST_LIBDIR)/ && ln -sf $(PREFIX)/$(SHAPEFILE_STATIC_LIB).$(OSARCH) $(SHAPEFILE_STATIC_LIB)
	@cd $(SHAPEFILE_DIST_LIBDIR)/ && ln -sf $(PREFIX)/$(SHAPEFILE_DYNAMIC_LIB).$(OSARCH) $(SHAPEFILE_DYNAMIC_LIB)
	@cd $(SHAPEFILE_DIST_LIBDIR)/ && ln -sf $(SHAPEFILE_DYNAMIC_LIB) libshapefile.so


clean:
	-rm -f *.stackdump
	-rm -f $(COBJS) $(MINGW_COBJS)
	-rm -f $(SHAPEFILE_STATIC_LIB)
	-rm -f $(SHAPEFILE_DYNAMIC_LIB)
	-rm -f $(SHAPEFILE_STATIC_LIB).$(OSARCH)
	-rm -f $(SHAPEFILE_DYNAMIC_LIB).$(OSARCH)


cleanall: clean
	-rm -rf $(DISTROOT)