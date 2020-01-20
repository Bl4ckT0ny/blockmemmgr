CFLAGS = -g -Wall -Wextra -Werror -std=c99
CFLAGS += -D_POSIX_C_SOURCE=199309L -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE
CFLAGS += $(shell pkg-config --cflags check)
LDLIBS += $(shell pkg-config --libs check)

TARGET_SOURCES:=$(wildcard test_*.c)
TARGET_NAMES:=$(TARGET_SOURCES:.c=)

# include sources
CFLAGS += -I../src

# configure memory manager

CFLAGS += -DMEMMGR_CHECK_OVERFLOW=1

# define target-specific flags
CFLAGS-test_ThreadSafe = -DMEMMGR_THREAD_SAFE=1
CFLAGS-test_StaticPool = -DMEMMGR_USE_STATIC_POOL=1 -DMEMMGR_STATIC_POOL_SIZE=1024


test-outdir = ./$1-obj
test-target = ./$1-obj/$1

define pkg-test-rules

$(call test-target,$1): $(call test-target,$1).o $(call test-outdir,$1)/memmgr.o 

$(call test-outdir,$1)/memmgr.o: ../src/memmgr.c | $(call test-outdir,$1)
	$(CC) $(CFLAGS) $(CFLAGS-$1) -c -o $$@ $$<

$(call test-target,$1).o: ./$1.c | $(call test-outdir,$1)
	$(CC) $(CFLAGS) $(CFLAGS-$1) -c -o $$@ $$<

$1: $(call test-target,$1)

$(call test-outdir,$1):
	@echo Creating directory $$@
	@mkdir $$@

clean-$1:
	rm -rf $(call test-outdir,$1)
test-$1: $1
	./$(call test-target,$1)
test: test-$1
all: $1
clean: clean-$1
.PHONY: $1 clean-$1 test-$1
endef


add-test-pkg = $(eval $(call pkg-test-rules,$1))
$(foreach p,$(TARGET_NAMES),$(call add-test-pkg,$p))


.PHONY: all clean test

.DEFAULT_GOAL=all

