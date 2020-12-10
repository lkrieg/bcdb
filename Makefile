.POSIX:
.DELETE_ON_ERROR:
.SUFFIXES:

CC       = gcc
LD       = $(CC)
RM       = rm -f
TEST     = test
INSTALL  = install
MKDIR    = mkdir -p
GZIP     = gzip -f

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
all: $(TARGET)

.PHONY: run
run: $(TARGET)
	$(E) "[RUN]\t$<"
	$(Q) ./$<

.PHONY: check
check: $(CHECKS)
	$(E) "[RUN]\t$<"
	$(Q) ./$<

$(TARGET): $(OBJECTS) $(DEPENDS)
	$(E) "[LD]\t$@"
	$(Q) $(LD) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)

$(CHECKS): $(TESTOBJ) $(TESTDEP)
	$(E) "[LD]\t$@"
	$(Q) $(LD) -o $@ $(LDFLAGS) $(TESTOBJ) $(LDLIBS)

$(TMPDIR)/%.o: %.c # OBJECTS
	$(E) "[CC]\t$<"; $(TEST) -d $(@D) || $(MKDIR) $(@D)
	$(Q) $(CC) -c -o $@ $(CFLAGS) $(CPPFLAGS) $<

$(TMPDIR)/%.d: %.c # DEPENDS
	$(Q) $(TEST) -d $(@D) || $(MKDIR) $(@D)
	$(Q) $(CC) $(CFLAGS) $(CPPFLAGS) $< $(MDFLAGS) > $@

.PHONY: install
install:
	$(E) "Error: Target not implemented."
	$(Q) exit 1

.PHONY: install-docs
install-docs: $(DOCPATH)
	$(Q) $(INSTALL) -m 0644 $< /usr/share/man/man8/
	$(Q) $(GZIP) /usr/share/man/man8/barkeeper.8

.PHONY: clean
clean:
	$(E) "[RM]\t$(TARGET)"
	$(Q) $(RM) $(TARGET)
	$(E) "[RM]\t$(CHECKS)"
	$(Q) $(RM) $(CHECKS)
	$(E) "[RM]\t$(TMPDIR)"
	$(Q) $(RM) -r $(TMPDIR)
	$(E) "[RM]\tsrc/common/config.h"
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
