#Grupo 05
#Joana Sousa no47084, Joao Leal no46394,
#Liliana Delgadinho no43334

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = include
OBJS = $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/test_table.o $(OBJDIR)/test_message.o $(OBJDIR)/network_client.o $(OBJDIR)/table-client.o $(OBJDIR)/table-server.o

make: $(OBJS) test_table test_message table-server table-client

$(OBJDIR)/data.o: $(SRCDIR)/data.c $(INCDIR)/data.h
	gcc -g -c $(SRCDIR)/data.c -iquote./include -lpthread -o $(OBJDIR)/data.o

$(OBJDIR)/entry.o: $(SRCDIR)/entry.c $(INCDIR)/entry.h
	gcc -g -c $(SRCDIR)/entry.c -iquote./include -lpthread -o $(OBJDIR)/entry.o

$(OBJDIR)/list.o: $(SRCDIR)/list.c $(INCDIR)/list.h $(INCDIR)/list-private.h
	gcc -g -c $(SRCDIR)/list.c -iquote./include -lpthread -o $(OBJDIR)/list.o

$(OBJDIR)/message.o: $(SRCDIR)/message.c $(INCDIR)/message.h $(INCDIR)/message-private.h
	gcc -g -c $(SRCDIR)/message.c -iquote./include -lpthread -o $(OBJDIR)/message.o

$(OBJDIR)/table.o: $(SRCDIR)/table.c $(INCDIR)/table.h $(INCDIR)/table-private.h
	gcc -g -c $(SRCDIR)/table.c -iquote./include -lpthread -o $(OBJDIR)/table.o

$(OBJDIR)/network_client.o: $(SRCDIR)/network_client.c $(INCDIR)/network_client-private.h
	gcc -g -c $(SRCDIR)/network_client.c -iquote./include -lpthread -o $(OBJDIR)/network_client.o

$(OBJDIR)/table-client.o: $(SRCDIR)/table-client.c $(INCDIR)/network_client-private.h
	gcc -g -c $(SRCDIR)/table-client.c -iquote./include -lpthread -o $(OBJDIR)/table-client.o

$(OBJDIR)/table-server.o: $(SRCDIR)/table-server.c $(INCDIR)/inet.h $(INCDIR)/table-private.h $(INCDIR)/message-private.h $(INCDIR)/network_client-private.h
	gcc -g -c $(SRCDIR)/table-server.c -iquote./include -lpthread -o $(OBJDIR)/table-server.o

$(OBJDIR)/test_message.o: test_message.c $(INCDIR)/list-private.h $(INCDIR)/message-private.h
	gcc -g -c test_message.c -iquote./include -lpthread -o $(OBJDIR)/test_message.o

$(OBJDIR)/test_table.o: test_table.c $(INCDIR)/table-private.h
	gcc -g -c test_table.c -iquote./include -lpthread -o $(OBJDIR)/test_table.o

test_message: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/test_message.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/test_message.o -lm -pthread -o test_message

test_table: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/test_table.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/test_table.o -lm -pthread -o test_table

table-server: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o $(OBJDIR)/table-server.o -lm -pthread -o table-server

table-client: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o $(OBJDIR)/table-client.o -lm -pthread -o table-client

clean:
	rm -f $(OBJDIR)/*.o test_message test_table table-server table-client