CC = g++
CFLAG = -std=c++11 -g

objdir = .obj/

exedir = bin/

TARGET = $(exedir)mpc 
all: $(TARGET)
	@echo "Compilation ends successfully!"

$(exedir)mpc: $(objdir)graph.o $(objdir)utils.o $(objdir)mpc.o
	$(CC) $(CFLAG) $(objdir)graph.o $(objdir)utils.o $(objdir)mpc.o -o $(exedir)mpc

$(objdir)graph.o: graph.cpp graph.hpp
	$(CC) $(CFLAG) -c graph.cpp -o $(objdir)graph.o

$(objdir)utils.o: utils.cpp utils.hpp
	$(CC) $(CFLAG) -c utils.cpp -o $(objdir)utils.o

$(objdir)mpc.o: mpc.cpp
	$(CC) $(CFLAG) -c mpc.cpp -o $(objdir)mpc.o

clean: 
	rm -rf *.o bin/*