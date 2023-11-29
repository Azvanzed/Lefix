CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++23
SRCDIR := src
OBJDIR := obj
BINDIR := bin
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET := $(BINDIR)/compiler

all: $(TARGET) run

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	@$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-@rm -rf $(OBJDIR) $(BINDIR)

run:
	-@./$(TARGET) || true

.PHONY: all run