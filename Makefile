CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET := table
SRCS   := main.cpp ignoreComments.cpp Tokenization.cpp Parser.cpp SymbolTable.cpp
OBJS   := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean