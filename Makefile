CC = gcc
FlAGS = -Wall -Wextra
FLAGS += -std=c11 -Werror
FLAGS += -g -O2 -march=native
LINKER = -lm

TEST_SRC := $(wildcard test/*.c)
TEST_OBJ := $(TEST_SRC:.c=.o)

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

OUTPUTDIR := target
TARGET := $(OUTPUTDIR)/pmcs
TEST := test/run_test 

.PHONY: all run test clean

all: $(TARGET) $(TEST)

run: $(TARGET)
	$^

test: $(TEST)
	$^

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TARGET) $(TEST)
	rm -rf target

%.o: %.c %.h
	$(CC) $(FLAGS) -c $< -o $@

# the leading '@' means don't echo the command
$(OUTPUTDIR):
	@mkdir -p $@

$(TEST): $(filter-out src/main.o,$(OBJ)) $(TEST_OBJ)
	gcc $(FLAGS) $^ -o $@

$(TARGET): $(OBJ) | $(OUTPUTDIR)
	$(CC) $(FLAGS) $(LINKER) $^ -o $@

