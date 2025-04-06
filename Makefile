# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17

# Target executable
TARGET = main

# Source files
SRCS = main.cpp

# Header files
HEADERS = Verbose.h \
		  Token.h \
		  Lexer.h \
		  Parser.h \
		  SemanticAnalyzer.h \
		  Tree.h \
		  Node.h \
		  HashNode.h \
		  SymbolTable.h

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default rule to build and run the executable
all: $(TARGET) run

# Rule to link object files into the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .cpp files into .o files, adding header dependencies
main.o: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

# Rule to run the executable
run: $(TARGET)
	./$(TARGET) ${FILE}

# Clean rule to remove generated files
# Uses del if on windows, uses rm on Unix-like systems (and Git Bash)
clean: 
	rm -f main $(OBJS) || del main.exe $(OBJS)

# For Valgrind
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) ${FILE}