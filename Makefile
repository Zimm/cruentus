all:builddir libcruentus cruentus

builddir:
	@mkdir -p build/

libcruentus: Socket.o
	@echo "Building libcruentus...."
	@g++ build/Socket.o -lpthread -shared -o build/libcruentus.a

Socket.o: Socket.cpp
	g++ -c -Wall -Werror -O3 -fPIC -o build/Socket.o Socket.cpp

cruentus: cruentus.o Server.o
	@echo "Building cruentus...."
	@g++ build/Server.o build/cruentus.o -lpthread -lcruentus -o build/cruentus

cruentus.o: cruentus.cpp
	g++ -c -O3 -Wall -Werror -o build/cruentus.o cruentus.cpp

Server.o: Server.cpp
	g++ -c -O3 -Wall -Werror -o build/Server.o Server.cpp

clean:
	rm -rf build/
