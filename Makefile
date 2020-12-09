.POSIX:
.DELETE_ON_ERROR:
.SUFFIXES:

CC       = gcc
LD       = $(CC)
RM       = rm -f
TEST     = test
VERBOSE  = false
MKDIR    = mkdir -p
CPPFLAGS = -Isrc -Ilib -pthread
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Werror
CFLAGS  += -D_POSIX_C_SOURCE=200809L
MDFLAGS  = -MM -MT $(@:.d=.o)
LDLIBS   = -lz -lpthread
LDFLAGS  =

DSTDIR   = bin
TMPDIR  := $(DSTDIR)/.build
TARGET  := $(DSTDIR)/barkeeper
CHECKS  := $(DSTDIR)/unit-test
SOURCES := $(shell find src -name "*.c")
TESTSRC := $(shell find tests -name "*.c")
OBJECTS := $(SOURCES:%.c=$(TMPDIR)/%.o)
DEPENDS := $(SOURCES:%.c=$(TMPDIR)/%.d)
TESTOBJ := $(TESTSRC:%.c=$(TMPDIR)/%.o)
TESTDEP := $(TESTSRC:%.c=$(TMPDIR)/%.d)

.PHONY: all
all: $(TARGET) $(CHECKS)

.PHONY: check
check: $(CHECKS)
	$(Q) ./$<

$(TARGET): $(OBJECTS) $(DEPENDS)
	$(E) "[LD] $@"
	$(Q) $(LD) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)

$(CHECKS): $(TESTOBJ) $(TESTDEP)
	$(E) "[LD] $@"
	$(Q) $(LD) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)

$(TMPDIR)/%.o: %.c # OBJECTS
	$(E) "[CC] $<"; $(TEST) -d $(@D) || $(MKDIR) $(@D)
	$(Q) $(CC) -c -o $@ $(CFLAGS) $(CPPFLAGS) $<

$(TMPDIR)/%.d: %.c # DEPENDS
	$(Q) $(TEST) -d $(@D) || $(MKDIR) $(@D)
	$(Q) $(CC) $(CFLAGS) $(CPPFLAGS) $< $(MDFLAGS) > $@

.PHONY: clean
clean:
	$(E) "[RM] $(TARGET)"
	$(Q) $(RM) $(TARGET)
	$(E) "[RM] $(CHECKS)"
	$(Q) $(RM) $(CHECKS)
	$(E) "[RM] $(OBJECTS)"
	$(Q) $(RM) $(OBJECTS)
	$(E) "[RM] $(DEPENDS)"
	$(Q) $(RM) $(DEPENDS)
	$(E) "[RM] $(TESTOBJ)"
	$(Q) $(RM) $(TESTOBJ)
	$(E) "[RM] $(TESTDEP)"
	$(Q) $(RM) $(TESTDEP)
	$(E) "[RM] src/config.h"
	$(Q) $(RM) src/config.h

# Require processed config header
ifeq (,$(wildcard src/config.h))
$(error Please run ./configure before make)
endif

# Dependencies and build verbosity
include $(wildcard $(DEPENDS))
include $(wildcard $(TESTDEP))
ifeq ($(VERBOSE), true)
E = @true
else
E = @echo
Q = @
endif
