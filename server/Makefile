all: search_server

#Which compiler
CC = g++

#Where to install
INSTDIR = ./bin

#Where are include files kept
INCLUDE = ./include -I./lib/elus_interface

#Where are lib files kept
LIB = ./lib/elus_interface -lcode -lelus -lpthread

#Options for development
CFLAGS = -g -Wall

#Options for rel
#CFLAGS = -Wall

#Source file
SRC = ./src/main.cc ./src/searchSys.cc ./src/fileOperator.cc ./src/tcp_socket.cc
	
search_server: $(SRC) ./include/searchSys.h ./include/tcp_socket.h
	$(CC) -o ./bin/search_server $(SRC) -I$(INCLUDE) $(CFLAGS) -L$(LIB);

install: search_server
	@if [ -d $(INSTDIR) ];\
	then\
		mv ./bin/search_server $(INSTDIR);\
		chmod a+x $(INSTDIR)/search_server;\
		chmod og-w $(INSTDIR)/search_server;\
		echo "Installed in $(INSTDIR)";\
	else\
		echo "Sorry, $(INSTDIR) does not exist";\
	fi
clean: 
	-rm search_server
