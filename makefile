SRCS =src/mainA.cpp src/assemblerHelp.cpp parser.c lexer.c
CC = g++
SRCL=src/mainL.cpp src/linkerHelper.cpp
SRCE=src/mainE.cpp src/emulatorHelper.cpp

emulator: $(SRCE) inc/emulatorHelper.hpp
	$(CC) $(SRCE) -g -o emulator

linker:$(SRCL) inc/linkerHelper.hpp
	$(CC) $(SRCL) -g -o linker

# if we compile them all together, we should get out what we want!
assembler: $(SRCS) inc/assemblerHelp.hpp
	$(CC) $(SRCS) -g -o assembler

# to regenerate the lexer, we call `flex` on it, which will
# create the lexer.c and lexer.h files
lexer.c: misc/lexer.l inc/assemblerHelp.hpp
	flex misc/lexer.l

# to regenerate the parser, we call `bison` on it, which will
# create the bison.c and bison.h files
parser.c: misc/parser.y misc/lexer.l inc/assemblerHelp.hpp
	bison misc/parser.y

clean:
	rm -rf *.o lexer.c lexer.h parser.c parser.h parser *.hex assembler linker emulator
