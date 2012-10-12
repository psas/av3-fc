# Compiler flag detection based on similar logic in Kbuild
try-run = $(shell if ($(1)) >/dev/null 2>&1; then echo '$(2)'; else echo '$(3)'; fi)
cc-option = $(call try-run,$(CC) $(1) -S -xc /dev/null -o /dev/null,$(1),$(2))

OPTS := -O3 -ffast-math $(call cc-option,-flto -fwhole-program)
WARNINGS := -Werror -Wall -Wextra -Wmissing-prototypes -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
CFLAGS := -g -MD -std=gnu99 $(OPTS) $(WARNINGS) -fno-strict-aliasing $(shell pkg-config --cflags libusb-1.0)
LDLIBS := -lrt $(shell pkg-config --libs libusb-1.0)

all: fc

fc: libusb-util.o