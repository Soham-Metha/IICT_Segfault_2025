BUILDS := ./build

CC	   := gcc
CFLAGS := -Wall -Wextra -Werror -Wfatal-errors -Wswitch-enum -pedantic -O3 -std=c2x
LIBS   := -lncursesw -I ./include

$(BUILDS):
	@mkdir -p $@

clean: | $(BUILDS)
	@rm -f $(BUILDS)/*
	@printf  "\n\e[36m  CLEANED ALL OBJECT FILES AND EXECUTABLES	\e[0m\n\n"

define BUILD_RULE
$1: $2 | $(BUILDS)
	@$(CC) $$^ $(CFLAGS) $(LIBS) -o $$@
	@printf "\e[32m		[ BUILD COMPLETED ]\t: [ $$@ ] \e[0m\n\n"
endef

MAIN_FILE := ./src/main.c
EXEC_FILE := $(BUILDS)/a
SRC_FILES := $(wildcard ./src/Utils/*.c)
SRC_FILES += $(wildcard ./src/Backend/*.c)
SRC_FILES += $(wildcard ./src/Frontend/*.c)
SRC_FILES += $(wildcard ./src/Middleend/*.c)
SRC_FILES += $(wildcard ./src/Wrapper/*.c)

$(eval $(call BUILD_RULE, $(EXEC_FILE), $(MAIN_FILE) $(SRC_FILES)))

all: $(EXEC_FILE)

IN_FILE := examples/helloWorld.iict
OUT_FILE:= examples/exampleOut

run_all: all
	@$(EXEC_FILE) -i $(IN_FILE) -o $(OUT_FILE)
