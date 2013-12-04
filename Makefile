INCLUDE_DIRS := -Isrc -Isrc/devices -Isrc/utilities -I.
OPTSLIVE := -flto -O3 -D FCF_FC_NETWORK
OPTSDEV  := -flto -O3 -g
OPTSPROF := -O3 -pg
OPTS     := -ffast-math
WARNINGS := -Wall -Wextra -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
CFLAGS   := -MD -std=gnu99 $(OPTS) $(WARNINGS) -fno-strict-aliasing $(INCLUDE_DIRS)
LDLIBS   := -lrt -lm
.DEFAULT_GOAL := all
OBJECTS  += elderberry/fcfutils.o fcfmain.o

MAINMIML ?= main.miml
MIMLMK   ?= miml.mk

-include $(MIMLMK)

all: debug

live: CFLAGS += $(OPTSLIVE)
live: fc

debug: CFLAGS  += $(OPTSDEV)
debug: LDFLAGS += $(OPTSDEV)
debug: fc

prof: CFLAGS  += $(OPTSPROF)
prof: LDFLAGS += $(OPTSPROF)
prof: fc

fc: miml $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LOADLIBES) $(LDLIBS)

miml: $(MIMLMK)

$(MIMLMK): $(MAINMIML)
	python ./elderberry/codeGen.py -mch $^

cleanbuild:
	rm `find . -name '*.o'` -f
	rm `find . -name '*.d'` -f

clean: cleanbuild
	rm -f $(MIMLMK) fcfmain.c fcfmain.h
	rm -f fc
