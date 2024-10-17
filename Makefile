CFLAGS := -std=c99 -g -Og -Werror -Wall -Wpedantic
EXE := lib/main
OBJS := obj/main.o
CC := gcc

$(EXE): $(OBJS) lib
	$(CC) -Iinc/ $(CFLAGS) -o $(EXE) $(OBJS)

obj/%.o: src/%.c obj
	gcc -c -Iinc/ $(CFLAGS) -o $@ $<

obj:
	mkdir obj

lib:
	mkdir lib

.PHONY: run
run: $(EXE)
	./$(EXE)

.PHONY: clean
clean:
	del /q "obj/"
	rmdir obj
	del /q "lib/"
	rmdir lib
