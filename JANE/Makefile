all: JANEcompilator

OBJECTS = lexer.o interpret.o generator.o

INCLUDE =
LIBDIR =
LIBRARIES =

lexer.o: lexer.c lexer.h
	cc -c -o lexer.o lexer.c

generator.o: generator.c generator.h
	cc -c -o generator.o generator.c

interpret.o: interpret.c
	cc -c -o interpret.o interpret.c

JANEcompilator: $(OBJECTS)
	cc -o JANEcompilator $(OBJECTS) $(LIBDIR) $(LIBRARIES)

clean:; rm -f *.o JANEcompilator
run:; JANEcompilator $(args)
