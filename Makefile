CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

TARGET = interpreter

SRCS = main.cpp ignoreComments.cpp Tokenization.cpp Parser.cpp AST.cpp Interpreter.cpp SymbolTable.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp ignoreComments.h Tokenization.h Parser.h AST.h Node.h Interpreter.h SymbolTable.h
ignoreComments.o: ignoreComments.cpp ignoreComments.h
Tokenization.o: Tokenization.cpp Tokenization.h
Parser.o: Parser.cpp Parser.h Tokenization.h Node.h
AST.o: AST.cpp AST.h Node.h
Interpreter.o: Interpreter.cpp Interpreter.h AST.h Node.h SymbolTable.h
SymbolTable.o: SymbolTable.cpp SymbolTable.h Node.h

clean:
	rm -f $(TARGET) $(OBJS) test_symbol_table test_symbol_table.o