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
CPPFLAGS = -Isrc -Isrc/common -Ilib -pthread
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Werror
CFLAGS  += -D_POSIX_C_SOURCE=200809L
MDFLAGS  = -MM -MT $(@:.d=.o)
LDLIBS   = -lz -lpthread
LDFLAGS  =

BINDIR   = bin
TMPDIR   = $(BINDIR)/.build
TARGET   = $(BINDIR)/barkeeper
CHECKS   = $(BINDIR)/unit-test

CONFIG   = etc/barkeeper.conf
MANFILE  = doc/man/man8/barkeeper.8
DOCPATH  = /usr/share/man/man8
VARPATH  = /var/lib/barkeeper
BINPATH  = /usr/bin
ETCPATH  = /etc

SOURCES := $(shell find src -name "*.c")
TESTSRC := $(shell find tests -name "*.c")
OBJECTS := $(SOURCES:%.c=$(TMPDIR)/%.o)
DEPENDS := $(SOURCES:%.c=$(TMPDIR)/%.d)
TESTOBJ := $(TESTSRC:%.c=$(TMPDIR)/%.o)
TESTDEP := $(TESTSRC:%.c=$(TMPDIR)/%.d)

# Config script variables
-include etc/config.mk

.PHONY: default
default: $(DEFAULT)

.PHONY: all
all: $(TARGET) $(CHECKS)

.PHONY: run
run: $(TARGET)
	$(Q) sudo ./$< -v

.PHONY: check
check: $(CHECKS)
	$(Q) ./$<

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

.PHONY: install
install: $(TARGET) check-root install-docs
	$(E) "[INSTALL] $(BINPATH)/barkeeper"
	$(Q) $(INSTALL) -m 0755 $(TARGET) $(BINPATH)
	$(E) "[INSTALL] $(ETCPATH)/barkeeper.conf"
	$(Q) $(INSTALL) -m 0777 $(CONFIG) $(ETCPATH)
	$(E) "[INSTALL] $(VARPATH)/http"
	$(Q) $(INSTALL) -d $(VARPATH)
	$(Q) $(INSTALL) -m 0777 etc/http/* $(VARPATH)

.PHONY: install-docs
install-docs: $(MANFILE) check-root
	$(E) "[INSTALL] $(DOCPATH)/barkeeper.8"
	$(Q) $(INSTALL) -m 0644 $(MANFILE) $(DOCPATH)
	$(Q) $(GZIP) $(DOCPATH)/barkeeper.8

.PHONY: check-root
check-root:
ifneq ($(shell id -u), 0) # Check for root permissions
	$(E) "You must be root to perform this action."
	$(Q) exit 1
else
	$(Q) true
endif

.PHONY: clean
clean: # Highwaaay to the danger zone
	$(E) "[RM] $(TARGET)"
	$(Q) $(RM) $(TARGET)
	$(E) "[RM] $(CHECKS)"
	$(Q) $(RM) $(CHECKS)
	$(E) "[RM] $(TMPDIR)"
	$(Q) $(RM) -r $(TMPDIR)
	$(E) "[RM] src/config.h"
	$(Q) $(RM) src/config.h
	$(E) "[RM] etc/config.mk"
	$(Q) $(RM) etc/config.mk

# Require processed config file
ifeq (,$(wildcard etc/config.mk))
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
