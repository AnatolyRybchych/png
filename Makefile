CC = gcc
CCARGS = -c -g -Wpedantic -Wall
out = run.exe

$(out): objects/main.o objects/png.o
	$(CC) -o $@ -g $+

objects/main.o: src/main.c
	$(CC) -o $@ $(CCARGS) $<

objects/png.o: src/png.c src/include/png.h
	$(CC) -o $@ $(CCARGS) $<


#make args

gdb: $(out)
	@gdb $(out)

run: $(out)
	@$(out)