INCLUDE_DIRS := -Isrc -Isrc/devices -Isrc/utilities -I.
OPTSLIVE := -flto -D FCF_FC_NETWORK
OPTSDEV  := -g
OPTSPROF := -pg
OPTS     := -Ofast
WARNINGS := -Wall -Wextra -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
MIML_FLAGS := -DMIML_INIT= -DMIML_FINAL= -DMIML_SENDER= -DMIML_RECEIVER=
CFLAGS   := -MD $(OPTS) $(WARNINGS) -fno-strict-aliasing $(INCLUDE_DIRS) $(MIML_FLAGS)
LDLIBS   := -lrt -lm -lev
.DEFAULT_GOAL := all
OBJECTS  += fcfmain.o

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
	python3 ./elderberry/codeGen.py -mcb $^

cleanbuild:
	rm `find . -name '*.o'` -f
	rm `find . -name '*.d'` -f

clean: cleanbuild
	rm -f $(MIMLMK) fcfmain.c fcfmain.h
	rm -f fc
