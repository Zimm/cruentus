all:cruentus

cruentus:Socket.cpp cruentus.cpp Server.cpp
	@mkdir -p build/
	@echo "Building...."
	@g++ $? -O3 -Wall -Werror -lpthread -o build/cruentus
clean:
	rm -rf build/
start:
	@echo "Starting server..."
	@./build/cruentus
