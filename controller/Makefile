controller: main.o Controller.o
	g++ -o controller main.o Controller.o

main.o: main.cpp Controller.h
	g++ -c main.cpp

Controller.o: Controller.cpp Controller.h
	g++ -c -std=c++11 Controller.cpp

clean:
	rm controller *.o
