###### CONFIGURATION ######

### List of applications to be build
APPLICATIONS = temperature # exemple : APPLICATIONS = test_TachePer

###### USER SPACE BUILD (no change required normally) ######
ifeq ($(KERNELRELEASE),)
ifneq ($(APPLICATIONS),)

### Default Xenomai installation path
XENO ?= /usr/xenomai

XENOCONFIG=$(shell PATH=$(XENO):$(XENO)/bin:$(PATH) which xeno-config 2>/dev/null)

### Sanity check
ifeq ($(XENOCONFIG),)
all::
	@echo ">>> Invoke make like this: \"make XENO=/path/to/xeno-config\" <<<"
	@echo
endif

CC=$(shell $(XENOCONFIG) --cc)

CFLAGS=$(shell $(XENOCONFIG) --xeno-cflags) $(MY_CFLAGS)

LDFLAGS=$(shell $(XENOCONFIG) --xeno-ldflags) $(MY_LDFLAGS) -lnative

# This includes the library path of given Xenomai into the binary to make live
# easier for beginners if Xenomai's libs are not in any default search path.
LDFLAGS+=-Xlinker -rpath -Xlinker $(shell $(XENOCONFIG) --libdir)

all:: $(APPLICATIONS)

clean::
	$(RM) $(APPLICATIONS) *.o

endif
endif


