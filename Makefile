#
#
#
# Makefile template for C code
#
# Author: Gustavo Pantuza Coelho Pinto
# Since: 24.03.2016
#
#
#


# Includes the project configurations
include project.conf

#
# Validating project variables defined in project.conf
#
ifndef PROJECT_NAME
$(error Missing PROJECT_NAME. Put variables at project.conf file)
endif
ifndef BINARY
$(error Missing BINARY. Put variables at project.conf file)
endif
ifndef PROJECT_PATH
$(error Missing PROJECT_PATH. Put variables at project.conf file)
endif


# Gets the Operating system name
OS := $(shell uname -s)

# Default shell
SHELL := bash

# Color prefix for Linux distributions
COLOR_PREFIX := e

ifeq ($(OS),Darwin)
	COLOR_PREFIX := 033
endif

# Color definition for print purpose
BROWN=\$(COLOR_PREFIX)[0;33m
BLUE=\$(COLOR_PREFIX)[1;34m
END_COLOR=\$(COLOR_PREFIX)[0m



# Source code directory structure
BINDIR := make/bin
SRCDIR := src
LOGDIR := make/logs
LIBDIR := make/lib
TESTDIR := test
UTILSDIR = $(SRCDIR)/utils

ifeq ($(OS),Windows_NT)
    # define CPLEXDIR
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CPLEXDIR := /opt/ibm/ILOG/CPLEX_Studio2211/cplex
    endif
    ifeq ($(UNAME_S),Darwin)
        CPLEXDIR := /Applications/CPLEX_Studio2211/cplex
    endif
endif


# Source code file extension
SRCEXT := c


# Defines the C Compiler
CC := gcc


# Defines the language standards for GCC
STD := -std=gnu99 # See man gcc for more options

# Protection for stack-smashing attack
STACK := -fstack-protector-all -Wstack-protector

# Specifies to GCC the required warnings
WARNS := -Wall -Wextra -pedantic # -pedantic warns on language standards

# Flags for compiling
CFLAGS := -O3 $(STD) $(STACK) $(WARNS)

# Debug options
DEBUG := -g3 -DDEBUG=1

# Dependency libraries

ifeq ($(OS),Windows_NT)
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        # define libs
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            # define libs
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            # define libs
        endif
    endif
else
    UNAME_S := $(shell uname -s)
	UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_S),Linux)
        LIBS := -L${CPLEXDIR}/lib/x86-64_linux/static_pic -L/home/ricky/concorde -L. -lcplex -lm -lpthread -ldl -lconcorde
    endif
    ifeq ($(UNAME_S),Darwin)
		ifeq ($(UNAME_P),x86_64)
        	LIBS := -L${CPLEXDIR}/lib/x86_64_osx/static_pic -L. -lcplex -lm -lpthread -ldl
    	endif
		ifneq ($(filter arm%,$(UNAME_P)),)
        	LIBS := -L${CPLEXDIR}/lib/arm64_osx/static_pic -L. -lcplex -lpthread -ldl
    	endif
    endif
endif

INC := -I. -I$(CPLEXDIR)/include/ilcplex

# Test libraries
TEST_LIBS := -l cmocka -L /usr/local/lib -rpath /usr/local/lib

# Tests binary file
TEST_BINARY := $(BINARY)_test_runner


# Find all .c files in SRCDIR and its subdirectories
SRCS := $(wildcard $(SRCDIR)/**/*.c)
SRCS += $(wildcard $(SRCDIR)/*.c)

# Extract the names of the files with their relative paths
NAMES := $(patsubst $(SRCDIR)/%.$(SRCEXT),%,$(SRCS))

# Generate object file paths
OBJECTS := $(patsubst %,$(LIBDIR)/%.o,$(NAMES))


#
# COMPILATION RULES
#

default: all

# Help message
help:
	@echo "C Project Template"
	@echo
	@echo "Target rules:"
	@echo "    all      - Compiles and generates binary file"
	@echo "    tests    - Compiles with cmocka and run tests binary file"
	@echo "    start    - Starts a new project using C project template"
	@echo "    valgrind - Runs binary file using valgrind tool"
	@echo "    clean    - Clean the project by removing binaries"
	@echo "    help     - Prints a help message with target rules"

# Starts a new project using C project template
start:
	@echo "Creating project: $(PROJECT_NAME)"
	@mkdir -pv $(PROJECT_PATH)
	@echo "Copying files from template to new directory:"
	@cp -rvf ./* $(PROJECT_PATH)/
	@echo
	@echo "Go to $(PROJECT_PATH) and compile your project: make"
	@echo "Then execute it: bin/$(BINARY) --help"
	@echo "Happy hacking o/"


all: $(OBJECTS)
	@echo -en "$(BROWN)LD $(END_COLOR)";
	$(CC) -o $(BINDIR)/$(BINARY) $+ $(DEBUG) $(CFLAGS) $(LIBS)


$(LIBDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo -en "$(BROWN)CC $(END_COLOR)";
	$(CC) -c $< -o $@ $(DEBUG) $(CFLAGS) $(INC)


# Rule for run valgrind tool
valgrind:
	valgrind \
		--track-origins=yes \
		--leak-check=full \
		--show-leak-kinds=all \
		--leak-resolution=high \
		--log-file=$(LOGDIR)/$@.log \
		$(BINDIR)/$(BINARY) $(ARGS)
	@echo -en "\nCheck the log file: $(LOGDIR)/$@.log\n"


# Compile tests and run the test binary
tests:
	@echo -en "$(BROWN)CC $(END_COLOR)";
	$(CC) $(TESTDIR)/main.c $(shell find src ! -name 'main.c' -type f -name "*.c") -I $(SRCDIR) -o $(BINDIR)/$(TEST_BINARY) $(DEBUG) $(CFLAGS) $(LIBS) $(TEST_LIBS)
	@which ldconfig && ldconfig -C /tmp/ld.so.cache || true # caching the library linking
	@echo -en "$(BROWN) Running tests: $(END_COLOR)";
	./$(BINDIR)/$(TEST_BINARY)


# Rule for cleaning the project
clean:
	@rm -rvf $(BINDIR)/* $(LIBDIR)/utils/* $(LIBDIR)/algorithms/* $(LIBDIR)/*.o $(LOGDIR)/*;
