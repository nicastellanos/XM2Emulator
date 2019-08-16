# Targets & general dependencies
PROGRAM = xm2
HEADERS = 
OBJS = xm2.o
# add your additional object files here
ADD_OBJS = loader.o memory.o psw.o execute.o bus.o debugger.o

# compilers, linkers, utilities, and flags
CC = clang
CFLAGS = -g
COMPILE = $(CC) $(CFLAGS)
LINK = $(CC) $(CFLAGS) -o $@ 

# implicit rule to build .o from .c files
%.o: %.c $(HEADERS)
	$(COMPILE) -c -o $@ $<


# explicit rules
all: $(PROGRAM)

$(PROGRAM): $(OBJS) $(ADD_OBJS)
	$(LINK) $(OBJS) $(ADD_OBJS)



clean:
	rm -f *.o $(PROGRAM) 
