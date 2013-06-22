# Wouldn't accept flto flag. Not sure why. Seems to work without it.
# OPTSLIVE := -flto -O3
OPTSLIVE := -O3 -D FCF_FC_NETWORK
OPTSDEV  := -g
OPTSPROF := -O3 -pg
OPTS     := -ffast-math
#WARNINGS := -Werror -Wall -Wextra -Wmissing-prototypes -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
WARNINGS := -Wall
CFLAGS   := -MD -std=gnu99 $(OPTS) $(WARNINGS) -fno-strict-aliasing $(shell pkg-config --cflags libusb-1.0)
LDLIBS   := -lrt $(shell pkg-config --libs libusb-1.0)
.DEFAULT_GOAL := all
DOXYFILE := ./Doxyfile
# Took out libusb object since it was causing error.
# OBJECTS  += fcfutils.o fcfmain.o utils_sockets.o utils_libusb-1.0.o
OBJECTS  += fcfutils.o fcfmain.o utils_sockets.o

MAINMIML ?= test.miml
MIMLMK   ?= miml.mk

-include $(MIMLMK)

all: fc

live: CFLAGS += $(OPTSLIVE)
live: fc

debug: CFLAGS  += $(OPTSDEV)
debug: LDFLAGS += $(OPTSDEV)
debug: fc


prof: CFLAGS  += $(OPTSPROF)
prof: LDFLAGS += $(OPTSPROF)
prof: fc


fc: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

miml: $(MIMLMK)

$(MIMLMK): $(MAINMIML)
	python ./codeGen.py -mch $^


.PHONY: html
html:
	rm -rf ./html
	doxygen $(DOXYFILE)

pdf:
	(grep -v GENERATE_LATEX $(DOXYFILE) ; echo GENERATE_LATEX = YES) | doxygen -
	cd ./latex && $(MAKE) pdf
	mv ./latex/refman.pdf .
	rm -rf ./latex

distclean: clean
	rm -rf refman.pdf #./html 


clean:
	rm -f *.o *.d fc core
	rm -f $(MIMLMK) fcfmain.c fcfmain.h
