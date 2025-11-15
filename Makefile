CXX := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++23

TARGET_EXAMPLE := example
TARGET_TESTS := tests

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

TEST_ENTRY_SRC := fruit_picking_tests.cpp
EXAMPLE_ENTRY_SRC := fruit_picking_example.cpp

TEST_ENTRY_OBJ := $(TEST_ENTRY_SRC:.cpp=.o)
EXAMPLE_ENTRY_OBJ := $(EXAMPLE_ENTRY_SRC:.cpp=.o)

ENTRY_OBJ := $(TEST_ENTRY_OBJ) $(EXAMPLE_ENTRY_OBJ)
CORE_OBJ := $(filter-out $(ENTRY_OBJ), $(OBJ))

EXAMPLE_DEPENDS := $(EXAMPLE_ENTRY_OBJ) $(CORE_OBJ)
TEST_DEPENDS := $(TEST_ENTRY_OBJ) $(CORE_OBJ)


.PHONY: all clean tests example

all: $(TARGET_EXAMPLE) $(TARGET_TESTS)

$(TARGET_EXAMPLE): $(EXAMPLE_DEPENDS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET_TESTS): $(TEST_DEPENDS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET_EXAMPLE) $(TARGET_TESTS)