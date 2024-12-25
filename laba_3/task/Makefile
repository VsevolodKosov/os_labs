CC = gcc

CFLAGS = -Wall -Werror

OUT_DIR = out

TARGETS = $(OUT_DIR)/parent $(OUT_DIR)/child1 $(OUT_DIR)/child2

OBJS = parent.o child1.o child2.o

all: $(TARGETS)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_DIR)/parent: parent.o | $(OUT_DIR)
	$(CC) -o $@ parent.o

$(OUT_DIR)/child1: child1.o | $(OUT_DIR)
	$(CC) -o $@ child1.o

$(OUT_DIR)/child2: child2.o | $(OUT_DIR)
	$(CC) -o $@ child2.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) $(OBJS)

distclean: clean
	rm -f *.o

mrproper: distclean
	rm -rf $(OUT_DIR)