# Makefile

PROJECT = nmh
SHARED_LIB = lib$(PROJECT).so.1.0
OBJS = $(PROJECT).o

CC = gcc
CFLAGS = -g -Wall -fPIC
#CFLAGS = -O2 -fPIC
LDFLAGS = -shared
DESTDIR = /usr/local

.PHONY: all clean install uninstall upload
all : $(SHARED_LIB) $(PROJECT).hpp

$(SHARED_LIB) : $(OBJS)
	$(CC) -shared -o $(SHARED_LIB) $^

$(PROJECT).hpp :
	python codegen.py > $(PROJECT).hpp

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(SHARED_LIB) $(OBJS) $(PROJECT).hpp

install: $(SHARED_LIB) $(PROJECT).hpp
	[ ! -e $(DESTDIR)/include ] && mkdir -p $(DESTDIR)/include; \
	cp $(PROJECT).h $(DESTDIR)/include/$(PROJECT).h
	cp $(PROJECT).h $(DESTDIR)/include/$(PROJECT).hpp
	[ ! -e $(DESTDIR)/lib ] && mkdir -p $(DESTDIR)/lib; \
	cp $(SHARED_LIB) $(DESTDIR)/lib/$(SHARED_LIB)
	ln -s $(DESTDIR)/lib/$(SHARED_LIB) $(DESTDIR)/lib/lib$(PROJECT).so

uninstall:
	$(RM) $(DESTDIR)/lib/$(SHARED_LIB)
	$(RM) $(DESTDIR)/lib/lib$(PROJECT).so
	$(RM) $(DESTDIR)/include/$(PROJECT).h
	$(RM) $(DESTDIR)/include/$(PROJECT).hpp

test: $(OBJS)
	$(CC) $(CFLAGS) -lcunit -o $(PROJECT)_test $(PROJECT)_test.c $^
	./$(PROJECT)_test
	$(RM) ./$(PROJECT)_test
