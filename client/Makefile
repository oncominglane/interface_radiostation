CXX = g++

BUILDDIR = build
SRCDIR   = src
ASSETDIR = assets

DBGFLAGS = -g -O0 -Wall -Wextra -D_DEBUG -D_SHOW_STACK_TRACE
CXXFLAGS = $(DBGFLAGS) -Iinclude -std=c++17 -pthread 
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system -lX11 -lasound

SRC = main_old.cpp buttons.cpp lamps.cpp message.cpp TxEth.cpp RxEth.cpp audioTxEth.cpp audioRxEth.cpp 
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