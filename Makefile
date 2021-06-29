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
CFLAGS += -std=gnu99 -D_GNU_SOURCE -fPIC -Wall -Wno-unused-function -Wno-unused-variable
#......

LDFLAGS += -lpthread -lm
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

LIBCLOGGER_DIR = $(PREFIX)/deps/libclogger


# Given dirs for all source (*.c) files
SRC_DIR = $(PREFIX)/src
COMMON_DIR = $(SRC_DIR)/common
APPS_DIR = $(SRC_DIR)/apps

SHAPEFILE_DIR = $(SRC_DIR)/shapefile

APPS_DISTROOT = $(DISTROOT)/apps

SHAPEFILE_VERSION_FILE = $(SHAPEFILE_DIR)/VERSION
SHAPEFILE_VERSION = $(shell cat $(SHAPEFILE_VERSION_FILE))

SHAPEFILE_STATIC_LIB = libshapefile.a
SHAPEFILE_DYNAMIC_LIB = libshapefile.so.$(SHAPEFILE_VERSION)

SHAPEFILE_DISTROOT = $(DISTROOT)/libshapefile-$(SHAPEFILE_VERSION)
SHAPEFILE_DIST_LIBDIR=$(SHAPEFILE_DISTROOT)/lib/$(OSARCH)/$(BITS)/$(BUILDCFG)

#......


# Set all dirs for C source: './src/a ./src/b'
ALLCDIRS += $(SRCDIR) $(COMMON_DIR) $(SHAPEFILE_DIR)

# Get pathfiles for C source files: './src/a/1.c ./src/b/2.c'
CSRCS := $(foreach cdir, $(ALLCDIRS), $(wildcard $(cdir)/*.c))

# Get names of object files: '1.o 2.o'
COBJS = $(patsubst %.c, %.o, $(notdir $(CSRCS)))


# Given dirs for all header (*.h) files
INCDIRS += -I$(PREFIX) \
	-I$(SRC_DIR) \
	-I$(COMMON_DIR) \
	-I$(SHAPEFILE_DIR) \
	-I$(LIBCLOGGER_DIR)/include
#......


ifeq ($(MINGW_FLAG), 1)
	MINGW_CSRCS = $(COMMON_DIR)/win32/syslog-client.c
	MINGW_LINKS = -lws2_32
else
	MINGW_CSRCS =
	MINGW_LINKS =
endif

MINGW_COBJS = $(patsubst %.c, %.o, $(notdir $(MINGW_CSRCS)))

###########################################################
# Build Target Configuration
.PHONY: all apps clean cleanall dist


all: $(SHAPEFILE_DYNAMIC_LIB).$(OSARCH) $(SHAPEFILE_STATIC_LIB).$(OSARCH)



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
	@echo Build libshapefile-$(shell cat $(SHAPEFILE_VERSION_FILE)) for:
	@echo
	@echo "1) 64 bits release (default)"
	@echo "    $$ make"
	@echo
	@echo "2) 32 bits debug"
	@echo "    $$ make RELEASE=0 BITS=32"
	@echo
	@echo Dist target into default path: $(SHAPEFILE_DISTROOT)"
	@echo "    $$ make dist"
	@echo
	@echo Dist target into given path:
	@echo "    $$ make SHAPEFILE_DISTROOT=/path/to/YourInstallDir dist"
	@echo
	@echo "Show make options:"
	@echo "    $$ make help"


$(SHAPEFILE_STATIC_LIB).$(OSARCH): $(COBJS) $(MINGW_COBJS)
	rm -f $@
	rm -f $(SHAPEFILE_STATIC_LIB)
	ar cr $@ $^
	ln -s $@ $(SHAPEFILE_STATIC_LIB)


$(SHAPEFILE_DYNAMIC_LIB).$(OSARCH): $(COBJS) $(MINGW_COBJS)
	$(CC) $(CFLAGS) -shared \
		-Wl,--soname=$(SHAPEFILE_DYNAMIC_LIB) \
		-Wl,--rpath='$(PREFIX):$(PREFIX)/lib:$(PREFIX)/../lib:$(PREFIX)/../libs/lib' \
		-o $@ \
		$^ \
		$(LDFLAGS) \
		$(MINGW_LINKS)
	ln -s $@ $(SHAPEFILE_DYNAMIC_LIB)


apps: dist test_shapefile.exe.$(OSARCH) test_shapefiledll.exe.$(OSARCH)


# -lrt for Linux

test_shapefile.exe.$(OSARCH): $(APPS_DIR)/test_shapefile/app_main.c
	@echo Building test_shapefile.exe.$(OSARCH)
	$(CC) $(CFLAGS) $< $(INCDIRS) \
	-o $@ \
	$(SHAPEFILE_STATIC_LIB) \
	$(LDFLAGS) \
	$(MINGW_LINKS) \
	-lrt


test_shapefiledll.exe.$(OSARCH): $(APPS_DIR)/test_shapefile/app_main.c
	@echo Building test_shapefiledll.exe.$(OSARCH)
	$(CC) $(CFLAGS) $< $(INCDIRS) \
	-Wl,--rpath='$(PREFIX):$(PREFIX)/lib:$(PREFIX)/../lib:$(PREFIX)/../libs/lib' \
	-o $@ \
	$(SHAPEFILE_DYNAMIC_LIB) \
	$(LDFLAGS) \
	$(MINGW_LINKS) \
	-lrt


dist: all
	@mkdir -p $(SHAPEFILE_DISTROOT)/include/shapefile
	@mkdir -p $(SHAPEFILE_DISTROOT)/include/common
	@mkdir -p $(SHAPEFILE_DIST_LIBDIR)
	@cp $(COMMON_DIR)/unitypes.h $(SHAPEFILE_DISTROOT)/include/common/
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
	-rm -f ./msvc/*.VC.db
	-rm -rf ./msvc/libshapefile/build
	-rm -rf ./msvc/libshapefile/target
	-rm -rf ./msvc/libshapefile_dll/build
	-rm -rf ./msvc/libshapefile_dll/target
	-rm -rf ./msvc/test_shapefile/build
	-rm -rf ./msvc/test_shapefile/target
	-rm -rf ./msvc/test_shapefiledll/build
	-rm -rf ./msvc/test_shapefiledll/target
	-rm -f test_shapefile.exe.$(OSARCH)
	-rm -f test_shapefiledll.exe.$(OSARCH)
	

cleanall: clean
	-rm -rf $(DISTROOT)