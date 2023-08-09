# SHELL ALLIASSES
RM := rm -rf

# LOCAL FUNCTIONS
define print_cc
	$(if $(Q), @echo "[CC]        $(1)")
endef

define print_bin
	$(if $(Q), @echo "[BIN]       $(1)")
endef

define print_rm
    $(if $(Q), @echo "[RM]        $(1)")
endef

# make V=1 --> verbose, otherwise silence
ifeq ("$(origin V)", "command line")
	Q :=
else
	Q ?= @
endif

# PROJECT STRUCTURE
SDIR := ./src
IDIR := ./include
ADIR := ./app

# FILES
SRC := $(wildcard $(SDIR)/*.c)    # Change .cpp to .c
ASRC := $(SRC) $(wildcard $(ADIR)/*.c)    # Change .cpp to .c
AOBJ := $(ASRC:%.c=%.o)    # Change .cpp to .c
OBJ := $(AOBJ)

DEPS := $(OBJ:%.o=%.d)

LIB :=

# EXEC
EXEC := main.out

# By default use gcc
CC ?= CC    # Change CXX to CC

CFLAGS := -std=c11    # Change CXXSTD to CFLAGS

DEP_FLAGS := -MMD -MP

H_INC := $(foreach d, $(IDIR), -I$d)
L_INC := $(foreach l, $(LIB), -l$l)
L_INC += -pthread    # Add pthread library

ifeq ($(CC),clang)    # Change CXX to CC
	CFLAGS += -Weverything -pedantic -Wno-c++98-compat -pedantic -ggdb3
else
	CFLAGS += -Wall -Wextra -Wcast-align -Wconversion -Wunreachable-code -Wuninitialized
endif

ifeq ("$(origin O)", "command line")
	OPT := -O$(O)
else
	OPT := -O3
endif

ifeq ("$(origin G)", "command line")
	GGDB := -ggdb$(G)
else
	GGDB :=
endif

CFLAGS += $(OPT) $(GGDB) $(DEP_FLAGS)

all: $(EXEC)

$(EXEC): $(AOBJ)
	$(call print_bin,$@)
	$(Q)$(CC) $(CFLAGS) $(H_INC) $(AOBJ) -o $@ $(L_INC)    # Change CXX to CC

%.o:%.c %.d    # Change .cpp to .c
	$(call print_cc,$<)
	$(Q)$(CC) $(CFLAGS) $(H_INC) -c $< -o $@    # Change CXX to CC

clean:
	$(call print_rm,EXEC)
	$(Q)$(RM) $(EXEC)
	$(call print_rm,OBJ)
	$(Q)$(RM) $(OBJ)
	$(call print_rm,DEPS)
	$(Q)$(RM) $(DEPS)

$(DEPS):

include $(wildcard $(DEPS))