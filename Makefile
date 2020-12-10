.POSIX:
.DELETE_ON_ERROR:
.SUFFIXES:

CC       = gcc
LD       = $(CC)
RM       = rm -f
GZIP     = gzip
TEST     = test
INSTALL  = install
MKDIR    = mkdir -p

VERBOSE  = false
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
DOCPATH := doc/man/man8/barkeeper.8
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

.PHONY: install
	$(E) "Not implemented"

.PHONY: install-docs
install-docs: $(DOCPATH)
	$(Q) $(INSTALL) -m 0644 $< /usr/share/man/man8/
	$(Q) $(GZIP) /usr/share/man/man8/barkeeper.8

$(TARGET): $(OBJECTS) $(DEPENDS)
	$(E) "[LD] $@"
	$(Q) $(LD) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)

$(CHECKS): $(TESTOBJ) $(TESTDEP)
	$(E) "[LD] $@"
	$(Q) $(LD) -o $@ $(LDFLAGS) $(TESTOBJ) $(LDLIBS)

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
	$(E) "[RM] src/common/config.h"
	$(Q) $(RM) src/common/config.h

# Require processed config header
ifeq (,$(wildcard src/common/config.h))
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
