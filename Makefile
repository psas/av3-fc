INCLUDE_DIRS := -Idevices -I.
OPTSLIVE := -flto -O3 -D FCF_FC_NETWORK
OPTSDEV  := -flto -O3 -g
OPTSPROF := -O3 -pg
OPTS     := -ffast-math
WARNINGS := -Wall -Wextra -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
CFLAGS   := -MD -std=gnu99 $(OPTS) $(WARNINGS) -fno-strict-aliasing $(INCLUDE_DIRS)
LDLIBS   := -lrt -lm
.DEFAULT_GOAL := all
OBJECTS  += elderberry/fcfutils.o fcfmain.o elderberry/utils_sockets.o

MAINMIML ?= main.miml
MIMLMK   ?= miml.mk

-include $(MIMLMK)

all: miml fc

live: CFLAGS += $(OPTSLIVE)
live: miml fc

debug: CFLAGS  += $(OPTSDEV)
debug: LDFLAGS += $(OPTSDEV)
debug: miml fc


prof: CFLAGS  += $(OPTSPROF)
prof: LDFLAGS += $(OPTSPROF)
prof: miml fc


fc: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)


miml: $(MIMLMK)

$(MIMLMK): $(MAINMIML)
	python ./elderberry/codeGen.py -mch $^

clean:
	rm -f *.o *.d fc core
	rm -f $(MIMLMK) fcfmain.c fcfmain.h
