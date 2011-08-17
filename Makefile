all:cruentus

cruentus:Socket.cpp cruentus.cpp
	@mkdir -p build/
	@echo "Building...."
	@g++ $? -O3 -Wall -Werror -o build/cruentus
server:Socket.cpp cruentus.cpp Server.cpp
	@mkdir -p build/
	@echo "Building server..."
	@g++ $? -O3 -Wall -Werror -DWeb_Server=1 -o build/cruentus
clean:
	rm -rf build/
start:
	@echo "Starting server..."
	@./build/cruentus
