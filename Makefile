################################################################################
# Deprecated build shim
################################################################################

.DEFAULT_GOAL := help

LEX = flex
YACC = bison
YFLAGS ?= -dv

GENERATED = src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c \
	src/lexer.h src/parser.h src/event_lexer.h src/event_parser.h

define DEPRECATED_BUILD_MSG
	@printf '%s\n' \
	'The legacy Makefile build has been retired.' \
	'Use the supported CMake workflow instead:' \
	'  cmake -S . -B build' \
	'  cmake --build build -j4' \
	'  ctest --test-dir build --output-on-failure'
endef

help:
	@printf '%s\n' \
	'Supported targets:' \
	'  make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c' \
	'    Regenerate tracked lexer/parser sources with flex/bison.' \
	'' \
	'The old Make-based library/test build has been retired.' \
	'Use CMake for builds and tests.'

all:
	$(DEPRECATED_BUILD_MSG)
	@false

dev:
	$(DEPRECATED_BUILD_MSG)
	@false

test:
	$(DEPRECATED_BUILD_MSG)
	@false

valgrind:
	$(DEPRECATED_BUILD_MSG)
	@false

build-test-benchmark:
	$(DEPRECATED_BUILD_MSG)
	@false

%.c %.h: %.l
	$(LEX) --header-file=$*.h -o $*.c $<

%.c %.h: %.y
	$(YACC) $(YFLAGS) -o $*.c $<

src/parser.c src/parser.h: src/parser.y
src/event_parser.c src/event_parser.h: src/event_parser.y
src/lexer.c src/lexer.h: src/lexer.l src/parser.h
src/event_lexer.c src/event_lexer.h: src/event_lexer.l src/event_parser.h

clean:
	$(RM) $(GENERATED)

realclean: clean

.PHONY: help all dev test valgrind build-test-benchmark clean realclean
