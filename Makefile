default: hello
CFLAGS = -O3 -ffast-math -fPIC -Inanovg -I.
LDFLAGS = -lm -lGL -lglfw -lrunt -ldl -lpthread

OBJ = nanovg/nanovg.o vg.o

default: vg.so hello 

%.o: %.c
	$(CC) $^ -c $(CFLAGS) -o $@ 

vg.so: $(OBJ)
	ld $(CFLAGS) -shared -fPIC -o $@ $(OBJ) $(LDFLAGS)

hello: example/hello.c nanovg/nanovg.o vg.o
	$(CC) $(CFLAGS) example/hello.c -o hello $(OBJ) $(LDFLAGS)

clean: 
	rm -rf vg.so hello $(OBJ)
