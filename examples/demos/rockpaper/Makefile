OPTSLIVE := -flto -O3 
OPTSDEV  := -g
OPTSPROF := -flto -O3 -pg
OPTS     := -ffast-math
#WARNINGS := -Werror -Wall -Wextra -Wmissing-prototypes -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
WARNINGS := -Wall
CFLAGS   := -MD -std=gnu99 $(OPTS) $(WARNINGS) -fno-strict-aliasing $(shell pkg-config --cflags libusb-1.0)
LDLIBS   := -lrt $(shell pkg-config --libs libusb-1.0)
.DEFAULT_GOAL := all
OBJECTS  += fcfutils.o fcfmain.o utils_libusb-1.0.o

MAINMIML ?= Main.miml
MIMLMK   ?= miml.mk

-include $(MIMLMK)

all: CFLAGS += $(OPTSLIVE)
all: fc

debug: CFLAGS  += $(OPTSDEV)
debug: LDFLAGS += $(OPTSDEV)
debug: fc


prof: CFLAGS  += $(OPTSPROF)
prof: LDFLAGS += $(OPTSPROF)
prof: fc


fc: $(OBJECTS)

miml: $(MIMLMK)

$(MIMLMK): $(MAINMIML)
	./codeGen.py -mch $^


clean:
	rm -f *.o *.d fc core
	rm -f $(MIMLMK) fcfmain.c fcfmain.h #also remove auto-generated files
