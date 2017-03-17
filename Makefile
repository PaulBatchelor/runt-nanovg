default: hello
CFLAGS = -O3 -ffast-math -fPIC -Inanovg -I. -Wall 
CFLAGS += -Wno-implicit-function-declaration
LDFLAGS = -lm -lGL -lglfw -lrunt -ldl -lpthread

NAME=vg

OBJ = nanovg/nanovg.o vg.o

default: librunt_vg.a hello rnt_$(NAME)

%.o: %.c
	$(CC) $^ -c $(CFLAGS) -shared -fPIC -o $@ 

librunt_$(NAME).a: $(OBJ)
	$(AR) rcs $@ $(OBJ) 

rnt_$(NAME): parse.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ parse.c $(OBJ) $(LDFLAGS)

hello: example/hello.c $(OBJ)
	$(CC) $(CFLAGS) example/hello.c -o hello $(OBJ) $(LDFLAGS)

install: librunt_$(NAME).a
	mkdir -p ~/.runt/lib
	mkdir -p ~/.runt/bin
	cp $< ~/.runt/lib
	cp rnt_$(NAME) ~/.runt/bin/

clean: 
	rm -rf librunt_$(NAME).a hello rnt_$(NAME) $(OBJ)
