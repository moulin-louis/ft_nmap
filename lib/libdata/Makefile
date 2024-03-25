NAME=libdata.a
SH_NAME=libdata.so

ARRAY_TEST_BIN=array_test

INCLUDE_PATHS=inc
C_STANDARD=gnu17

CC=gcc
CFLAGS=-Wall -Werror -Wextra -std=$(C_STANDARD) $(addprefix -I, $(INCLUDE_PATHS)) -MMD

SRC_DIR=src
OBJ_DIR=build

SRC=$(wildcard $(SRC_DIR)/array/*.c $(SRC_DIR)/array/impl/*.c $(SRC_DIR)/tools/*.c)
OBJ=$(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP=$(OBJ:%.o=%.d)

DEBUG?=false

ifeq ($(DEBUG),true)
	CFLAGS+=-g3
else ifeq ($(DEBUG),false)
	CFLAGS+=-O3 -DNDEBUG
endif

all: $(NAME)

-include $(DEP)

shared: $(SH_NAME)

debug:
	$(MAKE) --no-print-directory DEBUG=true

$(ARRAY_TEST_BIN): array_test.c $(NAME)
	$(CC) $< $(NAME) -Wall -Werror -Wextra -o $@ -L. -ldata -lcriterion -lm

test: debug $(ARRAY_TEST_BIN)
	./$(ARRAY_TEST_BIN)

$(NAME): $(OBJ)
	ar rcs $@ $^

$(SH_NAME): $(OBJ)
	$(CC) -shared -o $@ $^ -lm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(SH_NAME)

re: fclean all

.PHONY: all debug test clean fclean re
