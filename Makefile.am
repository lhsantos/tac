SUBDIRS = flex-bison
EXTRA_DIST = autogen.sh

bin_PROGRAMS = tac
tac_CPPFLAGS = -Iflex-bison -Iinclude
tac_LDADD = flex-bison/libparser.a
tac_SOURCES = src/main.cpp src/interpreter.cpp src/memmngr.cpp src/instruction.cpp src/scanner.cpp src/table.cpp src/symbol.cpp src/error.cpp

CLEANFILES = *~

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.log config.status \
	config.sub configure install-sh Makefile.in missing autom4te.cache depcomp ylwrap

maintainer-clean-local:
	-rm -rf m4

