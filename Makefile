CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

TARGET = ast_program

SRCS = main.cpp ignoreComments.cpp Tokenization.cpp Parser.cpp AST.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp ignoreComments.h Tokenization.h Parser.h AST.h Node.h
ignoreComments.o: ignoreComments.cpp ignoreComments.h
Tokenization.o: Tokenization.cpp Tokenization.h
Parser.o: Parser.cpp Parser.h Tokenization.h Node.h
AST.o: AST.cpp AST.h Node.h

clean:
	rm -f $(TARGET) $(OBJS)