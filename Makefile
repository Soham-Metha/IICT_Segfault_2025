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

SRC_DIR   := ./src
MAIN_FILE := $(SRC_DIR)/main.c
EXEC_FILE := $(BUILDS)/a
SRC_FILES := $(filter-out $(MAIN_FILE), 	$(wildcard $(SRC_DIR)/*.c) )

$(eval $(call BUILD_RULE, $(EXEC_FILE), $(MAIN_FILE) $(SRC_FILES)))

all: $(EXEC_FILE)

run_all: all
	@$(EXEC_FILE)
