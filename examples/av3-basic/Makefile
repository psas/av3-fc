OPTSLIVE := -O3 
OPTSDEV  := -g -pg
OPTS     := -ffast-math $(call cc-option,-flto -fwhole-program)
WARNINGS := -Werror -Wall -Wextra -Wmissing-prototypes -Wwrite-strings -Wno-missing-field-initializers -Wno-unused-parameter
CFLAGS := -MD -std=gnu99 $(OPTS) $(WARNINGS) -fno-strict-aliasing $(shell pkg-config --cflags libusb-1.0)
LDLIBS := -lrt $(shell pkg-config --libs libusb-1.0)

MIML  := mimltest.yaml
#MIML  := mimllive.yaml

#ifeq ($(MODE),LIVE)
#	CFLAGS += $(OPTSLIVE)
#else
#	CFLAGS += $(OPTSDEV)
#	LDFLAGS += $(OPTSDEV)
#	MODE := DEV
#endif


all: CFLAGS += $(OPTSLIVE)
all: fc
#	@echo "** compile: LIVE / $(MIML)"

debug: CFLAGS += $(OPTSDEV)
debug: LDFLAGS += $(OPTSDEV)
debug: fc
	@echo "** compile: DEBUG / $(MIML)"

fc: fc.o fcfutils.o logging.o usbutils.o theo-imu.o adis-imu.o gps.o mouse.o mouse2.o netutils.o virtdevsrv.o miml.o profile.o

#miml.h miml.c: $(MIML)
#	@echo "running code generator with MIML=$(MIML)"
#	./cg.sh $(MIML) miml.c

clean:
	rm -f *.o *.d fc core
