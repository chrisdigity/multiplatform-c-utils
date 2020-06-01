###
# Testing makefile to compile and test all *.c files.
#  - makefile (31 May 2020)
#
###
# Original work Copyright (c) 2020 Zalamanda
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
###
# Usage:
# make                  # compile all test binaries
# make <testname>       # compile specific test binary
# make test             # run all test binaries (compiling if necessary)
# make <testname>.test  # run specific test binary (compiling if necessary)
# make clean            # remove all binary, object and log file types
#

.PHONY = all test clean
SHELL = bash

CC = gcc
CFLAGS = -pthread -Werror -Wall
LOG = error.log

SRCS := $(wildcard *.c)
BINS := $(SRCS:%.c=%)
TEST := $(SRCS:%.c=%.test)

all: ${BINS}
test: ${TEST}

%: %.c
	@echo Building $@ test...
	${CC} ${CFLAGS} -o $@ $< 2>&1 | tee ${LOG}; exit $${PIPESTATUS[0]}
	@echo

%.test: %
	@echo "Executing" $< "test..."
	./$<
	@echo

clean:
	@echo "Cleaning up..."
	rm -f *.log *.o ${BINS}
	@echo

