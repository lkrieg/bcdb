.POSIX:
.DELETE_ON_ERROR:
.SUFFIXES:

CC       = gcc
LD       = $(CC)
RM       = rm -f
TEST     = test
MKDIR    = mkdir -p
CPPFLAGS = -Isrc -Ilib
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Werror
CFLAGS  += -D_POSIX_C_SOURCE=200809L -DHAVE_ZLIB
MDFLAGS  = -MM -MT $(@:.d=.o)
LDFLAGS  = -Llib/zlib
LDLIBS   = -lz -lncurses
VERBOSE  = false

DSTDIR   = bin
TARGET  := $(DSTDIR)/barkeeper
TMPDIR  := $(DSTDIR)/.build
SOURCES := $(shell find src -name "*.c")
OBJECTS := $(SOURCES:%.c=$(TMPDIR)/%.o)
DEPENDS := $(SOURCES:%.c=$(TMPDIR)/%.d)

.PHONY: run
run: all
	sudo ./$(TARGET)

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS) $(DEPENDS)
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
	$(E) "[RM] $(OBJECTS)"
	$(Q) $(RM) $(OBJECTS)
	$(E) "[RM] $(DEPENDS)"
	$(Q) $(RM) $(DEPENDS)

# Dependencies and build verbosity
include $(wildcard $(DEPENDS))
ifeq ($(VERBOSE), true)
E = @true
else
E = @echo
Q = @
endif
