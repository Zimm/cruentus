all:cruentus

libcruentus:Socket.cpp
	@mkdir -p build/
	@echo "Building libcruentus...."
	@g++ -c Socket.cpp -O3 -Wall -Werror -lpthread -fPIC -o build/libcruentus.a

cruentus:libcruentus
	@mkdir -p build/
	@echo "Building cruentus...."
	@g++ Server.cpp cruentus.cpp -O3 -Wall -Werror -lpthread -L./build/ -lcruentus -o build/cruentus
clean:
	rm -rf build/
