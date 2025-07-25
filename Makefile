CC 				:= gcc
PROGRAM 	:= a.out

SRC_DIR 	:= src
BIN_DIR 	:= bin
LIB_DIR 	:= lib

WARNINGS 	:= -Wall -Wextra -Wshadow -Wstrict-prototypes
WARNINGS 	+= -Wfloat-equal -Wmissing-declarations -Wmissing-include-dirs
WARNINGS 	+= -Wmissing-prototypes -Wredundant-decls -Wunreachable-code
CFLAGS 		:= $(WARNINGS) -g -MMD -MP `sdl2-config --cflags`

INCFLAGS 	:= $(addprefix -I,$(LIB_DIR))
LDFLAGS 	:= `sdl2-config --libs` -lm -lglfw

SRC_FILES := $(shell find $(SRC_DIR) $(LIB_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(LIB_DIR)/%.c,$(BIN_DIR)/%.o, \
              $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC_FILES)))
DEP_FILES := $(patsubst %.o,%.d,$(OBJ_FILES))

all: $(PROGRAM)

$(PROGRAM): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(PROGRAM) $(LDFLAGS)
	cp -R /Users/mdurcan/personal/git_projects/tools/my-libraries/c-lib src/

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(PROGRAM)

rebuild: clean all

-include $(DEP_FILES)

.PHONY: all clean rebuild
