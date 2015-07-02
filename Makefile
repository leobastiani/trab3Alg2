CC=gcc
FLAGS=-g
CFLAGS=-c $(FLAGS)
LDFLAGS=$(FLAGS)
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=t3

all: $(SOURCES) $(EXECUTABLE)
	
run: $(EXECUTABLE)
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

debug: $(SOURCES)
	$(CC) $(SOURCES) -o $(EXECUTABLE) $(LDFLAGS) -DDEBUG
	./$(EXECUTABLE)

test:  $(SOURCES)
	$(CC) $(SOURCES) -o $(EXECUTABLE) $(LDFLAGS) -DDEBUG -DTEST
	./$(EXECUTABLE)

.c.o:
	$(CC) $< -o $@ $(CFLAGS)

clear:
	rm *.o & rm $(EXECUTABLE) & rm arvoreB.btree & rm log_lbastiani.txt
