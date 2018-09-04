#Grupo 05
#Joana Sousa no47084, Joao Leal no46394,
#Liliana Delgadinho no43334

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = include
OBJS = $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/test_data.o $(OBJDIR)/test_entry.o $(OBJDIR)/test_list.o

make all: $(OBJS) test_data test_entry test_list

$(OBJDIR)/data.o: $(SRCDIR)/data.c $(INCDIR)/data.h
	gcc -g -c $(SRCDIR)/data.c -iquote./include -lpthread -o $(OBJDIR)/data.o

$(OBJDIR)/entry.o: $(SRCDIR)/entry.c $(INCDIR)/entry.h
	gcc -g -c $(SRCDIR)/entry.c -iquote./include -lpthread -o $(OBJDIR)/entry.o

$(OBJDIR)/list.o: $(SRCDIR)/list.c $(INCDIR)/list.h $(INCDIR)/list-private.h
	gcc -g -c $(SRCDIR)/list.c -iquote./include -lpthread -o $(OBJDIR)/list.o

$(OBJDIR)/test_data.o: test_data.c $(INCDIR)/data.h
	gcc -g -c test_data.c -iquote./include -lpthread -o $(OBJDIR)/test_data.o

$(OBJDIR)/test_entry.o: test_entry.c $(INCDIR)/data.h $(INCDIR)/entry.h
	gcc -g -c test_entry.c -iquote./include -lpthread -o $(OBJDIR)/test_entry.o

$(OBJDIR)/test_list.o: test_list.c $(INCDIR)/list-private.h 
	gcc -g -c test_list.c -iquote./include -lpthread -o $(OBJDIR)/test_list.o

test_data: $(OBJDIR)/data.o $(OBJDIR)/test_data.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/test_data.o -lm -pthread -o test_data

test_entry: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/test_entry.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/test_entry.o -lm -pthread -o test_entry 

test_list: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/test_list.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/test_list.o -lm -pthread -o test_list

clean:
	rm -f $(OBJDIR)/*.o test_data test_entry test_list
