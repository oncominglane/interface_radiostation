CXX = g++

BUILDDIR = build
SRCDIR   = src
ASSETDIR = assets

DBGFLAGS = -g -O0 -Wall -Wextra -D_DEBUG -D_SHOW_STACK_TRACE
CXXFLAGS = $(DBGFLAGS) -Iinclude -std=c++17 -pthread 
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system -lX11 -lasound

SRC = TxEth.cpp RxEth.cpp buttons.cpp message.cpp audioTxEth.cpp project_thread.cpp
OBJ = $(SRC:.cpp=.o)

.PHONY: all clean prepare
all: prepare main

$(addprefix $(BUILDDIR)/, $(OBJ)): $(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
		@echo [CC] $< -o $@
		@$(CXX) $(CXXFLAGS) -c $(realpath $<) -o $@

main: $(addprefix $(BUILDDIR)/, $(OBJ))
		@echo [CC] $^ -o $@
		@$(CXX) $(CXXFLAGS) $^ -o $(addprefix $(BUILDDIR)/, $@) $(LDFLAGS)

clean:
		rm -rf $(BUILDDIR)

prepare:
		@mkdir -p $(BUILDDIR)
		@cp -r $(ASSETDIR) $(BUILDDIR) 