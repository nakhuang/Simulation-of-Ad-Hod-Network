node: main.o Node.o
	g++ -o node main.o Node.o

main.o: main.cpp Node.h
	g++ -c main.cpp

Node.o: Node.cpp Node.h
	g++ -c Node.cpp

clean:
	rm node *.o
