AR=ar rcu
CC=cc
CP=cp -r
INSTALL=install
LEX=lex
MKDIR=mkdir -p
MV=mv
RANLIB=ranlib
RM=rm -f
YACC=yacc -d

CFLAGS=-Wall -Wextra -ansi -pedantic
LDFLAGS=-lm

PREFIX=/usr/local
TO_BIN=$(PREFIX)/bin
TO_LIB=$(PREFIX)/lib
TO_INCLUDE=$(PREFIX)/include

OBJDIR=obj
SRCDIR=src
LI_BIN=li
LI_LIB=libli.a
LI_OBJS_=li.o
LI_OBJS=$(addprefix $(OBJDIR)/, $(LI_OBJS_))
LI_LIB_OBJS_=li_read.o li_parse.o li_chr.o li_error.o li_eval.o li_nat.o \
	     li_num.o li_object.o li_proc.o li_rat.o li_str.o li_write.o
LI_LIB_OBJS=$(addprefix $(OBJDIR)/, $(LI_LIB_OBJS_))
LI_OPT_OBJS_=lib_bytevector.o
LI_OPT_OBJS=$(addprefix $(OBJDIR)/, $(LI_OPT_OBJS_))
ALL_OBJS=$(LI_OBJS) $(LI_LIB_OBJS)

.PHONY: all opt debug profile install uninstall clean test tags

all: $(LI_BIN)

$(LI_BIN): $(LI_OBJS) $(LI_LIB)
	$(CC) -o $@ $+ $(LDFLAGS)

$(LI_LIB): $(LI_LIB_OBJS)
	$(AR) $@ $(LI_LIB_OBJS)
	$(RANLIB) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(MKDIR) $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SRCDIR)/li_read.c: $(SRCDIR)/li_read.y
	yacc -d $(SRCDIR)/li_read.y
	mv y.tab.c $(SRCDIR)/li_read.c
	mv y.tab.h $(SRCDIR)/li_read.h

opt: $(LI_OPT_OBJS)
opt: LI_LIB_OBJS+=$(LI_OPT_OBJS)
opt: CFLAGS+=-DLI_OPTIONAL
opt: all

debug: CFLAGS+=-g -DDEBUG
debug: all

profile: CFLAGS+=-pg
profile: all

install: all
	$(INSTALL) $(LI_BIN) $(TO_BIN)
	$(INSTALL) $(LI_LIB) $(TO_LIB)
	$(INSTALL) src/li.h $(TO_INCLUDE)

uninstall:
	cd $(TO_BIN) && $(RM) $(LI_BIN)

clean:
	$(RM) $(LI_BIN) $(LI_LIB) src/li_parse.c src/li_read.[ch]
	$(RM) -r $(OBJDIR)

test: $(LI_BIN)
	./$(LI_BIN) test/test.li

tags: src/li.h
	ctags -f $@ $<

# automatically made with 'gcc -MM src/li_*.c'
$(OBJDIR)/li.o: src/li.c src/li.h
$(OBJDIR)/li_chr.o: src/li_chr.c src/li.h
$(OBJDIR)/li_error.o: src/li_error.c src/li.h
$(OBJDIR)/li_eval.o: src/li_eval.c src/li.h
$(OBJDIR)/li_nat.o: src/li_nat.c src/li.h
$(OBJDIR)/li_num.o: src/li_num.c src/li.h
$(OBJDIR)/li_object.o: src/li_object.c src/li.h
$(OBJDIR)/li_proc.o: src/li_proc.c src/li.h
$(OBJDIR)/li_rat.o: src/li_rat.c src/li.h
$(OBJDIR)/li_read.o: src/li_read.c src/li.h
$(OBJDIR)/li_str.o: src/li_str.c src/li.h
$(OBJDIR)/li_write.o: src/li_write.c src/li.h
$(OBJDIR)/lib_bytevector.o: src/lib_bytevector.c src/li.h
