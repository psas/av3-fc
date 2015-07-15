INCLUDE_DIRS := -Isrc -Isrc/devices -Isrc/utilities -I.
OPTSLIVE := -flto -D FCF_FC_NETWORK
OPTSDEV  := -flto
OPTSPROF := -pg
OPTS     := -Ofast
WARNINGS := -Wall -Wextra -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
CFLAGS   := -MD -std=gnu99 -g $(OPTS) $(WARNINGS) -fno-strict-aliasing $(INCLUDE_DIRS)
LDFLAGS  := -g
LDLIBS   := -lrt -lm
.DEFAULT_GOAL := all
OBJECTS  += elderberry/fcfutils.o fcfmain.o

MAINMIML ?= main.miml
MIMLMK   ?= miml.mk

-include $(MIMLMK)

all: debug

live: CFLAGS += $(OPTSLIVE)
live: LDFLAGS += $(OPTSLIVE)
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

packet:
	gen-psas-types > src/utilities/psas_packet.h

cleanbuild:
	rm `find . -name '*.o'` -f
	rm `find . -name '*.d'` -f

clean: cleanbuild
	rm -f $(MIMLMK) fcfmain.c fcfmain.h
	rm -f fc
