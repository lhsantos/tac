LEX=flex
YACC=bison
CC=g++
MKDIR=mkdir
IDIR=./include
SDIR=./src
LYDIR=./flex-bison
ODIR=./obj
BDIR=./bin
SODIR=$(ODIR)/src
LYODIR=$(ODIR)/flex-bison
CFLAGS=-I$(IDIR) -I$(LYDIR) -std=c++11 -O3 -c

CPPS := $(wildcard $(SDIR)/*.cpp)
OBJS := $(addprefix $(SODIR)/,$(notdir $(CPPS:.cpp=.o)))
LY_CPPS := $(LYDIR)/yyFlexLexer.cpp $(LYDIR)/parser.cpp
LY_OBJS := $(addprefix $(LYODIR)/,$(notdir $(LY_CPPS:.cpp=.o)))

.PHONY: all

all: mkdirs tac

.PHONY: clean

clean:
	rm -f $(LYDIR)/*.cpp $(LYDIR)/*.hh $(LYDIR)/*.hpp $(LYDIR)/*.output
	rm -rf $(LYODIR) $(SODIR) $(ODIR) $(BDIR)

$(LYDIR)/parser.cpp: $(LYDIR)/tac.ypp
	$(YACC) -Wall -v -o $@ $<

$(LYDIR)/yyFlexLexer.cpp: $(LYDIR)/parser.cpp $(LYDIR)/tac.l
	$(LEX) -o $@ $(LYDIR)/tac.l

$(LYODIR)/parser.o: $(LYDIR)/parser.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(LYODIR)/yyFlexLexer.o: $(LYDIR)/yyFlexLexer.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(SODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

mkdirs:
	$(MKDIR) -p $(BDIR) $(ODIR) $(SODIR) $(LYODIR)

tac: $(LY_OBJS) $(OBJS)
	$(CC) -o $(BDIR)/tac $^
