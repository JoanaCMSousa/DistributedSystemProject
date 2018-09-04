#Grupo 05
#Joana Sousa no47084, Joao Leal no46394,
#Liliana Delgadinho no43334

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = include
OBJS = $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o $(OBJDIR)/table-client.o $(OBJDIR)/table-server.o $(OBJDIR)/client_stub.o $(OBJDIR)/table_skel.o

make: $(OBJS) table-server table-client

$(OBJDIR)/data.o: $(SRCDIR)/data.c $(INCDIR)/data.h
	gcc -g -c $(SRCDIR)/data.c -iquote./include -o $(OBJDIR)/data.o

$(OBJDIR)/entry.o: $(SRCDIR)/entry.c $(INCDIR)/entry.h
	gcc -g -c $(SRCDIR)/entry.c -iquote./include -o $(OBJDIR)/entry.o

$(OBJDIR)/list.o: $(SRCDIR)/list.c $(INCDIR)/list.h $(INCDIR)/list-private.h
	gcc -g -c $(SRCDIR)/list.c -iquote./include -o $(OBJDIR)/list.o

$(OBJDIR)/message.o: $(SRCDIR)/message.c $(INCDIR)/message.h $(INCDIR)/message-private.h
	gcc -g -c $(SRCDIR)/message.c -iquote./include -o $(OBJDIR)/message.o

$(OBJDIR)/table.o: $(SRCDIR)/table.c $(INCDIR)/table.h $(INCDIR)/table-private.h
	gcc -g -c $(SRCDIR)/table.c -iquote./include -o $(OBJDIR)/table.o

$(OBJDIR)/network_client.o: $(SRCDIR)/network_client.c $(INCDIR)/network_client-private.h
	gcc -g -c $(SRCDIR)/network_client.c -iquote./include -o $(OBJDIR)/network_client.o

$(OBJDIR)/table-client.o: $(SRCDIR)/table-client.c $(INCDIR)/network_client-private.h
	gcc -g -c $(SRCDIR)/table-client.c -iquote./include -o $(OBJDIR)/table-client.o

$(OBJDIR)/table-server.o: $(SRCDIR)/table-server.c $(INCDIR)/inet.h $(INCDIR)/table-private.h $(INCDIR)/message-private.h $(INCDIR)/network_client-private.h $(INCDIR)/table_skel.h
	gcc -g -c $(SRCDIR)/table-server.c -iquote./include -o $(OBJDIR)/table-server.o

$(OBJDIR)/client_stub.o: $(SRCDIR)/client_stub.c $(INCDIR)/client_stub.h $(INCDIR)/client_stub-private.h
	gcc -g -c $(SRCDIR)/client_stub.c -iquote./include -o $(OBJDIR)/client_stub.o

$(OBJDIR)/table_skel.o: $(SRCDIR)/table_skel.c $(INCDIR)/table_skel.h
	gcc -g -c $(SRCDIR)/table_skel.c -iquote./include -o $(OBJDIR)/table_skel.o

table-server: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o $(OBJDIR)/table_skel.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o $(OBJDIR)/table_skel.o $(OBJDIR)/table-server.o -lm -o table-server

table-client: $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o $(OBJDIR)/client_stub.o
	gcc -Wall $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o $(OBJDIR)/message.o $(OBJDIR)/network_client.o $(OBJDIR)/client_stub.o $(OBJDIR)/table-client.o -lm -o table-client

clean:
	rm -f $(OBJDIR)/*.o table-server table-client